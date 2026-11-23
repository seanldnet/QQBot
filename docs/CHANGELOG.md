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


## 2026-11-23 — Callback DNS resolver Windows 适配（待 CI）

- 修改 `src/qq/callback_security.c`：加入 Windows Winsock2/WS2TCPIP 分支，使用 `InetPtonA` / `InetNtopA` 处理地址格式，并在 DNS 解析前 `WSAStartup`、退出时 `WSACleanup`；POSIX 分支保持原有接口。
- 修改 `CMakeLists.txt`：Windows 下为宿主和 callback security 测试链接 `ws2_32`。
- 修改 `Makefile`：Windows_NT 下增加 `ws2_32` 链接参数。
- 移除 `AI_ADDRCONFIG`，避免本机网卡配置导致地址族过滤；仍对 DNS 返回的每个 A/AAAA 地址执行公网分类。
- 验证状态：提交后 CI 待运行；本轮没有 Windows 主机可用，MSVC/MinGW 实机编译与测试未执行。此改动不启用 callback 出站请求；DNS pinning、写入口校验和 worker 仍待完成。


## 2026-11-23 — 增加 Windows callback security CI

- 修改 `.github/workflows/build.yml`：新增 Windows-latest + MSYS2/MinGW64 job，安装 MinGW GCC、pkgconf、libcurl，独立编译并运行 callback security 测试程序，链接 Winsock2。
- 该 job 验证 Windows resolver 源码和测试的编译/执行，不代表整个 QQBot 宿主、MSVC 或易语言 DLL 已完成 Windows 支持。
- GitHub Actions run 45179209231 已触发，当前排队；结果待确认。


## 2026-11-23 — Windows Server 成品包构建流水线

- 新增 Windows Server 2012 API baseline 的 MinGW64 x64 构建 job，配置 `_WIN32_WINNT=0x0602` / `WINVER=0x0602`，编译宿主与 SDK 并执行 Windows CTest。
- 新增 `scripts/package-windows.sh`，递归扫描 PE 导入 DLL，自动复制 MinGW runtime 依赖；缺失非系统 DLL 会立即失败，不生成不完整包。
- 构建流程组装 `QQBot-Windows-x64-Server2012plus.zip`，包含宿主、SDK DLL、依赖 DLL、web UI、data 目录、启动脚本和构建时随机生成的管理员 API Token。
- 此包当前为 CI 验证产物目标，不等同于正式 Release。Windows runner 和各 Server 版本实机验证未完成前，不宣称 2012–2026 全版本兼容。


## 2026-11-23 — Windows 完整宿主移植与成品 ZIP CI（待验证）

- `src/main.c`：增加 Windows Winsock、目录/文件访问、时间转换和线程等待的条件编译与初始化清理；POSIX 路径保持不变。
- `CMakeLists.txt`：Windows 下不定义 POSIX feature macro，避免污染 Winsock/CRT 声明。
- `.github/workflows/build.yml`：增加 Windows x64 完整依赖安装、宿主和 SDK 构建、CTest、运行目录安装、依赖 DLL 打包与 ZIP artifact 上传。
- 该工作流目前是首轮完整 Windows 构建探测；DLL 依赖收敛、CA 证书部署、Server 2012—2026 逐版本实机验收尚未完成，不能据此宣称全版本兼容。

- Windows 成品包增加 CA 根证书 bundle 自动发现与打包；启动 BAT 配置 `CURL_CA_BUNDLE`，降低目标系统缺少构建机 CA 路径导致 TLS 校验失败的风险。缺少 CA bundle 时构建直接失败。

- Windows 成品构建修复 MSYS2 MINGW64 包数据库未索引 libmicrohttpd 的问题：改为下载固定版本归档、校验 SHA-256 后由 pacman 安装，避免未校验的二进制依赖。


## 2026-11-23 — Windows Server 2012 构建依赖修正

- CI 实测发现 MSYS2 当前 MINGW64 索引不再提供 `mingw-w64-x86_64-libmicrohttpd`（pacman target not found）；已调整 Windows 构建流程，安装当前 MINGW64 GnuTLS 后固定下载并安装 1.0.10-1 历史 MINGW64 libmicrohttpd 包，避免转用要求较新 UCRT 的工具链。
- 修正后的完整 Windows 构建与 ZIP 生成仍待 CI 验证，不能视为已通过。


## 2026-11-23 — 修复 Windows ZIP 打包阶段 CA bundle 缺失

- 文件：`.github/workflows/build.yml`。原因：Windows full-build 已编译成功且 CTest 3/3 通过，但 MSYS2 环境不存在 `/etc/ssl/certs/ca-bundle.crt`，导致运行目录组装失败。
- 行为变化：改为从 `https://curl.se/ca/cacert.pem` 下载 CA bundle 至 `stage/certs/ca-bundle.crt`，并用 `test -s` 阻止空文件进入成品包；启动脚本继续引用包内路径。
- 验证：此前 run 45181038608 的 Windows 宿主/SDK 编译、3 项 CTest、固定版 libmicrohttpd 安装均成功；本次 CA 路径修复尚待新 CI 运行。DLL 依赖收集、ZIP 和 Server 全版本实机验证未完成。


## 2026-11-23 — 修复 Windows DLL 依赖扫描误报 wldap32.dll

- 文件：`scripts/package-windows.sh`。原因：Windows full-build 编译和 CTest 3/3 均成功，安装目录及 CA bundle 已就绪，但 PE 导入扫描将系统 DLL `wldap32.dll` 当作未解析的第三方运行库并中止。
- 行为变化：将 `wldap32.dll` 加入 Windows 系统 DLL 白名单；仍对其他无法解析的非系统 DLL 立即失败，避免静默漏打包。
- 验证：修复后 CI 尚待运行；ZIP 和 Artifact 尚未确认生成。


## 2026-11-23 — 修复 Windows DLL 扫描器漏识别 ncrypt.dll

- 文件：`scripts/package-windows.sh`。原因：Windows full-build 编译、CTest 3/3、运行目录安装和 CA bundle 下载均通过，但递归 PE 导入扫描发现 `libgnutls-30.dll` 依赖 `ncrypt.dll`，扫描器未将其识别为 Windows 系统组件。
- 行为变化：将 `ncrypt.dll` 加入系统 DLL 白名单；该 DLL 由 Windows 提供，不复制进包；未知非系统 DLL 仍触发失败，保持完整性检查。
- 验证：修复提交 `1fb5e24` 已提交；新 CI 尚未完成，ZIP/Artifact 尚未确认生成。Server 2012–2026 实机矩阵未执行。


## 2026-11-23 — Windows PE 子系统版本基线审计

- 新增 `scripts/check-windows-pe-baseline.sh`：扫描指定运行目录中的 EXE/DLL，检查 PE 格式为 x64，并验证 SubsystemVersion 不高于 6.2；缺少元数据、非 x64 或超出基线均失败关闭。
- 修改 `.github/workflows/build.yml`：在创建 Windows 一键运行 ZIP 前执行 PE 基线审计，阻止超出目标子系统标记的文件进入发行包。
- 对当前已生成 ZIP 的离线检查结果：宿主和大部分依赖标记 5.2，libmicrohttpd 标记 6.0；未发现高于 6.2 的 PE 子系统标记。
- 验证边界：本次离线检查不是 Server 2012 实机测试，也不能证明依赖 DLL 的所有导入 API 均受 Server 2012 支持；新的 CI 将负责验证脚本与流水线接线。


## 2026-11-23 — 修复 PE 审计脚本误判 x64 格式

- 文件：`scripts/check-windows-pe-baseline.sh`。
- 原因：`objdump -f` 输出格式行带有缩进，原 awk 分隔规则取出的字符串包含前导空格，导致合法的 `pei-x86-64` 被误判。
- 修复：改为读取匹配行的最后一个字段，确保格式标识精确匹配。
- 验证：失败 run `45183421951` 中 Windows 宿主/SDK 构建、CTest 3/3 和 DLL 依赖收集成功；PE 审计因上述脚本误判失败。修复提交 `ad19eb7` 的完整 CI 待确认。


## 2026-11-23 — 机器人登录接入方式与消息日志首版

- 新增 `src/qq/gateway.c/.h`：通过 libcurl WebSocket API 请求官方 Gateway URL 并建立 WSS 连接，首版处理 HELLO、IDENTIFY、心跳、READY 状态及断线退避重连。
- `POST /api/v1/auth/login` 增加 WebSocket/Webhook 模式选择；支持 access_token 的 expires_in 为数字或字符串的响应格式。
- `GET /api/v1/status` 返回认证状态、接入方式、传输状态和连接状态；WSS 握手不等于 READY，Webhook challenge 成功也不等于收到事件。
- 事件解析器增加 `C2C_MESSAGE_CREATE`；群 @ 消息与 C2C 单聊消息进入 SQLite event inbox，并生成包含会话类型、会话 ID、发送者 ID、文本和时间的运行日志。
- 管理页面增加 WSS/Webhook 选择、状态刷新和日志近实时刷新。
- 增加 C2C 消息解析测试；Make/CMake 纳入 Gateway 模块。
- 当前 CI run `45187828807` 执行中，未以此前失败的中间 run 作为通过依据。
- 限制：WSS 尚未实现 RESUME/session 持久化、完整关闭码策略和心跳 ACK 超时；尚无 QQ 实机联调。Webhook 需公网 HTTPS 和开放平台回调配置。消息日志首批仅覆盖群 @ 与 C2C 文本，不含频道/附件完整信息。


## 2026-11-23 — WSS 运行时能力与就绪状态校正

- 将 libcurl WebSocket 最低版本明确为 7.86.0，Make/CMake 增加版本检查；运行时通过 libcurl version_num 判断 WSS 支持。
- WSS 仅在收到 Gateway READY 后标记 connected，避免将 TLS/WebSocket 握手误报为机器人已鉴权上线。
- 修正后的完整 CI run 45188659260 正在执行；此前 run 45188150559 的 Linux/Windows 构建、测试和 ZIP 打包已通过。


## 2026-11-23 — 修复 WSS libcurl 版本检查脚本

- Makefile 的 shell 循环变量改回 `$$p`，避免被 Make 提前展开；保留 libcurl 7.86.0 最低版本检查。
- 修复提交对应 CI run 45188880579 执行中。


## 2026-11-23 — 完成 Makefile 依赖循环变量转义修复

- 将依赖失败提示中的 shell 变量从 `$p` 改为 `$$p`，修复 Make 提前展开导致的空依赖名。
- 修复提交 81a6624 的 CI 待验证；此前 Windows full-build、CTest、DLL 扫描、PE 审计和 ZIP 上传已成功。


## 2026-11-23 — 登录与消息日志测试包构建验证通过

- GitHub Actions run `45189464000` 全部成功：Linux Make/CMake/单元测试、Windows callback-security、Windows 完整宿主/SDK 构建、CTest、DLL 依赖扫描、PE 子系统审计和 ZIP 上传。
- 生成 Windows x64 测试 ZIP，内层压缩包 47 个文件，CRC 校验通过；包含 WSS/Webhook 选择、连接状态、群 @ 与 C2C 日志界面。
- 真实 QQ 官方机器人凭据登录与消息实测尚未执行，当前交付定位为用户联调测试包。


## 2026-11-23 — WSS 心跳与管理日志安全加固

- `src/qq/gateway.c`：心跳调度改为单调毫秒时钟；发送 HEARTBEAT 后等待 HEARTBEAT_ACK，超过一个心跳周期未确认即主动重连。
- `src/main.c`：`GET /api/v1/status` 与 `GET /api/v1/logs` 增加管理员 Bearer 令牌校验，避免匿名读取机器人状态和消息日志。
- `web/index.html`：增加管理员令牌输入，状态与日志轮询携带 Bearer 头；令牌不写入 localStorage。
- 更新 `docs/API.md` 和 `docs/TASKS.md` 记录接口要求与待验收项。
- 验证边界：代码提交后由 CI 自动编译/测试；当前尚未取得本次改动的 CI 结果，也未进行真实 QQ WSS/Webhook 实机联调。

## 2026-11-23 — AntdUI Windows 桌面版首次构建通过

- 修复 Desktop/MainForm.cs 中 AntdUI Message.error 缺少 Target 参数的问题，改为传入主窗口实例与提示文本。
- GitHub Actions run 45200701206 全部成功：Linux 构建、Windows callback-security、Windows full-build；AntdUI .NET 8 WinForms 单文件自包含发布成功，C 后端 CTest 3/3 通过。
- 完成 runtime DLL 依赖闭包扫描、PE x64 / Windows API 子系统基线审计及 ZIP Artifact 上传。
- Artifact QQBot-AntdUI-Windows-x64 下载后，内层 ZIP CRC 检查通过；根目录包含唯一桌面入口 QQBot.exe，协议核心和依赖位于 runtime/，另有 data/ 与 certs/。
- 验证边界：CI 构建成功不代表已在用户 Windows 桌面验证 UI 外观、中文字体/DPI 和交互；QQ 官方 WSS/Webhook 实机联调仍待用户环境验收。


## 2026-11-23 — Windows 桌面 UI 切换 AntdUI

- 新增 Desktop/QQBot.Desktop.csproj、Desktop/Program.cs、Desktop/MainForm.cs：桌面客户端使用 C# WinForms + 官方 AntdUI 2.4.20，登录与主界面业务控件统一使用 AntdUI 组件。
- 移除最终用户 GUI 对手写 Win32 STATIC/EDIT/BUTTON/COMBOBOX 控件的依赖；中文界面使用 .NET Unicode 字符串与 AntdUI 文本渲染配置。
- Windows 发布流程新增 .NET 8 自包含单文件发布，根目录 QQBot.exe 为唯一桌面入口；C 协议核心、SDK DLL 与第三方依赖放在 runtime/，数据库和 CA 证书分别位于 data/、certs/。
- 新增 docs/DESKTOP-UI.md，记录 AntdUI-only 业务 UI 约束、项目结构、发布布局、构建和验收要求；更新 README 文档入口与技术路线。
- 首轮 CI 中 C 后端编译成功，Windows CTest 3/3 通过，AntdUI NuGet 还原成功；C# 编译发现 AntdUI.Message.error 调用缺少必需 Target 参数，已修复为 Message.error(this, text)。
- 验证边界：修复提交后的 C# 编译/发布、DLL 依赖扫描、ZIP 生成尚未确认；未在真实 Windows 桌面完成截图验收，也未进行 QQ 官方机器人实机联调。不得把旧 Win32 GUI 包当作 AntdUI 版本交付。


## 2026-11-23 — Windows 原生桌面 EXE 第一阶段

- 新增 `src/windows_gui.c`：基于 Win32 API 构建独立桌面窗口，包含 AppID/AppSecret 输入、WSS/Webhook 选择和登录按钮；登录后轮询本机状态与日志 API，并将日志格式化显示在桌面窗口中。
- 桌面 GUI 启动同目录 `qqbot-core.exe` 子进程，使用系统 CNG 随机数生成进程内管理员令牌，通过继承环境变量传递给核心；核心绑定 127.0.0.1，GUI 通过 WinHTTP 访问本机 API。
- CMake Windows 构建新增 GUI 子系统目标 `QQBot.exe`，核心目标输出名改为 `qqbot-core.exe`；GUI 链接 WinHTTP、BCrypt、Common Controls 和 cJSON。
- Windows 打包流程改为依赖扫描 GUI 与核心两个 EXE；ZIP 内提供双击 GUI 启动脚本，不再要求用户打开浏览器或输入管理员令牌。
- GUI 通过核心进程环境变量配置包内 CA bundle 路径。原有 Web UI 保留为开发/诊断资源，不再作为最终用户入口。
- 验证边界：本次提交后需等待 Windows CI 首次编译 GUI、扫描 DLL 并生成 ZIP；当前尚未声称 GUI 已编译通过，也未做 Windows 桌面实机或 QQ 官方平台联调。当前 GUI 的账号/应用/设置导航和视觉完善仍待后续迭代。
