#ifndef _INCLUDE_SOURCEMOD_SIMPLE_TRIE_H_
#define _INCLUDE_SOURCEMOD_SIMPLE_TRIE_H_

struct Trie;

typedef void (*SM_TRIE_BAD_ITERATOR)(Trie *pTrie, const char *key, void **value, void *data);

Trie *sm_trie_create();
void sm_trie_destroy(Trie *trie);
bool sm_trie_insert(Trie *trie, const char *key, void *value);
bool sm_trie_replace(Trie *trie, const char *key, void *value);
bool sm_trie_retrieve(Trie *trie, const char *key, void **value);
bool sm_trie_delete(Trie *trie, const char *key);
void sm_trie_clear(Trie *trie);
size_t sm_trie_mem_usage(Trie *trie);
void sm_trie_bad_iterator(Trie *trie,
						  char *buffer,
						  size_t maxlength,
						  SM_TRIE_BAD_ITERATOR iter,
						  void *data);

#endif //_INCLUDE_SOURCEMOD_SIMPLE_TRIE_H_
