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
};
}
