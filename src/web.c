#ifdef linux
    #include<sys/socket.h>
    #include<sys/types.h>
    #include<netinet/in.h>
    #include<arpa/inet.h>
#endif
#include<unistd.h>

#include "web.h"
#include"utils.h"
#include "llhttp.h"
#include<pthread.h>
#include<setjmp.h>
#include "thpool.h"
#include <signal.h>
#include "url.h"
#include "trie.h"


#define DEFAULT_BUFFER_SIZE 10240

#define MAX_QUEUE_NUM 1024

int listenfd;

struct _jmpBufferTable{
    jmp_buf buf;
    pthread_t pid;
    bool is_use;
}*JmpBufferTable;

const char* HttpMethodNames[]={
    "DELETE ",
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "CONNECT",
    "OPTIONS",
    "TRACE"
};

typedef struct{
    char* fieldBuffer;
    char* valueBuffer;
    size_t valueLength;
    Webc_RequestData *res;
}ParseBuffer;

static int OnHeaderField(llhttp_t *parser,const char *at,size_t length){    
    ((ParseBuffer*)parser->data)->fieldBuffer=copystr(at,length);
    return 0;

}

static int OnHeaderValue(llhttp_t *parser,const char *at,size_t length){
    ParseBuffer* buffer=(ParseBuffer*)parser->data;
    buffer->valueBuffer=copystr(at,length);
    MapSet(buffer->res->headers,buffer->fieldBuffer,buffer->valueBuffer,-1);
    return 0;
}

static int OnUrl(llhttp_t*parser,const char* at,size_t length){
    REPORT_DEBUG("解析url完毕");
    char urlBuffer[DEFAULT_BUFFER_SIZE];
    memcpy(urlBuffer,at,length);
    urlBuffer[length]='\0';
    ((ParseBuffer*)parser->data)->res->url=copystr(at,length);
    REPORT_DEBUG("Url:%s",urlBuffer);
    return  0;
}

static int OnBody(llhttp_t *parser,const char*at,size_t length){
    REPORT_DEBUG("解析Body完毕");
    ((ParseBuffer*)parser->data)->res->body=copystr(at,length);
    REPORT_DEBUG("Body:%s",((ParseBuffer*)parser->data)->res->body);    
    return 0;
}

static int ParserUrl(const char* url,Webc_RequestData *res){
    url_field_t *parse_res;
    parse_res=url_parse(url);
    res->url=copystr(parse_res->path,-1);
    for(int i=0;i<parse_res->query_num;i++)
        MapSet(res->args,parse_res->query[i].name,parse_res->query[i].value,-1);
    url_free(parse_res);
    return 0;    
}

static int ParserQuery(const char* url,Webc_RequestData *res){
    NOTNULL(url)
    NOTNULL(res)
    url_field_t *parse_res=MALLOC(url_field_t);
    memset(parse_res,0,sizeof(url_field_t));
    parse_query(parse_res,url);
    for(int i=0;i<parse_res->query_num;i++)
        MapSet(res->args,parse_res->query[i].name,parse_res->query[i].value,-1);
    url_free(parse_res);
    return 0;    
}

static int ParseFormData(Webc_RequestData* res,const char*data,size_t length){
    NOTNULL(res);
    NOTNULL(data);
}

static int ParseRequest(Webc_RequestData *res,const char* data,uint32_t length){
    NOTNULL(res)
    NOTNULL(data)
    int status;    
    llhttp_t parser;
    ParseBuffer buffer;
    llhttp_settings_t settings;
    llhttp_settings_init(&settings);
    settings.on_url=OnUrl;
    settings.on_header_value=OnHeaderValue;
    settings.on_header_field=OnHeaderField;
    settings.on_body=OnBody;
    
    llhttp_init(&parser,HTTP_REQUEST,&settings);
    buffer.res=res;
    parser.data=&buffer;
    enum llhttp_errno err=llhttp_execute(&parser,data,length);
    if(err!=HPE_OK)
    {
        REPORT_ERROR("llhttp解析报文失败，错误代号 %d,原因：%s",err,parser.reason);
        return -1;
    }

    res->requestType=parser.method;
    if(parser.method>HTTP_TRACE)
    {
        REPORT_ERROR("无法处理的请求类型：%d",parser.method);
        return -1;
    }
    if(parser.method==HTTP_POST){
        char* contentType=GetRequestHeader(res,"Content-Type");
        if(contentType!=NULL&&memcmp(contentType,"application/x-www-form-urlencoded",strlen(contentType))==0){
            status=ParserQuery(res->body,res);
            if(status!=0)
            {
                REPORT_ERROR("解析POST请求体失败！返回值: %s 请求体:\n%s",status,res->body);
                return -1;
            }
        }
        else if(contentType!=NULL&&memcpy(contentType,"multipart/form-data;",strlen(contentType)==0));
        
    }

    status=ParserUrl(res->url,res);

    if(status!=0)
    {
        REPORT_ERROR("解析url失败！返回值: %d 原url:%s",status,res->body);
        return -1;
    }

    REPORT_DEBUG("解析http报文成功，调用方法：%s",HttpMethodNames[parser.method]);

    REPORT_DEBUG("Url=%s",res->url);

    REPORT_DEBUG("参数列表：");

    WEBC_MAP_FOREACH(res->args,i){
        REPORT_DEBUG("%s=%s",i->key,i->value);
    }

    REPORT_DEBUG("Header列表：");

    WEBC_MAP_FOREACH(res->headers,i){
        REPORT_DEBUG("%s=%s",i->key,i->value);
    } 

    return 0;
}

static void RequestDataInit(Webc_RequestData* data){
    NOTNULL(data)
    data->url=data->version=data->body=NULL;
    data->cookies=NewMap();
    data->headers=NewMap();
    data->args=NewMap();
    data->requestType=RT_UNKNOWN;
}

static void RequestDataClear(Webc_RequestData* data){
    NOTNULL(data)
    MapClean(data->cookies);
    MapClean(data->headers);
    MapClean(data->args);
    if(data->body)
        free(data->body);
    if(data->url)
        free(data->url);
}


void ResponseInit(Webc_ResponseData* response){
    NOTNULL(response)
    response->headers=NewMap();
    response->body=NewBuffer();
}

void ResponseDataClear(Webc_ResponseData* response){
    NOTNULL(response)
    MapClean(response->headers);
    BufferClean(response->body);
}

void SetRequestHeader(Webc_RequestData *res,const char* name,const char* value){
    NOTNULL(res)
    MapSet(res->headers,name,value,-1);
}

char* GetRequestHeader(Webc_RequestData *res,const char* name){
    NOTNULL(res)
    return MapGet(res->headers,name,NULL);
}

char* GetArgment(Webc_RequestData* req,const char*name,size_t *valueLength){
    NOTNULL(req);
    return MapGet(req->args,name,valueLength);
}

long double GetNumArgment(Webc_RequestData* req,const char*name){
    char* res=MapGet(req->args,name,NULL);
    if(res==NULL)
        return 0;
    return strtold(res,NULL);
}

bool GetBoolArgment(Webc_RequestData* req,const char*name){
    char* res=MapGet(req->args,name,NULL);
    if(res==NULL)
        return NOT_BOOL;
    if(strcmp(res,"true"))
        return true;
    else if(strcmp(res,"false"))
        return false;
    return NOT_BOOL;
}

char* GetResponseHeader(Webc_ResponseData *req,const char* name){
    NOTNULL(req);
    return MapGet(req->headers,name,NULL);
}

void SetResponseHeader(Webc_ResponseData *res,const char* name,const char* value){
    NOTNULL(res);
    MapSet(res->headers,name,value,-1);
}

size_t WebQueueSize(size_t size){
    static size_t webQueueSize=MAX_QUEUE_NUM;
    if(size!=0)
        webQueueSize=size;
    return webQueueSize;
}

size_t WebBufferSize(size_t size){
    static size_t webBufferSize=DEFAULT_BUFFER_SIZE;
    if(size!=0)
        webBufferSize=size;
    return webBufferSize;
}

static void ExitHandler(int n){
    ExitApplication(n);
}

typedef struct{
    int connectfd;
    struct sockaddr_in in;
    Webc_Trie siteStructure;
}ConnectInfo;

static int ProcessConnect(void *args){
    ConnectInfo* info=(ConnectInfo*)args;
    int connectfd=info->connectfd;
    struct sockaddr_in in=info->in;
    Webc_Trie siteStructure=info->siteStructure;
    char *recvBuffer;
    recvBuffer=MALLOC_ARRAY(char,WebBufferSize(0));
    ssize_t length;
    memset(recvBuffer,0,WebBufferSize(0));
    length=recv(connectfd,recvBuffer,10240,0);
    REPORT_DEBUG("接收到信息：\n%s\n",recvBuffer);

    Webc_RequestData data;
    RequestDataInit(&data);
    Webc_ResponseData res;
    ResponseInit(&res);

    int parse_status=ParseRequest(&data,recvBuffer,length);
    if(parse_status!=0){
        REPORT_ERROR("解析报文失败，返回值%d",parse_status);
        free(recvBuffer);
        close(connectfd);
        return -1;;
    }

    SetResponseHeader(&res,"Content-Type","text/html");
    Webc_Processer processer=TrieGet(siteStructure,data.url);

    int idx;
    for(idx=0;JmpBufferTable[idx].is_use==true;idx++);
    JmpBufferTable[idx].is_use=true;
    JmpBufferTable[idx].pid=pthread_self();
    int status;
    int jmp_res=setjmp(JmpBufferTable[idx].buf);
    if(jmp_res==0){
        switch(data.requestType){
            case RT_GET:
                if(processer.GET!=NULL)
                    status=(*processer.GET)(&data,&res);
                else status=404;
                break;
            case RT_POST:
                if (processer.POST!=NULL)            
                    status=(*processer.POST)(&data,&res);
                else status=404;
                break;
            default:UNREACHED();
        }
    }else{
        status=500;
        BufferReset(res.body);
    }
    JmpBufferTable[idx].is_use=false;
        
    BinaryBuffer *buffer=NewBuffer();

    if(status<0)
    {
        PrintfToBuffer(buffer,"HTTP/1.1 %d\n",500);
        REPORT_NOTE("%s 请求: %d / %s 内部错误,程序返回值:%d",HttpMethodNames[data.requestType],500,inet_ntoa(in.sin_addr),status);
    }
    else{
        REPORT_NOTE("%s 请求: %d / %s %s",HttpMethodNames[data.requestType],status,inet_ntoa(in.sin_addr),data.url);
        PrintfToBuffer(buffer,"HTTP/1.1 %d\n",status);
        WEBC_MAP_FOREACH(res.headers,i){
            PrintfToBuffer(buffer,"%s : %s\n",i->key,i->value);
        }
        PrintfToBuffer(buffer,"Content-Length:%d\n\n",res.body->used);
        WriteToBuffer(buffer,res.body->data,res.body->used);
    }
    send(connectfd,buffer->data,buffer->used,0);
    RequestDataClear(&data);
    BufferClean(buffer);
    close(connectfd);
    return 0;
}

void RuntimeError(int n){
    pthread_t pid=pthread_self();
    REPORT_DEBUG("线程运行时错误！线程编号:%d，错误原因:%s",pid,({
        const char*res;
        switch(n){
            case SIGFPE:
                res="浮点运算错误！";
                break;
            case SIGSEGV:
                res="访问无效内存！";
                break;
            default:
                UNREACHED();
        }
        res;
    }));
    for(int idx=0;;idx++)
        if(pid==JmpBufferTable[idx].pid)
            longjmp(JmpBufferTable[idx].buf,n);
}

void RunWebApplication(Webc_Processer *processers,int port,size_t maxThreadNum){
    NOTNULL(processers);
    struct sockaddr_in servaddr;
    signal(SIGINT,ExitHandler);
    signal(SIGSEGV,RuntimeError);
    signal(SIGFPE,RuntimeError);
    if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
        REPORT_ERROR("创建套接字失败!返回值:%d",listenfd);
        exit(0);
    }
    REPORT_DEBUG("创建套接字成功");

    //debug下关闭time_wait
#ifdef _DEBUG
    int opt=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
#endif

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);

    REPORT_DEBUG("生成套接字绑定信息成功，端口:%d",port);

    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1){

        REPORT_ERROR("绑定套接字失败");
        exit(0);
    }
    REPORT_DEBUG("绑定套接字成功");
    if(listen(listenfd,WebQueueSize(0))==-1){
        REPORT_ERROR("套接字切换监听状态失败");
        exit(0);
    }
    REPORT_DEBUG("套接字已设置为监听状态");
    Webc_Trie siteStructure=NewTrie();
    for(Webc_Processer *i=processers;i->path!=NULL;i++)
        TrieInsert(siteStructure,i->path,*i);

    REPORT_DEBUG("进入消息循环");

    threadpool pool=thpool_init(maxThreadNum);
    JmpBufferTable=MALLOC_ARRAY(struct _jmpBufferTable,maxThreadNum);
    for(int i=0;i<maxThreadNum;i++)
        JmpBufferTable[i].is_use=false;

    REPORT_NOTE("服务器启动成功。端口:%d 线程数:%d.",port,maxThreadNum);
    while (true)
    {
        struct sockaddr_in in;
        socklen_t in_len=sizeof(struct sockaddr_in);
        int connectfd=accept(listenfd,(struct sockaddr*)&in,&in_len);
        if(connectfd==-1){
            REPORT_WARNING("接受 %s 的连接请求失败",inet_ntoa(in.sin_addr));
            continue;
        }
        REPORT_DEBUG("接受到 %s 的新连接",inet_ntoa(in.sin_addr));
        ConnectInfo info={
            .connectfd=connectfd,
            .in=in,
            .siteStructure=siteStructure
        };
        thpool_add_work(pool,ProcessConnect,(void*)&info);
    }
    
}

void ExitApplication(int n){
    REPORT_NOTE("程序退出,返回值 %d",n);
    close(listenfd);
    exit(n);
}