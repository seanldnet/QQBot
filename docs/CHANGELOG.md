# 变更日志

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
