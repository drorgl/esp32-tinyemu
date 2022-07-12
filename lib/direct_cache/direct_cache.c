#include "direct_cache.h"

#include <stdint.h>
#include <malloc.h>

struct cache_line
{
    void *value;
    void *key;
};

struct _direct_cache_t
{
    struct cache_line *cache;
    size_t cache_size;
};

direct_cache_t *direct_cache_init(size_t cache_size)
{
    direct_cache_t *cache = (direct_cache_t *)malloc(sizeof(direct_cache_t));
    cache->cache_size = cache_size;
    cache->cache = (struct cache_line *)malloc(sizeof(struct cache_line) * cache_size);
    for (size_t i = 0; i < cache_size;i++){
        cache->cache[i].key = i + 1;
    }
    return cache;
}

void *direct_cache_get(direct_cache_t *cache, void *key)
{
    uint16_t cell = (intptr_t)(key) % cache->cache_size;
    struct cache_line line = cache->cache[cell];
    if (line.key == key)
    {
        return line.value;
    }
    return NULL;
}

void direct_cache_set(direct_cache_t *cache, void *key, void *value)
{
    uint16_t cell = (intptr_t)(key) % cache->cache_size;
    cache->cache[cell].key = key;
    cache->cache[cell].value = value;
}

void direct_cache_remove(direct_cache_t *cache, void *key)
{
    uint16_t cell = (intptr_t)(key) % cache->cache_size;
    cache->cache[cell].key = cell + 1;
    cache->cache[cell].value = NULL;
}

void direct_cache_free(direct_cache_t *cache)
{
    free(cache->cache);
    free(cache);
}