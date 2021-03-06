/*
 * @Author: Junity
 * @Date: 2021-08-22 21:31:55
 * @Description: 键值对字典，内部用list+hashcode实现
 */
#ifndef _MAP_H
#define _MAP_H

#include"utils.h"

typedef void (*Webc_OnMapDataClean)(void* data);
#define WEBC_MAP_FREE_MEMORY NULL

//虽然叫map，其实是用list实现的。
typedef struct _webc_mapEntry{
    uint32_t hashcode;
    char* key;
    void* data;
    struct _webc_mapEntry *next;
}Webc_MapEntry;

Webc_MapEntry* Webc_NewEntry(const char* key,void* data);
void Webc_EntryClean(Webc_MapEntry* entry,Webc_OnMapDataClean handle);

typedef struct _map
{
    Webc_MapEntry* head;
    Webc_MapEntry* tail;
    Webc_OnMapDataClean handle;
    uint32_t cnt;
}*Webc_Map;

#define WEBC_MAP_FOREACH(map,iterator) for(Webc_MapEntry* iterator=(map)->head;iterator!=NULL;iterator=iterator->next)

Webc_Map NewMap(Webc_OnMapDataClean handle);

void MapReset(Webc_Map map);

void MapSet(Webc_Map map,const char* key,void* data);

void* MapGet(Webc_Map map,const char* key);

int MapRemove(Webc_Map map,const char* key);

void MapClean(Webc_Map map);

#endif