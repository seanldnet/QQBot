# 宿主 HTTP API 与插件协议

> 本文件是接口契约基线。具体路由、字段、状态码在实现前需固定，并在代码变更时同步更新。当前尚未实现任何业务路由。

## 1. 通用约定

- 管理面板与插件 API 使用 JSON；请求/响应使用 UTF-8。
- 所有接口设置请求体大小上限、连接/读取超时和统一错误结构。
- 管理接口必须通过会话认证与 CSRF 防护；插件接口使用插件 ID + 高熵随机 token 的安全比较方式鉴权。
- 凭据仅在创建/轮换时展示一次；日志禁止记录完整 token、AppSecret、Authorization 头。

## 2. 规划路由（待实现）

| 方法 | 路径 | 用途 |
|---|---|---|
| POST | `/api/v1/auth/login` | 提交 AppID、AppSecret、连接模式并启动机器人 |
| POST | `/api/v1/auth/logout` | 退出管理会话并按策略停止机器人 |
| GET | `/api/v1/status` | 获取运行状态与连接模式 |
| GET | `/api/v1/logs` | 读取最近 100 条运行日志（首版已实现） |
| GET | `/api/v1/logs/stream` | 实时日志流（协议待选型） |
| GET/POST | `/api/v1/plugins` | 查询/注册插件 |
| PATCH/DELETE | `/api/v1/plugins/{id}` | 启停/删除插件 |
| POST | `/api/v1/plugins/{id}/messages` | 插件发送消息 |
| POST | `/webhook/qq` | QQ 官方 Webhook 回调；op=13 challenge、op=0 Ed25519 验签与 op=12 ACK（首版已实现） |

其中 `/api/v1/auth/login`、`/api/v1/status`、`/api/v1/logs` 和 `/webhook/qq` 已有首版实现；插件 CRUD、插件消息发送和实时流尚未实现。

## 3. 插件事件 JSON 契约（初步）

事件至少表达 `event_type`、`chat_type`、`chat_id`、`user_id`、`content`。正式字段需按 QQ 官方事件模型映射；原始事件可作为受限字段保留。插件回调使用 POST application/json，配置独立连接与响应超时。

## 4. 插件安全

- callback_url 默认仅允许 HTTPS；开发环境 HTTP 必须显式开启。
- 禁止 localhost、回环、链路本地、私网、保留地址及云元数据地址；解析 DNS 后校验并防止 DNS rebinding。
- 禁止重定向或逐跳重新校验；限制响应体大小。
- 每插件并发数、队列长度、超时和重试次数均需有上限。
- 插件 token 使用密码学安全随机数生成，数据库仅保存安全摘要或加密值。

## 5. QQ 官方协议

QQ API 根域名、token 获取请求、Authorization 格式、Gateway、Webhook 验签与 ACK 必须根据官方文档核验后落地。用户原始描述存档于 `DEVELOPMENT.md`，不以未经核验的假设代替官方协议。
