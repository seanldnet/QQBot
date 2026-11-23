# QQ 群事件模型与官方字段

本文件依据腾讯官方 bot-docs 当前 main 分支以下页面整理：

- 群管理事件：`server-inter/group/manage/event.md`
- 消息事件：`server-inter/message/send-receive/event.md`

## 官方事件目录

| 事件 | Gateway Intent | 类型 | 关键字段 |
| --- | --- | --- | --- |
| `GROUP_ADD_ROBOT` | `1<<25` | 群管理 | `timestamp`（整数）、`group_openid`、`op_member_openid` |
| `GROUP_DEL_ROBOT` | `1<<25` | 群管理 | `timestamp`（整数）、`group_openid`、`op_member_openid` |
| `GROUP_MSG_REJECT` | `1<<25` | 群管理 | `timestamp`（整数）、`group_openid`、`op_member_openid` |
| `GROUP_MSG_RECEIVE` | `1<<25` | 群管理 | `timestamp`（整数）、`group_openid`、`op_member_openid` |
| `GROUP_AT_MESSAGE_CREATE` | `1<<25` | 群消息 | `id`、`author.member_openid`、`content`、`timestamp`（RFC3339 字符串）、`group_openid`、`attachments` |

四种群管理事件均通过 WebSocket 推送；群聊 @ 机器人消息也通过 WebSocket 推送。官方文档把全量群消息事件标为私域/暂不开放，不应擅自订阅或假设可用。

## 宿主统一事件 JSON

宿主内部解析器 `qq_event_normalize` 接收 Gateway 的外层事件名 `t` 和内层 `d` JSON 对象，输出既有 `sdk/event-schema.md` 契约：

- `event_type` 保留官方事件名。
- `chat_type` 为 `group`，`chat_id` 为 `group_openid`。
- 群管理事件将 `op_member_openid` 映射到 `user_id`；消息事件将 `author.member_openid` 映射到 `user_id`。
- 群消息的官方 `id` 映射为 `msg_id`，用于被动回复关联；`content` 仅在官方提供文本时映射。
- `timestamp` 标准化为文本：群管理事件的整数 Unix 秒转为十进制文本，群 @ 消息的 RFC3339 字符串原样保留。
- `event_id` 与 `msg_seq` 当前置为 JSON null，因为上述官方事件样例未提供可可靠映射的值。
- `raw_event` 保留完整内层事件对象，包括附件等未标准化字段。

解析器不启动网络连接，不处理 Gateway HELLO/IDENTIFY/心跳/重连，也不向插件投递事件。输入 JSON 必须为对象；缺失必要字段或不支持的事件类型会返回错误。输出字符串由调用方使用标准 `free()` 释放。当前解析器仅供宿主内部使用，不是稳定的跨 DLL ABI。

## 权限与去重边界

- 腾讯文档对 `GROUP_AT_MESSAGE_CREATE` 标注 Intent `1<<25`、无特殊权限要求；实际可用性仍由机器人应用类型、平台权限及控制台配置决定。
- 官方提示相同消息可能重复推送，建议按消息 ID 做去重；发送被动回复时使用 `msg_seq` 处理重复回复。当前解析器保留消息 ID，但宿主去重存储和回复序号管理尚未实现。
- 群管理事件的操作人标识是 `op_member_openid`；群消息作者标识是 `author.member_openid`。二者都不是频道的 `user_id`，更不能与 QQ 号混用。
