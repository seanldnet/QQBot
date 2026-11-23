# 易语言 SDK 导入定义（Windows DLL）
# DLL 名称：qqbot_sdk.dll
# 所有函数均为 __stdcall；字符串参数为 ANSI/UTF-8 指针约定，建议统一传 UTF-8。
# qqb_create(host_url, plugin_id, plugin_token) -> 整数型（客户端句柄）
# qqb_destroy(client_handle) -> 无返回值
# qqb_api_request(client, method, path, body_json, response_json*, http_status*) -> 整数型
# qqb_send_group_text(client, group_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_c2c_text(client, user_openid, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_send_channel_text(client, channel_id, content, event_id, msg_id, msg_seq, response_json*, http_status*) -> 整数型
# qqb_free_string(response_json) -> 无返回值
# qqb_version() -> 文本型
#
# 注意：本文件是易语言 DLL 命令声明映射说明，不是可直接双击编译的 .e 源文件。
# 正式易语言模块源码/示例在 SDK 后续阶段补齐并使用易语言 IDE 实测。
