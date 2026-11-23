
# QQ 群聊 API 专项核查（2026-11-23）

> 本文用于纠正此前过度依赖频道 API 的开发偏差。QQ 群（Group）与 QQ 频道（Guild）是不同场景，标识也不同：群使用 `group_openid` / `member_openid`，频道使用 `guild_id` / `channel_id` / `user_id`。禁止混用。

## 已确认的群聊能力

### 消息收发

- 发送群消息：`POST /v2/groups/{group_openid}/messages`。
- 群 @机器人消息：`GROUP_AT_MESSAGE_CREATE`，由 WebSocket 推送。
- 机器人入群/退群：`GROUP_ADD_ROBOT` / `GROUP_DEL_ROBOT`。
- 群主动消息开关变化：`GROUP_MSG_RECEIVE` / `GROUP_MSG_REJECT`。
- `GROUP_MESSAGE_CREATE` 全量群消息在官方消息事件页仍标注为私域/暂不开放，不能默认可订阅。

群消息被动回复窗口为 5 分钟，每条入站消息最多回复 5 次；主动推送自 2025-04-21 起停止提供。消息类型包括 0 文本、2 Markdown、3 Ark、4 Embed、7 Media；消息引用暂未支持。

### 近期群管理 REST 路由（待逐页核验官方自动生成页面）

近期维护的 QQ Bot Go SDK（`by-yitong/botgo`）与相关 QQ 机器人接口文档已列出下列群管理路由。它们属于群 API，不是频道 API；其中多项有白名单或管理员身份限制。由于腾讯 bot-docs 仓库当前可见的群管理 README 仍保留旧版注释内容，自动生成 API 页面的完整字段原文尚需逐页核验，故本表标记为“已发现、待官方原文复核”，暂不计入 42 条已核验 REST 路由，也不据此宣称已在 SDK 实现。

| 方法 | 路径 | 用途 | 当前核验状态 |
|---|---|---|---|
| GET | `/v2/groups/{group_openid}/info` | 群基本资料 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/bot_state` | 机器人在群内状态 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/restrict_chat_setting` | 查询群禁言状态 | 已发现，待官方原文复核 |
| POST | `/v2/groups/{group_openid}/restrict_chat_setting` | 成员禁言/解除禁言 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/members` | 群成员分页 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/members/{member_openid}` | 群成员详情 | 已发现，待官方原文复核 |
| POST | `/v2/groups/{group_openid}/batch_remove_members` | 批量移出群成员 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/member_blacklist` | 查询群黑名单 | 已发现，待官方原文复核 |
| POST | `/v2/groups/{group_openid}/member_blacklist` | 添加/移除群黑名单 | 已发现，待官方原文复核 |
| GET | `/v2/groups/{group_openid}/join_request_list` | 拉取入群申请 | 已发现，待官方原文复核 |
| POST | `/v2/groups/{group_openid}/approval_join_request/{member_openid}` | 审批入群申请 | 已发现，待官方原文复核 |
| GET | `/v2/groups/join_approval_strategy` | 查询自动审批策略 | 已发现，待官方原文复核 |
| POST | `/v2/groups/join_approval_strategy` | 创建自动审批策略 | 已发现，待官方原文复核 |
| PATCH | `/v2/groups/join_approval_strategy/{strategy_id}` | 修改自动审批策略 | 已发现，待官方原文复核 |
| DELETE | `/v2/groups/join_approval_strategy/{strategy_id}` | 删除自动审批策略 | 已发现，待官方原文复核 |
| POST | `/v2/groups/join_approval_strategy/{strategy_id}/execute` | 执行自动审批策略 | 已发现，待官方原文复核 |
| POST | `/v2/groups/join_approval_strategy/{strategy_id}/whitelist_users` | 修改策略白名单 | 已发现，待官方原文复核 |

第三方 SDK 的当前实现注释将群资料、机器人状态、群成员/黑名单查询标记为白名单能力（未授权可能返回 11253）；禁言、审批类操作要求机器人具备群管理员身份。此为交叉核对线索，不代替腾讯官方页面的最终权限核验。

## 本轮修正

- 已在 C ABI/C++/易语言中新增完整 JSON 群消息发送入口 `qqb_send_group_message`；此前已有的 `qqb_send_group_text` 仅是文本快捷入口。
- 修正上一版文档中“群管理接口暂不开放”的笼统结论：该结论依据旧版 README 注释，现已发现一组较新的群管理 REST 路由；下一轮必须逐页核对后再进入正式 endpoint manifest 和类型化 SDK。
- 后续开发顺序：群管理 REST 路由原文核验 → 群管理 C ABI/C++/易语言 wrappers → GROUP_JOIN_REQUEST / GROUP_MEMBER_ADD / GROUP_MEMBER_REMOVE 等事件订阅与分发 → 群聊消息闭环。
- 不将群接口误记入 Guild/Channel 分类；每条接口必须独立记录群标识、权限、QPM、参数、错误码及白名单状态。

## 参考

- 腾讯官方群消息发送文档：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/send.md
- 腾讯官方群消息事件文档：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/event.md
- 腾讯官方群管理事件文档：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/group/manage/event.md
- 当前可见的官方群管理概述（其中仍保留旧版注释）：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/group/manage/README.md
- 交叉核对的维护中 Go SDK 接口目录：https://pkg.go.dev/github.com/by-yitong/botgo/openapi
- 交叉核对的群接口 DTO 与约束：https://github.com/by-yitong/botgo/blob/main/dto/group.go


## 第一批 SDK 封装交叉核验补充（2026-11-23）

已新增 7 个类型化入口（C ABI / C++14 / 易语言 DLL）：
- 群资料 GET /info：字段结构参考第三方 fork 的 GroupInfo（group_openid、group_name、group_finger_memo、group_class_text、group_tags、group_member_num 等）。
- 机器人群状态 GET /bot_state：字段结构参考 GroupBotState（member_openid、joined_at、allow_proactive_msg、recv_msg_setting、member_role）。
- 群禁言 GET /restrict_chat_setting；POST 同一路由 body 为完整 JSON object。第三方 DTO 显示成员禁言项使用 op=add/update/del、member_openid、mute_expire_at；单次最多 10 人、机器人需群管理员。
- 群成员 GET /members：cursor 分页，SDK 对 limit 限制 1–30；GET /members/{member_openid} 查询单成员。
- 批量移出 POST /batch_remove_members：body 由调用方按接口 JSON 结构提供；第三方 DTO 显示单次最多 20 人，可选同时加入黑名单。

**重要来源边界：**以上详细 DTO 字段、QPM、分页限制和白名单提示来自维护中第三方 fork（https://github.com/by-yitong/botgo），不是腾讯官方文档，只作为实现线索。群管理 README 官方源文件仍是旧版内容，自动生成接口页未逐条成功读取，因此这些接口继续保持“已发现、官方原文待复核”，不计入已核验 42 条。

## 官方群管理事件字段核验

腾讯官方 bot-docs 的 server-inter/group/manage/event.md 明确列出以下四种 WebSocket 事件，均使用 intents 位 1<<25，权限要求标注“暂无”：
- GROUP_ADD_ROBOT：机器人加入群聊；字段 timestamp、group_openid、op_member_openid。
- GROUP_DEL_ROBOT：机器人退出群聊；字段 timestamp、group_openid、op_member_openid。
- GROUP_MSG_REJECT：群管理员关闭机器人主动消息；字段 timestamp、group_openid、op_member_openid。
- GROUP_MSG_RECEIVE：群管理员开启机器人主动消息；字段 timestamp、group_openid、op_member_openid。

该官方事件页标注推送方式为 WebSocket。群消息事件 GROUP_AT_MESSAGE_CREATE 位于单独消息事件文档，不与上述四个群管理事件混为一组。



## 第二批：黑名单与入群申请 SDK（2026-11-23）

新增 C ABI / C++14 / 易语言 DLL typed wrapper：
- GET /v2/groups/{group_openid}/member_blacklist：`qqb_list_group_blacklist`，固定请求 limit=100，cursor 自动 URL 编码。
- POST 同一路由：`qqb_update_group_blacklist`，body 由调用方传入 JSON object（第三方 DTO 交叉核验结构为 op=add/del、member_openids；单次最多 20 人）。
- GET /v2/groups/{group_openid}/join_request_list：`qqb_list_group_join_requests`，固定请求 limit=50，cursor 自动 URL 编码。
- POST /v2/groups/{group_openid}/approval_join_request/{member_openid}：`qqb_approve_group_join_request`，body 包含 op=approve/decline，可选 join_request_id、reject_reason。

第三方接口实现注释指出黑名单操作需白名单；入群申请列表/审批需机器人具备群管理员身份。该信息只作为交叉核验线索，尚未逐页核验腾讯官方自动生成接口页面，因此不计入已核验 42 条 REST 路由。黑名单与申请列表 limit/cursor 参数亦须在官方页面复核后才视为正式协议约束。



## 第三批：自动入群审批策略 SDK（2026-11-23）

六个策略路由现已增加 C ABI / C++14 / 易语言 DLL wrapper：
- GET /v2/groups/join_approval_strategy（limit=50、cursor 分页）
- POST /v2/groups/join_approval_strategy（创建）
- PATCH /v2/groups/join_approval_strategy/{strategy_id}（修改）
- DELETE 同一策略资源（删除）
- POST /v2/groups/join_approval_strategy/{strategy_id}/execute（执行）
- POST /v2/groups/join_approval_strategy/{strategy_id}/whitelist_users（白名单增删）

策略为机器人维度资源，列表/创建不带 group_openid；其余操作按 strategy_id 路径寻址。策略字段和约束目前来自第三方 fork 的 DTO/注释，官方自动生成页面仍需逐项复核，因此这些路由暂不计入官方逐页核验数量。
