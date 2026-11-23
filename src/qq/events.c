#include "events.h"
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int is_group_management(const char *type) {
    return strcmp(type, "GROUP_ADD_ROBOT") == 0 ||
           strcmp(type, "GROUP_DEL_ROBOT") == 0 ||
           strcmp(type, "GROUP_MSG_REJECT") == 0 ||
           strcmp(type, "GROUP_MSG_RECEIVE") == 0;
}

static int add_string_or_null(cJSON *obj, const char *key, const char *value) {
    return value ? cJSON_AddStringToObject(obj, key, value) != NULL
                 : cJSON_AddNullToObject(obj, key) != NULL;
}

int qq_event_normalize(const char *event_type, const char *data_json,
                       char **normalized_json) {
    cJSON *data = NULL, *root = NULL, *author = NULL, *field = NULL, *raw = NULL;
    const char *chat_type = "unknown", *chat_id = NULL, *user_id = NULL;
    const char *content = NULL, *msg_id = NULL, *timestamp = NULL;
    char timestamp_buf[32];
    int is_message = 0, is_management = 0;
    char *printed = NULL;

    if (!event_type || !event_type[0] || !data_json || !normalized_json) return -1;
    *normalized_json = NULL;
    is_management = is_group_management(event_type);
    is_message = strcmp(event_type, "GROUP_AT_MESSAGE_CREATE") == 0;
    if (!is_management && !is_message) return -2;

    data = cJSON_Parse(data_json);
    if (!cJSON_IsObject(data)) goto invalid;

    field = cJSON_GetObjectItemCaseSensitive(data, "group_openid");
    if (!cJSON_IsString(field) || !field->valuestring || !field->valuestring[0]) goto invalid;
    chat_type = "group";
    chat_id = field->valuestring;

    if (is_management) {
        field = cJSON_GetObjectItemCaseSensitive(data, "op_member_openid");
        if (!cJSON_IsString(field) || !field->valuestring || !field->valuestring[0]) goto invalid;
        user_id = field->valuestring;
        field = cJSON_GetObjectItemCaseSensitive(data, "timestamp");
        if (!cJSON_IsNumber(field)) goto invalid;
        snprintf(timestamp_buf, sizeof(timestamp_buf), "%lld", (long long)field->valuedouble);
        timestamp = timestamp_buf;
    } else {
        author = cJSON_GetObjectItemCaseSensitive(data, "author");
        field = author ? cJSON_GetObjectItemCaseSensitive(author, "member_openid") : NULL;
        if (!cJSON_IsString(field) || !field->valuestring || !field->valuestring[0]) goto invalid;
        user_id = field->valuestring;
        field = cJSON_GetObjectItemCaseSensitive(data, "content");
        if (cJSON_IsString(field)) content = field->valuestring;
        field = cJSON_GetObjectItemCaseSensitive(data, "timestamp");
        if (!cJSON_IsString(field) || !field->valuestring) goto invalid;
        timestamp = field->valuestring;
        field = cJSON_GetObjectItemCaseSensitive(data, "id");
        if (!cJSON_IsString(field) || !field->valuestring || !field->valuestring[0]) goto invalid;
        msg_id = field->valuestring;
    }

    root = cJSON_CreateObject();
    raw = cJSON_Duplicate(data, 1);
    if (!root || !raw ||
        !cJSON_AddNumberToObject(root, "schema_version", 1) ||
        !cJSON_AddStringToObject(root, "event_type", event_type) ||
        !cJSON_AddStringToObject(root, "chat_type", chat_type) ||
        !add_string_or_null(root, "chat_id", chat_id) ||
        !add_string_or_null(root, "user_id", user_id) ||
        !add_string_or_null(root, "content", content) ||
        !add_string_or_null(root, "timestamp", timestamp) ||
        !add_string_or_null(root, "event_id", NULL) ||
        !add_string_or_null(root, "msg_id", msg_id) ||
        !cJSON_AddNullToObject(root, "msg_seq") ||
        !cJSON_AddItemToObject(root, "raw_event", raw)) {
        cJSON_Delete(raw);
        goto invalid;
    }
    raw = NULL;
    printed = cJSON_PrintUnformatted(root);
    if (!printed) goto invalid;
    *normalized_json = printed;
    cJSON_Delete(root);
    cJSON_Delete(data);
    return 0;

invalid:
    cJSON_Delete(raw);
    cJSON_Delete(root);
    cJSON_Delete(data);
    free(printed);
    return -1;
}
