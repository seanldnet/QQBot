#ifndef QQBOT_WEBHOOK_H
#define QQBOT_WEBHOOK_H
#include <stddef.h>
int qq_webhook_process_ex(const char *bot_secret, const char *body, size_t body_len,
                          const char *signature_hex, const char *timestamp,
                          char **response_body, unsigned int *http_status,
                          char **event_type, char **event_data_json);
int qq_webhook_process(const char *bot_secret, const char *body, size_t body_len,
                       const char *signature_hex, const char *timestamp,
                       char **response_body, unsigned int *http_status);
#endif
