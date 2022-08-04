#include "fast_cache.h"

#include <llist.h>
#include <malloc.h>
#include <memory_indexer.h>
#include <direct_cache.h>
#include <stdbool.h>
#include <log.h>
#include <assert.h>

typedef struct
{
    void *key;
    void *value;
    list_node_t *node;

} fast_cache_item_t;

struct _fast_cache_t
{
    list_t *cache_item_list;
    memory_indexer_t *memory_indexer;
    direct_cache_t *direct_cache; // how to integrate
    size_t number_of_items;

    void *(*on_fault)(void *key, void *context);
    void (*on_flush)(void *key, void *value, void *context);
    void (*on_remove)(void *key, void *value, void *context);
    void *context;
};

fast_cache_t *fast_cache_init(
    size_t fast_index_size,
    void *(*on_fault)(void *key, void *context),
    void (*on_flush)(void *key, void *value, void *context),
    void (*on_remove)(void *key, void *value, void *context),
    void *context)
{
    fast_cache_t *cache = (fast_cache_t *)malloc(sizeof(fast_cache_t));
    assert(cache);
    cache->cache_item_list = list_new();
    cache->memory_indexer = memory_indexer_init();
    cache->direct_cache = direct_cache_init(fast_index_size);
    cache->on_flush = on_flush;
    cache->on_fault = on_fault;
    cache->on_remove = on_remove;
    cache->number_of_items = 0;
    cache->context = context;
    return cache;
}

size_t fast_cache_count(fast_cache_t *cache)
{
    return cache->number_of_items;
}

fast_cache_item_t *get_cache_item(fast_cache_t *cache, void *key)
{
    fast_cache_item_t *cache_item = direct_cache_get(cache->direct_cache, key);

    if (cache_item == NULL)
    {
        cache_item = memory_indexer_search(cache->memory_indexer, (intptr_t)key);
    }

    return cache_item;
}

void *fast_cache_get(fast_cache_t *cache, void *key)
{

    fast_cache_item_t *cache_item = direct_cache_get(cache->direct_cache, key);

    bool direct_cache_repopulate = false;

    if (cache_item == NULL)
    {
        cache_item = memory_indexer_search(cache->memory_indexer, (intptr_t)key);
        direct_cache_repopulate = true;
    }

    if (cache_item != NULL)
    {
        if (direct_cache_repopulate)
        {
            direct_cache_set(cache->direct_cache, key, cache_item);
        }

        list_lpush(cache->cache_item_list, cache_item->node);
        return cache_item->value;
    }
    else
    {
        log_debug("fault %d", (intptr_t)key);
        void *value = cache->on_fault(key, cache->context);
        fast_cache_add(cache, key, value);
        return value;
    }
    return NULL;
}

void *fast_cache_get_least_recently_used(fast_cache_t *cache)
{
    list_node_t *last = list_at(cache->cache_item_list, -1);
    if (last != NULL)
    {
        return ((fast_cache_item_t *)last->val)->value;
    }
    return NULL;
}

void fast_cache_remove(fast_cache_t *cache, void *key)
{
    fast_cache_item_t *cache_item = get_cache_item(cache, key);
    if (cache_item != NULL)
    {
        cache->on_flush(cache_item->key, cache_item->value, cache->context);
        cache->on_remove(cache_item->key, cache_item->value, cache->context);

        list_remove(cache->cache_item_list, cache_item->node);
        cache_item->node = NULL;

        direct_cache_remove(cache->direct_cache, key);
        memory_indexer_remove(cache->memory_indexer, (intptr_t)key);

        free(cache_item);

        cache->number_of_items--;
    }
}

void fast_cache_sync(fast_cache_t *cache)
{
    list_node_t *node;
    list_iterator_t *it = list_iterator_new(cache->cache_item_list, LIST_HEAD);
    while ((node = list_iterator_next(it)))
    {
        fast_cache_item_t *cache_item = (fast_cache_item_t *)node->val;
        if (cache_item != NULL)
        {
            cache->on_flush(cache_item->key, cache_item->value, cache->context);
        }
    }
    list_iterator_destroy(it);
}
void fast_cache_flush_items(fast_cache_t *cache, size_t number_of_items_to_flush)
{
    for (size_t i = 0; i < number_of_items_to_flush; i++)
    {
        list_node_t *last = list_rpop(cache->cache_item_list);

        if (last == NULL)
        {
            break;
        }

        fast_cache_item_t *cache_item = (fast_cache_item_t *)last->val;
        cache->on_flush(cache_item->key, cache_item->value, cache->context);
        cache->on_remove(cache_item->key, cache_item->value, cache->context);
        free(cache_item->node);
        cache_item->node = NULL;
        direct_cache_remove(cache->direct_cache, cache_item->key);
        memory_indexer_remove(cache->memory_indexer, (intptr_t)cache_item->key);
        free(cache_item);

        cache->number_of_items--;
    }
}

void fast_cache_add(fast_cache_t *cache, void *key, void *value)
{
    fast_cache_item_t *cache_item = (fast_cache_item_t *)malloc(sizeof(fast_cache_item_t));
    assert(cache_item);
    cache_item->key = key;
    cache_item->value = value;
    cache_item->node = list_node_new(cache_item);
    direct_cache_set(cache->direct_cache, key, cache_item);
    memory_indexer_set(cache->memory_indexer, (intptr_t)key, cache_item);
    list_lpush(cache->cache_item_list, cache_item->node);
    cache->number_of_items++;
}

void fast_cache_free(fast_cache_t *cache)
{
    fast_cache_flush_items(cache, cache->number_of_items);
    list_destroy(cache->cache_item_list);
    direct_cache_free(cache->direct_cache);
    memory_indexer_free(cache->memory_indexer);
    free(cache);
}