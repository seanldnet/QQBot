#ifndef QQBOT_CALLBACK_SECURITY_H
#define QQBOT_CALLBACK_SECURITY_H
#include <stddef.h>
#define QQ_CALLBACK_MAX_ADDRESSES 16
#define QQ_CALLBACK_HOST_MAX 256
#define QQ_CALLBACK_IP_MAX 64
typedef struct qq_callback_target {
    char host[QQ_CALLBACK_HOST_MAX];
    unsigned short port;
    size_t address_count;
    char addresses[QQ_CALLBACK_MAX_ADDRESSES][QQ_CALLBACK_IP_MAX];
    int address_is_ipv6[QQ_CALLBACK_MAX_ADDRESSES];
} qq_callback_target;
/* HTTPS/443 only; rejects IP literals and any DNS answer outside global unicast.
 * This validates and resolves only; it does not send a request or pin a socket.
 */
int qq_callback_target_validate(const char *url, qq_callback_target *out);
/* Format validated DNS answers for CURLOPT_RESOLVE; caller owns the buffer. */
int qq_callback_target_resolve_entry(const qq_callback_target *target, char *out, size_t capacity);
int qq_callback_ipv4_is_public(const char *address);
int qq_callback_ipv6_is_public(const char *address);
#endif
