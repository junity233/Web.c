# Web.c

一个轻量级、由C语言编写的HTTP框架

向web.py致敬！

## 使用方法：

第一步：声明请求处理函数，要求原型为 int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res)

第二步：定义处理器结构

第三步：在主函数中调用RunWebApplication函数启动服务器

第四步：构建并运行

具体请见↓
## 例子：
```C
#include "web.h"            //引用web.c的头文件

/** 
 * 定义一个处理方法，原型为 int (*RequestMethod)(Webc_RequestData *req,Webc_ResponseData* res);
 * 其中，第一个参数req内保存请求信息，第二个参数res内保存响应信息
 * echo宏就是向res的body成员写入信息
 */
int Index(Webc_RequestData *req,Webc_ResponseData *res){
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
- [x] 线程池实现多线程
- [x] 解析urlencoded参数
- [ ] 解析formdata参数
- [ ] url正则表达式
- [ ] 文件分片
- [ ] c格式字符串转HTML（换行符转<br>等）