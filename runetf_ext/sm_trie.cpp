#include <string.h>
#include <assert.h>
#include <sm_trie_tpl.h>
#include "sm_trie.h"

struct Trie
{
	KTrie<void *> k;
};

Trie *sm_trie_create()
{
	return new Trie;
}

void sm_trie_destroy(Trie *trie)
{
	delete trie;
}

bool sm_trie_insert(Trie *trie, const char *key, void *value)
{
	return trie->k.insert(key, value);
}

bool sm_trie_replace(Trie *trie, const char *key, void *value)
{
	return trie->k.replace(key, value);
}

bool sm_trie_retrieve(Trie *trie, const char *key, void **value)
{
	void **pValue = trie->k.retrieve(key);
	
	if (!pValue)
	{
		return false;
	}

	if (value)
	{
		*value = *pValue;
	}

	return true;
}

bool sm_trie_delete(Trie *trie, const char *key)
{
	return trie->k.remove(key);
}

void sm_trie_clear(Trie *trie)
{
	trie->k.clear();
}

size_t sm_trie_mem_usage(Trie *trie)
{
	return trie->k.mem_usage();
}

struct trie_iter_data
{
	SM_TRIE_BAD_ITERATOR iter;
	void *ptr;
	Trie *pTrie;
};

void our_trie_iterator(KTrie<void *> *pTrie, const char *name, void *& obj, void *data)
{
	trie_iter_data *our_iter;

	our_iter = (trie_iter_data *)data;
	our_iter->iter(our_iter->pTrie, name, &obj, our_iter->ptr);
}

void sm_trie_bad_iterator(Trie *trie,
						  char *buffer,
						  size_t maxlength,
						  SM_TRIE_BAD_ITERATOR iter,
						  void *data)
{
	trie_iter_data our_iter;

	our_iter.iter = iter;
	our_iter.ptr = data;
	our_iter.pTrie = trie;
	trie->k.bad_iterator(buffer, maxlength, &our_iter, our_trie_iterator);
}
