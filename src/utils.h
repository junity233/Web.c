#ifndef _UTILTS_H
#define _UTILTS_H


#include<stdio.h>
#include<stdint.h>
#include<stdarg.h>
#include<time.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>

#define DEFAULD_CAPCITY 64
#define CAPCITY_GROWTH 1.5

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef char byte;
typedef int bool;
#define true 1
#define false 0
#define NOT_BOOL -1

uint32_t HashString(const byte *data,size_t length);

uint32_t strnchr(const char* str,char C,size_t length);

char* copynstr(const char* src,size_t length);

char* copystr(const char* src);

typedef struct{
    byte* data;
    size_t used;
    size_t capcity;
}BinaryBuffer;

BinaryBuffer* NewBuffer();

void BufferInit(BinaryBuffer* buffer);

void WriteToBuffer(BinaryBuffer *buffer,void* src,size_t size);

void PrintfToBuffer(BinaryBuffer*buffer,const char* fmt,...);

void ReadFileToBuffer(BinaryBuffer* buffer,const char* file);

void BufferReset(BinaryBuffer* buffer);

void BufferClean(BinaryBuffer *buffer);

typedef enum{
    ET_DEBUG=1,
    ET_NOTE,
    ET_WARNING,
    ET_ERROR,
    ET_NONE
}ErrorType;

ErrorType ErrorGrade(ErrorType et);

void ErrorReport(ErrorType et,const char* fmt,...);

#define REPORT_DEBUG(...) ErrorReport(ET_DEBUG,__VA_ARGS__)
#define REPORT_NOTE(...) ErrorReport(ET_NOTE,__VA_ARGS__)
#define REPORT_WARNING(...) ErrorReport(ET_WARNING,__VA_ARGS__)
#define REPORT_ERROR(...) ErrorReport(ET_ERROR,__VA_ARGS__)


#ifdef _DEBUG
#define UNREACHED() \
    do{ \
        fprintf(stderr,"Unreached!于文件 %s,函数 %s,第 %d 行\n",__FILE__,__FUNCTION__,__LINE__); \
        while(1); \
    }while(0)

#define ASSERT(con,...) \
    do{ \
        if(!(con)){ \
            REPORT_DEBUG("断言条件 %s 失败！于文件 %s ，函数 %s ,第 %d 行.断言信息：",#con,__FILE__,__FUNCTION__,__LINE__); \
            REPORT_ERROR(__VA_ARGS__); \
            abort(); \
        } \
    }while(0);

#define NOTNULL(var) ASSERT((var)!=NULL,"执行 %s 的 %s 为NULL!",__FUNCTION__,#var);
#else
#define UNREACHED() ((void)0)
#define ASSERT(con,msg) ((void)0)
#define NOTNULL(var) ((void)0)
#endif

#define MALLOC(type) \
    ({ \
        void *ptr=malloc(sizeof(type)); \
        if(ptr==NULL) {\
            REPORT_ERROR("为类型 \""#type"\" 分配内存失败,程序退出."); \
            exit(0); \
        } \
        (type*)ptr; \
    })

#define MALLOC_ARRAY(type,length) \
    ({ \
        void *ptr=malloc(sizeof(type)*(length)); \
        if(ptr==NULL) {\
            REPORT_ERROR("为 \""#type"\" 数组分配内存失败（ %d 个）,程序退出.",length); \
            exit(0); \
        } \
        (type*)ptr; \
    })


#endif