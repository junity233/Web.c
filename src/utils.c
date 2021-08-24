#include"utils.h"
#include<sys/stat.h>

uint32_t strnchr(const char* str,char C,size_t length){
    uint32_t idx=0;
    while(idx<length){
        if(str[idx]==C)
            return idx;
        idx++;
    }
    return -1;
}

uint32_t HashString(const byte *data,size_t length){
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;
    for(int i=0;i<length;i++)
    {
        if(*data=='\0')
            break;
        hash = hash * a + *(data++);
        a *= b;
    }
 
    return (hash & 0x7FFFFFFF);
}

char* copystr(const char*src,size_t length){
    if(src==NULL){
        REPORT_DEBUG("函数 copystr 复制了一个空字符串！");
        return NULL;
    }
    if(length==-1)
        length=strlen(src);
    char* res=MALLOC_ARRAY(char,length+1);
    memcpy(res,src,length);
    res[length]='\0';
    return res;
}

BinaryBuffer* NewBuffer(){
    BinaryBuffer *res=MALLOC(BinaryBuffer);
    BufferInit(res);
    return res;
}

ErrorType ErrorGrade(ErrorType et){
    static ErrorType globalErrorGrade=ET_WARNING;
    if(et!=0)
        globalErrorGrade=et;
    return globalErrorGrade;
}

void BufferInit(BinaryBuffer* buffer){
    buffer->data=MALLOC_ARRAY(byte,DEFAULD_CAPCITY);
    buffer->capcity=DEFAULD_CAPCITY;
    buffer->used=0;
}

void WriteToBuffer(BinaryBuffer *buffer,void* src,size_t size){
    if(size+buffer->used+1>=buffer->capcity)
    {
        size_t newSize=MAX(buffer->capcity*CAPCITY_GROWTH,size+buffer->capcity+1);
        buffer->data=realloc(buffer->data,newSize);
        buffer->capcity=newSize;
    }
    memcpy(buffer->data+buffer->used,src,size);
    buffer->used+=size;
    buffer->data[buffer->used]='\0';
}

void BufferReset(BinaryBuffer *buffer){
    buffer->used=0;
}

void BufferClean(BinaryBuffer *buffer){
    free(buffer->data);
}

void PrintfToBuffer(BinaryBuffer*buffer,const char* fmt,...){
    va_list va;
    va_start(va,fmt);
    size_t length=vsnprintf(NULL,0,fmt,va);
    char *data=MALLOC_ARRAY(char,length+1);
    va_start(va,fmt);
    vsprintf(data,fmt,va);
    va_end(va);
    WriteToBuffer(buffer,data,length);
}


void ReadFileToBuffer(BinaryBuffer* buffer,const char* file){
    struct stat fs;
    stat(file,&fs);
    size_t length=fs.st_size;
    char* buf=MALLOC_ARRAY(char,length+1);
    FILE* fp=fopen(file,"rb");
    fread(buf,1,length,fp);
    WriteToBuffer(buffer,buf,length);
    fclose(fp);
    free(buf);

}

const char* MonthNames[]={
    NULL,
    "一月",
    "二月",
    "三月",
    "四月",
    "五月",
    "六月",
    "七月",
    "八月",
    "九月",
    "十月",
    "十一月",
    "十二月"
};

const char* ErrorTypeNames[]={
    NULL,
    "\033[35mDebug\033[0m",
    "\033[32m信息\033[0m",
    "\033[31m警告\033[0m",
    "\033[31m错误\033[0m"
};

void ErrorReport(ErrorType et,const char* fmt,...){
    if(et<ErrorGrade(0))
        return;
    va_list va;
    va_start(va,fmt);
    time_t t=time(NULL);
    struct tm *tm_ptr=gmtime(&t);
    printf("%d年%s%d日 %d:%d:%d [%s] : ",tm_ptr->tm_year+1900,MonthNames[tm_ptr->tm_mon],
        tm_ptr->tm_mday,tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec,ErrorTypeNames[et]);
    vprintf(fmt,va);
    putchar('\n');
    va_end(va);
}