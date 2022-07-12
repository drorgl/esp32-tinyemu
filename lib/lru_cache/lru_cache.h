#pragma once

#include <stddef.h>

struct _cache_t;
typedef struct _cache_t cache_t;

cache_t * lru_cache_init(int (*compare_key)(const void *, const void *), void (*on_flush)(void *key, void *value, void * context), void * context);

size_t lru_cache_count(cache_t * cache);

void *lru_cache_get(cache_t *cache, void *key);

void *lru_cache_get_least_recently_used(cache_t * cache);

void lru_cache_remove(cache_t * cache, void* key);

void lru_cache_sync(cache_t * cache);
void lru_cache_flush_items(cache_t *cache, size_t number_of_items_to_flush);

void lru_cache_add(cache_t *cache, void *key, void *value);

void lru_cache_free(cache_t * cache);