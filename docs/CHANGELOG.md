## 2026-11-23 — QQ 群聊 API 专项纠偏

- 修改 `sdk/include/qqbot_sdk.h`、`sdk/src/qqbot_sdk.c`：新增 `qqb_send_group_message`，支持完整 JSON body，不再局限纯文本；校验 group_openid 并调用官方 `POST /v2/groups/{group_openid}/messages`。
- 修改 `sdk/cpp/qqbot.hpp`：新增 `qqbot::Client::send_group_message`。
- 修改 `sdk/e-language/README.md`：补充群消息完整 JSON 发送函数、msg_type 和被动回复限制。
- 修改 `sdk/official-api-index.json`：补入官方群消息事件文档索引。
- 修改 `sdk/official-endpoints.json` / `sdk/official-endpoints.md`：标记群消息接口的完整类型化 wrapper，记录群聊开放能力边界。
- 修改 `docs/DEVELOPMENT.md`、`docs/TASKS.md`：明确群消息和频道 API 分离；群聊优先级上调，频道管理功能转入维护队列。
- 官方核验来源：腾讯 `bot-docs` main 分支的群消息发送/事件与群管理事件文档。另交叉发现 17 条较新的群管理 REST 路由，单独记录于 `sdk/group-api-discovery.md`，待官方自动生成 API 页面逐项复核；旧版群管理 README 注释不能作为接口不存在的最终结论。
- 验证：本次代码提交触发 CI，结果待确认；未执行易语言 IDE、Windows DLL 加载或 QQ 实机测试。
- 修正：群消息 C 文件首次构建因 `valid_path_id` 前置声明缺失而失败（run 45150884024）；新增静态函数原型后，修复提交 `44c2d5f89597e0c74cab75d8cddea2e2f5a26520` 对应 run 45150939145 已成功，Make、CMake、C++ SDK 示例通过。


## 2026-11-23 — QQ 群管理 SDK 第一批类型化入口

- 新增群资料、机器人群内状态、群禁言查询/设置、群成员分页/详情、批量移出群成员共 7 个 C ABI 入口；新增对应 C++14 Client 方法与易语言 DLL ABI 映射。
- 群接口独立使用 group_openid / member_openid，不与频道 API 的 guild_id / user_id 混用。
- 成员分页游标按 URL query component 编码；群成员分页 limit 限制为 1–30；所有群路径标识复用安全 ID 校验；body 参数要求 JSON object 起始符。
- 接口字段、分页限制、白名单及管理员要求来自维护中第三方 Go SDK fork 的交叉核验，不冒充腾讯官方原文。腾讯官方 bot-docs 群管理 README 仍保留旧版注释；17 条新发现路由尚未全部逐页核验官方自动生成 API 页面，因此本批 wrapper 不计入逐页核验 REST 数量。
- 验证：待 GitHub Actions 对 C11 Make/CMake 与 C++ 示例执行构建；Windows DLL 加载、易语言 IDE 和 QQ 实机权限测试未执行。



## 2026-11-23 — QQ 群管理 SDK 第一批 CI 结果

- GitHub Actions C11 build run 45152192633 已完成并成功：C++ SDK header example、Make build、CMake configure/build 全部通过。
- 本次构建覆盖群管理 SDK 第一批 7 个 C ABI 入口及对应 C++ wrapper；仍未执行 Windows DLL 加载、易语言 IDE 编译和 QQ 实机白名单/管理员权限联调。
- 官方群管理事件页确认 GROUP_ADD_ROBOT、GROUP_DEL_ROBOT、GROUP_MSG_REJECT、GROUP_MSG_RECEIVE 共用 intents 位 1<<25，推送方式为 WebSocket，事件字段含 timestamp/group_openid/op_member_openid。详见 sdk/group-api-discovery.md。



## 2026-11-23 — QQ 群管理 SDK 第二批

- 新增群黑名单查询/修改、群入群申请分页查询、入群申请审批共 4 个 C ABI 入口，并提供 C++14 与易语言 DLL 映射。
- 黑名单列表请求固定 limit=100；入群申请列表固定 limit=50；两者 cursor 均按 URL query component 编码。
- 黑名单修改 body 使用调用方提供的完整 JSON object；审批 body 使用调用方提供的完整 JSON object。SDK 不猜测/强制写入可选字段。
- 第三方维护 Go SDK fork 提示黑名单为白名单能力、单次最多 20 人；入群申请/审批需要机器人群管理员身份。官方自动生成页面尚未逐项核验，故不计入已核验 REST 路由统计。
- CI 待本次代码提交触发后确认；Windows DLL、易语言 IDE 和 QQ 实机权限测试仍未执行。
