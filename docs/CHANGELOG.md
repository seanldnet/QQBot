# 变更日志

## 2026-11-23 — 首批频道管理接口三语言类型化封装

- 修改 `sdk/include/qqbot_sdk.h`、`sdk/src/qqbot_sdk.c`：新增 `qqb_get_guild`、`qqb_list_channels`、`qqb_get_channel`、`qqb_create_channel`、`qqb_update_channel`、`qqb_delete_channel` 六个 C ABI 函数；路径 ID 仅允许字母、数字、下划线和连字符，避免将路径分隔符或路径穿越字符带入 URL。
- 修改 `sdk/cpp/qqbot.hpp`：新增对应的 `qqbot::Client` 方法，自动释放 SDK 响应缓冲区。
- 修改 `sdk/e-language/README.md`：补充六个函数的 DLL 导入映射。
- 修改 `sdk/official-endpoints.json` / `sdk/official-endpoints.md`：将类型化封装映射回已核验 endpoint；消息四场景继续使用现有文本发送便利函数。
- 修改 `sdk/cpp/example.cpp`、`docs/DEVELOPMENT.md`、`docs/TASKS.md`：同步示例和开发状态；同时修正 TASKS 中 endpoint 清单条目的换行格式。
- 验证：前一提交的 GitHub Actions Make/CMake/C++ 示例构建已成功；本次代码提交触发的新一轮 CI 仍需单独确认。未执行易语言 IDE 编译、Windows DLL 构建或 QQ 实机 API 调用。
- 边界：创建/修改接口接收调用者提供的完整 JSON 对象；SDK 不替调用者静默补齐必填字段，实际权限仍由 QQ 平台校验。


## 2026-11-23 — 官方 REST endpoint 首批逐项核验

- 新增 `sdk/official-endpoints.json`：记录 10 条从腾讯官方原文直接核验的 REST endpoint，包括 method、path、路径参数、请求字段、权限说明、回复限制和已明确的错误码；对官方页面未写明的权限/错误码明确标注，不推断。
- 新增 `sdk/official-endpoints.md`：提供便于开发者阅读的 endpoint 表、字段约束、消息频控/被动回复时效及官方原文链接。
- 修改 `sdk/official-api-catalog.md`：同步增量核验进度，纠正代理状态描述，明确全量类型化 wrapper 尚未完成。
- 修改 `docs/DEVELOPMENT.md`、`docs/TASKS.md`：记录首批 10 条接口和剩余接口范围。
- 原因：81 页文档目录索引不等于 REST endpoint 清单；SDK 要逐项记录官方 method/path/参数/权限/错误码，不能把目录页面数量当成接口数量。
- 验证：已读取并对照腾讯官方 bot-docs 的 7 份原始页面（发送消息文档包含四场景；另含频道详情、子频道列表/详情/创建/修改/删除）；清单 JSON 已按结构生成并提交。未执行 C/C++/易语言编译及 QQ 实机请求测试，未生成独立类型化 wrapper，仍标记为未完成。


## 2026-11-23 — 跨语言插件事件模型

- 最后同步：完善 API.md 插件注册/启停/删除请求响应示例与管理令牌说明；更新易语言回调字段映射、C++ Event 结构、README SDK 导航及 TASKS 当前状态；修正 SDK 头文件对代理鉴权的描述，明确当前采用插件身份认证而非细粒度权限 scope。

- 新增 sdk/event-schema.md：定义宿主向插件 POST 的统一事件 JSON 字段，保留官方 event_type 与 raw_event，并明确 group/c2c/channel 的 chat_type、openid 映射和被动回复上下文。
- 新增 sdk/cpp/event.hpp 与 sdk/e-language/event-fields.md：提供 C++ 事件结构和易语言字段映射；事件投递器尚未实现，该文件是先行固定的跨语言契约。

## 2026-11-23 — 插件管理与 SDK 官方 API 代理首版

- 后续修正：插件 PATCH/DELETE 路由按无尾缀的 `/{id}` 正确匹配；SDK null body 不再向上游发送字面 `null`；状态接口在回调公网可达性未验证前固定报告 `transport_ready=false`；协议文档纠正此前 API host 记录冲突，以官方 api-use.md 的 `api.sgroup.qq.com` + `bots.qq.com/app/getAppAccessToken` 为默认。
- 文档同步：刷新 DEVELOPMENT 当前基线、API 插件代理契约、PROTOCOL-RESEARCH 域名核验和 TASKS 状态；README 增加 SDK 快速接入与 Windows DLL 指南。

- 修改 src/main.c：新增管理员 Bearer Token 鉴权（QQBOT_ADMIN_TOKEN，至少 24 字符）；新增插件注册、列表、启停、删除 API；注册时使用 OpenSSL RAND_bytes 生成高熵插件 ID/Token，数据库只存 SHA-256 Token 摘要，Token 仅返回一次。
- 修改 src/main.c：新增 POST /api/v1/plugins/{id}/qq-api；校验 X-Plugin-ID / X-Plugin-Token、插件启用状态和 method/path/body，再通过宿主 access_token 代理到官方 API Base URL；固定官方域名，拒绝绝对 URL、路径穿越和非法 method；设置 TLS 校验、超时和响应体上限。
- 修改 SDK：增加频道私信（/dms/{guild_id}/messages）便利发送函数；C++ 和易语言接口映射同步更新；修复 SDK JSON 字符串转义、插件 Header CRLF 注入风险和空 body 处理。
- 新增 sdk/windows/build-mingw.bat、sdk/windows/README.md：提供 MinGW Windows DLL 构建脚本和 x86/x64 位数匹配说明；CMake MinGW 构建增加 --kill-at。
- 新增 sdk/QUICKSTART.md：记录管理员令牌设置、插件注册、C++ 与易语言 SDK 接入步骤。
- 修改 .github/workflows/build.yml：增加 C++14 SDK 示例编译步骤。
- 修复 src/main.c 状态接口：未确认公网回调可达性前不再将 transport_ready 报告为 true；修复插件 PATCH/DELETE 按 /{id} 路由匹配及 GET 无 body 代理行为。
- 验证：提交成功；没有本地编译器/Windows 易语言 IDE 验证结果；CI 工作流尚未取得结果。插件 API 与 SDK 仅标记首版实现，端到端未验证。


## 2026-11-23 — C ABI / C++ / 易语言 SDK 首版与官方 API 目录

- 新增 sdk/include/qqbot_sdk.h、sdk/src/qqbot_sdk.c：提供统一 C ABI 插件 SDK；libcurl 调用宿主插件代理契约，包含任意官方 REST method/path/body 通用调用、响应内存释放、HTTPS 校验、超时、响应体上限和插件 Token 头鉴权。
- 新增 sdk/cpp/qqbot.hpp、sdk/cpp/example.cpp：提供 C++ RAII 客户端封装、通用 API 调用和群聊/单聊/频道文本发送封装；支持传递 event_id、msg_id、msg_seq 被动回复上下文。
- 新增 sdk/e-language/README.md、sdk/e-language/example.md：记录易语言 Windows DLL ABI、__stdcall、x86/x64 位数匹配、UTF-8 参数和返回内存释放约定。当前是 DLL 导入映射文档，尚未在易语言 IDE 实测。
- 新增 sdk/official-api-catalog.md 与 sdk/official-api-index.json：从腾讯官方 tencent-connect/bot-docs API v2 目录配置整理 81 个官方文档页面，覆盖消息、消息结构/交互、频道、群聊、用户、协议和错误说明；SDK 通用调用入口可将这些文档定义的 REST path 交给宿主代理。
- 修改 Makefile / CMake：增加 libqqbot_sdk 共享库构建与安装目标；make all 同时构建宿主和 SDK。
- 修正 src/main.c 默认官方 API 地址：OpenAPI 根地址 https://api.sgroup.qq.com，Token URL https://bots.qq.com/app/getAppAccessToken。与用户原始需求中的 https://api.bot.qq.com 不同；原始需求保留，实际默认值按官方 api-use.md，并支持 QQBOT_API_BASE / QQBOT_TOKEN_URL 覆盖。
- 新增接口契约 POST /api/v1/plugins/{id}/qq-api：插件 ID + Token 鉴权，代理 {method,path,body}；当前宿主端代理路由和插件注册/Token 管理尚未实现，因此 SDK 尚不能端到端调用真实 QQ API。
- 官方消息发送规则：主动推送自 2025-04-21 起不再提供，SDK 便利方法不能绕过被动回复时效、频次和事件上下文限制。
- 验证：GitHub 文件提交成功；未运行本地 Make/CMake 编译，未进行 Windows DLL 构建、易语言 IDE 验证或真实 QQ API 联调。不得标记为已验证。


所有代码或文档变更均按时间倒序记录。每条记录必须包含变更范围、原因、行为影响和验证结果。不得将未执行的测试描述为通过。




## 2026-11-23 — 日志接口、安全默认值与 CI 构建流程

- 修改 `src/main.c`：新增 `/api/v1/logs` 最近 100 条日志查询；Webhook 请求校验 `X-Bot-Appid`（请求头存在时必须与当前配置一致）；管理 HTTP 服务默认仅绑定 `127.0.0.1`，通过 `QQBOT_BIND` 才可显式暴露其他 IPv4 地址。
- 修改 `src/qq/webhook.c`：签名 hex 解码改为逐字符严格校验，拒绝包含非十六进制字符的签名。
- 修改 `web/index.html`：默认连接方式切换为 Webhook，登录后显示并轮询运行日志。
- 新增 `.github/workflows/build.yml`：在 push / pull request 时安装依赖，分别执行 Make 与 CMake 构建。
- 修改 `README.md`、`docs/DEVELOPMENT.md`、`docs/API.md`、`docs/TASKS.md`、`DEPENDENCIES.md`：同步当前实现范围、接口状态、官方协议约束与未完成事项。
- 验证：GitHub 已接受工作流文件提交；当前未取得 Actions 构建结果，因此编译仍标记为未验证。

## 2026-11-23 — Webhook 接入与运行日志首版

- 新增 `src/qq/webhook.h` / `src/qq/webhook.c`：实现 QQ op=13 challenge 签名生成，以及 op=0 事件的 Ed25519 验签；seed 按官方规则由 Bot Secret 重复并截取 32 字节，事件签名消息按 `timestamp + raw_body` 拼接。
- 新增 `tests/README.md`：记录官方签名测试向量与篡改/缺失签名负向测试要求。
- 修改 `src/main.c`：挂载 `/webhook/qq`，验签成功返回 `{"op":12}`，验证回调 AppID（如平台提供该头），增加 `/api/v1/logs` 最近 100 条日志接口；登录仅开放 Webhook 模式，WebSocket 明确返回未实现，不伪装连接成功。
- 修改 Makefile / CMake：将 Webhook 模块纳入构建。
- 修改 `web/index.html`：默认选择 Webhook，登录后轮询并显示运行日志。
- 安全修复：管理 HTTP 服务默认绑定 `127.0.0.1`，可通过 `QQBOT_BIND` 显式指定 IPv4 监听地址；签名 hex 解析改为严格字符校验。
- 协议依据：官方 `event-emit.md` 与 `sign.md`。官方文档同时标注 WebSocket 事件链路计划逐步下线，Webhook 作为当前实现优先级。
- 验证：文件提交成功；未运行本地编译，未执行官方固定向量的 C 端测试，也未用真实 QQ 回调联调。以上均仍待验证。

## 2026-11-23 — C11 宿主首阶段骨架

- 新增 `Makefile` 与 `CMakeLists.txt`：建立 C11 构建入口，检测 libmicrohttpd、libcurl、SQLite、cJSON、OpenSSL。
- 新增 `src/main.c`：实现 HTTP 服务启动、SQLite WAL 与迁移元数据初始化、日志持久化、登录请求 JSON 校验、QQ Access Token 请求与内存缓存、状态查询、静态首页托管。
- 新增 `web/index.html`：实现 Argon Dashboard 风格登录入口，收集 AppID / AppSecret 与连接模式。
- 新增 `DEPENDENCIES.md` 与 `.gitignore`：记录依赖安装和本地敏感数据忽略规则。
- 修复 `src/main.c`：纠正 HTTP 响应头函数调用错误；状态接口将“凭据认证成功”与“传输已连接”分开，避免把拿到 token 误报为机器人已上线。
- 重要边界：当前 WebSocket / Webhook 传输、消息发送、插件 API、实时日志尚未实现；Webhook 选择会返回明确的 501，不会假装上线。登录成功只代表 access_token 获取成功。
- 验证：GitHub 文件提交成功；本执行环境未提供仓库本地编译运行能力，因此 Make/CMake 编译与真实 QQ API 联调均标记为未验证。

## 2026-11-23 — QQ API v2 协议核验记录

- 新增 `docs/PROTOCOL-RESEARCH.md`：记录当前 API v2 域名、Access Token、消息发送限制、Gateway 与 Webhook 已确认事项及待核验边界。
- 确认用户指定的 `https://api.bot.qq.com` 与当前 API v2 文档检索到的统一 API 根域名一致；旧版域名信息仍在官方首页资料中并存，token endpoint 将配置化。
- 明确 2025-04-21 起主动推送策略变化，不能把“任意会话主动发消息”作为最小闭环前提；最小闭环以真实收到事件后被动回复为准。
- 验证：检索官方文档仓库与 API v2 消息文档；未进行真实 QQ 账号 API 联调。

## 2026-11-23 — 仓库文档基线初始化

- 新增 `README.md`：项目入口、文档导航和工程原则。
- 新增 `docs/DEVELOPMENT.md`：保存用户最初完整需求原文、技术栈、模块划分和协议核验要求。
- 新增 `docs/CHANGELOG.md`：建立逐次变更记录规则。
- 新增 `docs/TASKS.md`：建立任务状态追踪。
- 新增 `docs/API.md`：建立宿主与插件 API 契约文档骨架。
- 新增 `docs/DATABASE.md`：建立 SQLite 设计与迁移文档骨架。
- 原因：用户明确要求最初指令永久存档，并要求所有新增、修复、调整均可从仓库文档恢复。
- 验证：GitHub 返回上述文件提交成功；未执行代码编译（仓库初始为空，尚无代码）。


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
