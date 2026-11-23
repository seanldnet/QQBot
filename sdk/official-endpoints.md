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

C ABI 的 `qqb_api_request(method,path,body)` 已提供通用调用底座；C++ 与易语言经同一 C ABI 调用。因此上述路由可通过通用方法调用，但本批尚未为每个 endpoint 生成独立的类型化函数。下一批继续覆盖群管理、成员、角色、权限、公告、日程、论坛、音频及用户接口。

## 官方原文

- [发送消息](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/send.md)
- [获取频道详情](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/guild/get_guild.md)
- [获取子频道列表](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/get_channels.md)
- [获取子频道详情](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/get_channel.md)
- [创建子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/post_channels.md)
- [修改子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/patch_channel.md)
- [删除子频道](https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/channel/manage/channel/delete_channel.md)
