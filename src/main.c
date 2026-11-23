#include <microhttpd.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <sqlite3.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "qq/webhook.h"
#include "qq/events.h"
#include "qq/event_queue.h"

#define DEFAULT_PORT 8080
#define MAX_BODY (1024U * 1024U)
#define API_BASE_DEFAULT "https://api.sgroup.qq.com"
#define TOKEN_URL_DEFAULT "https://bots.qq.com/app/getAppAccessToken"

typedef struct { char *data; size_t len; } buffer_t;
typedef struct {
    sqlite3 *db;
    pthread_mutex_t lock;
    char app_id[128];
    char app_secret[256];
    char access_token[4096];
    time_t token_expires;
    char api_base[256];
    char token_url[512];
    char connection_mode[16];
} app_t;

typedef struct { char *body; size_t size; } request_t;
static volatile sig_atomic_t g_stop = 0;
static app_t g_app;

static void on_signal(int sig) { (void)sig; g_stop = 1; }
static size_t curl_write(char *ptr, size_t size, size_t nmemb, void *userdata) {
    buffer_t *b = (buffer_t *)userdata; size_t n = size * nmemb;
    char *p = realloc(b->data, b->len + n + 1); if (!p) return 0;
    b->data = p; memcpy(b->data + b->len, ptr, n); b->len += n; b->data[b->len] = 0; return n;
}
static void log_line(const char *level, const char *message) {
    time_t now = time(NULL); struct tm tmv; localtime_r(&now, &tmv);
    char ts[32]; strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);
    fprintf(stdout, "[%s] [%s] %s\n", ts, level, message); fflush(stdout);
    if (g_app.db) {
        pthread_mutex_lock(&g_app.lock); sqlite3_stmt *st = NULL;
        if (sqlite3_prepare_v2(g_app.db, "INSERT INTO logs(level,component,message,created_at) VALUES(?1,'host',?2,datetime('now'))", -1, &st, NULL) == SQLITE_OK) {
            sqlite3_bind_text(st, 1, level, -1, SQLITE_TRANSIENT); sqlite3_bind_text(st, 2, message, -1, SQLITE_TRANSIENT); sqlite3_step(st);
        }
        sqlite3_finalize(st); pthread_mutex_unlock(&g_app.lock);
    }
}
static int db_init(void) {
    if (sqlite3_open("data/qqbot.db", &g_app.db) != SQLITE_OK) return -1;
    sqlite3_busy_timeout(g_app.db, 5000);
    const char *sql = "PRAGMA foreign_keys=ON; PRAGMA journal_mode=WAL;"
      "CREATE TABLE IF NOT EXISTS schema_migrations(version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);"
      "CREATE TABLE IF NOT EXISTS logs(id INTEGER PRIMARY KEY AUTOINCREMENT,level TEXT NOT NULL,component TEXT NOT NULL,message TEXT NOT NULL,created_at TEXT NOT NULL);"
      "CREATE TABLE IF NOT EXISTS plugins(id TEXT PRIMARY KEY,name TEXT NOT NULL,callback_url TEXT NOT NULL,token_hash TEXT NOT NULL,enabled INTEGER NOT NULL DEFAULT 1,created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP);"
      "INSERT OR IGNORE INTO schema_migrations(version) VALUES(1);";
    char *err = NULL; int rc = sqlite3_exec(g_app.db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) { fprintf(stderr, "SQLite init: %s\n", err ? err : "unknown"); sqlite3_free(err); return -1; }
    if (qq_event_queue_init(g_app.db) != 0) return -1;
    return 0;
}
static int fetch_access_token(const char *app_id, const char *secret, char *out, size_t outcap, long *expires) {
    cJSON *root = cJSON_CreateObject(); if (!root) return -1;
    cJSON_AddStringToObject(root, "appId", app_id); cJSON_AddStringToObject(root, "clientSecret", secret);
    char *payload = cJSON_PrintUnformatted(root); cJSON_Delete(root); if (!payload) return -1;
    buffer_t response = {0}; CURL *curl = curl_easy_init(); if (!curl) { free(payload); return -1; }
    struct curl_slist *headers = NULL; headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, g_app.token_url); curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload); curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L); curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    CURLcode cr = curl_easy_perform(curl); long status = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(headers); curl_easy_cleanup(curl); free(payload);
    if (cr != CURLE_OK || status < 200 || status >= 300 || !response.data) { free(response.data); return -1; }
    cJSON *json = cJSON_Parse(response.data); free(response.data); if (!json) return -1;
    cJSON *token = cJSON_GetObjectItemCaseSensitive(json, "access_token");
    cJSON *ttl = cJSON_GetObjectItemCaseSensitive(json, "expires_in");
    int ok = cJSON_IsString(token) && token->valuestring && strlen(token->valuestring) < outcap;
    if (ok) { snprintf(out, outcap, "%s", token->valuestring); *expires = cJSON_IsNumber(ttl) ? (long)ttl->valuedouble : 7200L; }
    cJSON_Delete(json); return ok ? 0 : -1;
}
static int ensure_token(void) {
    pthread_mutex_lock(&g_app.lock);
    if (g_app.access_token[0] && time(NULL) < g_app.token_expires - 120) { pthread_mutex_unlock(&g_app.lock); return 0; }
    char token[sizeof(g_app.access_token)] = {0}; long ttl = 7200;
    int rc = fetch_access_token(g_app.app_id, g_app.app_secret, token, sizeof(token), &ttl);
    if (rc == 0) { OPENSSL_cleanse(g_app.access_token, sizeof(g_app.access_token)); snprintf(g_app.access_token, sizeof(g_app.access_token), "%s", token); g_app.token_expires = time(NULL) + ttl; }
    OPENSSL_cleanse(token, sizeof(token)); pthread_mutex_unlock(&g_app.lock);
    if (rc == 0) log_line("INFO", "QQ access_token acquired (value redacted)");
    return rc;
}
static enum MHD_Result reply(struct MHD_Connection *c, unsigned int status, const char *body, const char *type) {
    struct MHD_Response *r = MHD_create_response_from_buffer(strlen(body), (void *)body, MHD_RESPMEM_MUST_COPY);
    if (!r) return MHD_NO;
    MHD_add_response_header(r, "Content-Type", type); MHD_add_response_header(r, "Cache-Control", "no-store");
    enum MHD_Result ret = MHD_queue_response(c, status, r); MHD_destroy_response(r); return ret;
}
static enum MHD_Result json_reply(struct MHD_Connection *c, unsigned int status, const char *body) { return reply(c, status, body, "application/json; charset=utf-8"); }

static void bytes_hex(const unsigned char *src, size_t n, char *dst) {
    static const char h[]="0123456789abcdef";
    for(size_t i=0;i<n;i++){dst[i*2]=h[src[i]>>4];dst[i*2+1]=h[src[i]&15];}
    dst[n*2]=0;
}
static int sha256_hex(const char *value, char out[65]) {
    unsigned char digest[32]; unsigned int n=0;
    if(EVP_Digest(value,strlen(value),digest,&n,EVP_sha256(),NULL)!=1||n!=32)return -1;
    bytes_hex(digest,sizeof(digest),out); OPENSSL_cleanse(digest,sizeof(digest)); return 0;
}
static int admin_authorized(struct MHD_Connection *c) {
    const char *expected=getenv("QQBOT_ADMIN_TOKEN");
    const char *provided=MHD_lookup_connection_value(c,MHD_HEADER_KIND,"Authorization");
    if(!expected||strlen(expected)<24||!provided||strncmp(provided,"Bearer ",7)!=0)return 0;
    provided+=7; size_t a=strlen(expected),b=strlen(provided);
    return a==b && CRYPTO_memcmp(expected,provided,a)==0;
}
static int plugin_authorized(const char *id,const char *token) {
    char hash[65]; sqlite3_stmt *st=NULL; int ok=0;
    if(!id||!token||sha256_hex(token,hash)!=0)return 0;
    pthread_mutex_lock(&g_app.lock);
    if(sqlite3_prepare_v2(g_app.db,"SELECT token_hash,enabled FROM plugins WHERE id=?1",-1,&st,NULL)==SQLITE_OK){
        sqlite3_bind_text(st,1,id,-1,SQLITE_TRANSIENT);
        if(sqlite3_step(st)==SQLITE_ROW){
            const char *stored=(const char*)sqlite3_column_text(st,0);
            int enabled=sqlite3_column_int(st,1);
            if(stored&&enabled&&strlen(stored)==64&&CRYPTO_memcmp(stored,hash,64)==0)ok=1;
        }
    }
    sqlite3_finalize(st); pthread_mutex_unlock(&g_app.lock); OPENSSL_cleanse(hash,sizeof(hash)); return ok;
}
typedef struct { char *data; size_t len; } proxy_buffer_t;
static size_t proxy_write(char *ptr,size_t size,size_t nmemb,void *userdata) {
    proxy_buffer_t *b=(proxy_buffer_t*)userdata; size_t n=size*nmemb;
    if(n>8U*1024U*1024U-b->len)return 0;
    char *p=(char*)realloc(b->data,b->len+n+1);if(!p)return 0;
    b->data=p;memcpy(b->data+b->len,ptr,n);b->len+=n;b->data[b->len]=0;return n;
}
static enum MHD_Result plugin_register(struct MHD_Connection *c,const char *body) {
    if(!admin_authorized(c))return json_reply(c,401,"{\"error\":\"admin_auth_required\"}");
    cJSON *j=cJSON_Parse(body?body:"");cJSON *name=j?cJSON_GetObjectItemCaseSensitive(j,"name"):NULL;
    cJSON *callback=j?cJSON_GetObjectItemCaseSensitive(j,"callback_url"):NULL;
    if(!cJSON_IsString(name)||!name->valuestring[0]||strlen(name->valuestring)>128||
       (callback&&!cJSON_IsString(callback))||(cJSON_IsString(callback)&&callback->valuestring[0]&&strncmp(callback->valuestring,"https://",8)!=0)){cJSON_Delete(j);return json_reply(c,400,"{\"error\":\"invalid_plugin\"}");}
    unsigned char idraw[16],tokenraw[32];char id[33],token[65],hash[65];
    if(RAND_bytes(idraw,sizeof(idraw))!=1||RAND_bytes(tokenraw,sizeof(tokenraw))!=1){cJSON_Delete(j);return json_reply(c,500,"{\"error\":\"random_generation_failed\"}");}
    bytes_hex(idraw,sizeof(idraw),id);bytes_hex(tokenraw,sizeof(tokenraw),token);
    if(sha256_hex(token,hash)!=0){OPENSSL_cleanse(tokenraw,sizeof(tokenraw));cJSON_Delete(j);return json_reply(c,500,"{\"error\":\"token_hash_failed\"}");}
    pthread_mutex_lock(&g_app.lock);sqlite3_stmt *st=NULL;
    int rc=sqlite3_prepare_v2(g_app.db,"INSERT INTO plugins(id,name,callback_url,token_hash,enabled) VALUES(?1,?2,?3,?4,1)",-1,&st,NULL);
    if(rc==SQLITE_OK){sqlite3_bind_text(st,1,id,-1,SQLITE_TRANSIENT);sqlite3_bind_text(st,2,name->valuestring,-1,SQLITE_TRANSIENT);
      sqlite3_bind_text(st,3,cJSON_IsString(callback)?callback->valuestring:"",-1,SQLITE_TRANSIENT);sqlite3_bind_text(st,4,hash,-1,SQLITE_TRANSIENT);rc=sqlite3_step(st);}
    sqlite3_finalize(st);pthread_mutex_unlock(&g_app.lock);cJSON_Delete(j);
    OPENSSL_cleanse(tokenraw,sizeof(tokenraw));OPENSSL_cleanse(hash,sizeof(hash));
    if(rc!=SQLITE_DONE){OPENSSL_cleanse(token,sizeof(token));return json_reply(c,500,"{\"error\":\"plugin_create_failed\"}");}
    cJSON *out=cJSON_CreateObject();cJSON_AddStringToObject(out,"id",id);cJSON_AddStringToObject(out,"token",token);
    cJSON_AddBoolToObject(out,"enabled",1);char *encoded=cJSON_PrintUnformatted(out);cJSON_Delete(out);OPENSSL_cleanse(token,sizeof(token));
    if(!encoded)return json_reply(c,500,"{\"error\":\"json_encode_failed\"}");
    enum MHD_Result ret=json_reply(c,201,encoded);free(encoded);log_line("INFO","Plugin registered; token returned once");return ret;
}
static enum MHD_Result plugin_list(struct MHD_Connection *c) {
    if(!admin_authorized(c))return json_reply(c,401,"{\"error\":\"admin_auth_required\"}");
    cJSON *arr=cJSON_CreateArray();sqlite3_stmt *st=NULL;if(!arr)return json_reply(c,500,"{\"error\":\"out_of_memory\"}");
    pthread_mutex_lock(&g_app.lock);
    if(sqlite3_prepare_v2(g_app.db,"SELECT id,name,enabled,created_at FROM plugins ORDER BY created_at DESC",-1,&st,NULL)==SQLITE_OK){
      while(sqlite3_step(st)==SQLITE_ROW){cJSON *o=cJSON_CreateObject();if(!o)break;
        cJSON_AddStringToObject(o,"id",(const char*)sqlite3_column_text(st,0));cJSON_AddStringToObject(o,"name",(const char*)sqlite3_column_text(st,1));
        cJSON_AddBoolToObject(o,"enabled",sqlite3_column_int(st,2));cJSON_AddStringToObject(o,"created_at",(const char*)sqlite3_column_text(st,3));cJSON_AddItemToArray(arr,o);}
    }
    sqlite3_finalize(st);pthread_mutex_unlock(&g_app.lock);
    char *encoded=cJSON_PrintUnformatted(arr);cJSON_Delete(arr);if(!encoded)return json_reply(c,500,"{\"error\":\"json_encode_failed\"}");
    enum MHD_Result ret=json_reply(c,200,encoded);free(encoded);return ret;
}
static enum MHD_Result event_inbox_list(struct MHD_Connection *c) {
    if (!admin_authorized(c)) return json_reply(c, 401, "{\"error\":\"admin_auth_required\"}");
    cJSON *array = cJSON_CreateArray();
    sqlite3_stmt *st = NULL;
    if (!array) return json_reply(c, 500, "{\"error\":\"out_of_memory\"}");
    pthread_mutex_lock(&g_app.lock);
    int rc = sqlite3_prepare_v2(g_app.db,
        "SELECT id,event_type,event_json,state,created_at FROM event_inbox ORDER BY id DESC LIMIT 100",
        -1, &st, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(st) == SQLITE_ROW) {
            cJSON *item = cJSON_CreateObject();
            if (!item) break;
            cJSON_AddNumberToObject(item, "id", sqlite3_column_int64(st, 0));
            cJSON_AddStringToObject(item, "event_type", (const char *)sqlite3_column_text(st, 1));
            cJSON_AddStringToObject(item, "event_json", (const char *)sqlite3_column_text(st, 2));
            cJSON_AddStringToObject(item, "state", (const char *)sqlite3_column_text(st, 3));
            cJSON_AddStringToObject(item, "created_at", (const char *)sqlite3_column_text(st, 4));
            cJSON_AddItemToArray(array, item);
        }
    }
    sqlite3_finalize(st);
    pthread_mutex_unlock(&g_app.lock);
    char *encoded = cJSON_PrintUnformatted(array);
    cJSON_Delete(array);
    if (!encoded) return json_reply(c, 500, "{\"error\":\"json_encode_failed\"}");
    enum MHD_Result result = json_reply(c, rc == SQLITE_OK ? 200 : 500, encoded);
    free(encoded);
    return result;
}
static enum MHD_Result plugin_proxy(struct MHD_Connection *c,const char *plugin_id,const char *body) {
    const char *header_id=MHD_lookup_connection_value(c,MHD_HEADER_KIND,"X-Plugin-ID");
    const char *token=MHD_lookup_connection_value(c,MHD_HEADER_KIND,"X-Plugin-Token");
    if(!header_id||strcmp(header_id,plugin_id)!=0||!plugin_authorized(plugin_id,token))
      return json_reply(c,403,"{\"error\":\"plugin_auth_failed\"}");
    cJSON *j=cJSON_Parse(body?body:"");cJSON *m=j?cJSON_GetObjectItemCaseSensitive(j,"method"):NULL;
    cJSON *p=j?cJSON_GetObjectItemCaseSensitive(j,"path"):NULL;cJSON *b=j?cJSON_GetObjectItemCaseSensitive(j,"body"):NULL;
    if(!cJSON_IsString(m)||!cJSON_IsString(p)||p->valuestring[0]!='/'||
       (strcmp(m->valuestring,"GET")&&strcmp(m->valuestring,"POST")&&strcmp(m->valuestring,"PUT")&&strcmp(m->valuestring,"PATCH")&&strcmp(m->valuestring,"DELETE"))||
       strstr(p->valuestring,"..")||strstr(p->valuestring,"://")||strchr(p->valuestring,'\r')||strchr(p->valuestring,'\n')||
       (b&&!cJSON_IsNull(b)&&!cJSON_IsObject(b)&&!cJSON_IsArray(b))){cJSON_Delete(j);return json_reply(c,400,"{\"error\":\"invalid_qq_api_request\"}");}
    if(ensure_token()!=0){cJSON_Delete(j);return json_reply(c,502,"{\"error\":\"qq_token_unavailable\"}");}
    char token_copy[sizeof(g_app.access_token)]={0};pthread_mutex_lock(&g_app.lock);snprintf(token_copy,sizeof(token_copy),"%s",g_app.access_token);pthread_mutex_unlock(&g_app.lock);
    char url[1024];if(strlen(g_app.api_base)+strlen(p->valuestring)+1>=sizeof(url)){OPENSSL_cleanse(token_copy,sizeof(token_copy));cJSON_Delete(j);return json_reply(c,414,"{\"error\":\"qq_api_path_too_long\"}");}
    snprintf(url,sizeof(url),"%s%s",g_app.api_base,p->valuestring);
    char auth[4200];snprintf(auth,sizeof(auth),"Authorization: QQBot %s",token_copy);OPENSSL_cleanse(token_copy,sizeof(token_copy));
    char *payload=(b&&!cJSON_IsNull(b))?cJSON_PrintUnformatted(b):NULL;struct curl_slist *headers=NULL;headers=curl_slist_append(headers,auth);headers=curl_slist_append(headers,"Content-Type: application/json");
    CURL *curl=curl_easy_init();proxy_buffer_t response={0};long status=0;CURLcode cr=CURLE_FAILED_INIT;
    if(curl){curl_easy_setopt(curl,CURLOPT_URL,url);curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,m->valuestring);curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
      if(payload){curl_easy_setopt(curl,CURLOPT_POSTFIELDS,payload);curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,(long)strlen(payload));}
      curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,proxy_write);curl_easy_setopt(curl,CURLOPT_WRITEDATA,&response);
      curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,5L);curl_easy_setopt(curl,CURLOPT_TIMEOUT,10L);curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,0L);
      curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
      cr=curl_easy_perform(curl);curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&status);}
    if(curl)curl_easy_cleanup(curl);curl_slist_free_all(headers);free(payload);cJSON_Delete(j);
    if(cr!=CURLE_OK){free(response.data);log_line("ERROR","Plugin QQ API proxy transport failed");return json_reply(c,502,"{\"error\":\"qq_api_transport_failed\"}");}
    if(!response.data)response.data=strdup("{}");
    enum MHD_Result ret=reply(c,(unsigned int)(status>=100&&status<=599?status:502),response.data?response.data:"{}","application/json; charset=utf-8");
    free(response.data);return ret;
}

static enum MHD_Result handler(void *cls, struct MHD_Connection *c, const char *url, const char *method, const char *version, const char *upload, size_t *upload_size, void **con_cls) {
    (void)cls; (void)version;
    if (!*con_cls) { request_t *q = calloc(1, sizeof(*q)); if (!q) return MHD_NO; *con_cls = q; return MHD_YES; }
    request_t *q = (request_t *)*con_cls;
    if (strcmp(method, "POST") == 0 && *upload_size) {
        if (q->size + *upload_size > MAX_BODY) { *upload_size = 0; return json_reply(c, 413, "{\"error\":\"request_too_large\"}"); }
        char *p = realloc(q->body, q->size + *upload_size + 1); if (!p) return MHD_NO;
        q->body = p; memcpy(q->body + q->size, upload, *upload_size); q->size += *upload_size; q->body[q->size] = 0; *upload_size = 0; return MHD_YES;
    }
    if (strcmp(method, "POST") == 0 && strcmp(url, "/webhook/qq") == 0) {
        char secret[sizeof(g_app.app_secret)] = {0}; char configured_app_id[sizeof(g_app.app_id)] = {0};
        pthread_mutex_lock(&g_app.lock);
        snprintf(secret, sizeof(secret), "%s", g_app.app_secret);
        snprintf(configured_app_id, sizeof(configured_app_id), "%s", g_app.app_id);
        pthread_mutex_unlock(&g_app.lock);
        const char *header_app_id = MHD_lookup_connection_value(c, MHD_HEADER_KIND, "X-Bot-Appid");
        if (header_app_id && strcmp(header_app_id, configured_app_id) != 0) {
            OPENSSL_cleanse(secret, sizeof(secret)); return json_reply(c, 403, "{\"error\":\"app_id_mismatch\"}");
        }
        const char *sig = MHD_lookup_connection_value(c, MHD_HEADER_KIND, "X-Signature-Ed25519");
        const char *ts = MHD_lookup_connection_value(c, MHD_HEADER_KIND, "X-Signature-Timestamp");
        char *response = NULL, *event_type = NULL, *event_data = NULL;
        unsigned int status = 400;
        int rc = qq_webhook_process_ex(secret, q->body ? q->body : "", q->size,
                                       sig, ts, &response, &status,
                                       &event_type, &event_data);
        OPENSSL_cleanse(secret, sizeof(secret));
        if (rc != 0 || !response) {
            free(response); free(event_type); free(event_data);
            return json_reply(c, 400, "{\"error\":\"invalid_webhook_payload\"}");
        }
        if (status == 200 && event_type && event_data) {
            char *normalized = NULL;
            if (qq_event_normalize(event_type, event_data, &normalized) == 0) {
                cJSON *evt = cJSON_Parse(normalized);
                cJSON *chat = evt ? cJSON_GetObjectItemCaseSensitive(evt, "chat_id") : NULL;
                cJSON *user = evt ? cJSON_GetObjectItemCaseSensitive(evt, "user_id") : NULL;
                cJSON *msg = evt ? cJSON_GetObjectItemCaseSensitive(evt, "msg_id") : NULL;
                cJSON *stamp = evt ? cJSON_GetObjectItemCaseSensitive(evt, "timestamp") : NULL;
                char dedupe[768];
                const char *primary = cJSON_IsString(msg) ? msg->valuestring :
                                      (cJSON_IsString(stamp) ? stamp->valuestring : "no-time");
                snprintf(dedupe, sizeof(dedupe), "%s|%s|%s|%s",
                         event_type, cJSON_IsString(chat) ? chat->valuestring : "",
                         cJSON_IsString(user) ? user->valuestring : "", primary);
                pthread_mutex_lock(&g_app.lock);
                int queued = qq_event_queue_enqueue(g_app.db, dedupe, event_type, normalized);
                pthread_mutex_unlock(&g_app.lock);
                if (queued > 0) log_line("INFO", "Verified QQ group event stored in persistent inbox");
                else if (queued == 0) log_line("INFO", "Duplicate QQ group event ignored");
                else log_line("ERROR", "Failed to persist verified QQ group event");
                cJSON_Delete(evt);
                free(normalized);
            }
        }
        free(event_type); free(event_data);
        if (status == 200) log_line("INFO", "QQ Webhook payload accepted; signature/challenge processed");
        enum MHD_Result result = json_reply(c, status, response);
        free(response);
        return result;
    }
    if (strcmp(method,"GET")==0 && strcmp(url,"/api/v1/events")==0) return event_inbox_list(c);
    if (strcmp(method,"POST")==0 && strcmp(url,"/api/v1/plugins")==0) return plugin_register(c,q->body);
    if (strcmp(method,"GET")==0 && strcmp(url,"/api/v1/plugins")==0) return plugin_list(c);
    if (strncmp(url,"/api/v1/plugins/",16)==0) {
        const char *id=url+16; const char *slash=strchr(id,'/');
        size_t idlen=slash?(size_t)(slash-id):strlen(id);
        if(idlen==0||idlen>=128)return json_reply(c,404,"{\"error\":\"plugin_not_found\"}");
        char plugin_id[128];memcpy(plugin_id,id,idlen);plugin_id[idlen]=0;
        if(!slash && strcmp(method,"DELETE")==0) {
            if(!admin_authorized(c))return json_reply(c,401,"{\"error\":\"admin_auth_required\"}");
            pthread_mutex_lock(&g_app.lock);sqlite3_stmt *st=NULL;
            int rc=sqlite3_prepare_v2(g_app.db,"DELETE FROM plugins WHERE id=?1",-1,&st,NULL);
            if(rc==SQLITE_OK){sqlite3_bind_text(st,1,plugin_id,-1,SQLITE_TRANSIENT);rc=sqlite3_step(st);}
            int changed=sqlite3_changes(g_app.db);sqlite3_finalize(st);pthread_mutex_unlock(&g_app.lock);
            if(rc!=SQLITE_DONE||!changed)return json_reply(c,404,"{\"error\":\"plugin_not_found\"}");
            return json_reply(c,200,"{\"ok\":true}");
        }
        if(!slash && strcmp(method,"PATCH")==0) {
            if(!admin_authorized(c))return json_reply(c,401,"{\"error\":\"admin_auth_required\"}");
            cJSON *patch=cJSON_Parse(q->body?q->body:"");
            cJSON *enabled=patch?cJSON_GetObjectItemCaseSensitive(patch,"enabled"):NULL;
            if(!cJSON_IsBool(enabled)){cJSON_Delete(patch);return json_reply(c,400,"{\"error\":\"enabled_boolean_required\"}");}
            pthread_mutex_lock(&g_app.lock);sqlite3_stmt *st=NULL;
            int rc=sqlite3_prepare_v2(g_app.db,"UPDATE plugins SET enabled=?1 WHERE id=?2",-1,&st,NULL);
            if(rc==SQLITE_OK){sqlite3_bind_int(st,1,cJSON_IsTrue(enabled)?1:0);sqlite3_bind_text(st,2,plugin_id,-1,SQLITE_TRANSIENT);rc=sqlite3_step(st);}
            int changed=sqlite3_changes(g_app.db);sqlite3_finalize(st);pthread_mutex_unlock(&g_app.lock);cJSON_Delete(patch);
            if(rc!=SQLITE_DONE||!changed)return json_reply(c,404,"{\"error\":\"plugin_not_found\"}");
            return json_reply(c,200,"{\"ok\":true}");
        }
        if(slash && strcmp(slash,"/qq-api")==0 && strcmp(method,"POST")==0)
            return plugin_proxy(c,plugin_id,q->body);
    }
    if (strcmp(method, "GET") == 0 && strcmp(url, "/api/v1/status") == 0) {
        char out[256]; snprintf(out, sizeof(out), "{\"ok\":true,\"authenticated\":%s,\"transport_ready\":%s,\"mode\":\"%s\",\"version\":\"0.1.0\"}", g_app.access_token[0] ? "true" : "false", "false", g_app.connection_mode[0] ? g_app.connection_mode : "none"); return json_reply(c, 200, out);
    }
    if (strcmp(method, "GET") == 0 && strcmp(url, "/api/v1/logs") == 0) {
        cJSON *array = cJSON_CreateArray(); sqlite3_stmt *st = NULL;
        if (!array) return json_reply(c, 500, "{\"error\":\"out_of_memory\"}");
        pthread_mutex_lock(&g_app.lock);
        if (sqlite3_prepare_v2(g_app.db, "SELECT level,component,message,created_at FROM logs ORDER BY id DESC LIMIT 100", -1, &st, NULL) == SQLITE_OK) {
            while (sqlite3_step(st) == SQLITE_ROW) {
                cJSON *item = cJSON_CreateObject(); if (!item) break;
                cJSON_AddStringToObject(item, "level", (const char *)sqlite3_column_text(st, 0));
                cJSON_AddStringToObject(item, "component", (const char *)sqlite3_column_text(st, 1));
                cJSON_AddStringToObject(item, "message", (const char *)sqlite3_column_text(st, 2));
                cJSON_AddStringToObject(item, "created_at", (const char *)sqlite3_column_text(st, 3));
                cJSON_AddItemToArray(array, item);
            }
        }
        sqlite3_finalize(st); pthread_mutex_unlock(&g_app.lock);
        char *encoded = cJSON_PrintUnformatted(array); cJSON_Delete(array);
        if (!encoded) return json_reply(c, 500, "{\"error\":\"json_encode_failed\"}");
        enum MHD_Result result = json_reply(c, 200, encoded); free(encoded); return result;
    }
    if (strcmp(method, "POST") == 0 && strcmp(url, "/api/v1/auth/login") == 0) {
        cJSON *j = cJSON_Parse(q->body ? q->body : ""); cJSON *id = j ? cJSON_GetObjectItemCaseSensitive(j, "app_id") : NULL;
        cJSON *secret = j ? cJSON_GetObjectItemCaseSensitive(j, "app_secret") : NULL;
        cJSON *mode = j ? cJSON_GetObjectItemCaseSensitive(j, "mode") : NULL;
        if (!cJSON_IsString(id) || !cJSON_IsString(secret) || !id->valuestring[0] || !secret->valuestring[0] || strlen(id->valuestring) >= sizeof(g_app.app_id) || strlen(secret->valuestring) >= sizeof(g_app.app_secret)) { cJSON_Delete(j); return json_reply(c, 400, "{\"error\":\"invalid_credentials\"}"); }
        const char *selected_mode = cJSON_IsString(mode) ? mode->valuestring : "websocket";
        if (strcmp(selected_mode, "webhook") != 0 && strcmp(selected_mode, "websocket") != 0) { cJSON_Delete(j); return json_reply(c, 400, "{\"error\":\"invalid_connection_mode\"}"); }
        if (strcmp(selected_mode, "websocket") == 0) { cJSON_Delete(j); return json_reply(c, 501, "{\"error\":\"websocket_transport_not_available_use_webhook\"}"); }
        pthread_mutex_lock(&g_app.lock); snprintf(g_app.app_id, sizeof(g_app.app_id), "%s", id->valuestring); OPENSSL_cleanse(g_app.app_secret, sizeof(g_app.app_secret)); snprintf(g_app.app_secret, sizeof(g_app.app_secret), "%s", secret->valuestring);  snprintf(g_app.connection_mode, sizeof(g_app.connection_mode), "%s", selected_mode); pthread_mutex_unlock(&g_app.lock); cJSON_Delete(j);
        if (ensure_token() != 0) { OPENSSL_cleanse(g_app.app_secret, sizeof(g_app.app_secret)); g_app.app_id[0] = 0; log_line("ERROR", "QQ login failed: access_token request failed"); return json_reply(c, 502, "{\"error\":\"qq_auth_failed\"}"); }
        log_line("INFO", "QQ credentials accepted; token acquired"); return json_reply(c, 200, "{\"ok\":true,\"message\":\"QQ access_token acquired. Configure the public HTTPS callback URL as /webhook/qq in the QQ developer console.\"}");
    }
    if (strcmp(method, "GET") == 0 && strcmp(url, "/") == 0) {
        FILE *f = fopen("web/index.html", "rb"); if (!f) return reply(c, 404, "web/index.html not found", "text/plain");
        fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f); if (n < 0 || n > 2*1024*1024) { fclose(f); return reply(c, 500, "invalid UI asset", "text/plain"); }
        char *data = malloc((size_t)n + 1); if (!data) { fclose(f); return MHD_NO; } size_t got = fread(data, 1, (size_t)n, f); fclose(f); data[got] = 0;
        struct MHD_Response *r = MHD_create_response_from_buffer(got, data, MHD_RESPMEM_MUST_FREE); if (!r) { free(data); return MHD_NO; }
        MHD_add_response_header(r, "Content-Type", "text/html; charset=utf-8"); enum MHD_Result ret = MHD_queue_response(c, 200, r); MHD_destroy_response(r); return ret;
    }
    return json_reply(c, 404, "{\"error\":\"not_found\"}");
}
static void request_completed(void *cls, struct MHD_Connection *c, void **con_cls, enum MHD_RequestTerminationCode toe) {
    (void)cls; (void)c; (void)toe; request_t *q = (request_t *)*con_cls; if (q) { free(q->body); free(q); *con_cls = NULL; }
}
int main(void) {
    memset(&g_app, 0, sizeof(g_app)); pthread_mutex_init(&g_app.lock, NULL);
    snprintf(g_app.api_base, sizeof(g_app.api_base), "%s", getenv("QQBOT_API_BASE") ? getenv("QQBOT_API_BASE") : API_BASE_DEFAULT);
    snprintf(g_app.token_url, sizeof(g_app.token_url), "%s", getenv("QQBOT_TOKEN_URL") ? getenv("QQBOT_TOKEN_URL") : TOKEN_URL_DEFAULT);
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) return 1;
    if (mkdir("data", 0700) != 0 && access("data", F_OK) != 0) { perror("mkdir data"); return 1; }
    if (db_init() != 0) return 1;
    unsigned int port = getenv("QQBOT_PORT") ? (unsigned int)atoi(getenv("QQBOT_PORT")) : DEFAULT_PORT;
    struct sockaddr_in bind_addr; memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET; bind_addr.sin_port = htons((uint16_t)port);
    const char *bind_host = getenv("QQBOT_BIND") ? getenv("QQBOT_BIND") : "127.0.0.1";
    if (inet_pton(AF_INET, bind_host, &bind_addr.sin_addr) != 1) { fprintf(stderr, "Invalid QQBOT_BIND IPv4 address\n"); return 1; }
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, (uint16_t)port, NULL, NULL, &handler, NULL, MHD_OPTION_SOCK_ADDR, (struct sockaddr *)&bind_addr, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);
    if (!daemon) { log_line("ERROR", "failed to start HTTP server"); return 1; }
    signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
    char msg[128]; snprintf(msg, sizeof(msg), "QQBot host listening on http://%s:%u", bind_host, port); log_line("INFO", msg);
    while (!g_stop) sleep(1);
    MHD_stop_daemon(daemon); OPENSSL_cleanse(g_app.app_secret, sizeof(g_app.app_secret)); OPENSSL_cleanse(g_app.access_token, sizeof(g_app.access_token));
    sqlite3_close(g_app.db); curl_global_cleanup(); pthread_mutex_destroy(&g_app.lock); log_line("INFO", "host stopped"); return 0;
}
