#include <lru_cache.h>

#include <llist.h>
#include <malloc.h>
#include <memory_indexer.h>

typedef struct
{
    void *key;
    void *value;
    list_node_t *node;

} cache_item_t;

struct _cache_t
{
    list_t *cache_item_list;
    memory_indexer_t * memory_indexer;
    size_t number_of_items;

    void (*on_flush)(void *key, void *value, void *context);
    void *context;
};

cache_t *lru_cache_init(int (*compare_key)(const void *, const void *), void (*on_flush)(void *key, void *value, void *context), void *context)
{
    cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
    cache->cache_item_list = list_new();
    cache->memory_indexer = memory_indexer_init();
    cache->on_flush = on_flush;
    cache->number_of_items = 0;
    cache->context = context;
    return cache;
}

size_t lru_cache_count(cache_t *cache)
{
    return cache->number_of_items;
}


void *lru_cache_get(cache_t *cache, void *key)
{
    cache_item_t * cache_item = memory_indexer_search(cache->memory_indexer, (intptr_t)key);
    // cache_item_t *cache_item = (cache_item_t *)splaytree_get(cache->cache_search_tree, key);

    if (cache_item != NULL)
    {
        list_lpush(cache->cache_item_list, cache_item->node);
        return cache_item->value;
    }
    return NULL;
}

void lru_cache_sync(cache_t *cache)
{
    list_node_t *node;
    list_iterator_t *it = list_iterator_new(cache->cache_item_list, LIST_HEAD);
    while ((node = list_iterator_next(it)))
    {
        cache_item_t *cache_item = (cache_item_t *)node->val;
        if (cache_item != NULL)
        {
            cache->on_flush(cache_item->key, cache_item->value, cache->context);
        }
    }
    list_iterator_destroy(it);
}

void lru_cache_remove(cache_t *cache, void *key)
{
    cache_item_t * cache_item = memory_indexer_search(cache->memory_indexer, (intptr_t)key);
    // cache_item_t *cache_item = (cache_item_t *)splaytree_get(cache->cache_search_tree, key);
    if (cache_item != NULL)
    {
        cache->on_flush(cache_item->key, cache_item->value, cache->context);

        list_remove(cache->cache_item_list, cache_item->node);
        cache_item->node = NULL;

        memory_indexer_remove(cache->memory_indexer,(intptr_t) key);

        free(cache_item);

        cache->number_of_items--;
    }
}

void lru_cache_flush_items(cache_t *cache, size_t number_of_items_to_flush)
{
    for (size_t i = 0; i < number_of_items_to_flush; i++)
    {
        list_node_t *last = list_rpop(cache->cache_item_list);

        if (last == NULL)
        {
            break;
        }

        cache_item_t *cache_item = (cache_item_t *)last->val;
        cache->on_flush(cache_item->key, cache_item->value, cache->context);
        free(cache_item->node);
        cache_item->node = NULL;
        memory_indexer_remove(cache->memory_indexer, cache_item->key);
        free(cache_item);
        cache->number_of_items--;
    }
}

void *lru_cache_get_least_recently_used(cache_t *cache)
{
    list_node_t *last = list_at(cache->cache_item_list, -1);
    if (last != NULL)
    {
        return ((cache_item_t*)last->val)->value;
    }
    return NULL;
}

void lru_cache_add(cache_t *cache, void *key, void *value)
{
    cache_item_t *cache_item = (cache_item_t *)malloc(sizeof(cache_item_t));
    cache_item->key = key;
    cache_item->value = value;
    cache_item->node = list_node_new(cache_item);
    memory_indexer_set(cache->memory_indexer, (intptr_t) key, cache_item);
    list_lpush(cache->cache_item_list, cache_item->node);
    cache->number_of_items++;
}

void lru_cache_free(cache_t *cache)
{
    lru_cache_flush_items(cache, cache->number_of_items);
    list_destroy(cache->cache_item_list);
    memory_indexer_free(cache->memory_indexer);
    free(cache);
}