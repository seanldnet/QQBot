# QQ 官方 API v2 文档目录与 SDK 覆盖清单

> 本目录依据腾讯官方文档仓库 `tencent-connect/bot-docs` 的 `docs/develop/api-v2/config.js` 生成，记录该官方 API v2 目录当前列出的全部文档页面。它包含接口说明、事件说明、消息结构和通用协议，不代表每个页面都是一个独立 HTTP endpoint。SDK 的 `qqb_api_request(method, path, body)` 通用调用用于覆盖官方文档定义的 REST 路由；调用权限、机器人类型、事件上下文和频控仍以每个接口原文为准。

- 官方文档仓库：https://github.com/tencent-connect/bot-docs/tree/master/docs/develop/api-v2
- 官方目录配置：https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/config.js
- 目录抓取时间：2026-11-23
- 当前目录页面数：81

## SDK 覆盖原则

1. C ABI 提供通用 REST 调用入口，method 支持 GET/POST/PUT/PATCH/DELETE，path 使用官方 API 相对路径，body 为 JSON。
2. C++ 与易语言都调用同一 C ABI，不能各自维护不同的 endpoint 实现。
3. 官方新增接口只需更新本目录/接口元数据，通用调用入口即可访问；高频接口再增加类型安全的便利封装。
4. 官方文档标注“暂未支持”、权限受限、已下线或有频控的能力，不得包装成无条件可用。
5. SDK 通过宿主插件代理调用，AppSecret/access_token 留在宿主，不下发给插件。
6. 当前宿主 `/api/v1/plugins/{id}/qq-api` 代理路由尚未实现；SDK 目前是客户端底座，代理路由完成并通过集成测试后才形成可运行闭环。

## 鉴权与域名纠正（以官方当前文档为准）

官方 `api-use.md` 当前写明：Access Token 获取地址为 `https://bots.qq.com/app/getAppAccessToken`，统一 OpenAPI 地址为 `https://api.sgroup.qq.com`，Authorization 为 `QQBot ACCESS_TOKEN`。这与用户最初给出的 `https://api.bot.qq.com` 不一致。保留用户原始需求原文，但实际实现必须以官方文档为准，并将 API Base URL 配置化，避免硬编码错误域名。

## 官方 API v2 目录全量索引

### 起步指南与通用框架

- [client-func/intro/baseinfo.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/client-func/intro/baseinfo.md)
- [client-func/intro/datacard.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/client-func/intro/datacard.md)
- [client-func/intro/find.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/client-func/intro/find.md)
- [dev-prepare/interface-framework/api-use.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/dev-prepare/interface-framework/api-use.md)
- [dev-prepare/interface-framework/event-emit.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/dev-prepare/interface-framework/event-emit.md)
- [dev-prepare/unique-id.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/dev-prepare/unique-id.md)
- [dev-prepare/error-trace/openapi.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/dev-prepare/error-trace/openapi.md)
- [dev-prepare/error-trace/websocket.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/dev-prepare/error-trace/websocket.md)

### 消息接口、消息类型与交互

- [server-inter/message/send-receive/send.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/send-receive/send.md)
- [server-inter/message/send-receive/rich-media.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/send-receive/rich-media.md)
- [server-inter/message/send-receive/reset.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/send-receive/reset.md)
- [server-inter/message/send-receive/event.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/send-receive/event.md)
- [server-inter/message/type/text.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/text.md)
- [server-inter/message/type/rich-text.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/rich-text.md)
- [server-inter/message/type/image.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/image.md)
- [server-inter/message/type/audio.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/audio.md)
- [server-inter/message/type/video.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/video.md)
- [server-inter/message/type/file.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/file.md)
- [server-inter/message/type/sticker.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/sticker.md)
- [server-inter/message/type/ark.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/ark.md)
- [server-inter/message/type/embed.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/embed.md)
- [server-inter/message/type/markdown.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/type/markdown.md)
- [server-inter/message/trans/emoji.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/trans/emoji.md)
- [server-inter/message/trans/text-chain.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/trans/text-chain.md)
- [server-inter/message/trans/msg-btn.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/message/trans/msg-btn.md)

### 频道管理、成员、权限、发言与内容

- [server-inter/channel/manage/user/me.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/user/me.md)
- [server-inter/channel/manage/guild/guilds.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/guild/guilds.md)
- [server-inter/channel/manage/guild/get_guild.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/guild/get_guild.md)
- [server-inter/channel/manage/channel/get_channels.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/channel/get_channels.md)
- [server-inter/channel/manage/channel/get_channel.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/channel/get_channel.md)
- [server-inter/channel/manage/channel/post_channels.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/channel/post_channels.md)
- [server-inter/channel/manage/channel/patch_channel.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/channel/patch_channel.md)
- [server-inter/channel/manage/channel/delete_channel.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/channel/delete_channel.md)
- [server-inter/channel/manage/event/guild.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/event/guild.md)
- [server-inter/channel/manage/event/channel.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/manage/event/channel.md)
- [server-inter/channel/role/get_online_nums.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/get_online_nums.md)
- [server-inter/channel/role/member/get_members.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/member/get_members.md)
- [server-inter/channel/role/member/get_role_members.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/member/get_role_members.md)
- [server-inter/channel/role/member/get_member.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/member/get_member.md)
- [server-inter/channel/role/member/delete_member.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/member/delete_member.md)
- [server-inter/channel/role/guild_member.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/guild_member.md)
- [server-inter/channel/role/audio_or_live_channel_member.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role/audio_or_live_channel_member.md)
- [server-inter/channel/role-group/get_guild_roles.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/get_guild_roles.md)
- [server-inter/channel/role-group/post_guild_role.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/post_guild_role.md)
- [server-inter/channel/role-group/patch_guild_role.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/patch_guild_role.md)
- [server-inter/channel/role-group/delete_guild_role.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/delete_guild_role.md)
- [server-inter/channel/role-group/put_guild_member_role.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/put_guild_member_role.md)
- [server-inter/channel/role-group/delete_guild_member_role.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/delete_guild_member_role.md)
- [server-inter/channel/role-group/channel_permissions/get_channel_permissions.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/get_channel_permissions.md)
- [server-inter/channel/role-group/channel_permissions/put_channel_permissions.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/put_channel_permissions.md)
- [server-inter/channel/role-group/channel_permissions/get_channel_roles_permissions.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/get_channel_roles_permissions.md)
- [server-inter/channel/role-group/channel_permissions/put_channel_roles_permissions.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/role-group/channel_permissions/put_channel_roles_permissions.md)
- [server-inter/channel/api_permissions/get_guild_api_permission.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/api_permissions/get_guild_api_permission.md)
- [server-inter/channel/api_permissions/post_api_permission_demand.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/api_permissions/post_api_permission_demand.md)
- [server-inter/channel/speak/setting/message_setting.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/speak/setting/message_setting.md)
- [server-inter/channel/speak/patch_guild_mute.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/speak/patch_guild_mute.md)
- [server-inter/channel/speak/patch_guild_member_mute.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/speak/patch_guild_member_mute.md)
- [server-inter/channel/speak/patch_guild_mute_multi_member.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/speak/patch_guild_mute_multi_member.md)
- [server-inter/channel/content/announces/post_guild_announces.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/announces/post_guild_announces.md)
- [server-inter/channel/content/announces/delete_guild_announces.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/announces/delete_guild_announces.md)
- [server-inter/channel/content/pins/put_pins_message.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/pins/put_pins_message.md)
- [server-inter/channel/content/pins/delete_pins_message.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/pins/delete_pins_message.md)
- [server-inter/channel/content/pins/get_pins_message.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/pins/get_pins_message.md)
- [server-inter/channel/content/schedule/get_schedules.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/schedule/get_schedules.md)
- [server-inter/channel/content/schedule/get_schedule.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/schedule/get_schedule.md)
- [server-inter/channel/content/schedule/post_schedule.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/schedule/post_schedule.md)
- [server-inter/channel/content/schedule/patch_schedule.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/schedule/patch_schedule.md)
- [server-inter/channel/content/schedule/delete_schedule.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/schedule/delete_schedule.md)
- [server-inter/channel/content/audio/audio_control.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/audio/audio_control.md)
- [server-inter/channel/content/audio/put_mic.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/audio/put_mic.md)
- [server-inter/channel/content/audio/delete_mic.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/audio/delete_mic.md)
- [server-inter/channel/content/forum/get_threads_list.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/get_threads_list.md)
- [server-inter/channel/content/forum/get_thread.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/get_thread.md)
- [server-inter/channel/content/forum/put_thread.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/put_thread.md)
- [server-inter/channel/content/forum/delete_thread.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/delete_thread.md)
- [server-inter/channel/content/forum/forum.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/forum.md)
- [server-inter/channel/content/forum/open_forum.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/content/forum/open_forum.md)
- [server-inter/channel/miniapp/opendata.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/miniapp/opendata.md)
- [server-inter/channel/miniapp/interface.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/channel/miniapp/interface.md)

### QQ群事件

- [server-inter/group/manage/event.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/group/manage/event.md)

### 用户事件

- [server-inter/user/manage/event.md](https://github.com/tencent-connect/bot-docs/blob/master/docs/develop/api-v2/server-inter/user/manage/event.md)

## 已从官方原文核验的核心 REST 路由

| 场景 | 方法 | 官方路径 | 备注 |
|---|---|---|---|
| 获取子频道列表 | GET | `/guilds/{guild_id}/channels` | 频道管理 |
| 单聊发送消息 | POST | `/v2/users/{openid}/messages` | 被动回复需 event_id/msg_id 等上下文 |
| 群聊发送消息 | POST | `/v2/groups/{group_openid}/messages` | 被动回复需 event_id/msg_id 等上下文 |
| 频道文字子频道发送 | POST | `/channels/{channel_id}/messages` | 官方 send.md 指向详细接口 |
| 频道私信发送 | POST | `/dms/{guild_id}/messages` | 官方 send.md 指向详细接口 |

## 消息发送规则

官方 `send.md` 明确：自 2025-04-21 起主动推送能力不再提供；单聊、群聊、文字子频道和频道私信均需遵守被动回复时效、频次、事件上下文及机器人权限。SDK 便利函数不能绕过平台限制。发送接口 timeout 建议至少 5 秒。详情见官方原文。

## SDK 方法覆盖状态

| 能力 | C ABI | C++ | 易语言 |
|---|---|---|---|
| 任意官方 REST endpoint 通用调用 | 已编写 | 已封装 | DLL 导入映射已定义 |
| 群聊文本发送便利函数 | 已编写 | 已封装 | 导出函数可调用 |
| 单聊文本发送便利函数 | 已编写 | 已封装 | 导出函数可调用 |
| 频道文本发送便利函数 | 已编写 | 已封装 | 导出函数可调用 |
| 频道私信文本发送便利函数 | 已编写 | 已封装 | 导出函数可调用 |
| 全量类型化 endpoint 函数 | 后续按官方接口逐项生成 | 同步生成 | 同步生成 |
| Webhook/WebSocket 事件订阅 | 宿主负责，SDK 走插件事件回调 | 同左 | 同左 |

> “通用调用已覆盖”指 SDK 可把任意官方 REST method/path/body 交给宿主代理，不代表宿主代理路由已完成，也不代表所有官方权限已开通。