#ifndef _WEB_H
#define _WEB_H

#include"map.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * 请求传输的参数的类型
 */
typedef enum{
    AT_RAW,             //原始数据
    AT_FILE             //文件
}Webc_ArgmentType;

/**
 * 请求传输的参数的结构
 */
typedef struct{
    Webc_ArgmentType at;    //本参数类型
    size_t length;          //长度
    char* fileName;         //若为文件，则本项为文件名
    char* contentType;      //若为文件，则本项为content-type
    void* data;             //数据
}Webc_ArgmentData;

/**
 * http请求类型，目前仅支持GET和POST
 */
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
    char* name;
    void* body;
}Webc_FileArgment;

/**
 * Web请求数据
 */
typedef struct{
    Webc_RequestType requestType;   //请求类型
    char* url;                      //请求的url（解析后不包含参数和锚点）
    char* version;                  //http版本
    Webc_Map headers;               //header列表
    Webc_Map cookies;               //cookies列表（未实现，请勿使用）
    char* body;                     //请求体
    size_t bodyLength;              //请求体长度
    Webc_Map args;                  //请求参数，由web.c自动解析
    Webc_Map files;                 //请求传来的文件，
}Webc_RequestData;

/**
 * @description: 获取一个请求头
 * @param {Webc_RequestData} *req
 * @param {const char*} name
 * @return {*}
 */
char* GetRequestHeader(Webc_RequestData *req,const char* name);

/**
 * @description: 设置请求头，不建议使用
 * @param {Webc_RequestData} *req
 * @param {const char*} name
 * @param {const char*} value
 * @return {*}
 */
void SetRequestHeader(Webc_RequestData *req,const char* name,const char* value);

/**
 * @description: 获取请求参数（返回原始数据），无参时返回NULL
 * @param {Webc_RequestData*} req
 * @param {const} char
 * @param {size_t} *valueLength
 * @return {*}
 */
char* GetArgment(Webc_RequestData* req,const char*name);

/**
 * @description: 获取一个数字参数（为兼容小数，返回值为long double），无此参数时返回0（以后一定改进）
 * @param {Webc_RequestData*} req
 * @param {const} char
 * @return {*}
 */
long double GetNumArgment(Webc_RequestData* req,const char*name);

/**
 * @description: 获取bool参数，无此参数时返回NOT_BOOL
 * @param {Webc_RequestData*} req
 * @param {const} char
 * @return {*}
 */
int GetBoolArgment(Webc_RequestData* req,const char*name);


/**
 * 响应结构
 */
typedef struct{
    Webc_Map headers;       //响应头
    int statusCode;         //HTTP状态码
    BinaryBuffer *body;     //响应体
}Webc_ResponseData;

#define RET_OK() return(200)

/**
 * @description: 向buffer写入文件
 * @param {Webc_ResponseData*} res
 * @param {char*} file 文件路径
 * @param {char*} type MIME类型1
 * @return {*}
 */
void ReturnFile(Webc_ResponseData* res,const char* file,const char* type);

#define RET_FILE(file,type) \
    do{ \
        ReturnFile(res,file,type); \
        return 200; \
    }while(0)

#define RET_HTML_FILE(file) RET_FILE(file,"text/html")

#define RET_BINARY_FILE(file) RET_FILE(file,"application/octet-stream")

#define RET_NOTFOUND() return(404)

#define RET_302(url) \
    do{ \
        SetResponseHeader(res,"Location",url); \
        return 302; \
    }while(0)

#define RET_301(url) \
    do{ \
        SetResponseHeader(res,"Location",url); \
        return 301; \
    }while(0)

/**
 * @description: 向响应体中写入数据，参数类似printf（需将响应体变量名设置为res）
 * @param {*}
 * @return {*}
 */
#define echo(...) PrintfToBuffer(res->body,__VA_ARGS__)

/**
 * @description: 设置响应的header
 * @param {Webc_ResponseData} *res 响应
 * @param {const char*} name
 * @param {const char*} value
 * @return {*}
 */
void SetResponseHeader(Webc_ResponseData *res,const char* name,const char* value);

/**
 * @description: 获取响应头
 * @param {Webc_ResponseData} *res
 * @param {const char*} name
 * @return {*}
 */
char* GetResponseHeader(Webc_ResponseData *res,const char* name);


/**
 * @description: 请求的处理函数原型
 * @param {Webc_RequestData} *req 请求体
 * @param {Webc_ResponseData*} *res  响应体
 * @return {*}
 */
typedef int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res);

/**
 * 处理器结构
 */
typedef struct{
    RequestMethod GET;  //GET请求处理函数
    RequestMethod POST; //POST请求处理函数
    const char *path;   //请求路径
}Webc_Processer;

/**
 * @description: 设置或获取socket队列长度
 * @param {size_t} size size=0时为获取
 * @return {*}
 */
size_t WebQueueSize(size_t size);

/**
 * @description: 设置或获取recv缓冲区大小
 * @param {size_t} size size=0时为获取
 * @return {*}
 */
size_t WebBufferSize(size_t size);

/**
 * @description: 加载服务器
 * @param {Webc_Processer} *processer
 * @param {int} port
 * @param {size_t} maxThreadNum
 * @return {*}
 */
void RunWebApplication(Webc_Processer *processer,int port,size_t maxThreadNum);

/**
 * @description: 退出程序
 * @param {int} n 返回值，同main函数
 * @return {*}
 */
void ExitApplication(int n);

#ifdef __cplusplus
}
#endif

#endif