# Windows 桌面端 UI 规范（AntdUI）

## 技术路线

- Windows 桌面入口使用 C# WinForms + AntdUI，项目位于 `Desktop/QQBot.Desktop.csproj`。
- AntdUI NuGet 包版本固定为 2.4.20；当前目标框架为 `.NET 8.0-windows`。
- 主窗口基于 `AntdUI.Window`；表单、导航、提示、卡片、日志输入区均使用 AntdUI 控件。
- 不允许以 Win32 `CreateWindow`、原生 STATIC/EDIT/BUTTON/COMBOBOX 控件、WPF、自绘 GDI 控件或 WebView/浏览器页面实现业务 UI。
- WinForms 只负责应用生命周期、窗口承载、布局容器 API 和系统集成；所有可见业务控件必须来自 AntdUI。
- UI 源文件统一 UTF-8，中文文案直接使用 Unicode 字符串；不再通过 ANSI Win32 API 传递中文。

## 当前桌面结构

```
Desktop/
  QQBot.Desktop.csproj
  Program.cs
  MainForm.cs
```

- 启动后直接进入主界面，不再显示独立登录窗口。
- 账号管理页：使用 AntdUI.Panel、Label、Input、Select、Button 添加并连接机器人；当前 C 宿主核心只支持一个活动机器人实例，后续多机器人需扩展核心实例隔离。
- 认证失败时显示 QQ 返回的 HTTP 状态、错误码和 message；不再只显示笼统的 qq_auth_failed。
- 主界面：AntdUI.Window、Panel、Label、Button、Input。
- 消息/运行日志：AntdUI.Input 多行只读视图，按 1.8 秒轮询宿主日志 API。
- 连接方式：WSS 对应后端 `websocket`；Webhook 对应后端 `webhook`。
- 本地后台核心：`runtime/qqbot-core.exe`，只监听 `127.0.0.1:8080`；桌面程序启动时生成高熵管理员令牌并通过环境变量传入核心进程。

## 发布包布局

```
QQBot.exe                 # 唯一桌面主程序，AntdUI + .NET 8 自包含单文件发布
runtime/
  qqbot-core.exe          # C 语言 QQ 协议宿主
  libqqbot_sdk.dll        # SDK
  ...                     # 后端依赖 DLL
data/                     # SQLite 数据
certs/                    # CA bundle
```

用户双击根目录 `QQBot.exe` 直接进入主界面，在“账号管理”页添加机器人。不提供 BAT 作为启动入口，不要求用户打开浏览器或手动启动后台核心。桌面程序采用 .NET 8 self-contained single-file 发布，.NET Runtime、AntdUI 与业务程序集一并打包进根目录 QQBot.exe；目标电脑无需预先安装 .NET Runtime 或 .NET Desktop Runtime。为实现单文件发布，必要的原生运行库在首次启动时由 .NET 从 EXE 内解压到系统临时目录。协议核心及其原生依赖仍放在 runtime/ 文件夹，不与桌面 EXE 混淆。

## 构建与验收

开发机安装 .NET 8 SDK 后执行：

```powershell
dotnet restore Desktop/QQBot.Desktop.csproj
dotnet build Desktop/QQBot.Desktop.csproj -c Release
dotnet publish Desktop/QQBot.Desktop.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -o publish/desktop
```

Windows CI 必须完成 .NET self-contained single-file restore/build/publish、C 后端构建与测试、DLL 依赖闭包扫描、PE 基线检查及 ZIP 完整性检查；还要检查桌面 EXE 已包含 .NET runtimeconfig/deps 所需内容，不依赖目标机全局 .NET 安装。

构建成功不等于 GUI 验收成功。发布候选还必须在 Windows 10/11 桌面环境实测：
1. 首次启动无空白窗口、无中文乱码。
2. 主界面直接打开；在账号管理页添加 AppID/AppSecret 并选择 WSS/Webhook。
3. WSS 能显示 READY 后连接状态；Webhook 显示等待公网回调配置状态。
4. 消息日志能显示事件数据。
5. 关闭主窗口会终止后台核心进程。
6. Access Token 认证失败能显示 QQ 返回的 HTTP 状态/错误码/message；Token 获取成功但 WSS 被拒绝时，应提示检查 QQ 开放平台 IP 白名单、机器人状态和订阅权限，不能把提交请求成功误报为 READY。

## AntdUI 官方资料

- Gitee 仓库：https://gitee.com/antdui/AntdUI
- 中文文档：https://gitee.com/antdui/AntdUI/tree/main/doc/wiki/zh
- NuGet：https://www.nuget.org/packages/AntdUI/
- 官方示例：https://github.com/AntdUI/AntdUI-Demo

AntdUI 为基于 Ant Design 设计语言的 WinForms UI 库。项目使用其官方控件与主题/文本渲染配置，不复制其内部绘制代码。

## QQ IP 白名单与认证阶段

- Access Token 通过 `https://bots.qq.com/app/getAppAccessToken` 获取；凭证交换失败应优先检查 AppID、AppSecret、网络及 QQ 返回错误码。
- QQ 官方 IP 白名单限制的是启用白名单后的正式环境 WSS 连接与 OpenAPI 调用。若未启用白名单，不要求填写 IP；若已启用，则必须把运行此程序的公网出口 IP 添加到 QQ 开放平台白名单。
- 本机 `127.0.0.1` 仅是桌面程序与本机 C 核心之间的管理接口绑定地址，不是 QQ 平台需要加入白名单的公网 IP。
- 新版本不再把所有认证失败折叠成 `qq_auth_failed`，应保留 QQ 响应中的 message/code 供界面显示。
