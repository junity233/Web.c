/*
 * @Author: your name
 * @Date: 2021-08-18 16:25:55
 * @LastEditTime: 2021-08-21 22:49:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /web.c/map.h
 */
#ifndef _MAP_H
#define _MAP_H

#include"utils.h"

typedef struct _webc_mapEntry{
    uint32_t hashcode;
    char* key;
    char* value;
    size_t valueLength;//map可能储存二进制数据，其中可能会有\0
    struct _webc_mapEntry *next;
}Webc_MapEntry;

Webc_MapEntry* Webc_NewEntry(const char* key,const char* value,size_t valueLength);
void Webc_EntryClean(Webc_MapEntry* entry);

typedef struct _map
{
    Webc_MapEntry* head;
    Webc_MapEntry* tail;
    uint32_t cnt;
}*Webc_Map;

#define WEBC_MAP_FOREACH(map,iterator) for(Webc_MapEntry* iterator=(map)->head;iterator!=NULL;iterator=iterator->next)

Webc_Map NewMap();

void MapReset(Webc_Map map);

void MapSet(Webc_Map map,const char* key,const char* value,size_t valueLength);

char* MapGet(Webc_Map map,const char* key,size_t* length);

int MapRemove(Webc_Map map,const char* key);

void MapClean(Webc_Map map);

#endif