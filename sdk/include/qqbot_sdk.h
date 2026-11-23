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
/* Full QQ group message payload: msg_type 0/2/3/4/7, markdown/ark/media/keyboard.
 * For passive replies, supply event_id or msg_id as required by the official API.
 */
QQB_API int QQB_CALL qqb_send_group_message(qqb_client *client, const char *group_openid,
    const char *body_json, char **response_json, long *http_status);
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
/* Typed wrappers for the verified guild/channel management endpoints.
 * body_json is required for create/update; pass a complete JSON object.
 */
QQB_API int QQB_CALL qqb_get_guild(qqb_client *client, const char *guild_id,
                                    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_list_channels(qqb_client *client, const char *guild_id,
                                        char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_channel(qqb_client *client, const char *channel_id,
                                      char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_create_channel(qqb_client *client, const char *guild_id,
                                         const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_channel(qqb_client *client, const char *channel_id,
                                         const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_delete_channel(qqb_client *client, const char *channel_id,
                                         char **response_json, long *http_status);

/* Typed member and role management wrappers. Query cursor IDs use official opaque IDs.
 * limit must be in [1,400]. Mute duration/timestamp are decimal strings; "0" un-mutes.
 */
QQB_API int QQB_CALL qqb_list_guild_members(qqb_client *client, const char *guild_id,
    const char *after, unsigned int limit, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_guild_member(qqb_client *client, const char *guild_id,
    const char *user_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_list_guild_roles(qqb_client *client, const char *guild_id,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_create_guild_role(qqb_client *client, const char *guild_id,
    const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_guild_role(qqb_client *client, const char *guild_id,
    const char *role_id, const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_delete_guild_role(qqb_client *client, const char *guild_id,
    const char *role_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_mute_guild(qqb_client *client, const char *guild_id,
    const char *mute_end_timestamp, const char *mute_seconds,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_mute_guild_member(qqb_client *client, const char *guild_id,
    const char *user_id, const char *mute_end_timestamp, const char *mute_seconds,
    char **response_json, long *http_status);


/* Typed schedule and audio-channel controls. since_ms is a decimal millisecond timestamp;
 * pass NULL to use the platform's default current-day window.
 */
QQB_API int QQB_CALL qqb_list_channel_schedules(qqb_client *client, const char *channel_id,
    const char *since_ms, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_channel_schedule(qqb_client *client, const char *channel_id,
    const char *schedule_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_create_channel_schedule(qqb_client *client, const char *channel_id,
    const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_channel_schedule(qqb_client *client, const char *channel_id,
    const char *schedule_id, const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_delete_channel_schedule(qqb_client *client, const char *channel_id,
    const char *schedule_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_audio_mic_up(qqb_client *client, const char *channel_id,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_audio_mic_down(qqb_client *client, const char *channel_id,
    char **response_json, long *http_status);

/* Typed role assignment, member removal and per-channel permission wrappers.
 * JSON bodies are complete objects; pass NULL only for bodyless operations.
 */
QQB_API int QQB_CALL qqb_list_guild_role_members(qqb_client *client, const char *guild_id,
    const char *role_id, const char *start_index, unsigned int limit,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_kick_guild_member(qqb_client *client, const char *guild_id,
    const char *user_id, const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_add_guild_member_role(qqb_client *client, const char *guild_id,
    const char *user_id, const char *role_id, const char *body_json,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_remove_guild_member_role(qqb_client *client, const char *guild_id,
    const char *user_id, const char *role_id, const char *body_json,
    char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_channel_member_permissions(qqb_client *client, const char *channel_id,
    const char *user_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_channel_member_permissions(qqb_client *client, const char *channel_id,
    const char *user_id, const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_channel_role_permissions(qqb_client *client, const char *channel_id,
    const char *role_id, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_channel_role_permissions(qqb_client *client, const char *channel_id,
    const char *role_id, const char *body_json, char **response_json, long *http_status);

/* QQ group administration (distinct from guild/channel APIs).
 * Cursor is an opaque server cursor; limit is 1..30 for member pages.
 * JSON request bodies must be complete objects.
 */
QQB_API int QQB_CALL qqb_get_group_info(qqb_client *client, const char *group_openid, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_group_bot_state(qqb_client *client, const char *group_openid, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_group_mute_state(qqb_client *client, const char *group_openid, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_set_group_member_mutes(qqb_client *client, const char *group_openid, const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_list_group_members(qqb_client *client, const char *group_openid, const char *cursor, unsigned int limit, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_get_group_member(qqb_client *client, const char *group_openid, const char *member_openid, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_batch_remove_group_members(qqb_client *client, const char *group_openid, const char *body_json, char **response_json, long *http_status);


/* QQ group blacklist and join-request operations. Cursor is opaque and URL-encoded. */
QQB_API int QQB_CALL qqb_list_group_blacklist(qqb_client *client, const char *group_openid,
    const char *cursor, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_update_group_blacklist(qqb_client *client, const char *group_openid,
    const char *body_json, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_list_group_join_requests(qqb_client *client, const char *group_openid,
    const char *cursor, char **response_json, long *http_status);
QQB_API int QQB_CALL qqb_approve_group_join_request(qqb_client *client, const char *group_openid,
    const char *member_openid, const char *body_json, char **response_json, long *http_status);

QQB_API const char *QQB_CALL qqb_version(void);
#ifdef __cplusplus
}
#endif
#endif
