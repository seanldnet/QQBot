# QQ 官方 API v2 REST endpoint 清单（增量核验）

> 本文件只列出已直接读取腾讯官方接口原文、并能确认 method/path 的路由。它不是全量接口清单；未抽取的文档页不能推断为 endpoint。结构化字段见同目录 `official-endpoints.json`。

核验日期：2026-11-23。统一 API Base：`https://api.sgroup.qq.com`。

| ID | 方法 | 路径 | 说明 |
|---|---|---|---|
| message.c2c.create | POST | `/v2/users/{openid}/messages` | QQ 单聊发送 |
| message.group.create | POST | `/v2/groups/{group_openid}/messages` | QQ 群聊发送 |
| message.channel.create | POST | `/channels/{channel_id}/messages` | 频道文字子频道发送 |
| message.dms.create | POST | `/dms/{guild_id}/messages` | 频道私信发送 |
| guild.get | GET | `/guilds/{guild_id}` | 获取频道详情 |
| channel.list | GET | `/guilds/{guild_id}/channels` | 获取子频道列表 |
| channel.get | GET | `/channels/{channel_id}` | 获取子频道详情 |
| channel.create | POST | `/guilds/{guild_id}/channels` | 创建子频道 |
| channel.update | PATCH | `/channels/{channel_id}` | 修改子频道 |
| channel.delete | DELETE | `/channels/{channel_id}` | 删除子频道 |
| guild.members.list | GET | `/guilds/{guild_id}/members` | 频道成员列表；分页结果可能重复，调用方按 user.id 去重。 |
| guild.role_members.list | GET | `/guilds/{guild_id}/roles/{role_id}/members` | 指定身份组成员列表；返回 data 与 next。 |
| guild.member.get | GET | `/guilds/{guild_id}/members/{user_id}` | 获取频道成员详情。 |
| guild.member.delete | DELETE | `/guilds/{guild_id}/members/{user_id}` | 踢出成员；成功 HTTP 204；不能移除管理员；可同步加入黑名单及撤回消息。 |
| guild.roles.list | GET | `/guilds/{guild_id}/roles` | 获取频道身份组列表。 |
| guild.role.create | POST | `/guilds/{guild_id}/roles` | 创建身份组；请求字段至少传一个。 |
| guild.role.update | PATCH | `/guilds/{guild_id}/roles/{role_id}` | 修改身份组；至少传一个字段，未传字段保持不变。 |
| guild.role.delete | DELETE | `/guilds/{guild_id}/roles/{role_id}` | 删除频道身份组。 |
| guild.member_role.add | PUT | `/guilds/{guild_id}/members/{user_id}/roles/{role_id}` | 将用户加入身份组。 |
| guild.member_role.remove | DELETE | `/guilds/{guild_id}/members/{user_id}/roles/{role_id}` | 将用户移出身份组。 |
| channel.member_permissions.get | GET | `/channels/{channel_id}/members/{user_id}/permissions` | 获取子频道指定用户权限。 |
| channel.member_permissions.update | PUT | `/channels/{channel_id}/members/{user_id}/permissions` | 修改用户权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 |
| channel.role_permissions.get | GET | `/channels/{channel_id}/roles/{role_id}/permissions` | 获取子频道身份组权限。 |
| channel.role_permissions.update | PUT | `/channels/{channel_id}/roles/{role_id}/permissions` | 修改身份组权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 |
| guild.mute.all | PATCH | `/guilds/{guild_id}/mute` | 全员禁言（非管理员）或解除；成功 HTTP 204。 |
| guild.member.mute | PATCH | `/guilds/{guild_id}/members/{user_id}/mute` | 指定成员禁言或解除；成功 HTTP 204。 |

## 已核实的关键字段与约束

- 单聊路径参数为 `openid`；群聊为 `group_openid`；频道消息为 `channel_id`；频道私信为 `guild_id`。
- 单聊消息要求 `msg_type`；群聊消息要求 `content` 和 `msg_type`。消息回复可使用 `event_id`、`msg_id`、`msg_seq`，其中 `msg_seq` 默认 1。
- 创建子频道要求 `name`、`type`、`position`；需要管理频道权限，机器人需配置为管理员。
- 修改子频道支持按需提交 `name`、`position`、`parent_id`、`private_type`、`speak_permission`。
- 删除子频道不可恢复，接口成功返回 HTTP 200。
- 发送消息文档列出错误码 22009（超频）、304082/304083（富媒体拉取/转换失败）；其他频道管理接口引用官方 OpenAPI 错误码总表，故 JSON 中保留原始引用，不臆造逐接口错误码。
- 官方 `send.md` 明确自 2025-04-21 起主动推送能力不再提供。被动回复时间窗、频控及事件上下文必须遵守：单聊 60 分钟；群聊、频道文字子频道、频道私信 5 分钟。群聊/单聊每条消息最多回复 5 次；频道文字子频道每秒最多 5 条。

## 权限字段状态

权限描述按接口原文摘录。若官方页面未说明具体权限名称，标记为 `undocumented_here`，这不表示无需权限。运行时仍需按机器人类型、后台开通权限和平台错误响应判断。

## SDK 对接状态

本批已为 6 条频道管理路由提供独立类型化函数：`qqb_get_guild`、`qqb_list_channels`、`qqb_get_channel`、`qqb_create_channel`、`qqb_update_channel`、`qqb_delete_channel`，并同步提供 C++ `qqbot::Client` 方法和易语言 DLL 导入映射。四种消息发送路由已有文本发送便利函数。其他复杂消息类型仍可通过通用 `qqb_api_request(method,path,body)` 调用。下一批继续覆盖群管理、成员、角色、权限、公告、日程、论坛、音频及用户接口。

## 官方原文

- [发送消息](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/send.md)
- [获取频道详情](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/guild/get_guild.md)
- [获取子频道列表](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/get_channels.md)
- [获取子频道详情](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/get_channel.md)
- [创建子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/post_channels.md)
- [修改子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/patch_channel.md)
- [删除子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/delete_channel.md)


## 本批次字段、权限与行为约束

- **guild.members.list**（GET）：路径参数 guild_id；字段/查询参数：after:string (默认 0；下一页使用上一页最后一个 user id)；limit:uint32 (1-400，默认 1)。频道成员列表；分页结果可能重复，调用方按 user.id 去重。 权限：页面未明确具体权限；错误码引用官方错误码总表。 返回：Member 对象数组
- **guild.role_members.list**（GET）：路径参数 guild_id, role_id；字段/查询参数：start_index:string (默认 0；下一页使用 next)；limit:uint32 (1-400，默认 1)。指定身份组成员列表；返回 data 与 next。 权限：页面未明确具体权限；错误码引用官方错误码总表。 返回：{data: Member[], next: string}
- **guild.member.get**（GET）：路径参数 guild_id, user_id；字段/查询参数：无。获取频道成员详情。 权限：页面未明确具体权限；错误码引用官方错误码总表。 返回：Member 对象
- **guild.member.delete**（DELETE）：路径参数 guild_id, user_id；字段/查询参数：add_blacklist:boolean；delete_history_msg_days:int (0,3,7,15,30,-1；-1 表示撤回全部消息)。踢出成员；成功 HTTP 204；不能移除管理员；可同步加入黑名单及撤回消息。 权限：调用方需有踢人权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **guild.roles.list**（GET）：路径参数 guild_id；字段/查询参数：无。获取频道身份组列表。 权限：页面未明确具体权限；错误码引用官方错误码总表。 返回：Guild roles 对象
- **guild.role.create**（POST）：路径参数 guild_id；字段/查询参数：name:string (可选)；color:uint32 (可选，ARGB 十进制)；hoist:int32 (可选，0/1)。创建身份组；请求字段至少传一个。 权限：需创建身份组权限；机器人必须为管理员。错误码引用官方总表。 返回：{role_id, role}
- **guild.role.update**（PATCH）：路径参数 guild_id, role_id；字段/查询参数：name:string (可选)；color:uint32 (可选，ARGB 十进制)；hoist:int32 (可选，0/1)。修改身份组；至少传一个字段，未传字段保持不变。 权限：需修改身份组权限；机器人必须为管理员。错误码引用官方总表。 返回：{guild_id, role_id, role}
- **guild.role.delete**（DELETE）：路径参数 guild_id, role_id；字段/查询参数：无。删除频道身份组。 权限：需删除身份组权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **guild.member_role.add**（PUT）：路径参数 guild_id, user_id, role_id；字段/查询参数：channel:Channel object (身份组为 5-子频道管理员时必需，至少含 id)。将用户加入身份组。 权限：需增加身份组成员权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **guild.member_role.remove**（DELETE）：路径参数 guild_id, user_id, role_id；字段/查询参数：channel:Channel object (身份组为 5-子频道管理员时用于指定子频道)。将用户移出身份组。 权限：需删除身份组成员权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **channel.member_permissions.get**（GET）：路径参数 channel_id, user_id；字段/查询参数：无。获取子频道指定用户权限。 权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。 返回：ChannelPermissions 对象
- **channel.member_permissions.update**（PUT）：路径参数 channel_id, user_id；字段/查询参数：add:string (权限位图)；remove:string (权限位图)。修改用户权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **channel.role_permissions.get**（GET）：路径参数 channel_id, role_id；字段/查询参数：无。获取子频道身份组权限。 权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。 返回：ChannelPermissions 对象
- **channel.role_permissions.update**（PUT）：路径参数 channel_id, role_id；字段/查询参数：add:string (权限位图)；remove:string (权限位图)。修改身份组权限；add 授权、remove 撤权；同一位同时置 1 时按删除处理；不支持修改“可管理子频道”权限。 权限：需具备管理子频道权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **guild.mute.all**（PATCH）：路径参数 guild_id；字段/查询参数：mute_end_timestamp:string (绝对秒级时间戳)；mute_seconds:string (秒数；与前者同时传时前者优先；任一传 '0' 可解除)。全员禁言（非管理员）或解除；成功 HTTP 204。 权限：需频道管理员权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204
- **guild.member.mute**（PATCH）：路径参数 guild_id, user_id；字段/查询参数：mute_end_timestamp:string (绝对秒级时间戳)；mute_seconds:string (秒数；与前者同时传时前者优先；任一传 '0' 可解除)。指定成员禁言或解除；成功 HTTP 204。 权限：需频道管理员权限；机器人必须为管理员。错误码引用官方总表。 返回：HTTP 204

本批次共新增 16 条已核验路由，累计已核验 26 条。错误码均链接官方错误码总表；源页面未列出逐接口错误码，因此未臆造。所有路由可经 C ABI 通用 qqb_api_request 调用，C++ 与易语言均复用该 ABI。

## 第三批：公告、精华消息与日程

| guild.announce.create | POST | `/guilds/{guild_id}/announces` | 消息公告与推荐子频道公告互斥；推荐子频道最多 3 条且必须全员可见。 |
| guild.announce.delete | DELETE | `/guilds/{guild_id}/announces/{message_id}` | 删除指定公告；message_id=all 删除全部推荐子频道公告。 |
| channel.pin.create | PUT | `/channels/{channel_id}/pins/{message_id}` | 添加精华消息；单个子频道最多 20 条，消息必须可见。 |
| channel.pin.delete | DELETE | `/channels/{channel_id}/pins/{message_id}` | 删除指定精华消息；message_id=all 删除全部精华消息。 |
| channel.pins.list | GET | `/channels/{channel_id}/pins` | 获取子频道精华消息。 |
| channel.schedules.list | GET | `/channels/{channel_id}/schedules` | 获取日程列表；since 单位为毫秒。 |
| channel.schedule.get | GET | `/channels/{channel_id}/schedules/{schedule_id}` | 获取日程详情。 |
| channel.schedule.create | POST | `/channels/{channel_id}/schedules` | 创建日程；单个管理员每天最多 10 次，单频道每天最多 100 次。 |

- **guild.announce.create**：路径参数 guild_id；字段 message_id:string (optional)；channel_id:string (required when message_id is set)；announces_type:uint32 (0 member, 1 welcome; default 0)；recommend_channels:RecommendChannel[] (replaces the complete recommendation list)。消息公告与推荐子频道公告互斥；推荐子频道最多 3 条且必须全员可见。 权限：源页引用错误码总表，未列具体权限。 返回：Announces object
- **guild.announce.delete**：路径参数 guild_id, message_id；字段 message_id='all' deletes all announcement items。删除指定公告；message_id=all 删除全部推荐子频道公告。 权限：源页引用错误码总表，未列具体权限。 返回：HTTP 204
- **channel.pin.create**：路径参数 channel_id, message_id；字段 无。添加精华消息；单个子频道最多 20 条，消息必须可见。 权限：源页引用错误码总表，未列具体权限。 返回：PinsMessage object
- **channel.pin.delete**：路径参数 channel_id, message_id；字段 message_id='all' deletes all pins in the channel。删除指定精华消息；message_id=all 删除全部精华消息。 权限：源页引用错误码总表，未列具体权限。 返回：HTTP 204
- **channel.pins.list**：路径参数 channel_id；字段 无。获取子频道精华消息。 权限：源页引用错误码总表，未列具体权限。 返回：PinsMessage object
- **channel.schedules.list**：路径参数 channel_id；字段 since:uint64 (milliseconds; returns schedules ending after this timestamp; default current day)。获取日程列表；since 单位为毫秒。 权限：源页引用错误码总表，未列具体权限。 返回：Schedule[]
- **channel.schedule.get**：路径参数 channel_id, schedule_id；字段 无。获取日程详情。 权限：源页引用错误码总表，未列具体权限。 返回：Schedule object
- **channel.schedule.create**：路径参数 channel_id；字段 schedule:Schedule (without id)。创建日程；单个管理员每天最多 10 次，单频道每天最多 100 次。 权限：需管理频道权限；机器人必须为管理员。错误码引用官方总表。 返回：Schedule object

本批新增 8 条路由，累计核验 34 条。日程 since 使用毫秒时间戳；创建日程有管理员/频道日配额。公告与精华消息支持 all 特殊值的语义按各自接口原文记录。

## 本批官方原文

- [guild.announce.create](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/announces/post_guild_announces.md)
- [guild.announce.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/announces/delete_guild_announces.md)
- [channel.pin.create](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/pins/put_pins_message.md)
- [channel.pin.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/pins/delete_pins_message.md)
- [channel.pins.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/pins/get_pins_message.md)
- [channel.schedules.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/schedule/get_schedules.md)
- [channel.schedule.get](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/schedule/get_schedule.md)
- [channel.schedule.create](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/schedule/post_schedule.md)



- [guild.members.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role/member/get_members.md)
- [guild.role_members.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role/member/get_role_members.md)
- [guild.member.get](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role/member/get_member.md)
- [guild.member.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role/member/delete_member.md)
- [guild.roles.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/get_guild_roles.md)
- [guild.role.create](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/post_guild_role.md)
- [guild.role.update](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/patch_guild_role.md)
- [guild.role.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/delete_guild_role.md)
- [guild.member_role.add](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/put_guild_member_role.md)
- [guild.member_role.remove](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/delete_guild_member_role.md)
- [channel.member_permissions.get](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/get_channel_permissions.md)
- [channel.member_permissions.update](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/put_channel_permissions.md)
- [channel.role_permissions.get](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/get_channel_roles_permissions.md)
- [channel.role_permissions.update](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/put_channel_roles_permissions.md)
- [guild.mute.all](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/speak/patch_guild_mute.md)
- [guild.member.mute](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/speak/patch_guild_member_mute.md)


## 第四批：日程修改/删除、语音上麦与论坛

| channel.schedule.update | PATCH | `/channels/{channel_id}/schedules/{schedule_id}` | 修改日程；要求管理频道权限，机器人需为管理员。 |
| channel.schedule.delete | DELETE | `/channels/{channel_id}/schedules/{schedule_id}` | 删除日程；要求管理频道权限，机器人需为管理员。 |
| channel.audio.mic_up | PUT | `/channels/{channel_id}/mic` | 机器人在语音子频道上麦；仅音频类机器人可用，当前需联系平台申请权限。 |
| channel.audio.mic_down | DELETE | `/channels/{channel_id}/mic` | 机器人在语音子频道下麦；仅音频类机器人可用，当前需联系平台申请权限。 |
| channel.forum.threads.list | GET | `/channels/{channel_id}/threads` | 获取子频道帖子列表；官方页面标记 PrivateDomain。 |
| channel.forum.thread.get | GET | `/channels/{channel_id}/threads/{thread_id}` | 获取帖子详情；官方页面标记 PrivateDomain。 |
| channel.forum.thread.create | PUT | `/channels/{channel_id}/threads` | 发表帖子并返回 task_id/create_time；官方页面标记 PrivateDomain。 |
| channel.forum.thread.delete | DELETE | `/channels/{channel_id}/threads/{thread_id}` | 删除帖子；官方页面标记 PrivateDomain。 |

- **channel.schedule.update**：路径参数 channel_id, schedule_id；字段 schedule:Schedule (without id)。修改日程；要求管理频道权限，机器人需为管理员。 返回：Schedule object。
- **channel.schedule.delete**：路径参数 channel_id, schedule_id；字段 无。删除日程；要求管理频道权限，机器人需为管理员。 返回：HTTP 204。
- **channel.audio.mic_up**：路径参数 channel_id；字段 无。机器人在语音子频道上麦；仅音频类机器人可用，当前需联系平台申请权限。 返回：Empty JSON object。
- **channel.audio.mic_down**：路径参数 channel_id；字段 无。机器人在语音子频道下麦；仅音频类机器人可用，当前需联系平台申请权限。 返回：Empty JSON object。
- **channel.forum.threads.list**：路径参数 channel_id；字段 无。获取子频道帖子列表；官方页面标记 PrivateDomain。 返回：threads[] and is_finish。官方标记 PrivateDomain，须具备私有域资格。
- **channel.forum.thread.get**：路径参数 channel_id, thread_id；字段 无。获取帖子详情；官方页面标记 PrivateDomain。 返回：thread object。官方标记 PrivateDomain，须具备私有域资格。
- **channel.forum.thread.create**：路径参数 channel_id；字段 title:string；content:string；format:uint32 (1 text, 2 HTML, 3 Markdown, 4 JSON)。发表帖子并返回 task_id/create_time；官方页面标记 PrivateDomain。 返回：task_id and create_time。官方标记 PrivateDomain，须具备私有域资格。
- **channel.forum.thread.delete**：路径参数 channel_id, thread_id；字段 无。删除帖子；官方页面标记 PrivateDomain。 返回：HTTP 204。官方标记 PrivateDomain，须具备私有域资格。

本批新增 8 条，累计核验 42 条。论坛四条接口均标记 PrivateDomain，不应将文档可见误解为所有机器人都可调用。音频上下麦仅限音频类机器人，且文档注明需联系平台申请权限。

## 本批官方原文

- [channel.schedule.update](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/schedule/patch_schedule.md)
- [channel.schedule.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/schedule/delete_schedule.md)
- [channel.audio.mic_up](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/audio/put_mic.md)
- [channel.audio.mic_down](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/audio/delete_mic.md)
- [channel.forum.threads.list](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/forum/get_threads_list.md)
- [channel.forum.thread.get](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/forum/get_thread.md)
- [channel.forum.thread.create](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/forum/put_thread.md)
- [channel.forum.thread.delete](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/content/forum/delete_thread.md)


## 第五批：成员踢出、身份组成员与子频道权限（类型化封装）

| ID | 方法 | 路径 | 说明 |
|---|---|---|---|
| guild.role_members.list | GET | `/guilds/{guild_id}/roles/{role_id}/members` | 指定身份组成员分页查询 |
| guild.member.delete | DELETE | `/guilds/{guild_id}/members/{user_id}` | 踢出成员；可加入黑名单及撤回历史消息 |
| guild.member_role.add | PUT | `/guilds/{guild_id}/members/{user_id}/roles/{role_id}` | 将成员加入身份组 |
| guild.member_role.remove | DELETE | `/guilds/{guild_id}/members/{user_id}/roles/{role_id}` | 将成员移出身份组 |
| channel.member_permissions.get | GET | `/channels/{channel_id}/members/{user_id}/permissions` | 查询成员子频道权限 |
| channel.member_permissions.update | PUT | `/channels/{channel_id}/members/{user_id}/permissions` | 修改成员子频道权限位图 |
| channel.role_permissions.get | GET | `/channels/{channel_id}/roles/{role_id}/permissions` | 查询身份组子频道权限 |
| channel.role_permissions.update | PUT | `/channels/{channel_id}/roles/{role_id}/permissions` | 修改身份组子频道权限位图 |

- 踢出成员：body 必须包含 `add_blacklist:boolean` 与 `delete_history_msg_days:int`；允许 0、3、7、15、30、-1（全部历史）。不能踢出管理员，机器人必须为管理员并具有踢人权限。
- 身份组成员分页：`start_index` 默认 0，`limit` 范围 1–400，使用响应的 `next` 继续分页。
- 身份组成员添加/移除：子频道管理员身份组（type 5）需要提供 channel 对象。
- 子频道权限更新：`add` / `remove` 均为权限位图字符串；同一权限位同时设置时按移除处理，且不能修改“可管理子频道”权限。
- 以上接口的精确 method/path/字段/权限/错误码来源记录在 JSON manifest；官方页面未逐项列出的错误码仍引用官方总表，不作猜测。
- SDK 本批增加 8 个独立 C ABI、C++ 和易语言 DLL 映射函数。