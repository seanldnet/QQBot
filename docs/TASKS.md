# 项目任务进度

状态定义：`未开始`、`进行中`、`阻塞`、`已完成（已验证）`。只有具备可复现验证结果的任务才可标记为已完成。

## 0. 仓库与文档基线

- [x] 确认仓库地址与可写权限
- [x] 创建 README 项目入口
- [x] 建立完整开发文档与原始需求存档
- [x] 建立变更日志、任务表、API 与数据库文档
- [x] 建立协议核验记录与 API 域名配置基线

## 1. 协议与架构

- [ ] 完整核验 QQ 官方 API v2 所有 REST endpoint、参数、权限和错误码（目前已逐页核验 42 条 REST 路由（增量清单，非全量）；继续抽取剩余页面）
- [ ] 核验 WebSocket Gateway、鉴权、心跳、重连和事件序号协议
- [ ] 核验 Webhook URL 验证、签名算法、请求体字段与 ACK 语义
- [ ] 确定 HTTP 服务端库与模块接口

## 2. 宿主基础

- [x] C11 工程骨架、Makefile、CMake、依赖检测（尚未编译验证）
- [ ] 配置、结构化日志、SQLite 初始化与迁移（已完成基础 SQLite 初始化、日志表和最近日志 API；完整迁移器待完善）
- [ ] HTTP 服务、静态资源托管、管理认证（已完成基础 HTTP、首页托管和本机默认绑定；正式管理会话/CSRF 待完善）

## 3. QQ 机器人

- [ ] AppID / AppSecret 凭据管理与 access_token 获取/刷新（已完成登录取 token 和进程内缓存；后台自动刷新/安全会话待完善）
- [ ] 群聊、单聊、频道消息收发
- [x] Webhook op=13 challenge 与 op=0 Ed25519 验签接入（尚待官方向量测试和实机回调）
- [ ] WebSocket WSS、心跳与重连（官方文档注明该链路计划逐步下线，仍保留兼容需求）

## 4. 插件与多语言 SDK

- [x] 建立 C ABI SDK 核心接口与 libcurl 客户端首版（未编译验证）
- [x] 建立 C++ SDK 封装与示例（未编译验证）
- [x] 建立易语言 DLL ABI 映射与调用示例文档（未在易语言 IDE 验证）
- [x] 建立官方 API v2 文档 81 页目录索引与 JSON 元数据
- [x] 建立增量 REST endpoint 清单（42 条已逐页核验，非全量）
- [x] 建立 C++ / 易语言统一插件事件 JSON 契约和字段映射
- [ ] 宿主插件注册/Token 鉴权与 `/api/v1/plugins/{id}/qq-api` 代理实现（首版路由已写入，仍待编译、权限和端到端测试）
- [ ] 官方所有 REST endpoint 逐项提取方法、路径、参数、权限和错误码，生成类型化 C/C++/易语言接口（当前 42 条已核验；33 条路由已提供三语言类型化 wrapper，4 条消息路由已有文本便利函数）
- [ ] Windows x86/x64 DLL 构建、易语言 IDE 导入和运行验证

## 5. 插件与前端

- [ ] 插件注册、启停、删除、凭据管理（首版 API 已写入，管理令牌由 QQBOT_ADMIN_TOKEN 提供；UI 和安全测试待完成）
- [ ] 事件并发分发、失败隔离、回调安全策略
- [ ] 插件发消息 API
- [ ] Argon Dashboard 登录、日志、插件、设置页面（当前仅有登录页和日志轮询基础界面）
- [ ] 日志实时推送

## 6. 验证与交付

- [ ] 单元测试与协议测试（已添加官方测试向量计划，尚未执行）
- [ ] 本地编译验证（已加入 GitHub Actions Make/CMake 构建工作流，结果尚未取得）
- [ ] 面板登录至机器人上线闭环
- [ ] 收发消息、日志和插件收发事件闭环
- [ ] 部署说明与安全检查

## 当前状态（2026-11-23）

已建立文档基线与 C11 宿主首阶段代码：HTTP 首页、登录取 token、SQLite 日志与查询、Webhook op=13 challenge 签名、op=0 Ed25519 验签及 op=12 ACK 已写入仓库。仍未实现消息自动回复、插件事件分发/回调投递、WebSocket、完整管理会话认证与设置页面。已新增 C ABI SDK、C++ 封装、易语言 DLL 映射、官方 API 文档全量目录索引（81 页）和 10 条逐页核验的 REST endpoint 清单；6 条频道管理路由已增加 C ABI/C++/易语言类型化封装；宿主插件注册/启停/删除及 QQ API 代理首版路由已写入；仍未编译验证，插件事件分发和端到端闭环未完成。均未取得编译或 Windows 易语言实机验证。未执行编译、运行、官方向量测试或 QQ 实机联调；相关任务不得标记为已验证完成。


## 官方 API / SDK 覆盖进展（2026-11-23）

- [x] 第二批 16 条成员、身份组、权限、禁言 REST 路由已核对官方原文并加入 endpoint manifest。
- [x] 易语言接口文档列出第二批全部路由的 method/path/参数/权限，统一通过通用 ABI 调用。
- [x] C++ `Client::request` 已具备对上述路由的通用访问能力（无新增独立 typed wrapper）。
- [ ] 为第二批高频管理路由增加 C ABI/C++/易语言逐项 typed wrapper。
- [ ] 补齐公告、置顶、日程、论坛、音频、API 权限申请、用户资料和群聊管理等接口。
- [ ] 对照官方 OpenAPI 错误码总表，建立错误码到接口的精确映射。
- [ ] 在 CI 中验证本批次文档/JSON 格式及 SDK 构建；易语言 IDE 和真实 API 联调仍待执行。


## 2026-11-23 第三批 API 进展

- [x] 公告、精华消息、日程 8 条 REST 路由已逐页核验并入清单。
- [ ] 日程修改/删除、论坛、音频、用户资料、群聊接口继续核验。


## 2026-11-23 — 三语言类型化封装进度

- [x] 频道成员分页查询、成员详情 C ABI/C++/易语言 DLL 映射。
- [x] 频道身份组列表、创建、修改、删除 C ABI/C++/易语言 DLL 映射。
- [x] 频道全员禁言、成员禁言/解除 C ABI/C++/易语言 DLL 映射。
- [ ] 成员踢出、身份组成员分配、用户/身份组子频道权限、公告/精华/日程等 typed wrappers。
- [ ] 增加 SDK 单元测试（参数校验、路径安全、JSON body）与易语言 IDE 实测。


## 2026-11-23 — 日程、音频与论坛接口进度

- [x] 日程修改/删除和音频上麦/下麦已新增 C ABI/C++ 类型化函数及易语言 DLL 映射。
- [x] 论坛帖子列表、详情、发表、删除四条路由已登记；均标记 PrivateDomain。
- [ ] 对论坛 PrivateDomain 能力、音频机器人权限进行应用侧实机确认。
- [ ] 为论坛接口评估后续 typed wrapper（仅在确认目标应用具备私有域资格后）。


当前 SDK 覆盖数字（2026-11-23 最新）：官方文档目录 81 页；REST endpoint 逐页核验 42 条；三语言类型化 wrapper 25 条；其余 endpoint 仍可通过通用 method/path/body 入口调用。此数字为增量核验，不代表全量 REST API。


## 2026-11-23 — 第六批 SDK 类型化管理接口

- [x] 身份组成员分页查询 C ABI/C++/易语言映射。
- [x] 踢出频道成员 C ABI/C++/易语言映射。
- [x] 成员加入/移除身份组 C ABI/C++/易语言映射。
- [x] 成员/身份组子频道权限查询与修改 C ABI/C++/易语言映射。
- [x] 新增 C ABI 通过 GitHub Actions Make、CMake、C++ 示例构建（run 45148983234）；Windows DLL 和易语言 IDE 实测仍待执行。

当前覆盖统计：81 个官方文档目录页；42 条逐页核验 REST 路由（增量，非全量）；33 条具备独立 C ABI/C++/易语言类型化入口。剩余 9 条可经通用请求入口调用。GitHub Actions run 45148983234 已通过 Make、CMake 与 C++ SDK 示例编译；Windows DLL 加载、易语言 IDE 和 QQ 实机联调未执行。


## 群聊 API 优先级纠偏（2026-11-23）

- [x] 复核腾讯官方 bot-docs 的群聊发送 API、群消息事件和群管理开放状态。
- [x] 新增群聊完整 JSON 消息发送函数 `qqb_send_group_message`，支持 msg_type 0/2/3/4/7 与官方 body 扩展字段；保留文本快捷接口。
- [x] 新增 C++ `Client::send_group_message` 与易语言 DLL 导入说明。
- [x] 将群消息事件文档纳入官方文档目录索引；明确区分群聊与频道。
- [ ] 实现 Gateway/WebSocket 对 GROUP_AT_MESSAGE_CREATE、GROUP_ADD_ROBOT、GROUP_DEL_ROBOT、GROUP_MSG_REJECT、GROUP_MSG_RECEIVE 的解析和插件事件分发。
- [ ] 逐页核验新发现的 17 条群管理 REST 路由（群资料、bot_state、成员、禁言、黑名单、入群申请/审批、自动审批策略）；当前状态记录于 sdk/group-api-discovery.md，未复核前不计入 verified endpoint。

群聊优先级调整：后续优先做群消息收发闭环和群事件分发；频道管理/日程/音频功能转入维护队列。


群聊专项纠偏：已新增 `qqb_send_group_message` 完整 JSON 发送接口；另发现 17 条较新的群管理 REST 路由，尚待逐页核验官方自动生成页面及白名单权限，后续优先处理群 API。



## 2026-11-23 — QQ 群管理 SDK 第一批

- [x] 新增群资料、机器人群内状态、群禁言查询/设置、群成员分页/详情、批量移出群成员 7 个 C ABI 类型化入口。
- [x] 新增对应 C++14 wrapper 与易语言 DLL ABI 映射。
- [x] 群成员分页 cursor 进行 URL 编码，limit 限制 1–30，群/成员 ID 做安全字符校验。
- [ ] 继续实现群黑名单、入群申请/审批、自动审批策略接口。
- [ ] 逐页核验上述群管理接口的腾讯官方自动生成页面，补齐官方参数、权限与错误码后再计入 verified endpoint。
- [x] GitHub Actions C11 build run 45152192633：C++ SDK header example、Make、CMake 均成功。
- [ ] Windows DLL 加载、易语言 IDE 与 QQ 实机权限测试。



## 2026-11-23 — QQ 群管理 SDK 第一批 CI 结果

- GitHub Actions C11 build run 45152192633 已完成并成功：C++ SDK header example、Make build、CMake configure/build 全部通过。
- 本次构建覆盖群管理 SDK 第一批 7 个 C ABI 入口及对应 C++ wrapper；仍未执行 Windows DLL 加载、易语言 IDE 编译和 QQ 实机白名单/管理员权限联调。
- 官方群管理事件页确认 GROUP_ADD_ROBOT、GROUP_DEL_ROBOT、GROUP_MSG_REJECT、GROUP_MSG_RECEIVE 共用 intents 位 1<<25，推送方式为 WebSocket，事件字段含 timestamp/group_openid/op_member_openid。详见 sdk/group-api-discovery.md。



## 2026-11-23 — QQ 群管理 SDK 第二批

- [x] 新增群黑名单分页查询、黑名单增删、入群申请分页查询、入群申请审批 4 个 C ABI 接口。
- [x] 新增对应 C++14 Client 方法与易语言 DLL ABI 映射。
- [x] cursor URL 编码；黑名单 limit=100、入群申请 limit=50。
- [x] 自动审批策略列表、创建、修改、删除、执行、白名单管理 6 个 C ABI/C++/易语言 wrapper。
- [ ] 腾讯官方自动生成页面逐条复核群管理接口参数、权限和错误码。
- [ ] Windows DLL、易语言 IDE、QQ 实机白名单/管理员权限联调。



## 2026-11-23 — QQ 群自动审批策略 SDK 第三批

- 新增自动审批策略列表、创建、修改、删除、执行、白名单维护 6 个 C ABI typed wrapper，配套 C++14 Client 与易语言 DLL 映射。
- 策略是机器人维度资源：列表/创建路径不包含 group_openid；按 strategy_id 修改/删除/执行/管理白名单。
- 策略列表分页 cursor 做 URL 编码，limit 固定 50；执行接口由 SDK 发送空 JSON object。
- 具体请求字段、策略数量上限、群绑定限制、白名单号码限制和执行时效来自第三方维护 fork 的交叉核验，官方自动生成页面尚待逐项复核，不纳入 verified endpoint 统计。
- CI 将由本批代码提交触发；Windows DLL、易语言 IDE 和 QQ 实机权限测试未执行。


## 2026-11-23 — 群事件归一化阶段

- [x] 逐页读取腾讯官方群管理事件文档，确认四种管理事件字段与 Intent `1<<25`。
- [x] 逐页读取腾讯官方消息事件文档，确认 `GROUP_AT_MESSAGE_CREATE` 字段、Intent `1<<25`、权限说明及全量群消息私域边界。
- [x] 新增 C11 群事件 JSON 归一化函数，映射群 openid、成员 openid、消息 id 和文本，保留完整 raw_event。
- [x] 新增群事件解析单元测试目标：Make `make test` 与 CMake/CTest。
- [ ] 接入真实 Gateway/WebSocket transport（连接协商、心跳、重连、断线恢复）；需先核验当前官方推送可用状态。
- [ ] 将已归一化事件送入插件队列，实现有界并发、单插件失败隔离、回调重试和 SSRF 防护。
- [ ] 实现 msg_id 去重存储与 msg_seq 被动回复序号策略。
- [ ] 对官方消息样例、缺字段、非法 JSON、未知事件、附件保留等增加更多协议测试。


- [x] 增加群事件解析器 Make/CMake 单元测试执行步骤；run 45160609516 的 Make unit tests 与 CMake 构建步骤成功。
- [x] 群事件 timestamp 增量及 C++ Event 示例换行修复后的最终 CI（run 45160886586）成功：C++ 示例、Make、Make tests、CMake/CTest 均通过。


## 2026-11-23 — Webhook 事件入箱

- [x] 扩展验签成功的 Webhook 处理函数，安全提取事件类型 t 与 d payload，保留旧函数兼容入口。
- [x] 已验签群管理/群 @ 事件接入统一事件解析器。
- [x] 新增 SQLite event_inbox 持久化队列、唯一键去重和初始化。
- [x] 新增管理员 GET /api/v1/events，查看最近100条事件收件箱记录。
- [x] 增加 SQLite 内存库队列测试，并加入 Make 与 CTest。
- [x] 实现每个启用插件的独立 delivery 记录、callback_url 快照、唯一约束和幂等 fanout。\n- [x] 新增管理员 GET /api/v1/event-deliveries 查询最近 200 条投递任务。\n- [ ] 实现安全 HTTP delivery worker、有界并发、单插件故障隔离和重试退避。
- [ ] callback_url 出站 SSRF 防护（DNS 解析/IP 分类/DNS pinning/禁止重定向/HTTPS only）完成前，不得启用实际 HTTP 投递。
- [ ] Webhook 官方签名向量与 QQ 控制台回调实测。
- [ ] 评估官方 Gateway WebSocket 当前状态；本阶段未实现 WebSocket 客户端。


- [x] 逐插件 pending delivery fanout 与管理员查询 API 的 Make/CMake/单元测试验证通过：run 45176688074 全部步骤成功。
- [ ] 安全 callback HTTP worker、DNS/IP 防护与 DNS pinning 仍未实现；当前 pending delivery 不会自动发送。



## 2026-11-23 — 插件 callback SSRF 防护第一阶段

- [x] 新增 HTTPS-only、443-only、拒绝 userinfo/IP literal 的 URL 解析策略。
- [x] 新增 IPv4/IPv6 公网地址分类；DNS 返回任意非公网地址时整体拒绝。
- [x] DNS 解析结果保存为结构化地址列表，并提供 `CURLOPT_RESOLVE` 参数字符串格式化函数（尚未接入实际 HTTP 请求）。
- [x] 增加确定性 IPv4/IPv6 地址分类和危险 URL 单元测试，并接入 Make/CMake/CTest。
- [ ] 将校验接入插件 callback_url 创建/更新入口。
- [ ] 实现 CURLOPT_RESOLVE DNS pinning、禁用代理和重定向、TLS 主机校验、连接/总超时及响应体上限。
- [ ] 实现 delivery worker、租约 claim、指数退避、dead-letter 和单插件故障隔离。
- [ ] 完成 Windows Winsock 适配和 Windows 构建验证。
- [ ] 只有完整 SSRF 防护通过后才允许实际 HTTP callback。


## Callback DNS 跨平台适配

- [x] 增加 Windows Winsock2/WS2TCPIP 条件编译与初始化/清理路径。
- [x] CMake 与 Makefile 增加 Windows `ws2_32` 链接。
- [ ] 在 Windows MSVC 与 MinGW 环境分别构建并运行 callback security tests。
- [ ] 接入 callback_url 写入校验，再实现 DNS pinning 与安全 HTTP worker；完成前禁止实际出站投递。

- [x] 增加 GitHub Actions Windows-latest + MSYS2/MinGW64 callback security 编译/测试 job。
- [ ] 确认首次 Windows runner 结果并按日志修复兼容问题；该 job 当前仅覆盖 callback security 模块。


## Windows 成品交付

- [x] 增加 Windows Server 2012 API baseline 的 x64 MinGW64 构建/测试/打包 job。
- [x] 增加 PE 导入依赖递归收集脚本，遇到缺失非系统 DLL 时让打包失败。
- [x] 成品包自动生成独立随机管理员 API Token，并提供 start.bat 一键启动入口。
- [ ] 首次 Windows CI 构建与 CTest 全部通过。
- [ ] 在 Windows Server 2012、2012 R2、2016、2019、2022、2025/2026 实机或虚拟机逐版本启动验证。
- [ ] 验证 SDK DLL 的 C ABI / stdcall 导出、插件加载和易语言调用；补齐 x86 发行包。
- [ ] 发布正式 Release ZIP，并提供稳定下载地址；未完成上述验证前不宣称全版本兼容。
