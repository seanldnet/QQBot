# QQ 官方机器人宿主框架

本仓库用于构建基于标准 C11 的 QQ 官方机器人宿主框架。项目需求、开发决策、每次代码变更和测试结果均以仓库文档为准。

## 文档入口

- [原始需求与总体开发规范](docs/DEVELOPMENT.md)
- [变更日志](docs/CHANGELOG.md)
- [任务进度](docs/TASKS.md)
- [HTTP / 插件 API](docs/API.md)
- [数据库设计与迁移](docs/DATABASE.md)
- [QQ 协议核验记录](docs/PROTOCOL-RESEARCH.md)
- [依赖安装说明](DEPENDENCIES.md)
- [多语言 SDK 总览](sdk/README.md)
- [官方 API v2 全量目录（81 页）](sdk/official-api-catalog.md)
- [机器可读 API 目录](sdk/official-api-index.json)
- [C ABI SDK 头文件](sdk/include/qqbot_sdk.h)
- [C++ SDK](sdk/cpp/qqbot.hpp)
- [易语言 DLL 接入说明](sdk/e-language/README.md)

## 当前状态

当前为首阶段开发版本：C11 HTTP 宿主、SQLite 日志、Access Token 获取、Webhook op=13 challenge 与 Ed25519 事件验签已写入；已新增 C ABI SDK、C++ 封装、易语言 DLL 接口映射和官方 API v2 目录索引。已加入插件注册/启停/删除和官方 API 代理首版；插件事件分发、WebSocket、完整会话认证、真实消息收发和实机联调仍未完成。详见 docs/TASKS.md。

## 开发原则

1. 不以伪代码、空实现或静态假数据冒充可用功能。
2. 每次新增、修复、删除或调整代码，都必须更新 docs/CHANGELOG.md。
3. 影响架构、接口、数据结构或运行方式时，必须同步更新对应文档。
4. 涉及 QQ 官方平台能力时，以官方最新接口文档为准；不得臆造 API 路径、事件字段或签名算法细节。
5. 未经验证的功能不得标记为已完成。
