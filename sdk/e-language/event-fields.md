# 易语言事件数据字段映射

插件 HTTP 回调收到 JSON 后，按 sdk/event-schema.md 字段解析：
- schema_version：整数型，当前为 1
- event_type：文本型，QQ 官方事件名称
- chat_type：文本型，group / c2c / channel / unknown
- chat_id：文本型，group_openid / user_openid / channel_id
- user_id：文本型或 null
- content：文本型或 null
- event_id：文本型或 null
- msg_id：文本型或 null
- msg_seq：整数型或 null
- raw_event：JSON 对象，保留官方原始事件

处理未知字段时忽略，不要拒绝整个事件。当前宿主事件并发投递尚未完成，该契约先行固定。

## 群事件字段映射

| 官方事件 | chat_type | chat_id | user_id | msg_id |
| --- | --- | --- | --- | --- |
| GROUP_ADD_ROBOT | group | group_openid | op_member_openid | null |
| GROUP_DEL_ROBOT | group | group_openid | op_member_openid | null |
| GROUP_MSG_REJECT | group | group_openid | op_member_openid | null |
| GROUP_MSG_RECEIVE | group | group_openid | op_member_openid | null |
| GROUP_AT_MESSAGE_CREATE | group | group_openid | author.member_openid | id |

群管理事件 timestamp 是整数；群 @ 消息 timestamp 是 RFC3339 文本。完整字段、Intent 和权限说明见 sdk/group-events.md。解析器当前仅为宿主内部 C 函数，不提供新的 DLL 导出，事件 JSON 的投递功能尚未完成。
