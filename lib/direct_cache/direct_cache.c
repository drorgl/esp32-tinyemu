#include "direct_cache.h"

#include <stdint.h>
#include <malloc.h>

#include <assert.h>

#include <math.h>

#ifdef ESP32
#include <esp_heap_caps.h>
#endif

struct cache_line
{
    void *value;
    void *key;
};

struct _direct_cache_t
{
    struct cache_line *cache;
    size_t cache_size;

    size_t cache_mask;
};

static bool is_power_of_2(size_t n){
    const float i = log2(n);
    // check if n is a power of 2
    if (ceil(i) == floor(i)) {
      return true;
    } 
    return false;
}

direct_cache_t *direct_cache_init(size_t cache_size)
{
    direct_cache_t *cache = (direct_cache_t *)malloc(sizeof(direct_cache_t));

    assert(is_power_of_2(cache_size) && "cache size must be power of 2");
    cache->cache_mask = cache_size -1;

    cache->cache_size = cache_size;
#ifdef ESP32
    cache->cache = (struct cache_line *)heap_caps_malloc(sizeof(struct cache_line) * cache_size,MALLOC_CAP_INTERNAL );
#else
    cache->cache = (struct cache_line *)malloc(sizeof(struct cache_line) * cache_size);
#endif
    for (size_t i = 0; i < cache_size;i++){
        cache->cache[i].key = (void*)i + 1;
    }
    return cache;
}

void *direct_cache_get(direct_cache_t *cache, void *key)
{
    const uint16_t cell = (intptr_t)(key) % cache->cache_size;
    // uint16_t cell = (intptr_t)(key) & cache->cache_mask;
    const struct cache_line line = cache->cache[cell];

    // static uint32_t cache_miss = 0;
    // static uint32_t cache_hit = 0;


    // if ((cache_miss > 0 && cache_miss % 1000000 == 0) || (cache_hit > 0 && cache_hit % 1000000 == 0))
    // {
    //     printf("direct cache miss %zu, direct cache hit %zu\r\n", cache_miss, cache_hit);
    // }
    

    if (line.key == key)
    {
        // cache_hit++;
        return line.value;
    }
    // cache_miss++;
    return NULL;
}

void direct_cache_set(direct_cache_t *cache, void *key, void *value)
{
    const uint16_t cell = (intptr_t)(key) % cache->cache_size;
    // uint16_t cell = (intptr_t)(key) & cache->cache_mask;
    cache->cache[cell].key = key;
    cache->cache[cell].value = value;
}

void direct_cache_remove(direct_cache_t *cache, void *key)
{
    const uint16_t cell = (intptr_t)(key) % cache->cache_size;
    // uint16_t cell = (intptr_t)(key) & cache->cache_mask;
    cache->cache[cell].key = (void*)cell + 1;
    cache->cache[cell].value = NULL;
}

void direct_cache_free(direct_cache_t *cache)
{
    free(cache->cache);
    free(cache);
}