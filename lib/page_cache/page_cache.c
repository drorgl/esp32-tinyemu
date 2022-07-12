
// should initialize himem to blocks of page_size
// should use splay-tree and direct-cache to store page indexes
// should evict unused pages

#include "page_cache.h"

#include <himem_allocator.h>
#include <lru_cache.h>
#include <direct_cache.h>

#include <malloc.h>
#include <stdio.h>

struct _page_cache_t
{
    cache_t *lru_cache;
    direct_cache_t * direct_cache;
    uint64_t page_size;

    uint16_t himem_maximum_blocks;
    // uint16_t himem_available_blocks;
    uint16_t himem_last_allocated_block;
    himem_t * himem;
    void * flush_buffer;
    void * flush_context;
    void (*on_flush)(uint64_t page_number, void * buf, void* flush_context)
};

struct page_cache_item_t
{
    uint64_t page_number;
    uint16_t block_id;
};

static int compare_page_number(const void *e1, const void *e2)
{
    return (intptr_t)e1 - (intptr_t)e2;
}

static void on_page_flush(void *key, void *value, void *context)
{
    page_cache_t *page_cache =context;
    struct page_cache_item_t *page_item = value;
    himem_read(page_cache->himem, page_item->block_id, page_cache->flush_buffer, page_cache->page_size);
    page_cache->on_flush(key,page_cache->flush_buffer, page_cache->flush_context);
}

page_cache_t *page_cache_init(size_t page_size, uint16_t maximum_himem_blocks, void (*on_flush)(uint64_t page_number, void * buf,void * flush_context), void* flush_context)
{
    page_cache_t *page_cache = (page_cache_t *)malloc(sizeof(page_cache_t));
    page_cache->flush_context = flush_context;
    page_cache->on_flush = on_flush;
    page_cache->page_size = page_size;
    page_cache->flush_buffer = malloc(page_size);

    page_cache->himem_maximum_blocks = maximum_himem_blocks;

    // printf("allocating %d blocks\n", page_cache->himem_maximum_blocks);
    //page_cache->himem_available_blocks = 
    page_cache->himem = himem_allocator_init(page_size, page_cache->himem_maximum_blocks);
    page_cache->himem_last_allocated_block = 0;

    page_cache->lru_cache = lru_cache_init(compare_page_number, on_page_flush, page_cache);
    page_cache->direct_cache = direct_cache_init(1024);

    return page_cache;
}

bool page_cache_get(page_cache_t *page_cache, uint64_t page_number, void *buff)
{
    // printf("getting page %d\n", page_number);
    struct page_cache_item_t *page_item = direct_cache_get(page_cache->direct_cache, page_number);
    if (page_item == NULL){
        page_item = lru_cache_get(page_cache->lru_cache, page_number);
    }
    if (page_item != NULL)
    {
        uint16_t block_id = page_item->block_id;
        // printf("got block %d\n", block_id);
        himem_read(page_cache->himem,block_id, buff, page_cache->page_size);
        return true;
    }
    return false;
}


void page_cache_set(page_cache_t *page_cache, uint64_t page_number, void *buff)
{
    // printf("setting page %d\n", page_number);
    struct page_cache_item_t *page_item = direct_cache_get(page_cache->direct_cache, page_number);
    if (page_item == NULL){
        page_item = lru_cache_get(page_cache->lru_cache, page_number);
    }

    if (page_item != NULL)
    {
        uint16_t block_id = page_item->block_id;
        // printf("on block %d \n", block_id);
        himem_write(page_cache->himem,block_id, buff, page_cache->page_size);
    }
    else
    {
        uint16_t block_id;
        if (page_cache->himem_last_allocated_block < page_cache->himem_maximum_blocks)
        {
            page_item = (struct page_cache_item_t *)malloc(sizeof(struct page_cache_item_t));
            block_id = page_cache->himem_last_allocated_block++;
            page_item->page_number = page_number;
            page_item->block_id = block_id;
            // printf("on new block %d \n", block_id);
        }
        else
        {
            page_item = lru_cache_get_least_recently_used(page_cache->lru_cache);
            lru_cache_remove(page_cache->lru_cache, page_item->page_number);
            direct_cache_remove(page_cache->direct_cache, page_item->page_number);

            block_id = page_item->block_id;
            page_item->page_number = page_number;

            // printf("on reused block %d \n", block_id);
        }

        // printf("writing block %d for page %d\n", (int)block_id, (int)page_number);
        himem_write(page_cache->himem,block_id, buff, page_cache->page_size);
        lru_cache_add(page_cache->lru_cache, page_number, page_item);
        direct_cache_set(page_cache->direct_cache,page_number, page_item);
    }
}
