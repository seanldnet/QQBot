# 变更日志

## 2026-11-23 — 插件管理与 SDK 官方 API 代理首版

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
