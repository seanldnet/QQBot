#include "gateway.h"
#include <curl/curl.h>
#include <curl/websockets.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define GATEWAY_FRAME_MAX (1024U * 1024U)

struct qq_gateway {
    pthread_t thread;
    volatile int stopping;
    char api_base[256];
    qq_gateway_token_fn token_fn;
    qq_gateway_event_fn event_fn;
    qq_gateway_state_fn state_fn;
    void *context;
};

typedef struct { char *data; size_t length; } response_buffer;

static size_t collect_http(char *ptr, size_t size, size_t count, void *opaque) {
    response_buffer *b = (response_buffer *)opaque;
    size_t n = size * count;
    if (n > 65536U - b->length) return 0;
    char *p = (char *)realloc(b->data, b->length + n + 1);
    if (!p) return 0;
    b->data = p;
    memcpy(b->data + b->length, ptr, n);
    b->length += n;
    b->data[b->length] = 0;
    return n;
}

static void state(qq_gateway_t *g, const char *name, const char *detail) {
    if (g->state_fn) g->state_fn(g->context, name, detail ? detail : "");
}

static void pause_seconds(unsigned int seconds) {
#ifdef _WIN32
    Sleep(seconds * 1000U);
#else
    sleep(seconds);
#endif
}

static int get_gateway_url(qq_gateway_t *g, const char *token, char *url, size_t cap) {
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    response_buffer response = {0};
    char endpoint[512];
    long status = 0;
    int ok = 0;
    if (!curl) return -1;
    snprintf(endpoint, sizeof(endpoint), "%s/gateway", g->api_base);
    char auth[4200];
    snprintf(auth, sizeof(auth), "Authorization: QQBot %s", token);
    headers = curl_slist_append(headers, auth);
    curl_easy_setopt(curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, collect_http);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    if (curl_easy_perform(curl) == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (status >= 200 && status < 300 && response.data) {
        cJSON *root = cJSON_Parse(response.data);
        cJSON *value = root ? cJSON_GetObjectItemCaseSensitive(root, "url") : NULL;
        if (cJSON_IsString(value) && value->valuestring &&
            strncmp(value->valuestring, "wss://", 6) == 0 &&
            strlen(value->valuestring) < cap) {
            snprintf(url, cap, "%s", value->valuestring);
            ok = 1;
        }
        cJSON_Delete(root);
    }
    free(response.data);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return ok ? 0 : -1;
}

static int send_json(CURL *ws, const char *json) {
    size_t length = strlen(json), sent = 0;
    CURLcode rc = curl_ws_send(ws, json, length, &sent, 0, CURLWS_TEXT);
    while (rc == CURLE_AGAIN) {
        int fds = 0;
        curl_easy_poll(ws, NULL, 0, 1000, &fds);
        rc = curl_ws_send(ws, json, length, &sent, 0, CURLWS_TEXT);
    }
    return rc == CURLE_OK && sent == length ? 0 : -1;
}

static int send_identify(CURL *ws, const char *token) {
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    cJSON *shard = cJSON_CreateArray();
    cJSON *properties = cJSON_CreateObject();
    char auth[4200];
    char *json;
    int ok = -1;
    if (!root || !data || !shard || !properties) goto done;
    snprintf(auth, sizeof(auth), "QQBot %s", token);
    cJSON_AddNumberToObject(root, "op", 2);
    cJSON_AddStringToObject(data, "token", auth);
    cJSON_AddNumberToObject(data, "intents", (double)(1U << 25));
    cJSON_AddItemToArray(shard, cJSON_CreateNumber(0));
    cJSON_AddItemToArray(shard, cJSON_CreateNumber(1));
    cJSON_AddItemToObject(data, "shard", shard); shard = NULL;
    cJSON_AddStringToObject(properties, "$os",
#ifdef _WIN32
        "windows"
#else
        "linux"
#endif
    );
    cJSON_AddStringToObject(properties, "$browser", "qqbot-c11");
    cJSON_AddStringToObject(properties, "$device", "qqbot-c11");
    cJSON_AddItemToObject(data, "properties", properties); properties = NULL;
    cJSON_AddItemToObject(root, "d", data); data = NULL;
    json = cJSON_PrintUnformatted(root);
    if (json) { ok = send_json(ws, json); free(json); }
done:
    cJSON_Delete(root); cJSON_Delete(data); cJSON_Delete(shard); cJSON_Delete(properties);
    return ok;
}

static int process_payload(qq_gateway_t *g, const char *payload,
                           const char *token, long long *sequence,
                           unsigned int *heartbeat_ms, int *ready) {
    cJSON *root = cJSON_Parse(payload);
    cJSON *op = root ? cJSON_GetObjectItemCaseSensitive(root, "op") : NULL;
    cJSON *data = root ? cJSON_GetObjectItemCaseSensitive(root, "d") : NULL;
    int reconnect = 0;
    (void)token;
    if (!cJSON_IsNumber(op)) { cJSON_Delete(root); return 0; }
    switch (op->valueint) {
    case 10: {
        cJSON *interval = data ? cJSON_GetObjectItemCaseSensitive(data, "heartbeat_interval") : NULL;
        if (!cJSON_IsNumber(interval) || interval->valuedouble < 1000 || interval->valuedouble > 300000) {
            state(g, "error", "Gateway HELLO 缺少有效 heartbeat_interval");
            reconnect = 1; break;
        }
        *heartbeat_ms = (unsigned int)interval->valuedouble;
        break;
    }
    case 0: {
        cJSON *seq = cJSON_GetObjectItemCaseSensitive(root, "s");
        cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "t");
        if (cJSON_IsNumber(seq)) *sequence = (long long)seq->valuedouble;
        if (cJSON_IsString(type) && type->valuestring && cJSON_IsObject(data)) {
            char *encoded = cJSON_PrintUnformatted(data);
            if (encoded && g->event_fn)
                g->event_fn(g->context, type->valuestring, encoded, *sequence);
            free(encoded);
            if (strcmp(type->valuestring, "READY") == 0 ||
                strcmp(type->valuestring, "RESUMED") == 0) {
                *ready = 1;
                state(g, "connected", type->valuestring);
            }
        }
        break;
    }
    case 7: reconnect = 1; state(g, "reconnecting", "Gateway 请求重新连接"); break;
    case 9: reconnect = 1; *ready = 0; state(g, "reconnecting", "Gateway 会话失效，重新鉴权"); break;
    case 11: break;
    default: break;
    }
    cJSON_Delete(root);
    return reconnect;
}

static void *gateway_thread(void *opaque) {
    qq_gateway_t *g = (qq_gateway_t *)opaque;
    unsigned int backoff = 1;
    state(g, "connecting", "正在获取 QQ Gateway 地址");
    while (!g->stopping) {
        char token[4096] = {0}, url[2048] = {0};
        CURL *ws = NULL;
        long long sequence = 0;
        unsigned int heartbeat_ms = 0;
        time_t next_heartbeat = 0;
        int ready = 0, reconnect = 0, hello = 0;
        char aggregate[GATEWAY_FRAME_MAX + 1];
        size_t aggregate_len = 0;
        if (g->token_fn(g->context, token, sizeof(token)) != 0 ||
            get_gateway_url(g, token, url, sizeof(url)) != 0) {
            state(g, "error", "获取 access_token 或 Gateway 地址失败");
            goto retry;
        }
        state(g, "connecting", "正在建立 WSS 连接");
        ws = curl_easy_init();
        if (!ws) goto retry;
        curl_easy_setopt(ws, CURLOPT_URL, url);
        curl_easy_setopt(ws, CURLOPT_CONNECT_ONLY, 2L);
        curl_easy_setopt(ws, CURLOPT_CONNECTTIMEOUT, 20L);
        curl_easy_setopt(ws, CURLOPT_TIMEOUT, 0L);
        curl_easy_setopt(ws, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(ws, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(ws, CURLOPT_NOSIGNAL, 1L);
        if (curl_easy_perform(ws) != CURLE_OK) {
            curl_easy_cleanup(ws); ws = NULL;
            state(g, "error", "WSS 握手失败");
            goto retry;
        }
        state(g, "connected", "WSS 握手完成，等待 HELLO");
        backoff = 1;
        while (!g->stopping && !reconnect) {
            char frame[65536];
            size_t received = 0;
            const struct curl_ws_frame *meta = NULL;
            CURLcode rc = curl_ws_recv(ws, frame, sizeof(frame), &received, &meta);
            if (rc == CURLE_AGAIN) {
                int fds = 0;
                curl_easy_poll(ws, NULL, 0, 250, &fds);
            } else if (rc != CURLE_OK) {
                state(g, "disconnected", "WSS 接收失败");
                reconnect = 1;
            } else if (received > 0 && meta) {
                if (received > GATEWAY_FRAME_MAX - aggregate_len) {
                    state(g, "error", "Gateway 消息超过 1 MiB 上限");
                    reconnect = 1;
                } else {
                    memcpy(aggregate + aggregate_len, frame, received);
                    aggregate_len += received;
                    if (meta->bytesleft == 0) {
                        aggregate[aggregate_len] = 0;
                        if (!hello) {
                            cJSON *probe = cJSON_Parse(aggregate);
                            cJSON *op = probe ? cJSON_GetObjectItemCaseSensitive(probe, "op") : NULL;
                            cJSON *d = probe ? cJSON_GetObjectItemCaseSensitive(probe, "d") : NULL;
                            cJSON *interval = d ? cJSON_GetObjectItemCaseSensitive(d, "heartbeat_interval") : NULL;
                            if (cJSON_IsNumber(op) && op->valueint == 10 && cJSON_IsNumber(interval)) {
                                heartbeat_ms = (unsigned int)interval->valuedouble;
                                if (send_identify(ws, token) != 0) reconnect = 1;
                                else { hello = 1; next_heartbeat = time(NULL) + (time_t)(heartbeat_ms / 1000U); }
                            } else reconnect = 1;
                            cJSON_Delete(probe);
                        } else {
                            int action = process_payload(g, aggregate, token, &sequence,
                                                          &heartbeat_ms, &ready);
                            if (action) reconnect = 1;
                        }
                        aggregate_len = 0;
                    }
                }
            }
            if (hello && heartbeat_ms && time(NULL) >= next_heartbeat && !reconnect) {
                cJSON *hb = cJSON_CreateObject();
                cJSON_AddNumberToObject(hb, "op", 1);
                if (sequence > 0) cJSON_AddNumberToObject(hb, "d", (double)sequence);
                else cJSON_AddNullToObject(hb, "d");
                char *json = cJSON_PrintUnformatted(hb);
                if (!json || send_json(ws, json) != 0) reconnect = 1;
                free(json); cJSON_Delete(hb);
                next_heartbeat = time(NULL) + (time_t)(heartbeat_ms / 1000U);
            }
        }
        if (ws) {
            const char close_frame[] = "";
            size_t sent = 0;
            curl_ws_send(ws, close_frame, 0, &sent, 0, CURLWS_CLOSE);
            curl_easy_cleanup(ws);
        }
        if (!g->stopping) state(g, "reconnecting", ready ? "连接中断，准备重连" : "连接未就绪，准备重试");
retry:
        memset(token, 0, sizeof(token));
        if (!g->stopping) {
            pause_seconds(backoff);
            if (backoff < 60) backoff = backoff < 30 ? backoff * 2 : 60;
        }
    }
    state(g, "disconnected", "WSS 客户端已停止");
    return NULL;
}

qq_gateway_t *qq_gateway_start(const char *api_base,
                               qq_gateway_token_fn token_fn,
                               qq_gateway_event_fn event_fn,
                               qq_gateway_state_fn state_fn,
                               void *context) {
    qq_gateway_t *g;
    if (!api_base || !token_fn || !event_fn) return NULL;
    g = (qq_gateway_t *)calloc(1, sizeof(*g));
    if (!g) return NULL;
    snprintf(g->api_base, sizeof(g->api_base), "%s", api_base);
    g->token_fn = token_fn; g->event_fn = event_fn;
    g->state_fn = state_fn; g->context = context;
    if (pthread_create(&g->thread, NULL, gateway_thread, g) != 0) { free(g); return NULL; }
    return g;
}

void qq_gateway_stop(qq_gateway_t *g) {
    if (!g) return;
    g->stopping = 1;
    pthread_join(g->thread, NULL);
    free(g);
}

int qq_gateway_available(void) {
    const curl_version_info_data *v = curl_version_info(CURLVERSION_NOW);
#ifdef CURL_VERSION_WEBSOCKETS
    return v && (v->features & CURL_VERSION_WEBSOCKETS) != 0;
#else
    (void)v;
    return 0;
#endif
}
