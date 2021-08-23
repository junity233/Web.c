/*
 * @Author: Junity
 * @Date: 2021-08-22 21:31:55
 * @Description: 用于描述网站结构的tried树的实现
 */
#include "trie.h"

Webc_Trie NewTrie(){
    Webc_Trie res=MALLOC(struct _trie_node);
    res->head=res->next=NULL;
    res->path=NULL;
    res->is_leaf=false;
    res->hashcode=0;//0是“”的hashcode
    return res;
}

void TrieInsert(Webc_Trie tree,const char* path,Webc_Processer processer){
    NOTNULL(tree);
    NOTNULL(path);
    char *buffer;
    uint32_t idx=1;//跳过第一个"/"
    size_t length=strlen(path);
    Webc_Trie cur=tree;
    uint32_t hashcode=0;
    while(idx<length){
        int next=strnchr(path+idx,'/',length-idx);
        if(next<0)
            next=length-1;
        buffer=strndup(path+idx,next-idx);
        hashcode=HashString(buffer,next-idx);
        Webc_Trie i;
        for(i=cur->head;i;i=i->next){
            if(hashcode==i->hashcode)
            {
                tree=i;
                break;
            }
        }
        if(i==NULL){
            Webc_Trie newNode=NewTrie();
            newNode->path=strdup(path);
            newNode->hashcode=hashcode;
            newNode->is_leaf=(next==length);
            if(newNode->is_leaf)
                newNode->processer=processer;
            newNode->next=cur->head;
            cur->head=newNode;
            cur=newNode;
        }
        else cur=i;
        idx=next;
        free(buffer);
    }
    NOTNULL(cur);
    if(cur->path)
        free(cur->path);
    cur->path=strdup(path);
    cur->processer=processer;
    cur->is_leaf=true;
    cur->hashcode=hashcode;
}
Webc_Processer TrieGet(Webc_Trie tree,const char* path){
    NOTNULL(tree);
    NOTNULL(path);
    uint32_t idx=1;//跳过第一个"/"
    size_t length=strlen(path);
    Webc_Trie cur=tree;
    uint32_t hashcode=0;
    while(idx<length){
        int next=strnchr(path+idx,'/',length-idx);
        if(next<0)
            next=length-1;
        if(path[next]=='/')
            hashcode=HashString(path+idx,next-idx);
        else
            hashcode=HashString(path+idx,next-idx+1);
        Webc_Trie i;
        for(i=cur->head;i;i=i->next){
            if(hashcode==i->hashcode)
            {
                tree=i;
                break;
            }
        }
        if(i==NULL)
            return (Webc_Processer){NULL,NULL,NULL};
        else{
            if(next==length)
                return i->processer;
            cur=i;
        }
        idx=next;
    }
    return cur->processer;
}
void TrieClean(Webc_Trie tree){
    NOTNULL(tree);
    for (Webc_Trie i = tree->head; i ; i=i->next)
        TrieClean(i);
    if(tree->path)
        free(tree->path);
    free(tree);
}