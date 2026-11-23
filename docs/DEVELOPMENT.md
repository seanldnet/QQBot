# QQBot 开发总文档

## 1. 项目目标

构建一个基于标准 C11 的「QQ 官方机器人」宿主框架，支持 QQ 官方机器人接入、Web 管理面板、Webhook / WebSocket 两种连接模式、多语言 HTTP 回调插件系统、运行日志与 SQLite 持久化。静态前端由 C HTTP 服务托管。

## 2. 用户最初需求（原文完整存档）

> 请直接在当前仓库中编写代码（创建文件、写入完整实现），不要只给说明或让我手动复制。用 C 语言开发一个「QQ 官方机器人」宿主框架：
>
> 【技术栈】
> - 后端纯 C（标准 C11）
> - HTTP 客户端：libcurl（调 QQ 官方 API）
> - HTTP 服务器：libmicrohttpd 或 mongoose（二选一，用于 Web 面板 + Webhook 回调和wss + 插件 API）
> - JSON：cJSON；存储：SQLite；WebSocket 模式用 libwebsockets
> - 前端 UI：Argon Dashboard（https://github.com/creativetimofficial/argon-dashboard），静态文件由 C 服务器托管
>
> 【QQ 官方机器人接入】
> - 用 AppID + AppSecret 换取 access_token，请求头 `Authorization: QQBot {ACCESS_TOKEN}`
> - 接口域名：https://api.bot.qq.com（API v2）
> - 面板可切换两种连接：
>   1) Webhook：HTTP 回调 + Ed25519 签名校验（op=13 验证 / op=12 ACK）
>   2) WebSocket：WSS 长连接 + 心跳 + 重连
> - 场景：群聊（group_openid）、单聊（user_openid）、频道
>
> 【Web 管理面板（Argon Dashboard UI）】
> - 登录页：输入 AppID、AppSecret，选 WebSocket 或 Webhook，登录即上线
> - 页面：运行日志（实时滚动）、插件管理、程序设置
>
> 【插件系统：多语言插件，HTTP 回调】
> - 宿主把消息事件 POST 到插件注册的 callback_url（JSON：event_type/chat_type/chat_id/user_id/content 等）
> - 插件调宿主 HTTP 接口发消息，带插件ID + token 鉴权
> - 支持注册/启停/删除，事件并发分发，单插件失败不影响其他
>
> 【要求】
> - 在当前仓库内直接生成全部代码文件、目录结构、Makefile/CMake 编译脚本和依赖说明
> - 保证能编译、能跑通最小闭环：面板登录 → 机器人上线 → 收发消息 → 日志可见 → 插件收发事件

## 3. 强制工程约束

- 后端业务实现使用 C11；不得以 C++、Go、Rust、Node.js 或 Python 代替宿主后端。
- HTTP 客户端使用 libcurl；JSON 使用 cJSON；持久化使用 SQLite。
- WebSocket 使用 libwebsockets。HTTP 服务端在 libmicrohttpd 与 mongoose 中择一，并通过独立适配层隔离。
- 前端采用 Argon Dashboard 视觉体系，静态资源由宿主服务托管。第三方模板需遵循其许可证和上游资源要求。
- AppSecret、access_token、插件 token 不得写入日志或返回给浏览器；数据库中的敏感凭据应采用加密或安全的密钥管理方案，并明确部署密钥要求。
- 所有网络输入均需长度限制、JSON 校验、超时、错误处理；插件回调必须防止 SSRF、内网探测和任意协议访问。
- 插件分发采用有界并发/队列，单插件失败隔离；设置连接超时、响应超时、重试策略与失败日志。
- 不允许空函数、伪实现或假数据伪装成完成状态。
- 每次代码新增、修改、修复、删除，必须在 `docs/CHANGELOG.md` 记录文件、原因、行为变化、测试结果；同步更新 `docs/TASKS.md`。架构、接口、数据库、部署变化时分别更新本文件及 API / DATABASE 文档。
- 未运行的测试标记为“未执行”；未验证的功能不得标记为“完成”。

## 4. 初步模块划分

1. `src/core`：生命周期、配置、日志、事件模型。
2. `src/qq`：令牌获取、REST API、Gateway / WebSocket、Webhook 验签与 ACK。
3. `src/http`：管理面板 HTTP、Webhook 路由、插件 API、静态资源。
4. `src/plugins`：插件注册、凭据校验、事件分发、并发隔离。
5. `src/storage`：SQLite 初始化、版本迁移、DAO。
6. `web/`：Argon Dashboard 静态前端。
7. `tests/`：单元、集成与协议测试。

## 5. 协议核验待办

用户指定的 API 根域名 `https://api.bot.qq.com`、API v2、Webhook op 语义、Ed25519 签名字段、Gateway 鉴权与心跳格式，必须在实现前逐项对照 QQ 官方开发者文档核验。不得将其他平台（例如 Discord）的 Gateway opcode 或签名格式套用到 QQ。若官方文档与需求中的描述不同，保留原始需求并在本节和变更日志中记录差异，采用官方实际协议实现。

## 6. 已核验的协议约束

- 官方 API v2 文档当前给出的 OpenAPI 根地址为 `https://api.sgroup.qq.com`，Access Token 地址为 `https://bots.qq.com/app/getAppAccessToken`。这与用户最初需求中的 `https://api.bot.qq.com` 不一致；保留原始需求原文，实际请求以官方当前文档为准，并通过 `QQBOT_API_BASE` / `QQBOT_TOKEN_URL` 配置覆盖。
- Webhook op=13：Bot Secret 重复至至少 32 字节并截取 32 字节 seed；签名消息为 `event_ts + plain_token`，响应 `plain_token` 与 hex `signature`。
- Webhook 普通事件：用 `X-Signature-Ed25519`（hex）和 `X-Signature-Timestamp` 验证 `timestamp + 原始 body`；通过后回复 `{"op":12}`。
- 官方文档标注 WebSocket 事件推送计划逐步下线，故 Webhook 是当前首选实现；WebSocket 仍按用户需求保留兼容目标，但必须在实机验证可用性后才能启用。

## 7. 多语言 SDK 强制要求

- 必须提供统一 C ABI SDK 核心，并以该 ABI 为唯一底层接口。
- 必须提供 C++ SDK（C++14 或以上封装、示例和插件模板）与易语言 Windows DLL SDK（导出函数、DLL 命令声明映射、示例和 x86/x64 验证说明）。
- SDK 必须覆盖官方 API v2 全部已公开接口：以官方文档目录生成全量索引；通用 method/path/body 调用保证新接口可用，常用接口再生成强类型便利函数。事件、消息结构、错误码、权限、频控也必须记录。
- AppSecret / access_token 保留在宿主，插件 SDK 仅使用插件 ID + Token 调用宿主代理；宿主必须执行权限、速率、路径和参数校验。
- 每次官方接口目录更新，必须同步更新 `sdk/official-api-catalog.md` 与 `sdk/official-api-index.json`，并记录变更日志。

## 8. 当前基线

- 仓库地址：https://github.com/seanldnet/QQBot
- 已有 C11 宿主首阶段：HTTP 服务、SQLite 日志、Access Token 获取、Webhook Ed25519 challenge / event 验签、日志查询。
- 已有插件注册/列表/启停/删除与插件 Token 鉴权、官方 API 代理首版；管理操作使用 QQBOT_ADMIN_TOKEN。
- 已有统一 C ABI SDK、C++14 封装、易语言 Windows DLL ABI 映射、Windows MinGW 构建脚本，以及官方 API v2 目录 81 页索引。\n- 已新增 `sdk/official-endpoints.md` / `sdk/official-endpoints.json`，从腾讯官方原文逐项核验 42 条 REST 路由（四种消息发送、频道详情与子频道 CRUD）；25 条 REST 路由已提供 C ABI、C++ 和易语言导入封装。这只是增量清单，不代表全部 API 已覆盖。
- 官方当前 API 根地址与 Token URL 已按官方文档修正；用户原始需求中的 api.bot.qq.com 仍保留为历史原文。
- 插件事件并发分发、完整前端、WebSocket 兼容、全量 endpoint 逐项核验与类型化 wrapper、真实消息闭环仍待开发。
- 当前上述新代码尚未取得 CI、本地编译、Windows 易语言 IDE 或 QQ 实机联调结果；不可标记为已验证完成。


## 2026-11-23 — 官方 API 第二批：成员、身份组、权限与禁言

- 逐页核验腾讯官方 bot-docs 第二批 16 条 REST 路由：频道成员列表/身份组成员列表/成员详情/踢出成员、身份组列表/创建/修改/删除、成员身份组增删、子频道用户权限读取/修改、子频道身份组权限读取/修改、频道全员禁言/指定成员禁言。
- 更新 `sdk/official-endpoints.json` 与 `.md`：记录 method/path、路径参数、查询参数或 body 字段、权限说明、响应语义及官方错误码总表引用。源页面未列出具体错误码的接口标注“详见官方总表”，未推测错误码。
- 更新 `sdk/e-language/README.md`：补充第二批接口的易语言调用清单，全部通过现有 `qqb_api_request` ABI 入口访问；C++ 同样通过 `qqbot::Client::request` 访问，避免维护不一致的重复实现。
- 重要约束：成员列表分页需要调用方去重；踢人接口不能移除管理员；撤回消息仅支持 3/7/15/30 天或 -1 全部；角色 5 子频道管理员增删成员需提供 channel 对象；权限位图 add/remove 同位冲突按删除；禁言可用字符串 0 解除。
- 验证：官方源文件读取成功，接口元数据和 SDK 文档提交成功；尚未执行 C/C++ 编译或易语言 IDE 实测，也未进行真实 QQ 权限联调。当前清单仍为增量核验，不宣称全量覆盖。


## 2026-11-23 — 官方 API 第三批：公告、精华消息与日程

- 逐页核验并登记 8 条腾讯官方 REST 路由：创建/删除频道公告、添加/删除/获取精华消息、获取日程列表/详情、创建日程。
- 更新 `sdk/official-endpoints.json`、`sdk/official-endpoints.md`：记录请求方法、路径参数、查询/请求字段、权限说明、返回类型、配额及特殊值语义；错误码按官方总表引用，不作猜测。累计核验 34 条，仍非全量。
- 更新易语言 SDK 接口映射文档；C++ 与易语言通过现有通用 C ABI 访问全部已登记路由。
- 约束：公告推荐子频道与消息公告互斥；推荐子频道最多 3 条；精华消息每子频道最多 20 条；日程 since 单位毫秒；创建日程单管理员每天 10 次、单频道每天 100 次。
- 验证：腾讯官方源文件读取成功，元数据和文档提交成功；CI 尚未取得本批代码构建结论；易语言 IDE 与真实 QQ API 权限联调未执行。


## 2026-11-23 — 第二批高频管理 API 三语言类型化封装

- 新增 C ABI 类型化函数：`qqb_list_guild_members`、`qqb_get_guild_member`、`qqb_list_guild_roles`、`qqb_create_guild_role`、`qqb_update_guild_role`、`qqb_delete_guild_role`、`qqb_mute_guild`、`qqb_mute_guild_member`。
- 新增 C++ `qqbot::Client` 对应方法；每个方法均释放底层响应缓冲区，并将 JSON 结果复制到 std::string。
- 更新易语言 DLL 导入声明，保持统一 `__stdcall` ABI；说明成员分页游标、1–400 limit、禁言秒/时间戳字符串和 `"0"` 解除禁言语义。
- 更新 `sdk/official-endpoints.json` 的 sdk_wrapper 映射。当前 34 条已核验路由中，18 条具备三语言类型化入口（四种消息发送、六种频道管理、八种本批成员/身份组/禁言操作中部分接口的 wrapper；其余仍走通用 API）。
- 安全约束：路径 ID 限定为字母/数字/下划线/连字符；limit 只允许 1–400；禁言时间字段仅接受十进制数字字符串；创建/修改身份组要求 JSON object。
- 验证：GitHub Actions 将对 C11 Make、CMake 和 C++ 示例执行编译；尚未取得本次提交 CI 结果。未执行 Windows DLL 实际加载、易语言 IDE 编译或 QQ 实机权限联调。


## 2026-11-23 — 日程与语音控制 API 三语言封装、论坛接口登记

- 新增 C ABI / C++ 类型化函数：日程列表、详情、创建、修改、删除；机器人上麦、下麦，共 7 个导出入口。
- 成员/身份组/禁言接口 8 个 typed wrapper 也已合入；累计 42 条已核验 REST 路由，其中 25 条具有 C ABI/C++/易语言类型化入口，其余可经通用 method/path/body API 调用。
- 从腾讯官方原文新增 8 条路由：日程修改/删除、语音上麦/下麦、论坛帖子列表/详情/发表/删除；更新 JSON 清单、Markdown 清单、易语言 DLL 声明和开发任务记录。
- 安全/权限约束：since_ms 按十进制字符串校验并保持毫秒单位；路径 ID 仅允许安全字符；音频上/下麦仅限音频类机器人且当前文档要求联系平台申请权限；论坛接口标记 PrivateDomain，不宣称普通机器人可直接调用。
- 验证：成员/身份组/禁言 C ABI 与 C++ 代码的 CI 已通过 Make、CMake 和 C++ 示例编译；本批日程/音频代码与文档提交后将触发新 CI，尚未取得结果。易语言 IDE、Windows DLL 加载和 QQ 实机权限测试仍未执行。


## 当前 SDK 覆盖快照（2026-11-23）

- 官方 API v2 文档目录索引：81 个页面。
- 逐页核验 REST endpoint：42 条（增量清单，非全量）。
- C ABI/C++/易语言类型化入口：25 条 endpoint；其余已登记路由可通过通用 `qqb_api_request` / `qqbot::Client::request` 调用。
- 当前类型化范围：消息发送 4、频道管理 6、成员/身份组/禁言 8、日程/音频 7。
- 易语言内容仍是 DLL ABI 声明映射文档，未声称已在易语言 IDE 编译验证。


## 2026-11-23 — 第六批：成员、身份组与子频道权限类型化 SDK

- 新增 8 个独立 C ABI 导出：身份组成员分页、踢出成员、身份组成员添加/移除、成员权限查询/修改、身份组权限查询/修改。
- 新增 8 个 C++ `qqbot::Client` 对应方法，响应缓冲区统一复制到 `std::string` 后释放。
- 易语言 DLL 映射同步列出全部导出及参数语义，统一使用 `__stdcall` 与 `qqb_free_string`。
- C 层对所有路径 ID 做安全字符校验，分页 limit 限制为 1–400；body 需为 JSON object；拒绝空/非法路径参数。
- 官方 endpoint manifest 中对应 8 条路由补充 typed wrapper 映射。覆盖统计更新为 42 条已核验 endpoint 中 33 条具有三语言类型化入口；仍为增量清单，不代表全量 API。
- 易语言 ABI 文档明确踢人权限、撤回天数、身份组 type 5 的 channel 参数及权限位图 add/remove 冲突语义。
- 尚未执行本批次 CI、Windows DLL 实际加载、易语言 IDE 编译和 QQ 实机权限测试；不得将这些项目标记为已验证完成。


## 2026-11-23 — QQ 群聊 API 专项纠偏

本次确认此前 SDK 开发偏重频道管理接口，现将群聊设为下一阶段优先级。核对腾讯官方 bot-docs 当前 main 分支：
- 群聊发送为 `POST /v2/groups/{group_openid}/messages`，支持 msg_type 0/2/3/4/7 与 Markdown、Ark、Embed、Media、Keyboard 等对象；被动回复窗口 5 分钟、每条消息最多 5 次；主动推送自 2025-04-21 起停止提供。
- `GROUP_AT_MESSAGE_CREATE` 为群 @机器人消息事件；`GROUP_ADD_ROBOT`、`GROUP_DEL_ROBOT`、`GROUP_MSG_REJECT`、`GROUP_MSG_RECEIVE` 为群管理状态事件，均通过 WebSocket 推送。
- 官方群管理 README 仍保留旧版注释；但进一步交叉核对发现近期新增的 17 条群管理 REST 路由（资料、bot_state、成员、禁言、黑名单、入群审批和自动审批策略）。已单独记入 `sdk/group-api-discovery.md`，标记为待逐页核验官方自动生成页面，暂不计入已核验接口。群全量消息事件在消息事件页仍标为私域/暂不开放。
- 新增 `qqb_send_group_message` 完整 JSON C ABI，C++ `Client::send_group_message` 和易语言导入说明；群消息发送已有类型化快捷函数。
- 官方索引新增群消息事件文档；endpoint manifest 标注群消息路由的完整 wrapper 和开放边界。
- 后续开发优先群聊消息与事件闭环，频道管理类新增功能暂停。


### QQ 群管理接口专项发现

已新增 `sdk/group-api-discovery.md`，记录 17 条新发现的群管理路由和核验状态。此前“群管理接口暂不开放”的结论基于旧版 README，现已纠正；群管理 API 是下一阶段第一优先级。群消息完整 JSON 发送 wrapper 已新增，详见 SDK。


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


## 2026-11-23 — 群事件归一化基础设施

- 新增 `src/qq/events.h` / `src/qq/events.c`：实现宿主内部 `qq_event_normalize`，支持 `GROUP_ADD_ROBOT`、`GROUP_DEL_ROBOT`、`GROUP_MSG_REJECT`、`GROUP_MSG_RECEIVE` 与 `GROUP_AT_MESSAGE_CREATE`。
- 群管理事件要求 `group_openid`、`op_member_openid`、整数 timestamp；群 @ 消息要求 `group_openid`、`author.member_openid`、消息 `id`，并映射 content。统一输出保留 `raw_event`。
- `GROUP_AT_MESSAGE_CREATE` 的官方消息 id 映射为 `msg_id`；时间戳保留 RFC3339 格式，群管理整数时间戳转为十进制文本；官方样例未提供的 `event_id` / `msg_seq` 不伪造，输出 null。
- 新增 `sdk/group-events.md`，记录官方事件字段、Intent、消息权限边界、重复投递风险；更新 SDK 通用事件契约和易语言字段映射。
- Makefile 增加 `make test`，CMake/CTest 增加 `qq_group_event_parser` 测试；宿主构建纳入 events.c。
- 明确限制：本次仅实现 Gateway dispatch payload 的 JSON 归一化，不包含 WebSocket 连接、HELLO/IDENTIFY、心跳、重连、事件持久化去重、插件并发分发或 HTTP callback。
- 腾讯官方来源：`server-inter/group/manage/event.md` 与 `server-inter/message/send-receive/event.md`（bot-docs main）。


## 2026-11-23 — Webhook 群事件持久化入口

- 扩展 `qq_webhook_process_ex`：仅在 op=0 Ed25519 签名验证成功后，额外返回外层事件名 `t` 与内层 `d` JSON；保留旧 `qq_webhook_process` ABI/调用方式作为兼容包装。
- 宿主 Webhook 路由将已验签的群管理/群 @ 事件交给 `qq_event_normalize`，并写入 SQLite `event_inbox` 持久化收件箱。
- 收件箱以唯一 dedupe_key 防止重复消息入库：群消息使用事件类型+群+作者+msg_id；群管理事件使用事件类型+群+操作人+timestamp。
- 新增 `GET /api/v1/events` 管理员认证查询接口，最多返回最近100条入箱记录；不暴露插件 Token。
- 新增 `qq_event_queue_init/enqueue` 数据层函数及 SQLite 内存库单元测试。
- 重要边界：这是 Webhook 验签后的群事件持久化，不是 Gateway WebSocket transport；当前不执行插件 callback_url 投递、不自动确认消费、不重试 HTTP 回调。入箱记录状态保持 pending，后续需增加 delivery worker、逐插件投递状态、重试退避和 callback SSRF 防护。


## 2026-11-23 — 逐插件事件投递任务扇出

- 扩展 `src/qq/event_queue.c/.h`：新增 `event_deliveries` 表，为每条已入箱事件创建每插件独立投递任务；只针对创建任务时启用且 callback_url 非空的插件。
- 投递记录快照保存 callback_url，唯一约束 `(event_id, plugin_id)` 防止重复投递任务；记录 state、attempts、last_error、next_attempt_at、时间戳，不保存插件 Token。
- Webhook 入箱后执行 fanout；即使事件此前已入箱（重复请求），也会再次尝试补齐缺失的 delivery rows，以覆盖入箱/扇出之间进程异常的恢复场景。
- 新增管理员 `GET /api/v1/event-deliveries`，最多返回最近 200 条投递任务状态。
- 测试覆盖启用插件扇出、空 callback/停用插件过滤、重复扇出幂等性。
- 安全边界：目前仅建立 durable pending delivery queue，未启动 HTTP callback worker；不向 callback_url 发起任何网络请求。DNS 解析与 IP 分类、DNS pinning、禁止重定向、HTTPS-only 复核完成前不得启用实际投递。

## 7. 插件回调地址安全基线（第一阶段）

新增 `src/qq/callback_security.h/.c`，提供 callback URL 校验和 DNS 解析结果结构：仅接受 HTTPS、443 端口、DNS 主机名；拒绝 userinfo、IP 字面量、非 443 端口；解析所有 DNS A/AAAA 结果，只要其中任意地址不是公网单播地址即整体拒绝。IPv4 拒绝私网、环回、链路本地、共享地址、文档网段、多播和保留网段；IPv6 仅接受 2000::/3 全球单播，并排除特殊用途、6to4 和文档地址。DNS 结果暂存于固定容量结构中，并可格式化为 libcurl `CURLOPT_RESOLVE` 所需的 host:port:address 列表。该格式化函数只生成 pinning 参数字符串；当前尚未由 HTTP 请求使用它，因此实际 DNS pinning 仍未完成。

重要边界：本阶段仅提供校验/解析原语，尚未接入插件注册 URL 校验，也未实现 DNS pinning、HTTP worker、禁止重定向、响应体限制和重试。因此不能把它视为完整 SSRF 防护，也不得启用 callback 出站请求。当前实现使用 POSIX `getaddrinfo`/`inet_pton`，Windows Winsock 适配尚待完成。


## 2026-11-23 — Callback DNS resolver Windows 适配

- `src/qq/callback_security.c`：按平台选择 POSIX sockets 或 Winsock2/WS2TCPIP；Windows 解析路径在调用 `getaddrinfo` 前初始化 Winsock，并在所有退出路径释放 Winsock 会话。
- 移除 `AI_ADDRCONFIG`，避免操作系统本地网卡配置影响 A/AAAA 查询结果；安全策略仍要求检查解析器返回的每个地址。
- `CMakeLists.txt` 与 `Makefile`：Windows 构建链接 `ws2_32`。
- 当前只是源代码级 Windows 适配；尚未在 MSVC/MinGW Windows runner 实际编译或运行，不能视为 Windows 验证通过。DNS pinning、callback URL 写入校验和 HTTP worker 仍未完成，出站请求继续禁用。
