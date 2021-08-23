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
$ make build # 编译
```

## 使用方法：

1. 第一步：声明请求处理函数，要求原型为 int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res)
2. 第二步：定义处理器结构
3. 第三步：在主函数中调用RunWebApplication函数启动服务器
4. 第四步：构建并运行

具体请见↓
## 例子：
``` C
#include "web.h"            //引用web.c的头文件

/** 
 * 定义一个处理方法，原型为 int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res);
 * 其中，第一个参数req内保存请求信息，第二个参数res内保存响应信息
 * echo宏就是向res的body成员写入信息
 */
int Index(Webc_RequestData *req,Webc_ResponseData *res){
    echo("<!doctype html>");

    int num=GetNumArgment(req,"num");   //调用GetNumArgment函数来获取一个数字型的参数（若不存在会返回0）
    for(int i=1;i<=num;i++)
    {                                   //打印一个三角形
        for(int j=1;j<=i;j++)
            echo("*");
        echo("<br>");
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
    ErrorGrade(ET_NOTE);//设置日志级别为NOTE级别
    WebQueueSize(1024);//设置socket队列的最大长度
    WebBufferSize(1024*1024);//设置recv缓冲区的长度
    RunWebApplication(processers,8080,16);//启动服务器，第一个参数为处理器结构，第二个参数为端口号，第三个参数为线程的数量
}
```
## TODO
- [x] 使用线程池实现多线程
- [x] 解析urlencoded参数
- [ ] windows兼容
- [ ] 详细的注释
- [ ] 解析formdata参数
- [ ] 使用正则表达式解析url，实现web.py的类似效果
- [ ] 数据分片收发
- [ ] 一键返回静态文件
- [ ] C格式字符串转HTML（换行符转\<br>等）
  
## 致谢

本项目使用了如下几个项目：

1. https://github.com/Xsoda/url
2. https://github.com/nodejs/llhttp
3. https://github.com/Pithikos/C-Thread-Pool