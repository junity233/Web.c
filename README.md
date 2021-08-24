# Web.c

web.c是一个轻量级的http服务器框架，可以像web.py一样快速编写web应用程序
项目在linux下编译通过，现在正在努力兼容windows平台。

## 特点
- 使用C-Thread-Pool库，可自定义线程数量且线程死亡不会影响进程（返回500）
- 自动解析GET/POST请求参数，可通过内置API获取
- 简单、易于上手的API
- 中文的运行日志

## 运行截图
- 源代码:
- ![Source Code](https://www.hualigs.cn/image/612255e2e8c7d.jpg)
- 服务端运行状态:
- ![Server](https://www.hualigs.cn/image/6122557638d0c.jpg)
- 页面效果:
- ![Client](https://pic4.58cdn.com.cn/nowater/webim/big/n_v28096af3640944869b0c76c5f4f369585.png)

## 如何编译
```bash
$ make install-third-party-library # 安装第三方库
$ make release # 编译（debug请用 make debug ）
```

## 例子：
``` C
#include "web.h"            //引用web.c的头文件

/** 
 * 定义一个处理方法，原型为 int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res);
 * 其中，第一个参数req内保存请求信息，第二个参数res内保存响应信息
 * echo宏就是向res的body成员写入信息
 */
int Index(Webc_RequestData *req,Webc_ResponseData *res){
    echo("<!DOCTYPE html>\n");
    int num=GetNumArgment(req,"num");   //调用GetNumArgment函数来获取一个数字型的参数（若不存在会返回0）
    for(int i=1;i<=num;i++)
    {                                   //打印一个三角形
        for(int j=1;j<=i;j++)
            echo("*&nbsp;");
        echo("\n<br>");
    }
    return 200;             //返回值就是http的状态码，这里返回200
}

/**
 * 定义处理器结构
 * 此结构为一个数组，其中每个元素表示一个站点位置，
 * GET、POST属性用来定义相应请求的处理方法，
 * path属性定义其路径。路径使用trie树保存。
 * 对于一个处理器结构，最后一项必须以path=NULL为结尾，并且
 * 对于不存在的处理方法必须设置其为NULL。
 */
Webc_Processer processers[]={
    {
        .GET=Index,
        .POST=NULL,
        .path="/"
    },
    {
        .path=NULL
    }
};

int main(){
    ErrorGrade(ET_DEBUG);//设置日志级别为NOTE级别
    WebQueueSize(1024);//设置socket队列的最大长度
    WebBufferSize(1024*1024);//设置recv缓冲区的长度
    RunWebApplication(processers,8080,16);//启动服务器，第一个参数为处理器结构，第二个参数为端口号，第三个参数为线程的数量
}
}
```

## 使用方法详解（更新中）
### 一.数据结构
#### 1.RequestMethod
定义：
```C
/**
 * @description: 请求的处理函数原型
 * @param {Webc_RequestData} *req 请求体
 * @param {Webc_ResponseData*} *res  响应体
 * @return {*}
 */
typedef int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res);
```
RequestMethod是用于处理web请求的回调函数，第一个参数req是本次请求的相关信息，包括headers,自动解析的参数和请求体（详见Webc_RequestData），可以用GetRequestHeader、GetArgment等函数获取其内容。

第二个参数res是响应的信息，可以通过向其成员body写入数据来向客户端返回信息。当此参数标识符为res时，可以使用宏 echo 来写入字符串，否则只能使用PrintfToBuffer与WriteToBuffer
两个函数写入。echo与PrintfToBuffer均支持printf相同的格式控制。

对于响应的header部分，也可以通过SetResponseHeader函数来操作。

res的dt成员对应了响应体的数据类型，包括raw（原始数据），HTML与file。需要注意的是dt默认为HTML，此时webc会自动将c字符串转为HTML格式，包括：在第一行添加<!DOCTYPE html>，将\n转为<br>以及将空格转为&nbsp;等。

本返回值标志了本次响应的状态，即HTTP状态码，比如200，404等。

### 2.Webc_RequestType
定义：
``` C
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
```
表示请求的类型

### 3.Webc_RequestData
定义：
``` C
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
}Webc_RequestData;
```
表示一个请求的信息，可通过GetRequestHeader，SetRequestHeader，GetArgment 等函数操作

### 4.Webc_ResponseData
定义：
``` C
/**
 * 响应结构
 */
typedef struct{
    Webc_Map headers;       //响应头
    int statusCode;         //HTTP状态码
    BinaryBuffer *body;     //响应体
}Webc_ResponseData;
```
表示响应数据，可以通过GetResponseHeader、SetResponseHeader、echo等宏或函数操作

### 5.Webc_Processer
定义：
``` C
/**
 * 处理器结构
 */
typedef struct{
    RequestMethod GET;  //GET请求处理函数
    RequestMethod POST; //POST请求处理函数
    const char *path;   //请求路径
}Webc_Processer;
```
描述网站结构的数据结构，一般定义成数组。在定义时，若此path下某个方法不存在，需定义为NULL（切记），用户访问时即返回404，且数组最后一项的path需定义为NULL。注意，本结构在加载服务器时会将path分解导入到一个trie树中，因此并不支持动态添加。
## 函数

## TODO
- [x] 使用线程池实现多线程
- [x] 解析urlencoded参数
- [x] 一键返回静态文件
- [ ] windows兼容
- [ ] 详细的注释
- [ ] 解析formdata参数
- [ ] 使用正则表达式解析url，实现web.py的类似效果
- [ ] 数据分片收发
  
## 致谢

本项目使用了如下几个项目：

1. https://github.com/Xsoda/url
2. https://github.com/nodejs/llhttp
3. https://github.com/Pithikos/C-Thread-Pool
