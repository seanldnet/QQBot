#pragma once
#include <string>
namespace qqbot {
struct Event {
    int schema_version{1};
    std::string event_type;
    std::string chat_type; // group / c2c / channel / unknown
    std::string chat_id;
    std::string user_id;
    std::string content;
    std::string timestamp;
    std::string event_id;
    std::string msg_id;
    bool has_msg_seq{false};
    int msg_seq{0};
    std::string raw_event_json;
};
}
