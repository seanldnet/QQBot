#include "../src/qq/events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check(int condition, const char *message) {
    if (!condition) fprintf(stderr, "FAIL: %s\n", message);
    return condition ? 0 : 1;
}

int main(void) {
    char *json = NULL;
    int failures = 0;
    const char *management =
        "{\"group_openid\":\"group-1\",\"op_member_openid\":\"member-1\",\"timestamp\":1699240248}";
    const char *message =
        "{\"author\":{\"member_openid\":\"member-2\"},\"content\":\"hello\","
        "\"group_openid\":\"group-2\",\"id\":\"message-1\","
        "\"timestamp\":\"2023-11-06T13:37:18+08:00\"}";

    failures += check(qq_event_normalize("GROUP_ADD_ROBOT", management, &json) == 0,
                      "normalize GROUP_ADD_ROBOT");
    failures += check(json && strstr(json, "\"chat_type\":\"group\"") != NULL,
                      "management event uses group chat type");
    failures += check(json && strstr(json, "\"user_id\":\"member-1\"") != NULL,
                      "management actor is mapped to user_id");
    failures += check(json && strstr(json, "\"timestamp\":\"1699240248\"") != NULL,
                      "management timestamp is normalized to text");
    free(json); json = NULL;

    failures += check(qq_event_normalize("GROUP_AT_MESSAGE_CREATE", message, &json) == 0,
                      "normalize GROUP_AT_MESSAGE_CREATE");
    failures += check(json && strstr(json, "\"msg_id\":\"message-1\"") != NULL,
                      "message id is preserved for passive reply");
    failures += check(json && strstr(json, "\"content\":\"hello\"") != NULL,
                      "message content is normalized");
    failures += check(json && strstr(json, "\"timestamp\":\"2023-11-06T13:37:18+08:00\"") != NULL,
                      "message timestamp is preserved");
    failures += check(json && strstr(json, "\"raw_event\"") != NULL,
                      "raw event is retained");
    free(json); json = NULL;

    const char *c2c = "{\"author\":{\"user_openid\":\"user-9\"},\"content\":\"private hello\",\"id\":\"c2c-1\",\"timestamp\":\"2026-11-23T12:00:00+08:00\"}";
    failures += check(qq_event_normalize("C2C_MESSAGE_CREATE", c2c, &json) == 0,
                      "normalize C2C_MESSAGE_CREATE");
    failures += check(json && strstr(json, "\"chat_type\":\"c2c\"") != NULL,
                      "C2C event uses c2c chat type");
    failures += check(json && strstr(json, "\"user_id\":\"user-9\"") != NULL,
                      "C2C sender is mapped to user_id");
    failures += check(json && strstr(json, "\"content\":\"private hello\"") != NULL,
                      "C2C message content is normalized");
    free(json); json = NULL;

    failures += check(qq_event_normalize("MESSAGE_CREATE", message, &json) == -2,
                      "unsupported event is rejected");
    failures += check(qq_event_normalize("GROUP_ADD_ROBOT", "{}", &json) == -1,
                      "missing required management fields are rejected");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
