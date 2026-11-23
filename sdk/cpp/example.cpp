#include "../include/qqbot_sdk.h"
#include "qqbot.hpp"
#include "event.hpp"
#include <iostream>

int main() {
    try {
        qqbot::Client bot("https://your-qqbot-host.example", "plugin-id", "plugin-token");
        qqbot::Event event;
        event.chat_type = "group";
        event.msg_seq = 1;

        std::string response;
        long status = 0;

        // Generic method/path/body access covers every documented REST endpoint.
        int rc = bot.request("GET", "/users/@me", "", response, status);
        std::cout << "transport=" << rc << " http=" << status << "\n" << response << "\n";

        // Compile-time examples for typed APIs. Kept disabled to avoid network calls.
        if (false) {
            bot.list_guild_members("GUILD_ID", "0", 100, response, status);
            bot.get_guild_member("GUILD_ID", "USER_ID", response, status);
            bot.list_guild_roles("GUILD_ID", response, status);
            bot.create_guild_role("GUILD_ID", "{\"name\":\"member\"}", response, status);
            bot.mute_guild("GUILD_ID", "", "0", response, status);
            bot.mute_guild_member("GUILD_ID", "USER_ID", "", "60", response, status);
            bot.list_channel_schedules("CHANNEL_ID", "", response, status);
            bot.create_channel_schedule("CHANNEL_ID", "{\"schedule\":{}}", response, status);
            bot.audio_mic_up("VOICE_CHANNEL_ID", response, status);
        }
        // Passive message replies should include event_id and/or msg_id from the event.
        // Use qqb_send_group_text / the corresponding C++ wrapper overload.
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
