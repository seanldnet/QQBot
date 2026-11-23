#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <commctrl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "comctl32.lib")

#define ID_APPID 101
#define ID_SECRET 102
#define ID_MODE 103
#define ID_LOGIN 104
#define ID_STATUS 105
#define ID_LOGS 106
#define ID_HOME 107
#define ID_MESSAGES 108
#define ID_SETTINGS 109
#define TIMER_REFRESH 1

static HINSTANCE g_instance;
static HWND g_main, g_appid, g_secret, g_mode, g_login, g_status, g_logs;
static HWND g_home, g_messages, g_settings;
static HANDLE g_core_process;
static char g_admin_token[65];
static char g_root[MAX_PATH], g_core_path[MAX_PATH];
static BOOL g_logged_in;

static void set_text(HWND h, const char *s) { SetWindowTextA(h, s ? s : ""); }
static void append_text(HWND h, const char *s) {
    int n=GetWindowTextLengthA(h); SendMessageA(h, EM_SETSEL, n, n);
    SendMessageA(h, EM_REPLACESEL, FALSE, (LPARAM)(s?s:""));
}
static void make_paths(void) {
    GetModuleFileNameA(NULL,g_core_path,sizeof(g_core_path));
    char *p=strrchr(g_core_path,'\\'); if(p)*p=0;
    snprintf(g_root,sizeof(g_root),"%s",g_core_path);
    p=strrchr(g_root,'\\'); if(p)*p=0;
    strncat(g_core_path,"\\qqbot-core.exe",sizeof(g_core_path)-strlen(g_core_path)-1);
}
static int start_core(void) {
    char raw[32], token[65], env[32768], exe[MAX_PATH+8], cwd[MAX_PATH];
    if(BCRYPT_SUCCESS(BCryptGenRandom(NULL,(PUCHAR)raw,sizeof(raw),BCRYPT_USE_SYSTEM_PREFERRED_RNG))==0)return -1;
    static const char hex[]="0123456789abcdef";
    for(int i=0;i<32;i++){token[i*2]=hex[(unsigned char)raw[i]>>4];token[i*2+1]=hex[(unsigned char)raw[i]&15];}
    token[64]=0; strcpy(g_admin_token,token);
    LPCH inherited=GetEnvironmentStringsA();
    if(!inherited)return -1;
    size_t used=0;
    for(LPCH item=inherited;*item;item+=strlen(item)+1){
        size_t len=strlen(item)+1;
        if(used+len+256>=sizeof(env)){FreeEnvironmentStringsA(inherited);return -1;}
        memcpy(env+used,item,len);used+=len;
    }
    FreeEnvironmentStringsA(inherited);
    char *q=env+used;
    q+=sprintf(q,"QQBOT_ADMIN_TOKEN=%s",g_admin_token)+1;
    q+=sprintf(q,"QQBOT_BIND=127.0.0.1")+1;
    q+=sprintf(q,"QQBOT_PORT=8080")+1;
    char ca_path[MAX_PATH+32];
    snprintf(ca_path,sizeof(ca_path),"CURL_CA_BUNDLE=%s\\certs\\ca-bundle.crt",g_root);
    q+=sprintf(q,"%s",ca_path)+1; *q=0;
    snprintf(exe,sizeof(exe),"\"%s\"",g_core_path);
    snprintf(cwd,sizeof(cwd),"%s",g_root);
    STARTUPINFOA si; PROCESS_INFORMATION pi; ZeroMemory(&si,sizeof(si)); ZeroMemory(&pi,sizeof(pi)); si.cb=sizeof(si);
    if(!CreateProcessA(NULL,exe,NULL,NULL,FALSE,CREATE_NO_WINDOW,(LPVOID)env,cwd,&si,&pi)) {
        /* MinGW builds use an ANSI environment block. */
        if(!CreateProcessA(NULL,exe,NULL,NULL,FALSE,CREATE_NO_WINDOW,env,cwd,&si,&pi))return -1;
    }
    CloseHandle(pi.hThread); g_core_process=pi.hProcess; return 0;
}
static int http_request(const char *method,const char *path,const char *body,char *out,DWORD cap) {
    HINTERNET s=NULL,c=NULL,r=NULL; DWORD got=0,total=0; BOOL ok=FALSE;
    s=WinHttpOpen(L"QQBotDesktop/0.1",WINHTTP_ACCESS_TYPE_NO_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
    if(!s)goto done;
    c=WinHttpConnect(s,L"127.0.0.1",8080,0); if(!c)goto done;
    wchar_t wpath[256]; MultiByteToWideChar(CP_UTF8,0,path,-1,wpath,256);
    wchar_t wmethod[16]; MultiByteToWideChar(CP_UTF8,0,method,-1,wmethod,16);
    r=WinHttpOpenRequest(c,wmethod,wpath,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0); if(!r)goto done;
    wchar_t auth[128]; swprintf(auth,128,L"Authorization: Bearer %S",g_admin_token);
    WinHttpAddRequestHeaders(r,auth,(DWORD)-1,WINHTTP_ADDREQ_FLAG_ADD);
    const char *payload=body?body:""; DWORD len=(DWORD)strlen(payload);
    if(!WinHttpSendRequest(r,L"Content-Type: application/json\r\n", (DWORD)-1,(LPVOID)payload,len,len,0))goto done;
    if(!WinHttpReceiveResponse(r,NULL))goto done;
    total=0; out[0]=0;
    do { got=0; if(!WinHttpQueryDataAvailable(r,&got)||!got)break;
        if(total+got>=cap)got=cap-total-1;
        if(got && !WinHttpReadData(r,out+total,got,&got))goto done;
        total+=got; out[total]=0;
    } while(got && total<cap-1);
    DWORD status=0,sz=sizeof(status); WinHttpQueryHeaders(r,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&sz,WINHTTP_NO_HEADER_INDEX);
    ok=status>=200&&status<300;
done:
    if(r)WinHttpCloseHandle(r); if(c)WinHttpCloseHandle(c); if(s)WinHttpCloseHandle(s); return ok?0:-1;
}
static void json_escape(const char *src,char *dst,size_t cap) {
    size_t n=0; if(cap)dst[0]=0;
    for(;*src&&n+2<cap;src++){unsigned char c=(unsigned char)*src;
        if(c=='"'||c=='\\'){dst[n++]='\\';dst[n++]=(char)c;}
        else if(c=='\r'||c=='\n'){dst[n++]=' ';} else if(c>=32)dst[n++]=(char)c;
    } dst[n]=0;
}
static void show_dashboard(void) {
    ShowWindow(g_appid,SW_HIDE); ShowWindow(g_secret,SW_HIDE); ShowWindow(g_mode,SW_HIDE); ShowWindow(g_login,SW_HIDE);
    ShowWindow(g_status,SW_SHOW); ShowWindow(g_logs,SW_SHOW);
    ShowWindow(g_home,SW_SHOW); ShowWindow(g_messages,SW_SHOW); ShowWindow(g_settings,SW_SHOW);
    SetWindowTextA(g_main,"QQBot 官方机器人桌面框架");
    g_logged_in=TRUE; SetTimer(g_main,TIMER_REFRESH,1500,NULL);
}
static void refresh_data(void) {
    char buf[32768],status[2048];
    if(http_request("GET","/api/v1/status",NULL,status,sizeof(status))==0)set_text(g_status,status);
    if(http_request("GET","/api/v1/logs",NULL,buf,sizeof(buf))==0){
        cJSON *arr=cJSON_Parse(buf); cJSON *item=NULL; char view[30000]="";
        cJSON_ArrayForEach(item,arr){
            cJSON *level=cJSON_GetObjectItemCaseSensitive(item,"level");
            cJSON *message=cJSON_GetObjectItemCaseSensitive(item,"message");
            cJSON *stamp=cJSON_GetObjectItemCaseSensitive(item,"created_at");
            char line[2048]; snprintf(line,sizeof(line),"[%s] [%s] %s\r\n",
                cJSON_IsString(stamp)?stamp->valuestring:"",cJSON_IsString(level)?level->valuestring:"INFO",
                cJSON_IsString(message)?message->valuestring:"");
            if(strlen(view)+strlen(line)+1<sizeof(view))strcat(view,line);
        }
        cJSON_Delete(arr); set_text(g_logs,view);
    }
}
static void do_login(void) {
    char id[256],secret[512],eid[520],esec[1024],body[1800],response[4096];
    GetWindowTextA(g_appid,id,sizeof(id)); GetWindowTextA(g_secret,secret,sizeof(secret));
    if(!id[0]||!secret[0]){MessageBoxA(g_main,"请输入机器人 AppID 和 AppSecret。","QQBot",MB_ICONWARNING);return;}
    json_escape(id,eid,sizeof(eid)); json_escape(secret,esec,sizeof(esec));
    int mode= (int)SendMessageA(g_mode,CB_GETCURSEL,0,0);
    snprintf(body,sizeof(body),"{\"app_id\":\"%s\",\"app_secret\":\"%s\",\"mode\":\"%s\"}",eid,esec,mode==0?"websocket":"webhook");
    EnableWindow(g_login,FALSE); set_text(g_login,"正在登录...");
    if(http_request("POST","/api/v1/auth/login",body,response,sizeof(response))==0){
        SecureZeroMemory(secret,sizeof(secret)); SecureZeroMemory(esec,sizeof(esec));
        SecureZeroMemory(body,sizeof(body)); SetWindowTextA(g_secret,"");
        show_dashboard(); refresh_data();
    } else {
        SecureZeroMemory(secret,sizeof(secret)); SecureZeroMemory(esec,sizeof(esec)); SecureZeroMemory(body,sizeof(body));
        MessageBoxA(g_main,response[0]?response:"无法连接本地宿主，请检查程序文件是否完整。","QQBot 登录失败",MB_ICONERROR);
        set_text(g_login,"登录"); EnableWindow(g_login,TRUE);
    }
}
static HWND control(const char *cls,const char *text,DWORD style,int x,int y,int w,int h,int id) {
    return CreateWindowExA(0,cls,text,WS_CHILD|style,x,y,w,h,g_main,(HMENU)(INT_PTR)id,g_instance,NULL);
}
static void create_login(void) {
    control("STATIC","QQBot",SS_CENTER|SS_CENTERIMAGE,190,38,420,52,0);
    control("STATIC","QQ 官方机器人桌面宿主",SS_CENTER,190,91,420,26,0);
    control("STATIC","AppID",0,220,160,100,24,0);
    g_appid=control("EDIT","",WS_BORDER|ES_AUTOHSCROLL,220,185,360,30,ID_APPID);
    control("STATIC","AppSecret",0,220,230,100,24,0);
    g_secret=control("EDIT","",WS_BORDER|ES_PASSWORD|ES_AUTOHSCROLL,220,255,360,30,ID_SECRET);
    control("STATIC","连接方式",0,220,300,100,24,0);
    g_mode=control("COMBOBOX","",CBS_DROPDOWNLIST|WS_VSCROLL,220,325,360,150,ID_MODE);
    SendMessageA(g_mode,CB_ADDSTRING,0,(LPARAM)"WSS（推荐）");
    SendMessageA(g_mode,CB_ADDSTRING,0,(LPARAM)"Webhook");
    SendMessageA(g_mode,CB_SETCURSEL,0,0);
    g_login=control("BUTTON","登录",BS_DEFPUSHBUTTON,220,390,360,42,ID_LOGIN);
    control("STATIC","凭据仅用于本机登录，不会上传到第三方管理后台。",SS_CENTER,170,455,460,28,0);
}
static void create_dashboard(void) {
    g_home=control("BUTTON","首页",BS_PUSHBUTTON,20,20,120,36,ID_HOME);
    g_messages=control("BUTTON","消息日志",BS_PUSHBUTTON,20,64,120,36,ID_MESSAGES);
    g_settings=control("BUTTON","设置",BS_PUSHBUTTON,20,108,120,36,ID_SETTINGS);
    g_status=control("EDIT","等待状态...",WS_BORDER|ES_MULTILINE|ES_READONLY|WS_VSCROLL,165,20,650,90,ID_STATUS);
    g_logs=control("EDIT","",WS_BORDER|ES_MULTILINE|ES_READONLY|WS_VSCROLL|ES_AUTOVSCROLL,165,125,650,430,ID_LOGS);
    ShowWindow(g_status,SW_HIDE); ShowWindow(g_logs,SW_HIDE);
    ShowWindow(g_home,SW_HIDE); ShowWindow(g_messages,SW_HIDE); ShowWindow(g_settings,SW_HIDE);
}
static LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg){
    case WM_NCCREATE: g_main=hwnd; return TRUE;
    case WM_CREATE: create_login(); create_dashboard(); return 0;
    case WM_COMMAND:
        if(LOWORD(wp)==ID_LOGIN){do_login();return 0;}
        if(LOWORD(wp)==ID_HOME||LOWORD(wp)==ID_MESSAGES){refresh_data();return 0;}
        if(LOWORD(wp)==ID_SETTINGS){MessageBoxA(hwnd,"设置页面将在下一阶段加入。","QQBot",MB_ICONINFORMATION);return 0;}
        break;
    case WM_TIMER: if(wp==TIMER_REFRESH)refresh_data(); return 0;
    case WM_CLOSE: KillTimer(hwnd,TIMER_REFRESH); if(g_core_process){TerminateProcess(g_core_process,0);CloseHandle(g_core_process);g_core_process=NULL;} DestroyWindow(hwnd);return 0;
    case WM_DESTROY: PostQuitMessage(0);return 0;
    }
    return DefWindowProcA(hwnd,msg,wp,lp);
}
int WINAPI WinMain(HINSTANCE h,HINSTANCE prev,LPSTR cmd,int show) {
    (void)prev;(void)cmd;g_instance=h;make_paths();
    if(start_core()!=0){MessageBoxA(NULL,"无法启动 QQBot 核心程序。请确认 qqbot-core.exe 与 QQBot.exe 位于同一安装目录的 bin 文件夹。","QQBot",MB_ICONERROR);return 1;}
    Sleep(1600);
    WNDCLASSA wc;ZeroMemory(&wc,sizeof(wc));wc.lpfnWndProc=wndproc;wc.hInstance=h;wc.lpszClassName="QQBotDesktopWindow";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClassA(&wc);
    g_main=CreateWindowExA(0,wc.lpszClassName,"QQBot 登录",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,860,640,NULL,NULL,h,NULL);
    if(!g_main){if(g_core_process)TerminateProcess(g_core_process,1);return 1;}
    ShowWindow(g_main,show);UpdateWindow(g_main);
    MSG m;while(GetMessageA(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageA(&m);}
    return (int)m.wParam;
}
