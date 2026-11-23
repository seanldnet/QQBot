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



## 2026-11-23 — QQ 群自动审批策略 SDK 第三批

- 新增自动审批策略列表、创建、修改、删除、执行、白名单维护 6 个 C ABI typed wrapper，配套 C++14 Client 与易语言 DLL 映射。
- 策略是机器人维度资源：列表/创建路径不包含 group_openid；按 strategy_id 修改/删除/执行/管理白名单。
- 策略列表分页 cursor 做 URL 编码，limit 固定 50；执行接口由 SDK 发送空 JSON object。
- 具体请求字段、策略数量上限、群绑定限制、白名单号码限制和执行时效来自第三方维护 fork 的交叉核验，官方自动生成页面尚待逐项复核，不纳入 verified endpoint 统计。
- CI 将由本批代码提交触发；Windows DLL、易语言 IDE 和 QQ 实机权限测试未执行。


## 2026-11-23 — QQ 群事件模型与归一化解析器

- 新增 `src/qq/events.h`、`src/qq/events.c`：内部函数 `qq_event_normalize` 将 Gateway 事件类型与内层 JSON payload 转为统一插件事件 JSON；支持四种群管理事件和 `GROUP_AT_MESSAGE_CREATE`，拒绝未知事件/缺少必要字段，完整保留 `raw_event`。
- 新增 `tests/test_event_parser.c`：覆盖群机器人加入事件、群 @ 消息、msg_id/timestamp 映射、raw_event 保留、未知事件拒绝和必需字段校验。
- 归一化 timestamp：群管理事件整数秒转为十进制字符串，群 @ 消息 RFC3339 字符串原样保留；C++ Event 增加 timestamp 与 has_msg_seq，避免把不存在的 msg_seq 默认伪造为 1。
- 修改 `CMakeLists.txt`、`Makefile`：宿主编译纳入 events.c；增加 Make `test` 与 CTest `qq_group_event_parser`。
- 新增 `sdk/group-events.md`；更新 `sdk/event-schema.md`、`sdk/e-language/event-fields.md`，同步 C/C++/易语言插件事件字段契约。
- 官方核验：腾讯 bot-docs 群管理事件页确认四种事件均为 Intent `1<<25`；消息事件页确认 `GROUP_AT_MESSAGE_CREATE` 同为 `1<<25`，群作者字段为 `author.member_openid`，消息 ID 为 `id`，timestamp 为 RFC3339。全量群消息事件仍标注私域/暂不开放。
- CI 工作流新增 `make test` 与 `ctest --output-on-failure` 执行步骤；此前已通过的编译 run 不包含本次新增测试执行。\n- WebSocket transport、Gateway 心跳/重连、插件分发/回调、msg_id 持久化去重和 QQ 实机事件接收仍未实现。


## 2026-11-23 — 群事件解析器 CI 修正记录

- 中间提交的 C++ SDK 示例编译失败，原因是示例源码误写入字面量 `\\n`，导致换行符未生效；已修正为真实换行并重新触发 CI。
- 失败发生在 C++ 示例编译步骤，Make/CMake/解析器测试因此被跳过；此前独立提交 run 45160609516 已通过 Make unit tests 与 CMake/CTest 步骤，但不包含后续 timestamp 增量。
- 最终修复提交对应 run 45160886586 已成功：C++ SDK 示例、Make 构建、Make 单元测试、CMake 构建及 CTest 所在步骤均通过。该 run 覆盖 timestamp 解析代码与修复后的 C++ 示例。


## 2026-11-23 — 验签 Webhook 事件持久化

- 新增 `src/qq/event_queue.h/.c`：SQLite `event_inbox` 表、唯一 dedupe_key、pending 状态和入箱函数。
- 新增 `tests/test_event_queue.c`：验证新事件插入、重复事件忽略、非法键拒绝和 pending 记录数；纳入 Make `make test` 与 CMake/CTest。
- 扩展 `qq_webhook_process_ex`，op=0 签名验证通过后提取 `t` 和 `d`；保留旧 `qq_webhook_process` 兼容包装。
- Webhook handler 只对已验签且能被群事件解析器识别的事件入箱；其他事件仍按原 ACK 逻辑处理。
- 新增管理员 `GET /api/v1/events` 查询最近100条持久化事件。
- 事件去重键：群消息为 event_type/group_openid/author_openid/msg_id；群管理事件为 event_type/group_openid/op_member_openid/timestamp。
- 安全边界：事件仅持久化，不会向 callback_url 发起出站请求；插件投递 worker、逐插件 delivery 状态、重试与 SSRF 防护尚未实现。


## 2026-11-23 — 逐插件事件投递任务扇出

- 修改 `src/qq/event_queue.h/.c`：新增 `qq_event_queue_fanout`，为已入箱事件创建每个启用插件独立的 pending delivery row；过滤空 callback_url 和停用插件。
- 新增 `event_deliveries` SQLite 表，快照 callback_url，保存 attempts、last_error、next_attempt_at；唯一约束 event_id/plugin_id，重复扇出幂等。
- 修改 Webhook handler：入箱后执行 fanout；对于重复事件也补做 fanout，保证进程在 inbox commit 后、fanout 前退出时可通过重复推送修复遗漏。
- 新增管理员 GET `/api/v1/event-deliveries`，查看最近 200 条任务状态，不返回插件 Token。
- 扩展 `tests/test_event_queue.c`：覆盖两条启用插件任务、停用插件与空 callback 排除、重复 fanout 不产生重复行。
- 更新 `docs/API.md`、`docs/DATABASE.md`、`docs/DEVELOPMENT.md`、`docs/TASKS.md`。
- 安全边界：当前只建立持久化 pending delivery 记录，尚未向插件发 HTTP 请求；真实 worker 必须等 DNS/IP 拒绝、DNS pinning、HTTPS-only、禁止重定向和响应/超时限制完成后才能启用。


## 2026-11-23 — 逐插件投递队列 CI 结果

- 中间提交曾因事件投递 API 错误 JSON 字符串转义和 event_queue.c 缺少标准 NULL 定义导致 Make 编译失败；已分别修正。失败的中间 run 未作为通过依据。
- 最终修复提交 `3e4320fb6ad939e5cd6e9ef5d529d77185fa0974` 对应 GitHub Actions run `45176688074` 已成功：C++ SDK 示例、Make 构建、Make 单元测试、CMake 构建和 CTest 均通过。
- 当前验证仅覆盖事件持久化与逐插件 pending 任务扇出；尚未启用 callback HTTP worker，也未做 DNS/IP SSRF 防护和真实 QQ Webhook 回调实测。



## 2026-11-23 — 插件 callback SSRF 防护第一阶段

- 新增 `src/qq/callback_security.h/.c`：严格解析 HTTPS callback URL，限定 TCP 443，拒绝 URL userinfo 和 IP literal；通过 `getaddrinfo` 检查全部 DNS A/AAAA 答案，任一答案落入非公网地址范围即拒绝。
- IPv4 地址分类拒绝私网、环回、链路本地、共享地址、文档、组播和保留网段；IPv6 仅接受全球单播 2000::/3，并拒绝特殊用途、6to4、文档地址。
- 新增 `tests/test_callback_security.c`，覆盖公网/私网/环回/链路本地/文档/IPv4-mapped IPv6 和 URL 协议、userinfo、IP literal、端口策略；接入 Make 与 CMake/CTest。
- 新增 `qq_callback_target_resolve_entry`，将已校验的 DNS A/AAAA 地址格式化为 libcurl `CURLOPT_RESOLVE` 所需的 pinning entry，并增加格式化测试；尚未接入实际 HTTP 请求。
- 当前仅为安全校验原语，未接入 callback_url 写入口，未实现 DNS pinning 和 HTTP worker；不允许出站投递。当前解析依赖 POSIX 网络 API，Windows 适配待做。
- CI 将由本批代码提交触发；本地构建未执行。
