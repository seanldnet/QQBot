# 易语言 SDK 导入定义（Windows DLL）

推荐使用 x64 易语言程序加载 x64 `qqbot_sdk.dll`；x86 程序必须加载 x86 DLL。DLL 与调用进程位数必须一致。SDK 导出函数采用 `__stdcall`，文本参数按 UTF-8 NUL 结尾字节串传入；返回的 JSON 字符串必须通过 `qqb_free_string` 释放。
# DLL 名称：qqbot_sdk.dll
# 所有函数均为 __stdcall；字符串参数为 ANSI/UTF-8 指针约定，建议统一传 UTF-8。
# qqb_create(host_url, plugin_id, plugin_token) -> 整数型（客户端句柄）
# qqb_destroy(client_handle) -> 无返回值
# qqb_api_request(client, method, path, body_json, response_json*, http_status*) -> 整数型
# 参数说明：client 为 qqb_create 返回句柄；method 为 GET/POST/PUT/PATCH/DELETE；path 为 /v2/... 等官方相对路径；body_json 为合法 JSON 或空；response_json 和 http_status 均传址。
# qqb_send_group_text(client, group_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_c2c_text(client, user_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_dms_text(client, guild_id, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_channel_text(client, channel_id, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_free_string(response_json) -> 无返回值
# qqb_version() -> 文本型
#
# 注意：本文件是易语言 DLL 命令声明映射说明，不是可直接双击编译的 .e 源文件。
# 正式易语言模块源码/示例在 SDK 后续阶段补齐并使用易语言 IDE 实测。

# 已核验频道管理接口的类型化导入
# qqb_get_guild(client, guild_id, response_json*, http_status*) -> 整数型
# qqb_list_channels(client, guild_id, response_json*, http_status*) -> 整数型
# qqb_get_channel(client, channel_id, response_json*, http_status*) -> 整数型
# qqb_create_channel(client, guild_id, body_json, response_json*, http_status*) -> 整数型
# qqb_update_channel(client, channel_id, body_json, response_json*, http_status*) -> 整数型
# qqb_delete_channel(client, channel_id, response_json*, http_status*) -> 整数型
# 创建/修改的 body_json 传入完整 JSON 对象；返回 JSON 缓冲区仍使用 qqb_free_string 释放。


## 第二批官方接口（通用调用）

下列路由均通过 `qqb_api_request(client, method, path, body_json, response_json*, http_status*)` 调用；路径参数需先进行 URL 编码，GET 查询参数拼接在 path 后。所有返回 JSON 指针用 `qqb_free_string` 释放。无需为每条路由重新导入 DLL 函数。

- GET /guilds/{guild_id}/members — 频道成员列表；分页结果可能重复，调用方按 user.id 去重。 参数：guild_id；字段：after:string (默认 0；下一页使用上一页最后一个 user id)；limit:uint32 (1-400，默认 1)；权限：页面未明确具体权限；错误码引用官方错误码总表。
- GET /guilds/{guild_id}/roles/{role_id}/members — 指定身份组成员列表；返回 data 与 next。 参数：guild_id, role_id；字段：start_index:string (默认 0；下一页使用 next)；limit:uint32 (1-400，默认 1)；权限：页面未明确具体权限；错误码引用官方错误码总表。
- GET /guilds/{guild_id}/members/{user_id} — 获取频道成员详情。 参数：guild_id, user_id；字段：无；权限：页面未明确具体权限；错误码引用官方错误码总表。
- DELETE /guilds/{guild_id}/members/{user_id} — 踢出成员；成功 HTTP 204；不能移除管理员；可同步加入黑名单及撤回消息。 参数：guild_id, user_id；字段：add_blacklist:boolean；delete_history_msg_days:int (0,3,7,15,30,-1；-1 表示撤回全部消息)；权限：调用方需有踢人权限；机器人必须为管理员。错误码引用官方总表。
- GET /guilds/{guild_id}/roles — 获取频道身份组列表。 参数：guild_id；字段：无；权限：页面未明确具体权限；错误码引用官方错误码总表。
- POST /guilds/{guild_id}/roles — 创建身份组；请求字段至少传一个。 参数：guild_id；字段：name:string (可选)；color:uint32 (可选，ARGB 十进制)；hoist:int32 (可选，0/1)；权限：需创建身份组权限；机器人必须为管理员。错误码引用官方总表。
- PATCH /guilds/{guild_id}/roles/{role_id} — 修改身份组；至少传一个字段，未传字段保持不变。 参数：guild_id, role_id；字段：name:string (可选)；color:uint32 (可选，ARGB 十进制)；hoist:int32 (可选，0/1)；权限：需修改身份组权限；机器人必须为管理员。错误码引用官方总表。
- DELETE /guilds/{guild_id}/roles/{role_id} — 删除频道身份组。 参数：guild_id, role_id；字段：无；权限：需删除身份组权限；机器人必须为管理员。错误码引用官方总表。
- PUT /guilds/{guild_id}/members/{user_id}/roles/{role_id} — 将用户加入身份组。 参数：guild_id, user_id, role_id；字段：channel:Channel object (身份组为 5-子频道管理员时必需，至少含 id)；权限：需增加身份组成员权限；机器人必须为管理员。错误码引用官方总表。
- DELETE /guilds/{guild_id}/members/{user_id}/roles/{role_id} — 将用户移出身份组。 参数：guild_id, user_id, role_id；字段：channel:Channel object (身份组为 5-子频道管理员时用于指定子频道)；权限：需删除身份组成员权限；机器人必须为管理员。错误码引用官方总表。
- GET /channels/{channel_id}/members/{user_id}/permissions — 获取子频道指定用户权限。 参数：channel_id, user_id；字段：无；权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。
- PUT /channels/{channel_id}/members/{user_id}/permissions — 修改用户权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 参数：channel_id, user_id；字段：add:string (权限位图)；remove:string (权限位图)；权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。
- GET /channels/{channel_id}/roles/{role_id}/permissions — 获取子频道身份组权限。 参数：channel_id, role_id；字段：无；权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。
- PUT /channels/{channel_id}/roles/{role_id}/permissions — 修改身份组权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 参数：channel_id, role_id；字段：add:string (权限位图)；remove:string (权限位图)；权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。
- PATCH /guilds/{guild_id}/mute — 全员禁言（非管理员）或解除；成功 HTTP 204。 参数：guild_id；字段：mute_end_timestamp:string (绝对秒级时间戳)；mute_seconds:string (秒数；与前者同时传时前者优先；任一传 '0' 可解除)；权限：需频道管理员权限；机器人必须为管理员。错误码引用官方总表。
- PATCH /guilds/{guild_id}/members/{user_id}/mute — 指定成员禁言或解除；成功 HTTP 204。 参数：guild_id, user_id；字段：mute_end_timestamp:string (绝对秒级时间戳)；mute_seconds:string (秒数；与前者同时传时前者优先；任一传 '0' 可解除)；权限：需频道管理员权限；机器人必须为管理员。错误码引用官方总表。


## 第三批官方接口：公告、精华消息、日程

全部通过 `qqb_api_request` 调用，method/path/body 遵循官方原文；`since` 为毫秒时间戳，删除公告/精华消息的 `all` 特殊值不可混用。

- POST /guilds/{guild_id}/announces — 消息公告与推荐子频道公告互斥；推荐子频道最多 3 条且必须全员可见。 参数：guild_id；字段：message_id:string (optional)；channel_id:string (required when message_id is set)；announces_type:uint32 (0 member, 1 welcome; default 0)；recommend_channels:RecommendChannel[] (replaces the complete recommendation list)；权限：源页引用错误码总表，未列具体权限。
- DELETE /guilds/{guild_id}/announces/{message_id} — 删除指定公告；message_id=all 删除全部推荐子频道公告。 参数：guild_id, message_id；字段：message_id='all' deletes all announcement items；权限：源页引用错误码总表，未列具体权限。
- PUT /channels/{channel_id}/pins/{message_id} — 添加精华消息；单个子频道最多 20 条，消息必须可见。 参数：channel_id, message_id；字段：无；权限：源页引用错误码总表，未列具体权限。
- DELETE /channels/{channel_id}/pins/{message_id} — 删除指定精华消息；message_id=all 删除全部精华消息。 参数：channel_id, message_id；字段：message_id='all' deletes all pins in the channel；权限：源页引用错误码总表，未列具体权限。
- GET /channels/{channel_id}/pins — 获取子频道精华消息。 参数：channel_id；字段：无；权限：源页引用错误码总表，未列具体权限。
- GET /channels/{channel_id}/schedules — 获取日程列表；since 单位为毫秒。 参数：channel_id；字段：since:uint64 (milliseconds; returns schedules ending after this timestamp; default current day)；权限：源页引用错误码总表，未列具体权限。
- GET /channels/{channel_id}/schedules/{schedule_id} — 获取日程详情。 参数：channel_id, schedule_id；字段：无；权限：源页引用错误码总表，未列具体权限。
- POST /channels/{channel_id}/schedules — 创建日程；单个管理员每天最多 10 次，单频道每天最多 100 次。 参数：channel_id；字段：schedule:Schedule (without id)；权限：需管理频道权限；机器人必须为管理员。错误码引用官方总表。
