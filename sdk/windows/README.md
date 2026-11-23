# Windows / 易语言 DLL 构建

1. 安装与目标架构一致的 MinGW-w64 GCC（x86 或 x64）。
2. 准备相同架构的 libcurl 开发包，设置 `CURL_ROOT` 指向其根目录。
3. 在仓库根目录执行 `sdk\\windows\\build-mingw.bat`。可通过 `CC` 指定编译器绝对路径。
4. 输出为 `bin/qqbot_sdk.dll`；部署时还需提供匹配架构的 libcurl runtime DLL。
5. 易语言进程与 DLL 位数必须一致。DLL 导出使用 `__stdcall`，MinGW 链接参数 `--kill-at` 去除 x86 名称修饰。

当前环境没有 Windows / MinGW / 易语言 IDE，脚本尚未执行；需在对应 Windows 环境验收。