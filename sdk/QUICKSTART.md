# QQBot 多语言插件 SDK 快速接入

## 1. 配置宿主

设置至少 24 字符的管理令牌并启动宿主：

```sh
export QQBOT_ADMIN_TOKEN='replace-with-a-long-random-secret-at-least-24-chars'
./build/qqbot
```

宿主默认仅监听 127.0.0.1:8080。生产环境请放在 HTTPS 反向代理后，并配置安全的管理令牌。不要把管理令牌或插件 Token 写进源码仓库。

## 2. 注册插件并获取插件凭据

```sh
curl -X POST 'http://127.0.0.1:8080/api/v1/plugins' \
  -H 'Authorization: Bearer replace-with-a-long-random-secret-at-least-24-chars' \
  -H 'Content-Type: application/json' \
  -d '{"name":"my-cpp-plugin"}'
```

响应包含 id 和 token。Token 只返回这一次，宿主数据库只保存 SHA-256 摘要；请安全保存。停用插件使用 PATCH，删除使用 DELETE，均需管理员 Bearer Token。

## 3. C++ 插件

链接 libqqbot_sdk，并包含 sdk/cpp/qqbot.hpp：

```cpp
qqbot::Client client("https://your-host.example", "PLUGIN_ID", "PLUGIN_TOKEN");
std::string response;
long http_status = 0;
int rc = client.request("GET", "/users/@me", "", response, http_status);
```

C++ SDK 要求 C++14。群聊、单聊、频道文字子频道和频道私信均提供文本发送便利函数；被动回复应传入 event_id/msg_id/msg_seq。

## 4. 易语言插件

1. 按目标进程位数构建 DLL：参阅 sdk/windows/README.md。
2. 易语言通过 DLL 命令导入 qqbot_sdk.dll 中的 qqb_create、qqb_api_request、消息发送函数和 qqb_free_string。
3. 先创建客户端句柄，再调用接口；返回 JSON 缓冲区用 qqb_free_string 释放；插件退出时调用 qqb_destroy。
4. x86 程序必须加载 x86 DLL；x64 程序必须加载 x64 DLL。字符串以 UTF-8 NUL 结尾字节串传入。

## 5. 官方 API 全覆盖策略

sdk/official-api-catalog.md 与 sdk/official-api-index.json 保存腾讯官方 API v2 文档目录（81 页）。C ABI 的通用 qqb_api_request(method,path,body_json,...) 可访问官方目录中定义的任意 REST 路由；C++ / 易语言共用该入口。常用接口以便利函数形式逐步补齐。每次官方文档增加接口，必须更新目录索引和变更日志。

## 6. 当前限制

- 宿主已实现插件注册、列表、启停、删除和官方 API 代理首版路由；尚未经过编译和端到端测试。
- 插件事件 callback_url 的安全 DNS 校验、事件并发投递和失败隔离仍待实现。
- 不是所有 API 都对所有机器人类型开放；调用失败时应检查官方权限、机器人类型、消息上下文、频控和错误码。
- 官方消息主动推送策略已调整；SDK 不绕过官方被动回复时效和频次限制。
- Windows DLL / 易语言 IDE 的编译和运行尚未实测。