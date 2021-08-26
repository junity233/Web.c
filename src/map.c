#include"map.h"


Webc_MapEntry* Webc_NewEntry(const char* key,void* data){
    Webc_MapEntry *res=MALLOC(Webc_MapEntry);
    res->key=copystr(key);
    res->hashcode=HashString(key,strlen(key));
    res->data=data;
    res->next=NULL;
    return res;
}

void Webc_EntryClean(Webc_MapEntry *entry,Webc_OnMapDataClean handle){
    ASSERT(entry!=NULL,"被清理的Entry为NULL！");
    if(entry->key)
        free(entry->key);
    if(entry->data){
        (*handle)(entry->data);
    }
    free(entry);
}

Webc_Map NewMap(Webc_OnMapDataClean handle){
    Webc_Map map=MALLOC(struct _map);
    MapReset(map);
    map->handle=handle;
    return map;
}

void MapReset(Webc_Map map){
    ASSERT(map!=NULL,"被Reset的Map为NULL！");
    map->cnt=0;
    map->head=map->tail=NULL;
}

void MapSet(Webc_Map map,const char* key,void* data){
    ASSERT(map!=NULL,"执行MapSet的map为NULL！");
    uint32_t hashcode=HashString(key,strlen(key));
    ASSERT(key!=NULL,"添加的key为NULL！");
    WEBC_MAP_FOREACH(map,i){
        if(i->hashcode==hashcode)
        {
            (*map->handle)(i->data);
            i->data=data;
            return;
        }
    }
    Webc_MapEntry* entry=Webc_NewEntry(key,data);
    if(map->tail){
        map->tail->next=entry;
        map->tail=entry;
    }
    else{
        map->head=map->tail=entry;
    }
    map->cnt++;
}

void* MapGet(Webc_Map map,const char* key){
    ASSERT(map!=NULL,"执行MapGet的map为NULL！");
    ASSERT(key!=NULL,"查找的key为NULL！");
    uint32_t hashcode=HashString(key,strlen(key));
    WEBC_MAP_FOREACH(map,i)
    {
        if(i->hashcode==hashcode)
        {
            return i->data;
        }
    }
    return NULL;
}

int MapRemove(Webc_Map map,const char* key){
    ASSERT(map!=NULL,"执行MapRemove的map为NULL！");
    ASSERT(key!=NULL,"移除的key为NULL！");
    uint32_t hashcode=HashString(key,strlen(key));
    for(Webc_MapEntry *i=map->head;i->next;i=i->next)
    {
        if(i->next->hashcode==hashcode)
        {
            Webc_MapEntry *next=i->next;
            i->next=next->next;
            Webc_EntryClean(i,map->handle);
            return 0;
        }
    }
    map->cnt--;
    return -1;
}

void MapClean(Webc_Map map){
    ASSERT(map!=NULL,"执行MapRemove的map为NULL！");
    Webc_MapEntry*pre=NULL;
    WEBC_MAP_FOREACH(map,i)
    {
        if(pre!=NULL)
            Webc_EntryClean(pre,map->handle);
        pre=i;
    }
    if(pre!=NULL)
        Webc_EntryClean(pre,map->handle);

}



