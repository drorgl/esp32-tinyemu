#pragma once

#include <stddef.h>
#include <stdint.h>

struct _memory_indexer_t;
typedef struct _memory_indexer_t memory_indexer_t;

memory_indexer_t * memory_indexer_init();
void memory_indexer_set(memory_indexer_t *indexer, uint64_t key, void * value);
void * memory_indexer_search(memory_indexer_t *indexer, uint64_t key);
void memory_indexer_remove(memory_indexer_t *indexer, uint64_t key);
size_t memory_indexer_free(memory_indexer_t * indexer);