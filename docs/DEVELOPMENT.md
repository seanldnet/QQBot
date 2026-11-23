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

## 6. 当前基线

- 仓库地址：https://github.com/seanldnet/QQBot
- 初始仓库为空；本次首先建立文档基线。
- 代码实现尚未开始；编译与运行测试尚未执行。
