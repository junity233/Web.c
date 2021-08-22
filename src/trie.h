#ifndef _TRIE_H
#define _TRIE_H

#include"map.h"
#include"web.h"

typedef struct _trie_node{
    uint32_t hashcode;
    const char* path;
    bool is_leaf;
    Webc_Processer processer;
    struct _trie_node* head;
    struct _trie_node* next;
}*Webc_Trie;

Webc_Trie NewTrie();

void TrieInsert(Webc_Trie tree,const char* path,Webc_Processer processer);

Webc_Processer TrieGet(Webc_Trie tree,const char* path);

void TrieClean(Webc_Trie tree);

#endif