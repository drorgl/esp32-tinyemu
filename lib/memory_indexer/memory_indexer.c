#include "memory_indexer.h"

#include <avltree.h>
#include <string.h>
#include <malloc.h>

struct _memory_indexer_t
{
    struct avl_tree avl_tree;
};

struct memory_wrapper
{
    struct avl_node avl;
    uint64_t key;
    void *value;
};

static int cmp_func(struct avl_node *a, struct avl_node *b, void *aux)
{
    (void)(aux);
    struct memory_wrapper *aa, *bb;
    aa = _get_entry(a, struct memory_wrapper, avl);
    bb = _get_entry(b, struct memory_wrapper, avl);

    if (aa->key < bb->key)
        return -1;
    else if (aa->key > bb->key)
        return 1;
    else
        return 0;
}

memory_indexer_t *memory_indexer_init()
{
    memory_indexer_t *indexer = (memory_indexer_t *)malloc(sizeof(memory_indexer_t));
    memset(indexer, 0, sizeof(memory_indexer_t));
    avl_init(&indexer->avl_tree, NULL);

    return indexer;
}
void memory_indexer_set(memory_indexer_t *indexer, uint64_t key, void *value)
{
    memory_indexer_remove(indexer, key);

    struct memory_wrapper *memory_item = (struct memory_wrapper *)malloc(sizeof(struct memory_wrapper));
    memory_item->key = key;
    memory_item->value = value;

    if (avl_insert(&indexer->avl_tree, &memory_item->avl, cmp_func) == &memory_item->avl)
    {
        // printf("added successfully %d\n", key);
        return; 
    }
    // printf("duplicate %d\n", key);

}
void *memory_indexer_search(memory_indexer_t *indexer, uint64_t key)
{
    struct memory_wrapper query = {};
    query.key = key;
    struct avl_node *avl_node = avl_search(&indexer->avl_tree, &query.avl, cmp_func);
    if (avl_node)
    {
        struct memory_wrapper *wrapper = _get_entry(avl_node, struct memory_wrapper, avl);
        return wrapper->value;
    }
    return NULL;
}
void memory_indexer_remove(memory_indexer_t *indexer, uint64_t key)
{
    struct memory_wrapper query;
    query.key = key;
    struct avl_node *avl_node = avl_search(&indexer->avl_tree, &query.avl, cmp_func);
    if (avl_node)
    {
        struct memory_wrapper *wrapper = _get_entry(avl_node, struct memory_wrapper, avl);
        avl_remove(&indexer->avl_tree, avl_node);
        free(wrapper);
    }
}

size_t memory_indexer_free(memory_indexer_t *indexer)
{
    size_t x = 0;
    struct avl_node *avl_node = avl_first(&indexer->avl_tree);
    while (avl_node)
    {
        struct memory_wrapper *node = _get_entry(avl_node, struct memory_wrapper, avl);
        avl_node = avl_next(avl_node);
        avl_remove(&indexer->avl_tree, &node->avl);
        x++;
    }
    free(indexer);
    return x;
}