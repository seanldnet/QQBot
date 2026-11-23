#include "callback_security.h"
#include <curl/curl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int in4(unsigned int ip, unsigned int net, unsigned int mask) {
    return (ip & mask) == net;
}
int qq_callback_ipv4_is_public(const char *address) {
    struct in_addr a; unsigned int ip;
    if (!address || inet_pton(AF_INET, address, &a) != 1) return 0;
    ip = ntohl(a.s_addr);
    if (in4(ip,0x00000000u,0xff000000u) || in4(ip,0x0a000000u,0xff000000u) ||
        in4(ip,0x64400000u,0xffc00000u) || in4(ip,0x7f000000u,0xff000000u) ||
        in4(ip,0xa9fe0000u,0xffff0000u) || in4(ip,0xac100000u,0xfff00000u) ||
        in4(ip,0xc0000000u,0xffffff00u) || in4(ip,0xc0000200u,0xffffff00u) ||
        in4(ip,0xc0586300u,0xffffff00u) || in4(ip,0xc0a80000u,0xffff0000u) ||
        in4(ip,0xc6120000u,0xfffe0000u) || in4(ip,0xc6336400u,0xffffff00u) ||
        in4(ip,0xcb007100u,0xffffff00u) || in4(ip,0xe0000000u,0xf0000000u) ||
        in4(ip,0xf0000000u,0xf0000000u)) return 0;
    return 1;
}
int qq_callback_ipv6_is_public(const char *address) {
    struct in6_addr a; const unsigned char *b;
    if (!address || inet_pton(AF_INET6,address,&a) != 1) return 0;
    b=a.s6_addr;
    if ((b[0] & 0xe0u) != 0x20u) return 0; /* only 2000::/3 */
    if (b[0]==0x20u && b[1]==0x01u && (b[2]&0xfeu)==0x00u) return 0;
    if (b[0]==0x20u && b[1]==0x01u && b[2]==0x0du && b[3]==0xb8u) return 0;
    if (b[0]==0x20u && b[1]==0x02u) return 0; /* 6to4 */
    if (b[0]==0x3fu && b[1]==0xffu) return 0; /* documentation */
    return 1;
}
static int part(CURLU *u, CURLUPart p, char *dst, size_t cap, unsigned int flags) {
    char *v=NULL; CURLUcode r=curl_url_get(u,p,&v,flags);
    if (r!=CURLUE_OK || !v || strlen(v)>=cap) { curl_free(v); return -1; }
    memcpy(dst,v,strlen(v)+1); curl_free(v); return 0;
}
int qq_callback_target_validate(const char *url, qq_callback_target *out) {
    CURLU *u=NULL; char scheme[16], port_text[16]; char *explicit_port=NULL;
    struct addrinfo hints, *list=NULL, *it; struct in_addr v4; struct in6_addr v6;
    size_t n=0; int rc=-1; long port;
    if (!url || !url[0] || !out || strlen(url)>2048) return -1;
    memset(out,0,sizeof(*out)); u=curl_url(); if (!u) return -1;
    if (curl_url_set(u,CURLUPART_URL,url,CURLU_DISALLOW_USER)!=CURLUE_OK ||
        part(u,CURLUPART_SCHEME,scheme,sizeof(scheme),0)!=0 ||
        part(u,CURLUPART_HOST,out->host,sizeof(out->host),0)!=0) goto done;
    if (strcmp(scheme,"https")!=0 && strcmp(scheme,"HTTPS")!=0) goto done;
    if (curl_url_get(u,CURLUPART_PORT,&explicit_port,0)==CURLUE_OK) {
        if (!explicit_port || strlen(explicit_port)>=sizeof(port_text)) goto done;
        strcpy(port_text,explicit_port);
    } else strcpy(port_text,"443");
    port=strtol(port_text,NULL,10); if (port!=443) goto done;
    out->port=443;
    if (inet_pton(AF_INET,out->host,&v4)==1 || inet_pton(AF_INET6,out->host,&v6)==1) goto done;
    if (strchr(out->host,'.')==NULL || strchr(out->host,'/')!=NULL) goto done;
    memset(&hints,0,sizeof(hints)); hints.ai_socktype=SOCK_STREAM;
    hints.ai_family=AF_UNSPEC; hints.ai_flags=AI_ADDRCONFIG;
    if (getaddrinfo(out->host,"443",&hints,&list)!=0 || !list) goto done;
    for (it=list;it;it=it->ai_next) {
        char ip[INET6_ADDRSTRLEN]; const void *src; int is6=0;
        if (it->ai_family==AF_INET) src=&((struct sockaddr_in*)it->ai_addr)->sin_addr;
        else if (it->ai_family==AF_INET6) { src=&((struct sockaddr_in6*)it->ai_addr)->sin6_addr; is6=1; }
        else continue;
        if (!inet_ntop(it->ai_family,src,ip,sizeof(ip))) goto done;
        if (!(is6?qq_callback_ipv6_is_public(ip):qq_callback_ipv4_is_public(ip))) goto done;
        if (n>=QQ_CALLBACK_MAX_ADDRESSES || strlen(ip)>=QQ_CALLBACK_IP_MAX) goto done;
        strcpy(out->addresses[n],ip); out->address_is_ipv6[n]=is6; ++n;
    }
    if (!n) goto done;
    out->address_count=n; rc=0;
done:
    if (explicit_port) curl_free(explicit_port);
    if (list) freeaddrinfo(list);
    curl_url_cleanup(u);
    if (rc) memset(out,0,sizeof(*out));
    return rc;
}

int qq_callback_target_resolve_entry(const qq_callback_target *target, char *out, size_t capacity) {
    size_t used = 0, i;
    int n;
    if (!target || !out || capacity == 0 || target->port != 443 ||
        !target->host[0] || target->address_count == 0 ||
        target->address_count > QQ_CALLBACK_MAX_ADDRESSES) return -1;
    n = snprintf(out, capacity, "%s:%u:", target->host, (unsigned int)target->port);
    if (n < 0 || (size_t)n >= capacity) return -1;
    used = (size_t)n;
    for (i = 0; i < target->address_count; ++i) {
        const char *ip = target->addresses[i];
        size_t len = strlen(ip);
        const char *prefix = i ? "," : "";
        size_t need = strlen(prefix) + len + (target->address_is_ipv6[i] ? 2u : 0u);
        if (!ip[0] || used + need >= capacity) return -1;
        if (i) out[used++] = ',';
        if (target->address_is_ipv6[i]) out[used++] = '[';
        memcpy(out + used, ip, len); used += len;
        if (target->address_is_ipv6[i]) out[used++] = ']';
        out[used] = '\\0';
    }
    return 0;
}
