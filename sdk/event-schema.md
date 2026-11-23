# 插件统一事件模型

插件 callback_url 收到宿主 POST 的 application/json。宿主尚未完成事件投递器；此文件是跨语言 SDK 的事件契约，正式投递必须保留 raw_event，不丢失官方原始字段。

## 示例

{
  "schema_version": 1,
  "event_type": "GROUP_MSG_RECEIVE",
  "chat_type": "group",
  "chat_id": "GROUP_OPENID",
  "user_id": "USER_OPENID",
  "content": "hello",
  "event_id": null,
  "msg_id": null,
  "msg_seq": null,
  "raw_event": {}
}

## 字段说明

- event_type：原样保留 QQ 官方事件类型；不得擅自改写官方事件名。
- chat_type：宿主标准化值 group、c2c、channel；无法识别时为 unknown。
- chat_id：群场景使用 group_openid，单聊使用 user_openid，频道场景使用 channel_id。
- user_id：对应场景中的用户 openid；不存在时为 null。
- content：从官方消息事件中提取的文本内容；媒体/富文本仍完整保留于 raw_event。
- event_id、msg_id、msg_seq：被动回复所需上下文；只有官方事件明确提供时才填充，缺失时为 null。
- raw_event：官方原始 JSON 对象，供插件访问未标准化字段。

## QQ 群事件实现状态

详见 [group-events.md](group-events.md)。当前 C11 内部解析器支持四种群管理事件和 GROUP_AT_MESSAGE_CREATE；该解析器只完成 JSON 归一化，不代表 Gateway/WebSocket 接收或插件 HTTP 投递已实现。

## 兼容原则

新增字段向后兼容；插件必须忽略未知字段；不得把群号/QQ号当作 openid。事件结构需按官方最新事件文档逐类映射后再启用投递。
