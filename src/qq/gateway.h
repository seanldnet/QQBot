#ifndef QQBOT_GATEWAY_H
#define QQBOT_GATEWAY_H
#include <stddef.h>

typedef int (*qq_gateway_token_fn)(void *context, char *token, size_t capacity);
typedef void (*qq_gateway_event_fn)(void *context, const char *event_type,
                                    const char *data_json, long long sequence);
typedef void (*qq_gateway_state_fn)(void *context, const char *state,
                                    const char *detail);

typedef struct qq_gateway qq_gateway_t;

qq_gateway_t *qq_gateway_start(const char *api_base,
                               qq_gateway_token_fn token_fn,
                               qq_gateway_event_fn event_fn,
                               qq_gateway_state_fn state_fn,
                               void *context);
void qq_gateway_stop(qq_gateway_t *gateway);
int qq_gateway_available(void);
#endif
