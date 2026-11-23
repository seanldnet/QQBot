#include "webhook.h"
#include <cjson/cJSON.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_decode(const char *src, unsigned char *dst, size_t dst_len) {
    if (!src || strlen(src) != dst_len * 2) return -1;
    for (size_t i = 0; i < dst_len; ++i) {
        unsigned int v = 0;
        for (size_t j = 0; j < 2; ++j) {
            unsigned char ch = (unsigned char)src[i * 2 + j];
            unsigned int nibble;
            if (ch >= '0' && ch <= '9') nibble = ch - '0';
            else if (ch >= 'a' && ch <= 'f') nibble = ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F') nibble = ch - 'A' + 10;
            else return -1;
            v = (v << 4) | nibble;
        }
        dst[i] = (unsigned char)v;
    }
    return 0;
}
static void hex_encode(const unsigned char *src, size_t n, char *dst) {
    static const char h[] = "0123456789abcdef";
    for (size_t i = 0; i < n; ++i) { dst[i * 2] = h[src[i] >> 4]; dst[i * 2 + 1] = h[src[i] & 15]; }
    dst[n * 2] = 0;
}
static EVP_PKEY *private_key_from_secret(const char *secret) {
    unsigned char seed[32]; size_t n = strlen(secret), copied = 0;
    if (!n) return NULL;
    while (copied < sizeof(seed)) {
        size_t take = n < sizeof(seed) - copied ? n : sizeof(seed) - copied;
        memcpy(seed + copied, secret, take); copied += take;
    }
    EVP_PKEY *key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, seed, sizeof(seed));
    OPENSSL_cleanse(seed, sizeof(seed)); return key;
}
static int sign_challenge(const char *secret, const char *ts, const char *plain, char **out) {
    EVP_PKEY *key = private_key_from_secret(secret); EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!key || !ctx) { EVP_PKEY_free(key); EVP_MD_CTX_free(ctx); return -1; }
    size_t msg_len = strlen(ts) + strlen(plain); char *msg = malloc(msg_len + 1);
    if (!msg) { EVP_PKEY_free(key); EVP_MD_CTX_free(ctx); return -1; }
    memcpy(msg, ts, strlen(ts)); memcpy(msg + strlen(ts), plain, strlen(plain)); msg[msg_len] = 0;
    unsigned char sig[64]; size_t sig_len = sizeof(sig);
    int ok = EVP_DigestSignInit(ctx, NULL, NULL, NULL, key) == 1 && EVP_DigestSign(ctx, sig, &sig_len, (unsigned char *)msg, msg_len) == 1 && sig_len == 64;
    free(msg); EVP_MD_CTX_free(ctx); EVP_PKEY_free(key);
    if (!ok) { OPENSSL_cleanse(sig, sizeof(sig)); return -1; }
    char encoded[129]; hex_encode(sig, sizeof(sig), encoded); OPENSSL_cleanse(sig, sizeof(sig));
    cJSON *res = cJSON_CreateObject(); if (!res) return -1;
    cJSON_AddStringToObject(res, "plain_token", plain); cJSON_AddStringToObject(res, "signature", encoded);
    *out = cJSON_PrintUnformatted(res); cJSON_Delete(res); return *out ? 0 : -1;
}
static int verify_event(const char *secret, const char *body, size_t body_len, const char *signature_hex, const char *timestamp) {
    unsigned char sig[64], pub[32]; if (hex_decode(signature_hex, sig, sizeof(sig)) != 0) return 0;
    EVP_PKEY *priv = private_key_from_secret(secret); if (!priv) return 0;
    size_t pub_len = sizeof(pub); int got = EVP_PKEY_get_raw_public_key(priv, pub, &pub_len);
    EVP_PKEY_free(priv); if (got != 1 || pub_len != sizeof(pub)) { OPENSSL_cleanse(sig, sizeof(sig)); return 0; }
    EVP_PKEY *key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub, sizeof(pub));
    OPENSSL_cleanse(pub, sizeof(pub)); if (!key) { OPENSSL_cleanse(sig, sizeof(sig)); return 0; }
    size_t ts_len = strlen(timestamp); unsigned char *msg = malloc(ts_len + body_len);
    if (!msg) { EVP_PKEY_free(key); OPENSSL_cleanse(sig, sizeof(sig)); return 0; }
    memcpy(msg, timestamp, ts_len); memcpy(msg + ts_len, body, body_len);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new(); int ok = ctx && EVP_DigestVerifyInit(ctx, NULL, NULL, NULL, key) == 1 && EVP_DigestVerify(ctx, sig, sizeof(sig), msg, ts_len + body_len) == 1;
    EVP_MD_CTX_free(ctx); EVP_PKEY_free(key); OPENSSL_cleanse(sig, sizeof(sig)); OPENSSL_cleanse(msg, ts_len + body_len); free(msg); return ok;
}
int qq_webhook_process_ex(const char *secret, const char *body, size_t body_len,
                          const char *sig, const char *timestamp,
                          char **response, unsigned int *status,
                          char **event_type, char **event_data_json) {
    cJSON *root = NULL, *op = NULL, *data = NULL, *type = NULL;
    if (!secret || !secret[0] || !body || !response || !status) return -1;
    *response = NULL; *status = 400;
    if (event_type) *event_type = NULL;
    if (event_data_json) *event_data_json = NULL;
    root = cJSON_ParseWithLength(body, body_len);
    if (!root) return -1;
    op = cJSON_GetObjectItemCaseSensitive(root, "op");
    data = cJSON_GetObjectItemCaseSensitive(root, "d");
    if (!cJSON_IsNumber(op)) { cJSON_Delete(root); return -1; }
    if (op->valueint == 13) {
        cJSON *plain = data ? cJSON_GetObjectItemCaseSensitive(data, "plain_token") : NULL;
        cJSON *ts = data ? cJSON_GetObjectItemCaseSensitive(data, "event_ts") : NULL;
        int rc = cJSON_IsString(plain) && cJSON_IsString(ts)
            ? sign_challenge(secret, ts->valuestring, plain->valuestring, response) : -1;
        *status = rc == 0 ? 200 : 400;
        cJSON_Delete(root);
        return rc;
    }
    if (op->valueint == 0) {
        int valid = sig && timestamp && verify_event(secret, body, body_len, sig, timestamp);
        if (!valid) {
            cJSON_Delete(root);
            *status = 401;
            *response = strdup("{\"error\":\"invalid_signature\"}");
            return *response ? 0 : -1;
        }
        type = cJSON_GetObjectItemCaseSensitive(root, "t");
        if (event_type && event_data_json && cJSON_IsString(type) && type->valuestring &&
            cJSON_IsObject(data)) {
            *event_type = strdup(type->valuestring);
            *event_data_json = cJSON_PrintUnformatted(data);
            if (!*event_type || !*event_data_json) {
                free(*event_type); free(*event_data_json);
                *event_type = NULL; *event_data_json = NULL;
                cJSON_Delete(root);
                return -1;
            }
        }
        cJSON_Delete(root);
        *status = 200;
        *response = strdup("{\"op\":12}");
        return *response ? 0 : -1;
    }
    cJSON_Delete(root);
    *status = 200;
    *response = strdup("{\"op\":12}");
    return *response ? 0 : -1;
}

int qq_webhook_process(const char *secret, const char *body, size_t body_len,
                       const char *sig, const char *timestamp,
                       char **response, unsigned int *status) {
    return qq_webhook_process_ex(secret, body, body_len, sig, timestamp,
                                 response, status, NULL, NULL);
}
