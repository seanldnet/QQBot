# SQLite 数据库设计

> 本文同时记录已落地首版表结构与后续规划。当前使用幂等 CREATE TABLE 初始化；完整版本化迁移框架仍待完善。

## 1. 基本原则

- SQLite 启用 `PRAGMA foreign_keys=ON`；根据部署场景评估 WAL 与 busy_timeout。
- 所有 schema 通过有版本号的迁移管理，启动时幂等检查；禁止简单重复执行非幂等 CREATE 导致二次启动失败。
- 使用参数化 SQL；事务覆盖相关联写操作；错误必须可观测。
- 密钥与 token 不得明文写入日志；优先使用系统密钥/环境注入的主密钥加密敏感字段。

## 2. 规划表

- `schema_migrations(version, applied_at)`：已应用迁移版本。
- `bot_config(id, app_id, app_secret_ciphertext, connection_mode, enabled, created_at, updated_at)`：机器人配置。
- `runtime_state(id, state, access_token_ciphertext, token_expires_at, connected_at, updated_at)`：运行状态与令牌缓存（按安全设计调整）。
- `plugins(id, name, callback_url, token_hash, enabled, created_at, updated_at)`：插件注册信息。
- `logs(id, level, component, message, created_at)`：结构化运行日志；敏感字段必须脱敏。
- `event_inbox(id, dedupe_key, event_type, event_json, state, created_at)`：已验签并归一化的事件收件箱；dedupe_key 唯一。
- `event_deliveries(id, event_id, plugin_id, callback_url, state, attempts, last_error, next_attempt_at, created_at, updated_at)`：按事件和插件拆分的待投递任务；唯一约束 `(event_id, plugin_id)`，插件删除时级联清理。callback_url 在创建任务时快照保存；不保存插件 token。
- `settings(key, value, updated_at)`：非敏感运行设置；敏感设置必须加密。

## 3. 迁移要求

1. 创建迁移元数据表。
2. 读取已应用版本，按顺序执行缺失迁移。
3. 每个迁移在事务中执行，并在同一事务写入版本记录。
4. 并发启动需通过 SQLite 写锁/事务串行化；重复启动不得重复创建表或丢失数据。
5. 测试空库首次启动、已有库二次启动、迁移中断恢复和外键约束。

## 4. 当前事件表行为\n\n- Webhook 只有在 Ed25519 验签通过且事件可被群事件解析器识别后，才写入 `event_inbox`。\n- 事件入箱后，系统为当时启用且 callback_url 非空的插件创建独立 `pending` delivery；重复 Webhook 不会重复创建同一事件/插件任务。\n- 当前没有实际 HTTP delivery worker；`attempts` 保持 0，任务不会被自动置为 delivered。启用 worker 前必须先完成 callback_url DNS/IP 防护与 DNS pinning。\n\n## 5. 数据保留与备份

日志需可配置保留期限与容量上限。升级前备份数据库；迁移失败时保留原库并明确报错，不得自动删除或重建用户数据库。
