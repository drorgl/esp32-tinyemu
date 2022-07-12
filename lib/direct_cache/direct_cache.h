#pragma once

#include <stddef.h>

struct _direct_cache_t;
typedef struct _direct_cache_t direct_cache_t;

direct_cache_t *direct_cache_init(size_t cache_size);

void *direct_cache_get(direct_cache_t *cache, void *key);

void direct_cache_set(direct_cache_t *cache, void *key, void *value);
void direct_cache_remove(direct_cache_t *cache, void *key);

void direct_cache_free(direct_cache_t *cache);