# 易语言 SDK 导入定义（Windows DLL）

推荐使用 x64 易语言程序加载 x64 `qqbot_sdk.dll`；x86 程序必须加载 x86 DLL。DLL 与调用进程位数必须一致。SDK 导出函数采用 `__stdcall`，文本参数按 UTF-8 NUL 结尾字节串传入；返回的 JSON 字符串必须通过 `qqb_free_string` 释放。
# DLL 名称：qqbot_sdk.dll
# 所有函数均为 __stdcall；字符串参数为 ANSI/UTF-8 指针约定，建议统一传 UTF-8。
# qqb_create(host_url, plugin_id, plugin_token) -> 整数型（客户端句柄）
# qqb_destroy(client_handle) -> 无返回值
# qqb_api_request(client, method, path, body_json, response_json*, http_status*) -> 整数型
# 参数说明：client 为 qqb_create 返回句柄；method 为 GET/POST/PUT/PATCH/DELETE；path 为 /v2/... 等官方相对路径；body_json 为合法 JSON 或空；response_json 和 http_status 均传址。
# qqb_send_group_text(client, group_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_c2c_text(client, user_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_dms_text(client, guild_id, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_channel_text(client, channel_id, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_free_string(response_json) -> 无返回值
# qqb_version() -> 文本型
#
# 注意：本文件是易语言 DLL 命令声明映射说明，不是可直接双击编译的 .e 源文件。
# 正式易语言模块源码/示例在 SDK 后续阶段补齐并使用易语言 IDE 实测。
