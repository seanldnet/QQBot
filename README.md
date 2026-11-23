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
- [SDK 快速接入](sdk/QUICKSTART.md)
- [Windows / 易语言 DLL 构建](sdk/windows/README.md)
- [官方 API v2 全量目录（81 页）](sdk/official-api-catalog.md)
- [机器可读 API 目录](sdk/official-api-index.json)
- [C ABI SDK 头文件](sdk/include/qqbot_sdk.h)
- [C++ SDK](sdk/cpp/qqbot.hpp)
- [插件统一事件模型](sdk/event-schema.md)
- [易语言 DLL 接入说明](sdk/e-language/README.md)

## 当前状态

当前包含 C11 QQ 机器人协议核心、SQLite 日志、Access Token 获取、WSS Gateway 首版、Webhook challenge/Ed25519 验签和 C/C++ SDK。Windows 正在转向原生桌面 EXE：新增 Win32 GUI 登录入口，桌面程序启动本机核心进程并通过本机 API 展示状态与日志。WSS RESUME、完整插件投递、真实 QQ 官方平台实机联调仍待完成。详见 docs/TASKS.md。

## 开发原则

1. 不以伪代码、空实现或静态假数据冒充可用功能。
2. 每次新增、修复、删除或调整代码，都必须更新 docs/CHANGELOG.md。
3. 影响架构、接口、数据结构或运行方式时，必须同步更新对应文档。
4. 涉及 QQ 官方平台能力时，以官方最新接口文档为准；不得臆造 API 路径、事件字段或签名算法细节。
5. 未经验证的功能不得标记为已完成。


## Windows 成品包

项目正在增加 Windows x64 一键运行包。目标为 Windows Server 2012 及更新版本，压缩包将包含宿主、SDK DLL、运行时依赖、Web UI、启动脚本和初始化管理员 Token。请从 GitHub Actions 的 Windows package job 获取验证产物；在 Windows CI 和逐版本实机测试全部通过前，不视为正式兼容性承诺。


## Windows 成品交付

目标提供可直接解压运行的 Windows x64 ZIP，包含宿主、SDK DLL、运行时依赖、启动脚本和默认资源。Windows Server 2012/2012 R2 至 2026 的逐版本实机兼容验收尚在进行；CI 成功不等于所有版本已验证。

Windows 成品包还会携带 CA 根证书 bundle，并由启动脚本设置 `CURL_CA_BUNDLE`，避免依赖构建机上的证书路径。


## Windows 原生桌面程序

Windows 构建目标为 `bin/QQBot.exe`（原生桌面窗口）和 `bin/qqbot-core.exe`（后台协议核心）。双击 QQBot.exe 后输入 AppID/AppSecret，选择 WSS 或 Webhook 登录；无需浏览器。Web UI 保留作开发诊断用途，不是最终桌面入口。桌面 GUI 首阶段的 CI 与 Windows 实机验收状态以 docs/TASKS.md 为准。
