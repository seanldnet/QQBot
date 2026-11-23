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

当前包含 C11 QQ 机器人协议核心、SQLite 日志、Access Token 获取、WSS Gateway 首版、Webhook challenge/Ed25519 验签和 C/C++ SDK。Windows 桌面端采用 C# WinForms + AntdUI；最终用户界面不再使用手写 Win32 控件。AntdUI 桌面构建和实机验收状态详见 docs/DESKTOP-UI.md 与 docs/TASKS.md。WSS RESUME、完整插件投递、真实 QQ 官方平台实机联调仍待完成。

## 开发原则

1. 不以伪代码、空实现或静态假数据冒充可用功能。
2. 每次新增、修复、删除或调整代码，都必须更新 docs/CHANGELOG.md。
3. 影响架构、接口、数据结构或运行方式时，必须同步更新对应文档。
4. 涉及 QQ 官方平台能力时，以官方最新接口文档为准；不得臆造 API 路径、事件字段或签名算法细节。
5. 未经验证的功能不得标记为已完成。


## Windows 成品包

项目正在增加 Windows x64 一键运行包。目标为 Windows Server 2012 及更新版本，压缩包将包含宿主、SDK DLL、运行时依赖、Web UI、启动脚本和初始化管理员 Token。请从 GitHub Actions 的 Windows package job 获取验证产物；在 Windows CI 和逐版本实机测试全部通过前，不视为正式兼容性承诺。


## Windows 成品交付

目标提供可直接解压运行的 Windows x64 ZIP，根目录只有一个桌面主程序 QQBot.exe；协议核心、SDK DLL 和第三方运行时依赖放入 runtime/，数据库和证书分别放入 data/ 与 certs/。Windows Server 2012/2012 R2 至 2026 的逐版本实机兼容验收尚在进行；CI 成功不等于所有版本已验证。

Windows 成品包还会携带 CA 根证书 bundle，并由启动脚本设置 `CURL_CA_BUNDLE`，避免依赖构建机上的证书路径。


## Windows 原生桌面程序

Windows 构建桌面入口为 C# WinForms + AntdUI 发布的根目录 QQBot.exe；后台协议核心为 runtime/qqbot-core.exe。双击 QQBot.exe 直接进入主界面，在“账号管理”页添加 AppID/AppSecret 并选择 WSS 或 Webhook；无需独立登录弹窗、浏览器或 BAT。桌面 EXE 使用 .NET 8 Desktop Runtime 框架依赖发布以降低体积，目标电脑需安装 .NET 8 Windows Desktop Runtime x64。Web UI 保留作开发诊断用途，不是最终桌面入口。业务 UI 必须使用 AntdUI 官方控件；禁止另写 Win32/WPF/WebView 控件界面。


## Windows 桌面客户端（AntdUI）

Windows GUI 已切换到 C# WinForms + AntdUI，代码位于 `Desktop/`。用户运行包只保留根目录 `QQBot.exe` 作为唯一启动入口；C 语言协议核心和运行依赖放在 `runtime/`，数据位于 `data/`。桌面端技术约束、构建命令和验收标准见 [docs/DESKTOP-UI.md](docs/DESKTOP-UI.md)。
