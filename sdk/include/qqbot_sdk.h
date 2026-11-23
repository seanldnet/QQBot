#ifndef QQBOT_SDK_H
#define QQBOT_SDK_H
#include <stddef.h>
#ifdef _WIN32
#  if defined(QQBOT_SDK_BUILD)
#    define QQB_API __declspec(dllexport)
#  else
#    define QQB_API __declspec(dllimport)
#  endif
#  define QQB_CALL __stdcall
#else
#  define QQB_API __attribute__((visibility("default")))
#  define QQB_CALL
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qqb_client qqb_client;
/* Create a plugin client. plugin_token is never logged or returned. */
QQB_API qqb_client *QQB_CALL qqb_create(const char *host_url, const char *plugin_id, const char *plugin_token);
QQB_API void QQB_CALL qqb_destroy(qqb_client *client);
/* Call any QQ official API path through the host's plugin-authenticated proxy.
 * method: GET/POST/PUT/PATCH/DELETE; path: /v2/...; body_json may be NULL.
 * response_json is allocated by SDK; release with qqb_free_string.
 * Returns 0 on transport success, negative SDK error otherwise.
 */
QQB_API int QQB_CALL qqb_api_request(qqb_client *client, const char *method, const char *path,
                                     const char *body_json, char **response_json, long *http_status);
QQB_API void QQB_CALL qqb_free_string(char *value);
/* Convenience wrappers; reply fields are optional. */
QQB_API int QQB_CALL qqb_send_group_text(qqb_client *client, const char *group_openid,
                                        const char *content, const char *event_id,
                                        const char *msg_id, int msg_seq,
                                        char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_send_c2c_text(qqb_client *client, const char *user_openid,
                                      const char *content, const char *event_id,
                                      const char *msg_id, int msg_seq,
                                      char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_send_dms_text(qqb_client *client, const char *guild_id,
                                       const char *content, const char *event_id,
                                       const char *msg_id, int msg_seq,
                                       char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_send_channel_text(qqb_client *client, const char *channel_id,
                                           const char *content, const char *event_id,
                                           const char *msg_id, int msg_seq,
                                           char **response_json, long *http_status);
QQB_API const char *QQB_CALL qqb_version(void);
#ifdef __cplusplus
}
#endif
#endif
