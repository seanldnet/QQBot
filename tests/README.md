# 测试说明

当前自动化测试尚未建立完整套件。Webhook 签名模块应至少覆盖：

1. 官方文档 `event-emit.md` 的 op=13 challenge 示例（固定 AppSecret、event_ts、plain_token），校验返回 signature 与官方示例完全一致。
2. 官方 `sign.md` 的 Ed25519 验签示例（固定 secret、timestamp、原始 body、signature），有效签名通过。
3. 修改 body、timestamp、签名任意一项均拒绝。
4. 缺失签名头、非法 hex、错误 opcode / JSON、空 secret 均安全拒绝。

官方协议参考：
- https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/dev-prepare/interface-framework/event-emit.md
- https://github.com/tencent-connect/bot-docs/blob/main/docs/develop/api-v2/dev-prepare/interface-framework/sign.md
