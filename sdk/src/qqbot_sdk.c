#include "qqbot_sdk.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
struct qqb_client { char *host; char *plugin_id; char *plugin_token; };
struct buffer { char *p; size_t n; };
static int safe_header_value(const char *s) { return s && !strchr(s,'\r') && !strchr(s,'\n'); }
static char *json_quote(const char *s) {
    size_t n=strlen(s),cap=n*6+3; char *o=(char*)malloc(cap); if(!o)return NULL;
    size_t k=0;o[k++]='"';
    for(size_t i=0;i<n;i++){unsigned char ch=(unsigned char)s[i];
        if(ch=='"'||ch=='\\'){o[k++]='\\';o[k++]=(char)ch;}
        else if(ch<0x20){static const char h[]="0123456789abcdef";o[k++]='\\';o[k++]='u';o[k++]='0';o[k++]='0';o[k++]=h[ch>>4];o[k++]=h[ch&15];}
        else o[k++]=(char)ch;
    }o[k++]='"';o[k]=0;return o;
}
static char *dupstr(const char *s) { size_t n; char *p; if(!s) return NULL; n=strlen(s); p=(char*)malloc(n+1); if(p) memcpy(p,s,n+1); return p; }
static size_t receive_data(char *ptr,size_t size,size_t nmemb,void *userdata) {
    struct buffer *b=(struct buffer*)userdata; size_t n=size*nmemb;
    if(n > 8U*1024U*1024U - b->n) return 0;
    char *p=(char*)realloc(b->p,b->n+n+1); if(!p) return 0;
    b->p=p; memcpy(b->p+b->n,ptr,n); b->n+=n; b->p[b->n]=0; return n;
}
qqb_client *QQB_CALL qqb_create(const char *host_url,const char *plugin_id,const char *plugin_token) {
    if(!host_url||!plugin_id||!plugin_token||!*host_url||!*plugin_id||!*plugin_token||!safe_header_value(plugin_id)||!safe_header_value(plugin_token)) return NULL;
    if(strncmp(host_url,"https://",8)!=0 && strncmp(host_url,"http://127.0.0.1",16)!=0 && strncmp(host_url,"http://localhost",16)!=0) return NULL;
    if(curl_global_init(CURL_GLOBAL_DEFAULT)!=CURLE_OK) return NULL;
    qqb_client *c=(qqb_client*)calloc(1,sizeof(*c)); if(!c) return NULL;
    c->host=dupstr(host_url); c->plugin_id=dupstr(plugin_id); c->plugin_token=dupstr(plugin_token);
    if(!c->host||!c->plugin_id||!c->plugin_token) { qqb_destroy(c); return NULL; }
    while(strlen(c->host)>0 && c->host[strlen(c->host)-1]=='/') c->host[strlen(c->host)-1]=0;
    return c;
}
void QQB_CALL qqb_destroy(qqb_client *c) {
    if(!c) return;
    if(c->plugin_token) { volatile char *p=(volatile char*)c->plugin_token; size_t n=strlen(c->plugin_token); while(n--) *p++=0; }
    free(c->host); free(c->plugin_id); free(c->plugin_token); free(c);
}
void QQB_CALL qqb_free_string(char *p) { free(p); }
int QQB_CALL qqb_api_request(qqb_client *c,const char *method,const char *path,const char *body,char **response,long *status) {
    CURL *curl=NULL; CURLcode rc; struct curl_slist *headers=NULL; struct buffer b={0}; char *url=NULL,*escaped=NULL,*payload=NULL,*qm=NULL,*qp=NULL; long code=0; int ret=-1;
    if(response) *response=NULL; if(status) *status=0;
    if(!c||!method||!path||path[0]!='/'||!response||!status) return -1;
    if((strcmp(method,"GET")&&strcmp(method,"POST")&&strcmp(method,"PUT")&&strcmp(method,"PATCH")&&strcmp(method,"DELETE"))||strstr(path,"..")||strstr(path,"://")) return -2;
    curl=curl_easy_init(); if(!curl) return -3;
    escaped=curl_easy_escape(c,c->plugin_id,0); if(!escaped) goto done;
    size_t un=strlen(c->host)+strlen(escaped)+32; url=(char*)malloc(un);
    if(!url) goto done;
    snprintf(url,un,"%s/api/v1/plugins/%s/qq-api",c->host,escaped);
    /* The host proxy receives the exact upstream method/path/body as a JSON envelope. */
    qm=json_quote(method); qp=json_quote(path); if(!qm||!qp) goto done;
    size_t pn=strlen(qm)+strlen(qp)+(body?strlen(body):4)+64; payload=(char*)malloc(pn);
    if(!payload) goto done;
    snprintf(payload,pn,"{\"method\":%s,\"path\":%s,\"body\":%s}",qm,qp,body?body:"null");
    headers=curl_slist_append(headers,"Content-Type: application/json");
    char *h1=NULL,*h2=NULL; size_t hn=strlen(c->plugin_id)+32; h1=(char*)malloc(hn);
    hn=strlen(c->plugin_token)+32; h2=(char*)malloc(hn);
    if(!h1||!h2) { free(h1); free(h2); goto done; }
    snprintf(h1,hn,"X-Plugin-ID: %s",c->plugin_id); snprintf(h2,hn,"X-Plugin-Token: %s",c->plugin_token);
    headers=curl_slist_append(headers,h1); headers=curl_slist_append(headers,h2); free(h1); free(h2);
    curl_easy_setopt(curl,CURLOPT_URL,url); curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(curl,CURLOPT_POST,1L); curl_easy_setopt(curl,CURLOPT_POSTFIELDS,payload);
    curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,(long)strlen(payload));
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,receive_data); curl_easy_setopt(curl,CURLOPT_WRITEDATA,&b);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,5L); curl_easy_setopt(curl,CURLOPT_TIMEOUT,15L);
    curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L); curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,0L);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L); curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
    rc=curl_easy_perform(curl); if(rc!=CURLE_OK) { ret=-4; goto done; }
    curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&code); *status=code;
    *response=b.p?b.p:dupstr(""); b.p=NULL; ret=0;
done:
    free(b.p); free(url); free(payload); free(qm); free(qp); if(escaped) curl_free(escaped);
    curl_slist_free_all(headers); curl_easy_cleanup(curl); return ret;
}
static int send_text(qqb_client*c,const char*path,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status) {
    if(!content) return -1;
    char *qc=json_quote(content), *qe=event_id?json_quote(event_id):NULL, *qm=msg_id?json_quote(msg_id):NULL;
    if(!qc||(event_id&&!qe)||(msg_id&&!qm)){free(qc);free(qe);free(qm);return -3;}
    size_t n=strlen(qc)+(qe?strlen(qe):0)+(qm?strlen(qm):0)+256;
    char *body=(char*)malloc(n); if(!body){free(qc);free(qe);free(qm);return -3;}
    snprintf(body,n,"{\"content\":%s,\"msg_type\":0%s%s%s%s%s}",
      qc,qe?",\"event_id\":":"",qe?qe:"",
      qm?",\"msg_id\":":"",qm?qm:"",
      qm?",\"msg_seq\":1":"");
    if(qm && seq>0){size_t used=strlen(body);char *p=strstr(body,"\"msg_seq\":1");if(p)snprintf(p,n-(size_t)(p-body),"\"msg_seq\":%d",seq);}
    int rc=qqb_api_request(c,"POST",path,body,out,status);
    free(qc);free(qe);free(qm);free(body);return rc;
}
static char *path_id(const char *prefix,const char *id,const char *suffix) {
    if(!id||!*id||strchr(id,'/')||strstr(id,"..")) return NULL;
    size_t n=strlen(prefix)+strlen(id)+strlen(suffix)+1; char*p=(char*)malloc(n);
    if(p) snprintf(p,n,"%s%s%s",prefix,id,suffix); return p;
}
static int valid_path_id(const char *id);
int QQB_CALL qqb_send_group_message(qqb_client*c,const char*id,const char*body,char**out,long*status){
    if(!valid_path_id(id)||!body||body[0]!='{') return -1;
    char *p=path_id("/v2/groups/",id,"/messages");if(!p)return -1;
    int r=qqb_api_request(c,"POST",p,body,out,status);free(p);return r;
}
int QQB_CALL qqb_send_group_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/v2/groups/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
int QQB_CALL qqb_send_c2c_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/v2/users/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
int QQB_CALL qqb_send_dms_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/dms/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
int QQB_CALL qqb_send_channel_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/channels/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
static int valid_path_id(const char *id) {
    const unsigned char *p=(const unsigned char*)id;
    if(!p||!*p) return 0;
    for(;*p;p++) if(!((*p>='0'&&*p<='9')||(*p>='A'&&*p<='Z')||
                       (*p>='a'&&*p<='z')||*p=='_'||*p=='-')) return 0;
    return 1;
}
static int endpoint_call(qqb_client *c,const char *method,const char *prefix,
                         const char *id,const char *suffix,const char *body,
                         char **out,long *status) {
    if(out) *out=NULL;
    if(status) *status=0;
    if(!valid_path_id(id)) return -1;
    size_t n=strlen(prefix)+strlen(id)+strlen(suffix)+1;
    char *path=(char*)malloc(n);
    if(!path) return -3;
    snprintf(path,n,"%s%s%s",prefix,id,suffix);
    int rc=qqb_api_request(c,method,path,body,out,status);
    free(path);
    return rc;
}
int QQB_CALL qqb_get_guild(qqb_client*c,const char*id,char**out,long*status) {
    return endpoint_call(c,"GET","/guilds/",id,"",NULL,out,status);
}
int QQB_CALL qqb_list_channels(qqb_client*c,const char*id,char**out,long*status) {
    return endpoint_call(c,"GET","/guilds/",id,"/channels",NULL,out,status);
}
int QQB_CALL qqb_get_channel(qqb_client*c,const char*id,char**out,long*status) {
    return endpoint_call(c,"GET","/channels/",id,"",NULL,out,status);
}
int QQB_CALL qqb_create_channel(qqb_client*c,const char*id,const char*body,char**out,long*status) {
    if(!body||body[0]!='{') return -1;
    return endpoint_call(c,"POST","/guilds/",id,"/channels",body,out,status);
}
int QQB_CALL qqb_update_channel(qqb_client*c,const char*id,const char*body,char**out,long*status) {
    if(!body||body[0]!='{') return -1;
    return endpoint_call(c,"PATCH","/channels/",id,"",body,out,status);
}
int QQB_CALL qqb_delete_channel(qqb_client*c,const char*id,char**out,long*status) {
    return endpoint_call(c,"DELETE","/channels/",id,"",NULL,out,status);
}

static int valid_decimal_string(const char *s) {
    if(!s || !*s) return 0;
    for(const unsigned char *p=(const unsigned char*)s; *p; ++p)
        if(*p<'0'||*p>'9') return 0;
    return 1;
}
int QQB_CALL qqb_list_guild_members(qqb_client*c,const char*guild,const char*after,
    unsigned int limit,char**out,long*status) {
    if(!valid_path_id(guild)|| (after && !valid_path_id(after)) || limit<1 || limit>400) return -1;
    if(!after) after="0";
    char path[512];
    int n=snprintf(path,sizeof(path),"/guilds/%s/members?after=%s&limit=%u",guild,after,limit);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_get_guild_member(qqb_client*c,const char*guild,const char*user,
    char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(user)) return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/guilds/%s/members/%s",guild,user);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_list_guild_roles(qqb_client*c,const char*guild,char**out,long*status) {
    return endpoint_call(c,"GET","/guilds/",guild,"/roles",NULL,out,status);
}
int QQB_CALL qqb_create_guild_role(qqb_client*c,const char*guild,const char*body,
    char**out,long*status) {
    if(!body||body[0]!='{') return -1;
    return endpoint_call(c,"POST","/guilds/",guild,"/roles",body,out,status);
}
int QQB_CALL qqb_update_guild_role(qqb_client*c,const char*guild,const char*role,
    const char*body,char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(role)||!body||body[0]!='{') return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/guilds/%s/roles/%s",guild,role);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"PATCH",path,body,out,status);
}
int QQB_CALL qqb_delete_guild_role(qqb_client*c,const char*guild,const char*role,
    char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(role)) return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/guilds/%s/roles/%s",guild,role);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"DELETE",path,NULL,out,status);
}
static int mute_request(qqb_client*c,const char*path,const char*end,const char*seconds,
    char**out,long*status) {
    if((!end||!*end)&&(!seconds||!*seconds)) return -1;
    if(end&&!valid_decimal_string(end)) return -1;
    if(seconds&&!valid_decimal_string(seconds)) return -1;
    char *qe=end?json_quote(end):NULL, *qs=seconds?json_quote(seconds):NULL;
    if((end&&!qe)||(seconds&&!qs)){free(qe);free(qs);return -3;}
    size_t cap=(qe?strlen(qe):0)+(qs?strlen(qs):0)+96;
    char *body=(char*)malloc(cap); if(!body){free(qe);free(qs);return -3;}
    snprintf(body,cap,"{%s%s%s%s%s}",
      qe?"\"mute_end_timestamp\":":"",qe?qe:"",
      (qe&&qs)?",":"",qs?"\"mute_seconds\":":"",qs?qs:"");
    int rc=qqb_api_request(c,"PATCH",path,body,out,status);
    free(qe);free(qs);free(body);return rc;
}
int QQB_CALL qqb_mute_guild(qqb_client*c,const char*guild,const char*end,const char*seconds,
    char**out,long*status) {
    if(!valid_path_id(guild)) return -1;
    char path[384]; int n=snprintf(path,sizeof(path),"/guilds/%s/mute",guild);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return mute_request(c,path,end,seconds,out,status);
}
int QQB_CALL qqb_mute_guild_member(qqb_client*c,const char*guild,const char*user,
    const char*end,const char*seconds,char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(user)) return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/guilds/%s/members/%s/mute",guild,user);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return mute_request(c,path,end,seconds,out,status);
}


int QQB_CALL qqb_list_channel_schedules(qqb_client*c,const char*channel,const char*since,
    char**out,long*status) {
    if(!valid_path_id(channel)||(since&&!valid_decimal_string(since))) return -1;
    char path[512]; int n;
    if(since&&*since) n=snprintf(path,sizeof(path),"/channels/%s/schedules?since=%s",channel,since);
    else n=snprintf(path,sizeof(path),"/channels/%s/schedules",channel);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_get_channel_schedule(qqb_client*c,const char*channel,const char*schedule,
    char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(schedule)) return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/channels/%s/schedules/%s",channel,schedule);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_create_channel_schedule(qqb_client*c,const char*channel,const char*body,
    char**out,long*status) {
    if(!body||body[0]!='{') return -1;
    return endpoint_call(c,"POST","/channels/",channel,"/schedules",body,out,status);
}
int QQB_CALL qqb_update_channel_schedule(qqb_client*c,const char*channel,const char*schedule,
    const char*body,char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(schedule)||!body||body[0]!='{') return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/channels/%s/schedules/%s",channel,schedule);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"PATCH",path,body,out,status);
}
int QQB_CALL qqb_delete_channel_schedule(qqb_client*c,const char*channel,const char*schedule,
    char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(schedule)) return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/channels/%s/schedules/%s",channel,schedule);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"DELETE",path,NULL,out,status);
}
int QQB_CALL qqb_audio_mic_up(qqb_client*c,const char*channel,char**out,long*status) {
    return endpoint_call(c,"PUT","/channels/",channel,"/mic","{}",out,status);
}
int QQB_CALL qqb_audio_mic_down(qqb_client*c,const char*channel,char**out,long*status) {
    return endpoint_call(c,"DELETE","/channels/",channel,"/mic",NULL,out,status);
}


int QQB_CALL qqb_list_guild_role_members(qqb_client*c,const char*guild,const char*role,
    const char*start,unsigned int limit,char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(role)||(start&&!valid_path_id(start))||limit<1||limit>400) return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/guilds/%s/roles/%s/members?start_index=%s&limit=%u",
        guild,role,(start&&*start)?start:"0",limit);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_kick_guild_member(qqb_client*c,const char*guild,const char*user,
    const char*body,char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(user)||!body||body[0]!='{') return -1;
    char path[512]; int n=snprintf(path,sizeof(path),"/guilds/%s/members/%s",guild,user);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"DELETE",path,body,out,status);
}
static int member_role_request(qqb_client*c,const char*method,const char*guild,const char*user,
    const char*role,const char*body,char**out,long*status) {
    if(!valid_path_id(guild)||!valid_path_id(user)||!valid_path_id(role)||(body&&body[0]!='{')) return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/guilds/%s/members/%s/roles/%s",guild,user,role);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,method,path,body,out,status);
}
int QQB_CALL qqb_add_guild_member_role(qqb_client*c,const char*g,const char*u,const char*r,
    const char*b,char**o,long*s) {
    if(!b||b[0]!='{') return -1;
    return member_role_request(c,"PUT",g,u,r,b,o,s);
}
int QQB_CALL qqb_remove_guild_member_role(qqb_client*c,const char*g,const char*u,const char*r,
    const char*b,char**o,long*s) {
    if(!b||b[0]!='{') return -1;
    return member_role_request(c,"DELETE",g,u,r,b,o,s);
}
int QQB_CALL qqb_get_channel_member_permissions(qqb_client*c,const char*channel,const char*user,
    char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(user)) return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/channels/%s/members/%s/permissions",channel,user);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_update_channel_member_permissions(qqb_client*c,const char*channel,const char*user,
    const char*body,char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(user)||!body||body[0]!='{') return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/channels/%s/members/%s/permissions",channel,user);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"PUT",path,body,out,status);
}
int QQB_CALL qqb_get_channel_role_permissions(qqb_client*c,const char*channel,const char*role,
    char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(role)) return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/channels/%s/roles/%s/permissions",channel,role);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"GET",path,NULL,out,status);
}
int QQB_CALL qqb_update_channel_role_permissions(qqb_client*c,const char*channel,const char*role,
    const char*body,char**out,long*status) {
    if(!valid_path_id(channel)||!valid_path_id(role)||!body||body[0]!='{') return -1;
    char path[640]; int n=snprintf(path,sizeof(path),"/channels/%s/roles/%s/permissions",channel,role);
    if(n<0||(size_t)n>=sizeof(path)) return -1;
    return qqb_api_request(c,"PUT",path,body,out,status);
}


/* Group APIs: group_openid/member_openid are not guild_id/user_id. */
static char *encode_query_value(const char *value) {
    CURL *c; char *escaped, *copy;
    if (!value) return NULL;
    c = curl_easy_init(); if (!c) return NULL;
    escaped = curl_easy_escape(c, value, 0);
    copy = escaped ? dupstr(escaped) : NULL;
    if (escaped) curl_free(escaped);
    curl_easy_cleanup(c); return copy;
}
int QQB_CALL qqb_get_group_info(qqb_client*c,const char*g,char**out,long*status) {
    return endpoint_call(c,"GET","/v2/groups/",g,"/info",NULL,out,status);
}
int QQB_CALL qqb_get_group_bot_state(qqb_client*c,const char*g,char**out,long*status) {
    return endpoint_call(c,"GET","/v2/groups/",g,"/bot_state",NULL,out,status);
}
int QQB_CALL qqb_get_group_mute_state(qqb_client*c,const char*g,char**out,long*status) {
    return endpoint_call(c,"GET","/v2/groups/",g,"/restrict_chat_setting",NULL,out,status);
}
int QQB_CALL qqb_set_group_member_mutes(qqb_client*c,const char*g,const char*body,char**out,long*status) {
    if(!body || body[0]!='{') return -1;
    return endpoint_call(c,"POST","/v2/groups/",g,"/restrict_chat_setting",body,out,status);
}
int QQB_CALL qqb_list_group_members(qqb_client*c,const char*g,const char*cursor,unsigned int limit,char**out,long*status) {
    char *base=NULL,*encoded=NULL,*path=NULL; size_t n; int rc;
    if(!valid_path_id(g) || limit<1 || limit>30) return -1;
    base=path_id("/v2/groups/",g,"/members"); if(!base) return -3;
    if(cursor && *cursor) { encoded=encode_query_value(cursor); if(!encoded) { free(base); return -3; } }
    n=strlen(base)+(encoded?strlen(encoded):0)+64; path=(char*)malloc(n);
    if(!path) { free(base); free(encoded); return -3; }
    if(encoded) snprintf(path,n,"%s?cursor=%s&limit=%u",base,encoded,limit);
    else snprintf(path,n,"%s?limit=%u",base,limit);
    rc=qqb_api_request(c,"GET",path,NULL,out,status);
    free(base); free(encoded); free(path); return rc;
}
int QQB_CALL qqb_get_group_member(qqb_client*c,const char*g,const char*m,char**out,long*status) {
    char *base=NULL,*path=NULL; size_t n; int rc;
    if(!valid_path_id(g)||!valid_path_id(m)) return -1;
    base=path_id("/v2/groups/",g,"/members/"); if(!base) return -3;
    n=strlen(base)+strlen(m)+1; path=(char*)malloc(n);
    if(!path) { free(base); return -3; }
    snprintf(path,n,"%s%s",base,m);
    rc=qqb_api_request(c,"GET",path,NULL,out,status);
    free(base); free(path); return rc;
}
int QQB_CALL qqb_batch_remove_group_members(qqb_client*c,const char*g,const char*body,char**out,long*status) {
    if(!body || body[0]!='{') return -1;
    return endpoint_call(c,"POST","/v2/groups/",g,"/batch_remove_members",body,out,status);
}


int QQB_CALL qqb_list_group_blacklist(qqb_client*c,const char*g,const char*cursor,char**out,long*status) {
    char *base=NULL,*encoded=NULL,*path=NULL; size_t n; int rc;
    if(!valid_path_id(g)) return -1;
    base=path_id("/v2/groups/",g,"/member_blacklist"); if(!base) return -3;
    if(cursor && *cursor) { encoded=encode_query_value(cursor); if(!encoded) { free(base); return -3; } }
    n=strlen(base)+(encoded?strlen(encoded):0)+40; path=(char*)malloc(n);
    if(!path) { free(base); free(encoded); return -3; }
    if(encoded) snprintf(path,n,"%s?limit=100&cursor=%s",base,encoded);
    else snprintf(path,n,"%s?limit=100",base);
    rc=qqb_api_request(c,"GET",path,NULL,out,status);
    free(base); free(encoded); free(path); return rc;
}
int QQB_CALL qqb_update_group_blacklist(qqb_client*c,const char*g,const char*body,char**out,long*status) {
    if(!body || body[0]!='{') return -1;
    return endpoint_call(c,"POST","/v2/groups/",g,"/member_blacklist",body,out,status);
}
int QQB_CALL qqb_list_group_join_requests(qqb_client*c,const char*g,const char*cursor,char**out,long*status) {
    char *base=NULL,*encoded=NULL,*path=NULL; size_t n; int rc;
    if(!valid_path_id(g)) return -1;
    base=path_id("/v2/groups/",g,"/join_request_list"); if(!base) return -3;
    if(cursor && *cursor) { encoded=encode_query_value(cursor); if(!encoded) { free(base); return -3; } }
    n=strlen(base)+(encoded?strlen(encoded):0)+40; path=(char*)malloc(n);
    if(!path) { free(base); free(encoded); return -3; }
    if(encoded) snprintf(path,n,"%s?limit=50&cursor=%s",base,encoded);
    else snprintf(path,n,"%s?limit=50",base);
    rc=qqb_api_request(c,"GET",path,NULL,out,status);
    free(base); free(encoded); free(path); return rc;
}
int QQB_CALL qqb_approve_group_join_request(qqb_client*c,const char*g,const char*m,const char*body,char**out,long*status) {
    char *base=NULL,*path=NULL; size_t n; int rc;
    if(!valid_path_id(g)||!valid_path_id(m)||!body||body[0]!='{') return -1;
    base=path_id("/v2/groups/",g,"/approval_join_request/"); if(!base) return -3;
    n=strlen(base)+strlen(m)+1; path=(char*)malloc(n);
    if(!path) { free(base); return -3; }
    snprintf(path,n,"%s%s",base,m);
    rc=qqb_api_request(c,"POST",path,body,out,status);
    free(base); free(path); return rc;
}

const char *QQB_CALL qqb_version(void){return "0.2.0";}
