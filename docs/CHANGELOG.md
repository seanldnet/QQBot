# 变更日志

所有代码或文档变更均按时间倒序记录。每条记录必须包含变更范围、原因、行为影响和验证结果。不得将未执行的测试描述为通过。


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
