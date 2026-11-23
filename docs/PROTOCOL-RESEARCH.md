# QQ 官方协议核验记录

核验日期：2026-11-23

## 已确认事项

1. QQ 官方文档仓库为 https://github.com/tencent-connect/bot-docs ，官方文档站为 https://bot.q.qq.com/wiki/ 。官方文档介绍 QQ 机器人覆盖频道、群、单聊场景，并提供 API 与 WebSocket 事件能力。
2. 当前 API v2 文档检索结果明确给出统一 API 根地址 `https://api.bot.qq.com`。因此需求中的该域名按当前 API v2 文档基线保留，不沿用旧版 `api.sgroup.qq.com` 作为默认值。
3. 新版 Access Token 请求形式为 `POST /app/getAppAccessToken`，JSON 字段为 `appId` 与 `clientSecret`；请求 API 时使用 `Authorization: QQBot {access_token}`。
4. 历史文档首页仍出现 `https://bots.qq.com/app/getAppAccessToken` 与 `api.sgroup.qq.com`，与新版 API v2 页面存在域名迁移并存信息。实现应将 token endpoint 和 API base 做配置项，不将其硬编码成不可替换常量；默认采用当前 API v2 页面给出的 `api.bot.qq.com`，并在联调时验证实际环境。
5. 消息接口示例使用 `/v2/users/{openid}/messages`；官方消息文档提示主动推送策略已于 2025-04-21 调整，主动推送接口会返回错误，发送功能必须围绕有效事件的被动回复窗口实现，并向面板明确显示平台频控/窗口错误。
6. WebSocket Gateway 采用 QQ Bot Gateway 信封与 opcode 协议；不得把其他平台的心跳间隔或鉴权参数直接写死。应先取得 Gateway URL，处理 HELLO、IDENTIFY、DISPATCH、HEARTBEAT ACK、RESUME / RECONNECT 等流程。
7. Webhook 模式存在 `op=13` 回调验证流程，并对事件推送使用 Ed25519 签名头 `X-Signature-Ed25519` 与 `X-Signature-Timestamp`。挑战签名、签名消息拼接、seed 派生必须按官方 webhook 文档/官方示例逐字节实现；不得仅因需求提到 Ed25519 就自行猜测拼接规则。

## 参考资料

- 官方文档仓库：https://github.com/tencent-connect/bot-docs
- 官方文档站：https://bot.q.qq.com/wiki/develop/api-v2/
- API v2 消息发送文档：https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/server-inter/message/send-receive/send.md
- 当前 API 根域名及令牌接口检索记录：官方 API v2 文档页面与文档站搜索结果（2026-11-23）。

## 尚需在编码前/联调时核验

- token endpoint 当前部署是否接受 `api.bot.qq.com`，以及旧 `bots.qq.com` 是否仍兼容。
- Webhook challenge 的 AppSecret 派生 seed、timestamp/nonce 拼接顺序、签名输出编码和 op=12 ACK 的精确响应体。
- Gateway `/gateway` 或 `/gateway/bot` 路径、intents 位图、分片与 resume 参数、heartbeat_interval 单位和 ACK 处理。
- 不同场景消息 endpoint 与 msg_id / event_id 的互斥规则、消息回复窗口和发送频率。

> 说明：本文件用于明确当前已确认信息与未确认边界。只有完成真实联调或依据官方规范逐项核对后，相关任务才可标记为已验证完成。
