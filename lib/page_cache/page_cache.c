
// should initialize himem to blocks of page_size
// should use splay-tree and direct-cache to store page indexes
// should evict unused pages

#include "page_cache.h"

#include <himem_allocator.h>
#include <lru_cache.h>
#include <direct_cache.h>

#include <malloc.h>
#include <stdio.h>

#include <assert.h>

#include <log.h>

struct _page_cache_t
{
    cache_t *lru_cache;
    direct_cache_t * direct_cache;
    size_t page_size;

    size_t himem_maximum_blocks;
    // size_t himem_available_blocks;
    size_t himem_last_allocated_block;
    himem_t * himem;
    void * flush_buffer;
    void * flush_context;
    void (*on_flush)(size_t page_number, void * buf, void* flush_context);
    void (*on_pre_flush)(size_t page_number, void * buf, void* flush_context);
};

struct page_cache_item_t
{
    size_t page_number;
    size_t block_id;
    bool active;
};

static int compare_page_number(const void *e1, const void *e2)
{
    return (intptr_t)e1 - (intptr_t)e2;
}

static void on_page_flush(void *key, void *value, void *context)
{
    page_cache_t *page_cache =context;
    if (page_cache->on_pre_flush != NULL){
        page_cache->on_pre_flush((size_t)key,page_cache->flush_buffer, page_cache->flush_context);
    }
    struct page_cache_item_t *page_item = value;
    if (page_item == NULL || !page_item->active)
    {
        log_trace("page %zu is not active, skipping flush", (size_t)page_item->page_number);
        return; // nothing to flush
    }   
    himem_read(page_cache->himem, page_item->block_id, page_cache->flush_buffer, page_cache->page_size);
    
    page_cache->on_flush((size_t)key,page_cache->flush_buffer, page_cache->flush_context);
}

page_cache_t *page_cache_init(size_t page_size, size_t maximum_himem_blocks, void (*on_pre_flush)(size_t page_number, void * buf,void * flush_context), void (*on_flush)(size_t page_number, void * buf,void * flush_context), void* flush_context)
{
    page_cache_t *page_cache = (page_cache_t *)malloc(sizeof(page_cache_t));
    page_cache->flush_context = flush_context;
    page_cache->on_flush = on_flush;
    page_cache->on_pre_flush = on_pre_flush;
    page_cache->page_size = page_size;
    page_cache->flush_buffer = malloc(page_size);

    assert(maximum_himem_blocks > 0 && "maximum_himem_blocks must be greater than 0");
    page_cache->himem_maximum_blocks = maximum_himem_blocks;

    log_debug("allocating %d blocks", page_cache->himem_maximum_blocks);
    //page_cache->himem_available_blocks = 
    page_cache->himem = himem_allocator_init(page_size, page_cache->himem_maximum_blocks);
    page_cache->himem_last_allocated_block = 0;

    page_cache->lru_cache = lru_cache_init(compare_page_number, on_page_flush, page_cache);
    page_cache->direct_cache = direct_cache_init(2048);

    return page_cache;
}

bool page_cache_get(page_cache_t *page_cache, size_t page_number, void *buff)
{
    log_trace("getting page %d", page_number);
    struct page_cache_item_t *page_item = direct_cache_get(page_cache->direct_cache, (void*)page_number);
    if (page_item == NULL){
        page_item = lru_cache_get(page_cache->lru_cache, (void*)page_number);
        direct_cache_set(page_cache->direct_cache,(void*)page_number, page_item);
    }
    if (page_item != NULL)
    {
        size_t block_id = page_item->block_id;
        if (!page_item->active)
        {
            log_trace("page %zu is not active, cannot read", page_number);
            return false; // page is not active, cannot read
        }

        log_trace("page %zu, retrieved from block %d", page_number, block_id);
        himem_read(page_cache->himem,block_id, buff, page_cache->page_size);
        return true;
    }
    log_trace("page %zu not found in cache", page_number);
    return false;
}

void page_cache_deactivate(page_cache_t *page_cache, size_t page_number)
{
    log_trace("deactivating page %d", page_number);
    struct page_cache_item_t *page_item = direct_cache_get(page_cache->direct_cache, (void*)page_number);
    if (page_item == NULL){
        page_item = lru_cache_get(page_cache->lru_cache, (void*)page_number);
        direct_cache_set(page_cache->direct_cache,(void*)page_number, page_item);
    }

    if (page_item != NULL)
    {
        log_trace("deactivating page %zu on block %d", page_number, page_item->block_id);
        page_item->active = false;
    }
}

void page_cache_set(page_cache_t *page_cache, size_t page_number,const void *buff)
{
    log_trace("setting page %d", page_number);
    struct page_cache_item_t *page_item = direct_cache_get(page_cache->direct_cache, (void*)page_number);
    if (page_item == NULL){
        page_item = lru_cache_get(page_cache->lru_cache, (void*)page_number);
        direct_cache_set(page_cache->direct_cache,(void*)page_number, page_item);
    }

    if (page_item != NULL)
    {
        size_t block_id = page_item->block_id;
        log_trace("setting page %zu on block %d", page_number, block_id);
        himem_write(page_cache->himem,block_id, buff, page_cache->page_size);
    }
    else
    {
        size_t block_id;
        if (page_cache->himem_last_allocated_block < page_cache->himem_maximum_blocks)
        {
            page_item = (struct page_cache_item_t *)malloc(sizeof(struct page_cache_item_t));
            block_id = page_cache->himem_last_allocated_block++;
            page_item->page_number = page_number;
            page_item->block_id = block_id;
            log_trace("setting page %zu on new block %d", page_number, block_id);
        }
        else
        {
            page_item = lru_cache_get_least_recently_used(page_cache->lru_cache,-1);
            lru_cache_remove(page_cache->lru_cache, (void*)page_item->page_number);
            direct_cache_remove(page_cache->direct_cache, (void*)page_item->page_number);
            
            block_id = page_item->block_id;
            page_item->page_number = page_number;
            
            log_trace("setting page %zu on reused block %d", page_number, block_id);
        }

        // printf("writing block %d for page %d", (int)block_id, (int)page_number);
        himem_write(page_cache->himem,block_id, buff, page_cache->page_size);
        lru_cache_add(page_cache->lru_cache, (void*)page_number, page_item);
        direct_cache_set(page_cache->direct_cache,(void*)page_number, page_item);
        page_item->active = true;
    }
}


void page_cache_flush(page_cache_t *page_cache){
    log_trace("flushing");
    lru_cache_sync(page_cache->lru_cache);
}