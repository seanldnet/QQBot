# QQBot 多语言 SDK

快速接入请先阅读 [SDK QUICKSTART](QUICKSTART.md)，Windows 易语言 DLL 构建见 [sdk/windows](windows/README.md)。

- `include/qqbot_sdk.h`：稳定 C ABI 导出接口。
- `src/qqbot_sdk.c`：libcurl 实现，插件 ID + Token 调用宿主代理。
- `cpp/qqbot.hpp`：C++ RAII 封装。
- `e-language/README.md`：易语言 DLL 导入映射与 ABI 约定。
- `official-api-catalog.md`：官方 API 文档全量目录索引。

SDK 的通用 `qqb_api_request` 以官方 API path 为参数，允许 SDK 覆盖目录中所有官方接口；宿主端必须执行插件权限校验、路径白名单/参数校验及速率限制。宿主已加入 `/api/v1/plugins` 注册/查询、`PATCH/DELETE /api/v1/plugins/{id}` 和 `/api/v1/plugins/{id}/qq-api` 首版路由。设置环境变量 `QQBOT_ADMIN_TOKEN`（至少 24 字符），使用 `Authorization: Bearer <管理令牌>` 注册插件；插件 Token 只返回一次。SDK 源码与代理契约已接通，但尚未取得编译/端到端测试结果。

默认使用宿主 URL，不将 AppSecret / access_token 下发到第三方插件。插件凭据由宿主管理。响应内存由 SDK 分配，必须调用 `qqb_free_string` 释放。
