# 易语言插件调用示例（逻辑示意）
1. 使用“DLL命令”导入 `qqbot_sdk.dll` 中的 qqb_create，传入宿主 HTTPS 地址、插件ID、插件Token，保存返回句柄。
2. 调用 qqb_api_request：
   - method = "GET"
   - path = "/users/@me"（仅示例；需有对应接口权限）
   - body_json = 空文本或 NULL
   - response_json、http_status 使用传址参数接收。
3. 读取 response_json 后，必须调用 qqb_free_string 释放。
4. 插件退出时调用 qqb_destroy 释放句柄。
5. 发送群聊被动回复时，优先使用 qqb_send_group_text，并传入事件上下文 event_id / msg_id / msg_seq。

注意：此文档给出 ABI 调用顺序，不冒充已由易语言 IDE 编译的工程文件。当前执行环境没有易语言 IDE，实际 DLL 导入声明和 x86/x64 运行需在 Windows 易语言环境验收。
