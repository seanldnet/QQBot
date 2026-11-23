# QQ 官方协议核验记录

核验日期：2026-11-23

## 已确认事项

1. QQ 官方文档仓库为 https://github.com/tencent-connect/bot-docs ，官方文档站为 https://bot.q.qq.com/wiki/ 。官方文档介绍 QQ 机器人覆盖频道、群、单聊场景，并提供 API 与 WebSocket 事件能力。
2. 当前 API v2 文档检索结果明确给出统一 API 根地址 `https://api.bot.qq.com`。因此需求中的该域名按当前 API v2 文档基线保留，不沿用旧版 `api.sgroup.qq.com` 作为默认值。
3. 新版 Access Token 请求形式为 `POST /app/getAppAccessToken`，JSON 字段为 `appId` 与 `clientSecret`；请求 API 时使用 `Authorization: QQBot {access_token}`。
4. 历史文档首页仍出现 `https://bots.qq.com/app/getAppAccessToken` 与 `api.sgroup.qq.com`，与新版 API v2 页面存在域名迁移并存信息。实现应将 token endpoint 和 API base 做配置项，不将其硬编码成不可替换常量；默认采用当前 API v2 页面给出的 `api.bot.qq.com`，并在联调时验证实际环境。
5. 消息接口示例使用 `/v2/users/{openid}/messages`；官方消息文档提示主动推送策略已于 2025-04-21 调整，主动推送接口会返回错误，发送功能必须围绕有效事件的被动回复窗口实现，并向面板明确显示平台频控/窗口错误。
6. WebSocket Gateway 采用 QQ Bot Gateway 信封与 opcode 协议；不得把其他平台的心跳间隔或鉴权参数直接写死。应先取得 Gateway URL，处理 HELLO、IDENTIFY、DISPATCH、HEARTBEAT ACK、RESUME / RECONNECT 等流程。
7. Webhook 模式存在 `op=13` 回调验证流程，并对事件推送使用 Ed25519 签名头 `X-Signature-Ed25519` 与 `X-Signature-Timestamp`。官方 `sign.md` 明确：将 Bot Secret 字符串重复拼接至至少 32 字节后截取前 32 字节作为 Ed25519 seed；op=13 对 `event_ts + plain_token` 签名并返回 `{plain_token, signature}`；普通事件验签消息为 `timestamp + 原始 HTTP body`，签名头为 hex 编码，验签通过后回 `op=12` ACK。
8. 官方事件文档当前仍列出 WebSocket opcode 0/1/2/6/7/9/10/11，并描述 HELLO、IDENTIFY、心跳 ACK、RESUME；但同一官方文档明确写明 WebSocket 事件推送链路计划于 2024 年底前逐步下线、后续不再维护。因此项目保留用户要求的 WebSocket 兼容模式，但 Webhook 应作为首选生产链路，并需实机确认当前账号是否仍允许 WebSocket。

## 参考资料

- 官方文档仓库：https://github.com/tencent-connect/bot-docs
- 官方文档站：https://bot.q.qq.com/wiki/develop/api-v2/
- API v2 消息发送文档：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/send.md
- 事件订阅与通知（含 opcode、Webhook、WebSocket）：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/dev-prepare/interface-framework/event-emit.md
- Ed25519 签名算法与示例：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/dev-prepare/interface-framework/sign.md
- 当前 API 根域名及令牌接口检索记录：官方 API v2 文档页面与文档站搜索结果（2026-11-23）。

## 尚需在编码前/联调时核验

- token endpoint 当前部署是否接受 `api.bot.qq.com`，以及旧 `bots.qq.com` 是否仍兼容。
- Webhook challenge 的 AppSecret 派生 seed、timestamp/nonce 拼接顺序、签名输出编码和 op=12 ACK 的精确响应体。
- Gateway `/gateway` 或 `/gateway/bot` 路径、intents 位图、分片与 resume 参数、heartbeat_interval 单位和 ACK 处理。
- 不同场景消息 endpoint 与 msg_id / event_id 的互斥规则、消息回复窗口和发送频率。

> 说明：本文件用于明确当前已确认信息与未确认边界。只有完成真实联调或依据官方规范逐项核对后，相关任务才可标记为已验证完成。
