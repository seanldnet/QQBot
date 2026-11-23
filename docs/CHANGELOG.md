## 2026-11-23 — QQ 群聊 API 专项纠偏

- 修改 `sdk/include/qqbot_sdk.h`、`sdk/src/qqbot_sdk.c`：新增 `qqb_send_group_message`，支持完整 JSON body，不再局限纯文本；校验 group_openid 并调用官方 `POST /v2/groups/{group_openid}/messages`。
- 修改 `sdk/cpp/qqbot.hpp`：新增 `qqbot::Client::send_group_message`。
- 修改 `sdk/e-language/README.md`：补充群消息完整 JSON 发送函数、msg_type 和被动回复限制。
- 修改 `sdk/official-api-index.json`：补入官方群消息事件文档索引。
- 修改 `sdk/official-endpoints.json` / `sdk/official-endpoints.md`：标记群消息接口的完整类型化 wrapper，记录群聊开放能力边界。
- 修改 `docs/DEVELOPMENT.md`、`docs/TASKS.md`：明确群消息和频道 API 分离；群聊优先级上调，频道管理功能转入维护队列。
- 官方核验来源：腾讯 `bot-docs` main 分支的 `server-inter/message/send-receive/send.md`、`server-inter/message/send-receive/event.md`、`server-inter/group/manage/event.md`、`server-inter/group/manage/README.md`。
- 验证：本次代码提交触发 CI，结果待确认；未执行易语言 IDE、Windows DLL 加载或 QQ 实机测试。
