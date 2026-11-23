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

- 登录视图：AntdUI.Panel、Label、Input、Select、Button。
- 主界面：AntdUI.Window、Panel、Label、Button、Input。
- 消息/运行日志：AntdUI.Input 多行只读视图，按 1.8 秒轮询宿主日志 API。
- 连接方式：WSS 对应后端 `websocket`；Webhook 对应后端 `webhook`。
- 本地后台核心：`runtime/qqbot-core.exe`，只监听 `127.0.0.1:8080`；桌面程序启动时生成高熵管理员令牌并通过环境变量传入核心进程。

## 发布包布局

```
QQBot.exe                 # 唯一桌面主程序，AntdUI + .NET 单文件自包含发布
runtime/
  qqbot-core.exe          # C 语言 QQ 协议宿主
  libqqbot_sdk.dll        # SDK
  ...                     # 后端依赖 DLL
data/                     # SQLite 数据
certs/                    # CA bundle
```

用户只需双击根目录 `QQBot.exe`。不提供 BAT 作为启动入口，不要求用户打开浏览器或手动启动后台核心。

## 构建与验收

开发机安装 .NET 8 SDK 后执行：

```powershell
dotnet restore Desktop/QQBot.Desktop.csproj
dotnet build Desktop/QQBot.Desktop.csproj -c Release
dotnet publish Desktop/QQBot.Desktop.csproj -c Release -r win-x64 --self-contained true -o publish/desktop
```

Windows CI 必须完成 .NET restore/build/publish、C 后端构建与测试、DLL 依赖闭包扫描、PE 基线检查及 ZIP 完整性检查。

构建成功不等于 GUI 验收成功。发布候选还必须在 Windows 10/11 桌面环境实测：
1. 首次启动无空白窗口、无中文乱码。
2. AppID/AppSecret 输入与 WSS/Webhook 选择可操作。
3. WSS 能显示 READY 后连接状态；Webhook 显示等待公网回调配置状态。
4. 消息日志能显示事件数据。
5. 关闭主窗口会终止后台核心进程。
6. 登录失败能显示可读错误，不能把“提交登录请求成功”误报为 QQ 已连接。

## AntdUI 官方资料

- Gitee 仓库：https://gitee.com/antdui/AntdUI
- 中文文档：https://gitee.com/antdui/AntdUI/tree/main/doc/wiki/zh
- NuGet：https://www.nuget.org/packages/AntdUI/
- 官方示例：https://github.com/AntdUI/AntdUI-Demo

AntdUI 为基于 Ant Design 设计语言的 WinForms UI 库。项目使用其官方控件与主题/文本渲染配置，不复制其内部绘制代码。
