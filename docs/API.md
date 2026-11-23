# 宿主 HTTP API 与插件协议

> 本文件是宿主与插件接口契约。标注“首版已实现”的路由已写入代码，但仍须通过编译、集成和安全测试后才能视为已验证可用。

## 1. 通用约定

- 管理面板与插件 API 使用 JSON；请求/响应使用 UTF-8。
- 所有接口设置请求体大小上限、连接/读取超时和统一错误结构。
- 管理接口必须通过会话认证与 CSRF 防护；插件接口使用插件 ID + 高熵随机 token 的 SHA-256 摘要及常量时间比较鉴权；插件创建/启停/删除要求 `QQBOT_ADMIN_TOKEN`（至少 24 字符）作为 Bearer 管理令牌。
- 凭据仅在创建/轮换时展示一次；日志禁止记录完整 token、AppSecret、Authorization 头。

## 2. 规划路由（待实现）

| 方法 | 路径 | 用途 |
|---|---|---|
| POST | `/api/v1/auth/login` | 提交 AppID、AppSecret、连接模式并启动机器人 |
| POST | `/api/v1/auth/logout` | 退出管理会话并按策略停止机器人 |
| GET | `/api/v1/status` | 获取运行状态与连接模式 |
| GET | `/api/v1/logs` | 读取最近 100 条运行日志（首版已实现） |
| GET | `/api/v1/logs/stream` | 实时日志流（协议待选型） |
| GET | `/api/v1/plugins` | 管理员查询插件列表 |
| POST | `/api/v1/plugins` | 管理员注册插件；注册返回一次性插件 Token（需 Authorization: Bearer 管理令牌） |
| PATCH/DELETE | `/api/v1/plugins/{id}` | 管理员启停/删除插件（PATCH body: `{"enabled":true}`） |
| POST | `/api/v1/plugins/{id}/messages` | 插件发送宿主标准消息（待实现） |
| POST | `/api/v1/plugins/{id}/qq-api` | SDK 官方 API 代理：插件 ID + Token 鉴权，JSON `{method,path,body}`（首版已实现，未实测） |
| POST | `/webhook/qq` | QQ 官方 Webhook 回调；op=13 challenge、op=0 Ed25519 验签与 op=12 ACK（首版已实现） |

其中 `/api/v1/auth/login`、`/api/v1/status`、`/api/v1/logs` 和 `/webhook/qq` 已有首版实现；插件列表/注册/启停/删除和 `/api/v1/plugins/{id}/qq-api` 官方 API 代理已有首版实现；管理操作通过环境变量 `QQBOT_ADMIN_TOKEN` 做 Bearer 鉴权。插件事件回调分发、完整管理会话认证、插件消息抽象 API 和实时流尚未实现。SDK 当前通过该代理路由调用；尚未执行端到端测试。

### 2.1 SDK 官方 API 代理请求

请求头：`X-Plugin-ID`、`X-Plugin-Token`。请求体：`{"method":"POST","path":"/v2/groups/{group_openid}/messages","body":{"content":"...","msg_type":0}}`。代理固定请求官方配置的 API Base URL，不接受绝对 URL；仅允许 GET/POST/PUT/PATCH/DELETE，插件必须启用且 Token 摘要校验成功。响应透传官方 HTTP 状态码与 JSON body。

管理员令牌通过环境变量 `QQBOT_ADMIN_TOKEN` 设置，长度至少 24 字符；注册插件时 Token 只返回一次，数据库仅存 SHA-256 摘要。

## 3. 插件事件 JSON 契约（初步）

事件至少表达 `event_type`、`chat_type`、`chat_id`、`user_id`、`content`。正式字段需按 QQ 官方事件模型映射；原始事件可作为受限字段保留。插件回调使用 POST application/json，配置独立连接与响应超时。

## 4. 插件安全

- 当前插件注册 API 允许 callback_url 留空或使用 HTTPS；事件投递器上线前，必须完成 DNS 解析后私网/回环/链路本地/云元数据地址拒绝与 DNS rebinding 防护。当前尚无事件投递器。
- 禁止 localhost、回环、链路本地、私网、保留地址及云元数据地址；解析 DNS 后校验并防止 DNS rebinding。
- 禁止重定向或逐跳重新校验；限制响应体大小。
- 每插件并发数、队列长度、超时和重试次数均需有上限。
- 插件 token 使用密码学安全随机数生成，数据库仅保存安全摘要或加密值。

## 5. QQ 官方协议

QQ API 根域名、token 获取请求、Authorization 格式、Gateway、Webhook 验签与 ACK 必须根据官方文档核验后落地。用户原始描述存档于 `DEVELOPMENT.md`，不以未经核验的假设代替官方协议。
