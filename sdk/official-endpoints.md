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

本批次共新增 16 条已核验路由。错误码均链接官方错误码总表；源页面未列出逐接口错误码，因此未臆造。所有路由可经 C ABI 通用 qqb_api_request 调用，C++ 与易语言均复用该 ABI。

## 本批官方原文

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
