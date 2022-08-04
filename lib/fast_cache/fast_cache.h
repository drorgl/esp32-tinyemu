#pragma once

#include <stddef.h>

struct _fast_cache_t;
typedef struct _fast_cache_t fast_cache_t;

fast_cache_t *fast_cache_init(
    size_t fast_index_size,
    void *(*on_fault)(void *key, void *context),
    void (*on_flush)(void *key, void *value, void *context),
    void (*on_remove)(void * key, void * value, void * context),
    void *context);

size_t fast_cache_count(fast_cache_t * cache);

void *fast_cache_get(fast_cache_t *cache, void *key);

void *fast_cache_get_least_recently_used(fast_cache_t * cache);

void fast_cache_remove(fast_cache_t * cache, void* key);

void fast_cache_sync(fast_cache_t * cache);
void fast_cache_flush_items(fast_cache_t *cache, size_t number_of_items_to_flush);

void fast_cache_add(fast_cache_t *cache, void *key, void *value);

void fast_cache_free(fast_cache_t * cache);