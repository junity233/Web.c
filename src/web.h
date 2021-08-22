#ifndef _WEB_H
#define _WEB_H

#include"map.h"

typedef enum{
    RT_DELETE = 0,
    RT_GET = 1,
    RT_HEAD = 2,
    RT_POST = 3,
    RT_PUT = 4,
    RT_CONNECT = 5,
    RT_OPTIONS = 6,
    RT_TRACE = 7,
    RT_UNKNOWN=8
}Webc_RequestType;

typedef struct{
    Webc_RequestType requestType;
    char* url;
    char* version;
    Webc_Map headers;
    Webc_Map cookies;
    char* body;
    size_t bodyLength;
    Webc_Map args;
}Webc_RequestData;

char* GetRequestHeader(Webc_RequestData *req,const char* name);

void SetRequestHeader(Webc_RequestData *req,const char* name,const char* value);

char* GetArgment(Webc_RequestData* req,const char*name,size_t *valueLength);

long double GetNumArgment(Webc_RequestData* req,const char*name);

bool GetBoolArgment(Webc_RequestData* req,const char*name);

typedef struct{
    Webc_Map headers;
    BinaryBuffer *body;
}Webc_ResponseData;

#define RET_OK() do{return 200;}while(0)

#define RET_NOTFOUND() do{return 404;}while(0)

#define echo(...) PrintfToBuffer(res->body,__VA_ARGS__)

void SetResponseHeader(Webc_ResponseData *res,const char* name,const char* value);

char* GetResponseHeader(Webc_ResponseData *res,const char* name);

typedef enum{
    AT_FORMDATA,
    AT_URLENCODED,
    AT_RAW
}Webc_ArgmentType;

typedef struct{
    Webc_ArgmentType type;
    union{
        Webc_Map args;
        char* data;
    };
}WebArgments;

typedef int (*RequestMethod)(Webc_RequestData *data,Webc_ResponseData* res);

typedef struct{
    RequestMethod GET;
    RequestMethod POST;
    const char *path;   
}Webc_Processer;

size_t WebQueueSize(size_t size);

size_t WebBufferSize(size_t size);

void RunWebApplication(Webc_Processer *processer,int port,size_t maxThreadNum);

void ExitApplication(int n);

#endif