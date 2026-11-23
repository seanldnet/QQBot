# 构建依赖

当前首阶段服务端依赖：

- C 编译器：支持 C11（GCC / Clang）
- libmicrohttpd：HTTP 服务
- libcurl：HTTPS 客户端
- SQLite 3：持久化
- cJSON：JSON 解析与生成
- OpenSSL：安全内存清理；Webhook Ed25519 验签模块后续使用 EVP
- POSIX pthreads

Debian / Ubuntu 示例：

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libmicrohttpd-dev libcurl4-openssl-dev libsqlite3-dev libcjson-dev libssl-dev
make check-deps
make
./build/qqbot
```

默认监听 `127.0.0.1:8080`。可通过 `QQBOT_BIND` 显式指定 IPv4 监听地址。可通过 `QQBOT_PORT` 调整端口，`QQBOT_API_BASE` 调整 API 根地址，`QQBOT_TOKEN_BASE` 单独覆盖令牌服务根地址。生产部署应放在 HTTPS 反向代理后，并限制管理端网络访问。

仓库包含 GitHub Actions 自动构建工作流（Make + CMake）。插件管理 API 要求设置环境变量 `QQBOT_ADMIN_TOKEN`，长度至少 24 字符；不要提交到仓库或写入日志。默认官方 OpenAPI 根地址为 `https://api.sgroup.qq.com`，Access Token URL 为 `https://bots.qq.com/app/getAppAccessToken`（以官方文档为准；均可通过环境变量覆盖）。当前已包含 Webhook Ed25519 challenge 与事件验签模块，以及独立 `libqqbot_sdk` 共享库目标（libcurl）。C++ SDK 为头文件封装；易语言使用 Windows DLL ABI，需 Windows 工具链构建/验证。尚未加入 libwebsockets 依赖。WebSocket、插件 API、QQ 消息发送/自动回复和端到端测试仍未完成，不应视为完整版本。
