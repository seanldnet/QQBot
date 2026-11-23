#pragma once
#include "qqbot_sdk.h"
#include <stdexcept>
#include <string>
#include <utility>
namespace qqbot {
class Client {
    qqb_client *p_{};
public:
    Client(const std::string& host,const std::string& plugin_id,const std::string& token)
      : p_(qqb_create(host.c_str(),plugin_id.c_str(),token.c_str())) {
        if(!p_) throw std::runtime_error("QQBot SDK initialization failed");
    }
    ~Client(){ if(p_) qqb_destroy(p_); }
    Client(const Client&)=delete; Client& operator=(const Client&)=delete;
    Client(Client&& o) noexcept : p_(std::exchange(o.p_,nullptr)) {}
    int request(const std::string& method,const std::string& path,const std::string& body,
                std::string& response,long& status) const {
        char *out=nullptr; int rc=qqb_api_request(p_,method.c_str(),path.c_str(),
          body.empty()?nullptr:body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);} return rc;
    }
    int get_guild(const std::string& id,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_guild(p_,id.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int list_channels(const std::string& guild_id,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_channels(p_,guild_id.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_channel(const std::string& id,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_channel(p_,id.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int create_channel(const std::string& guild_id,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_create_channel(p_,guild_id.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_channel(const std::string& id,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_channel(p_,id.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int delete_channel(const std::string& id,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_delete_channel(p_,id.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_group_message(const std::string& group_openid,const std::string& body,
                           std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_group_message(p_,group_openid.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_group_text(const std::string& id,const std::string& text,const std::string& event_id,
                        const std::string& msg_id,int seq,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_group_text(p_,id.c_str(),text.c_str(),
          event_id.empty()?nullptr:event_id.c_str(),msg_id.empty()?nullptr:msg_id.c_str(),seq,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_c2c_text(const std::string& id,const std::string& text,const std::string& event_id,
                      const std::string& msg_id,int seq,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_c2c_text(p_,id.c_str(),text.c_str(),
          event_id.empty()?nullptr:event_id.c_str(),msg_id.empty()?nullptr:msg_id.c_str(),seq,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_dms_text(const std::string& id,const std::string& text,const std::string& event_id,
                      const std::string& msg_id,int seq,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_dms_text(p_,id.c_str(),text.c_str(),
          event_id.empty()?nullptr:event_id.c_str(),msg_id.empty()?nullptr:msg_id.c_str(),seq,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_channel_text(const std::string& id,const std::string& text,const std::string& event_id,
                          const std::string& msg_id,int seq,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_channel_text(p_,id.c_str(),text.c_str(),
          event_id.empty()?nullptr:event_id.c_str(),msg_id.empty()?nullptr:msg_id.c_str(),seq,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_group_text(const std::string& id,const std::string& text,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_group_text(p_,id.c_str(),text.c_str(),nullptr,nullptr,1,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_c2c_text(const std::string& id,const std::string& text,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_c2c_text(p_,id.c_str(),text.c_str(),nullptr,nullptr,1,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int send_channel_text(const std::string& id,const std::string& text,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_send_channel_text(p_,id.c_str(),text.c_str(),nullptr,nullptr,1,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }

    int list_guild_members(const std::string& guild,const std::string& after,unsigned int limit,
                           std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_guild_members(p_,guild.c_str(),
          after.empty()?nullptr:after.c_str(),limit,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_guild_member(const std::string& guild,const std::string& user,
                         std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_guild_member(p_,guild.c_str(),user.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int list_guild_roles(const std::string& guild,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_guild_roles(p_,guild.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int create_guild_role(const std::string& guild,const std::string& body,
                          std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_create_guild_role(p_,guild.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_guild_role(const std::string& guild,const std::string& role,const std::string& body,
                          std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_guild_role(p_,guild.c_str(),role.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int delete_guild_role(const std::string& guild,const std::string& role,
                          std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_delete_guild_role(p_,guild.c_str(),role.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int mute_guild(const std::string& guild,const std::string& end_timestamp,const std::string& seconds,
                   std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_mute_guild(p_,guild.c_str(),
          end_timestamp.empty()?nullptr:end_timestamp.c_str(),seconds.empty()?nullptr:seconds.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int mute_guild_member(const std::string& guild,const std::string& user,
                          const std::string& end_timestamp,const std::string& seconds,
                          std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_mute_guild_member(p_,guild.c_str(),user.c_str(),
          end_timestamp.empty()?nullptr:end_timestamp.c_str(),seconds.empty()?nullptr:seconds.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }


    int list_channel_schedules(const std::string& channel,const std::string& since_ms,
                               std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_channel_schedules(p_,channel.c_str(),
          since_ms.empty()?nullptr:since_ms.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_channel_schedule(const std::string& channel,const std::string& schedule,
                             std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_channel_schedule(p_,channel.c_str(),schedule.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int create_channel_schedule(const std::string& channel,const std::string& body,
                                std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_create_channel_schedule(p_,channel.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_channel_schedule(const std::string& channel,const std::string& schedule,const std::string& body,
                                std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_channel_schedule(p_,channel.c_str(),schedule.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int delete_channel_schedule(const std::string& channel,const std::string& schedule,
                                std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_delete_channel_schedule(p_,channel.c_str(),schedule.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int audio_mic_up(const std::string& channel,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_audio_mic_up(p_,channel.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int audio_mic_down(const std::string& channel,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_audio_mic_down(p_,channel.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }


    int list_guild_role_members(const std::string& guild,const std::string& role,const std::string& start,
                                unsigned int limit,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_guild_role_members(p_,guild.c_str(),role.c_str(),
          start.empty()?nullptr:start.c_str(),limit,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int kick_guild_member(const std::string& guild,const std::string& user,const std::string& body,
                          std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_kick_guild_member(p_,guild.c_str(),user.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int add_guild_member_role(const std::string& guild,const std::string& user,const std::string& role,
                              const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_add_guild_member_role(p_,guild.c_str(),user.c_str(),role.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int remove_guild_member_role(const std::string& guild,const std::string& user,const std::string& role,
                                 const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_remove_guild_member_role(p_,guild.c_str(),user.c_str(),role.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_channel_member_permissions(const std::string& channel,const std::string& user,
                                        std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_channel_member_permissions(p_,channel.c_str(),user.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_channel_member_permissions(const std::string& channel,const std::string& user,const std::string& body,
                                           std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_channel_member_permissions(p_,channel.c_str(),user.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_channel_role_permissions(const std::string& channel,const std::string& role,
                                     std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_channel_role_permissions(p_,channel.c_str(),role.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_channel_role_permissions(const std::string& channel,const std::string& role,const std::string& body,
                                        std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_channel_role_permissions(p_,channel.c_str(),role.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }


    int get_group_info(const std::string& group,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_group_info(p_,group.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_group_bot_state(const std::string& group,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_group_bot_state(p_,group.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_group_mute_state(const std::string& group,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_group_mute_state(p_,group.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int set_group_member_mutes(const std::string& group,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_set_group_member_mutes(p_,group.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int list_group_members(const std::string& group,const std::string& cursor,unsigned int limit,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_group_members(p_,group.c_str(),cursor.empty()?nullptr:cursor.c_str(),limit,&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int get_group_member(const std::string& group,const std::string& member,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_get_group_member(p_,group.c_str(),member.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int batch_remove_group_members(const std::string& group,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_batch_remove_group_members(p_,group.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }

    int list_group_blacklist(const std::string& group,const std::string& cursor,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_group_blacklist(p_,group.c_str(),cursor.empty()?nullptr:cursor.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int update_group_blacklist(const std::string& group,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_update_group_blacklist(p_,group.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int list_group_join_requests(const std::string& group,const std::string& cursor,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_list_group_join_requests(p_,group.c_str(),cursor.empty()?nullptr:cursor.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
    int approve_group_join_request(const std::string& group,const std::string& member,const std::string& body,std::string& response,long& status) const {
        char*out=nullptr;int rc=qqb_approve_group_join_request(p_,group.c_str(),member.c_str(),body.c_str(),&out,&status);
        if(out){response.assign(out);qqb_free_string(out);}return rc;
    }
};
}
