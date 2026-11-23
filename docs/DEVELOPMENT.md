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
- 已有统一 C ABI SDK、C++14 封装、易语言 Windows DLL ABI 映射、Windows MinGW 构建脚本，以及官方 API v2 目录 81 页索引。\n- 已新增 `sdk/official-endpoints.md` / `sdk/official-endpoints.json`，从腾讯官方原文逐项核验 10 条 REST 路由（四种消息发送、频道详情与子频道 CRUD）；6 条频道管理路由已提供 C ABI、C++ 和易语言导入封装。这只是增量清单，不代表全部 API 已覆盖。
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
