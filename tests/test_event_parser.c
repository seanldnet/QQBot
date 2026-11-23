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
    free(json); json = NULL;

    failures += check(qq_event_normalize("GROUP_AT_MESSAGE_CREATE", message, &json) == 0,
                      "normalize GROUP_AT_MESSAGE_CREATE");
    failures += check(json && strstr(json, "\"msg_id\":\"message-1\"") != NULL,
                      "message id is preserved for passive reply");
    failures += check(json && strstr(json, "\"content\":\"hello\"") != NULL,
                      "message content is normalized");
    failures += check(json && strstr(json, "\"raw_event\"") != NULL,
                      "raw event is retained");
    free(json); json = NULL;

    failures += check(qq_event_normalize("MESSAGE_CREATE", message, &json) == -2,
                      "unsupported event is rejected");
    failures += check(qq_event_normalize("GROUP_ADD_ROBOT", "{}", &json) == -1,
                      "missing required management fields are rejected");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
