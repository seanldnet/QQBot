#include "../include/qqbot_sdk.h"
#include "qqbot.hpp"
#include "event.hpp"
#include <iostream>
int main() {
    try {
        qqbot::Client bot("https://your-qqbot-host.example", "plugin-id", "plugin-token");
        qqbot::Event event; event.chat_type = "group"; event.msg_seq = 1;
        std::string response; long status = 0;
        // Generic access: this same method exposes every officially documented REST route.
        int rc = bot.request("GET", "/users/@me", "", response, status);
        std::cout << "transport=" << rc << " http=" << status << "\n" << response << "\n";
        // Typed channel management helpers are available for verified routes.\n        // int list_rc = bot.list_channels("GUILD_ID", response, status);\n        // Passive replies should include event_id and/or msg_id. Use the C ABI overload
        // qqb_send_group_text when those fields are required by the event context.
    } catch (const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
