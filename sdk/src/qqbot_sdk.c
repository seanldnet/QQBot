#include "qqbot_sdk.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
struct qqb_client { char *host; char *plugin_id; char *plugin_token; };
struct buffer { char *p; size_t n; };
static char *dupstr(const char *s) { size_t n; char *p; if(!s) return NULL; n=strlen(s); p=(char*)malloc(n+1); if(p) memcpy(p,s,n+1); return p; }
static size_t receive_data(char *ptr,size_t size,size_t nmemb,void *userdata) {
    struct buffer *b=(struct buffer*)userdata; size_t n=size*nmemb;
    if(n > 8U*1024U*1024U - b->n) return 0;
    char *p=(char*)realloc(b->p,b->n+n+1); if(!p) return 0;
    b->p=p; memcpy(b->p+b->n,ptr,n); b->n+=n; b->p[b->n]=0; return n;
}
qqb_client *QQB_CALL qqb_create(const char *host_url,const char *plugin_id,const char *plugin_token) {
    if(!host_url||!plugin_id||!plugin_token||!*host_url||!*plugin_id||!*plugin_token) return NULL;
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
    CURL *curl=NULL; CURLcode rc; struct curl_slist *headers=NULL; struct buffer b={0}; char *url=NULL,*escaped=NULL,*payload=NULL; long code=0; int ret=-1;
    if(response) *response=NULL; if(status) *status=0;
    if(!c||!method||!path||path[0]!='/'||!response||!status) return -1;
    if(strstr(path,"..")||strstr(path,"://")) return -2;
    curl=curl_easy_init(); if(!curl) return -3;
    escaped=curl_easy_escape(c,c->plugin_id,0); if(!escaped) goto done;
    size_t un=strlen(c->host)+strlen(escaped)+32; url=(char*)malloc(un);
    if(!url) goto done;
    snprintf(url,un,"%s/api/v1/plugins/%s/qq-api",c->host,escaped);
    /* The host proxy receives the exact upstream method/path/body as a JSON envelope. */
    size_t pn=strlen(method)+strlen(path)+(body?strlen(body):4)+64; payload=(char*)malloc(pn);
    if(!payload) goto done;
    snprintf(payload,pn,"{\"method\":\"%s\",\"path\":\"%s\",\"body\":%s}",method,path,body?body:"null");
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
    free(b.p); free(url); free(payload); if(escaped) curl_free(escaped);
    curl_slist_free_all(headers); curl_easy_cleanup(curl); return ret;
}
static int send_text(qqb_client*c,const char*path,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status) {
    if(!content) return -1;
    size_t n=strlen(content)*6+((event_id?strlen(event_id):0)+(msg_id?strlen(msg_id):0))*6+256;
    char *body=(char*)malloc(n); if(!body) return -3;
    /* JSON escaping is delegated to a small, complete string encoder. */
    char *q=(char*)malloc(strlen(content)*6+1); if(!q){free(body);return -3;}
    size_t k=0; for(const unsigned char*p=(const unsigned char*)content;*p;p++){
        if(*p=='"'||*p=='\\'){q[k++]='\\';q[k++]=(char)*p;}
        else if(*p<0x20){snprintf(q+k,7,"\\u%04x",*p);k+=6;}
        else q[k++]=(char)*p;
    } q[k]=0;
    snprintf(body,n,"{\"content\":\"%s\",\"msg_type\":0%s%s%s%s%s%s%s}",
      q,event_id?",\"event_id\":\"":"",event_id?event_id:"",event_id?"\"":"",
      msg_id?",\"msg_id\":\"":"",msg_id?msg_id:"",msg_id?"\"":"",
      msg_id?",\"msg_seq\":":"");
    if(msg_id){ size_t used=strlen(body); snprintf(body+used,n-used,"%d",seq>0?seq:1); }
    int rc=qqb_api_request(c,"POST",path,body,out,status); free(q);free(body);return rc;
}
static char *path_id(const char *prefix,const char *id,const char *suffix) {
    if(!id||!*id||strchr(id,'/')||strstr(id,"..")) return NULL;
    size_t n=strlen(prefix)+strlen(id)+strlen(suffix)+1; char*p=(char*)malloc(n);
    if(p) snprintf(p,n,"%s%s%s",prefix,id,suffix); return p;
}
int QQB_CALL qqb_send_group_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/v2/groups/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
int QQB_CALL qqb_send_c2c_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/v2/users/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
int QQB_CALL qqb_send_channel_text(qqb_client*c,const char*id,const char*content,const char*event_id,const char*msg_id,int seq,char**out,long*status){
    char*p=path_id("/channels/",id,"/messages");if(!p)return -1;int r=send_text(c,p,content,event_id,msg_id,seq,out,status);free(p);return r;
}
const char *QQB_CALL qqb_version(void){return "0.1.0";}
