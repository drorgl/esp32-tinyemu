#include "himem_cache.h"

#include <stdbool.h>
#include <inttypes.h>

#include "fast_cache.h"
#include <malloc.h>
#include <log.h>
#include <string.h>
#include <assert.h>

#include <himem_allocator.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct _himem_page_item
{
    size_t page_number;
    uint16_t himem_block_id;
    int dirty_start;
    int dirty_length;
};

typedef struct _himem_page_item himem_page_item;

struct _himem_cache_t
{
    size_t page_size;
    size_t number_of_pages;
    fast_cache_t *cache;
    himem_t * himem;
    void *backend_context;
    void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length);
    void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length);

};

static void himem_cache_on_flush(void *key, void *value, void *context)
{
    log_debug("flushing page %d", (intptr_t)key);
    himem_cache_t *himem_cache = context;
    
    himem_page_item *item = value;
    if ((item->dirty_start != -1) && (item->dirty_length != -1))
    {
        log_debug("page %d, block %d is dirty from %d to %d", item->page_number, item->himem_block_id, item->dirty_start, item->dirty_length);
        void * buffer = malloc(himem_cache->page_size);//TODO: use memory pool
        assert(buffer);
        if (himem_read(himem_cache->himem, item->himem_block_id, item->dirty_start,buffer, item->dirty_length ) != item->dirty_length){
            log_error("Error reading from himem");
            assert(false);
        }
        himem_cache->write_backend(himem_cache->backend_context, item->page_number * himem_cache->page_size + item->dirty_start, buffer, item->dirty_length);
        free(buffer);
    }
}

static void himem_cache_on_remove(void *key, void *value, void *context)
{
    log_debug("removing page %d", (intptr_t)key);
    himem_cache_t *himem_cache = context;
    
    himem_page_item *item = value;
    himem_free_block(himem_cache->himem, item->himem_block_id);
    free(item);
}

static inline uint64_t get_page_number_by_address(himem_cache_t *himem_cache, size_t address)
{
    uint64_t page_number = address / himem_cache->page_size;
    return page_number;
}

static void *himem_cache_on_fault(void *key, void *context)
{
    log_trace("page fault %d", (intptr_t)key);
    himem_cache_t *himem_cache = context;

    himem_page_item *page;

    if (fast_cache_count(himem_cache->cache) >= himem_cache->number_of_pages)
    {
        page = fast_cache_get_least_recently_used(himem_cache->cache);
        log_trace("reusing existing page %d (%d - %d)", page->page_number, page->dirty_start, page->dirty_length);
        //todo: optimize remove so it can be reused, in this case page_bytes is reallocated!!
        fast_cache_remove(himem_cache->cache, (void *)page->page_number);

        page = (himem_page_item *)malloc(sizeof(himem_page_item));
        assert(page);
        page->himem_block_id = himem_allocate_block(himem_cache->himem);
    }
    else
    {
        log_trace("creating new page");
        page = (himem_page_item *)malloc(sizeof(himem_page_item));
        assert(page);
        page->himem_block_id = himem_allocate_block(himem_cache->himem);
    }

    page->page_number = (intptr_t)key;
    page->dirty_start = -1;
    page->dirty_length = -1;
    log_trace("reading into page %d, block_id %d", page->page_number, page->himem_block_id);

    void * buffer = malloc(himem_cache->page_size);//TODO: use memory pool
    assert(buffer);
    himem_cache->read_backend(himem_cache->backend_context, page->page_number * himem_cache->page_size, buffer, himem_cache->page_size);
    if (himem_write(himem_cache->himem, page->himem_block_id, 0, buffer, himem_cache->page_size) != himem_cache->page_size){
        log_error("Error writing to himem");
        assert(false);
    }
    free(buffer);

    log_trace("returning page %d (%d - %d) from fault", page->page_number, page->dirty_start, page->dirty_length);
    return page;
}

himem_cache_t *himem_cache_init(size_t page_size, size_t number_of_pages,
                          void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length),
                          void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length),
                          void *backend_context)
{
    himem_cache_t *himem_cache = (himem_cache_t *)malloc(sizeof(himem_cache_t));
    assert(himem_cache);
    himem_cache->page_size = page_size;
    himem_cache->number_of_pages = number_of_pages;
    himem_cache->cache = fast_cache_init(number_of_pages, himem_cache_on_fault, himem_cache_on_flush, himem_cache_on_remove, himem_cache);
    himem_cache->himem = himem_allocator_init(page_size, himem_cache->number_of_pages);
    himem_cache->write_backend = write_backend;
    himem_cache->read_backend = read_backend;
    himem_cache->backend_context = backend_context;

    return himem_cache;
}

static inline size_t himem_cache_read_internal(himem_cache_t *himem_cache, size_t address, void *target, size_t length)
{
    log_trace("internal reading from address 0x%zx %zu bytes", address, length);
    size_t page_number = get_page_number_by_address(himem_cache, address);
    himem_page_item *page = fast_cache_get(himem_cache->cache, (void *)page_number);
    log_trace("asked for page %d got %d, %d-%d", page_number, page->page_number, page->dirty_start, page->dirty_length);
    uint64_t page_address = page->page_number * himem_cache->page_size;
    size_t offset = address - page_address;
    size_t max_length = himem_cache->page_size - offset;
    log_trace("reading internal page %d(%d) offset 0x%zx, max length %zu", page_number, page->page_number, offset, max_length);
    assert(page_number == page->page_number);
    length = MIN(length, max_length);
    if (himem_read(himem_cache->himem, page->himem_block_id, offset, target,length) != length){
        log_error("Error reading from himem");
        assert(false);
    }
    return length;
}

void himem_cache_read(himem_cache_t *himem_cache, size_t address, void *buffer, size_t length)
{
    log_trace("reading from address 0x%zx %zu bytes", address, length);
    uint64_t remain = length;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = himem_cache_read_internal(himem_cache, address + offset, (uint8_t *)buffer + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }
}

static inline size_t himem_cache_write_internal(himem_cache_t *himem_cache, size_t address, void *target, size_t length)
{
    log_trace("internal writing to address 0x%" PRIx64 " %d bytes", address, length);
    size_t page_number = get_page_number_by_address(himem_cache, address);
    himem_page_item *page = fast_cache_get(himem_cache->cache, (void *)page_number);
    log_trace("asked for page %d got %d, %d-%d", page_number, page->page_number, page->dirty_start, page->dirty_length);
    uint64_t page_address = page->page_number * himem_cache->page_size;
    size_t offset = address - page_address;
    size_t max_length = himem_cache->page_size - offset;
    log_trace("writing internal page %d(%d) offset 0x%" PRIx64 ", max length %d", page_number, page->page_number, offset, max_length);
    assert(page_number == page->page_number);
    length = MIN(length, max_length);
    
    if (himem_write(himem_cache->himem,page->himem_block_id,offset, target, length) != length){
        log_error("Error writing to himem");
        assert(false);
    }

    if ((page->dirty_start == -1) && (page->dirty_length == -1)){
        page->dirty_start = offset;
        page->dirty_length = length;
    }else{
        size_t range_start = offset;
        size_t range_end = length + offset;

        size_t previous_range_start = page->dirty_start;
        size_t previous_range_end = previous_range_start + page->dirty_length;

        size_t dirty_start = MIN(range_start, previous_range_start);
        size_t dirty_end = MAX(range_end, previous_range_end);

        size_t dirty_length = dirty_end - dirty_start;

        page->dirty_start = dirty_start;
        page->dirty_length = dirty_length;

        log_trace("page %d info range (%d - %d), previous (%d - %d), dirty (%d - %d, len %d)", range_start, range_end, previous_range_start, previous_range_end, dirty_start, dirty_end, dirty_length);
    }

    log_trace("page %d dirty %d - %d", page->page_number, page->dirty_start, page->dirty_length);

    if ((page->dirty_start + page->dirty_length) > himem_cache->page_size){
        log_error("Error processing dirty length %d %d", offset, length);
        assert(false);
    }


    return length;
}

void himem_cache_write(himem_cache_t *himem_cache, size_t address, const void *buffer, size_t length)
{
    log_trace("writing to address 0x%" PRIx64 " %d bytes", address, length);
    uint64_t remain = length;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = himem_cache_write_internal(himem_cache, address + offset, (uint8_t *)buffer + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }
}
void himem_cache_flush(himem_cache_t *himem_cache)
{
    log_trace("flush"); 
    fast_cache_sync(himem_cache->cache);
}
void himem_cache_deinit(himem_cache_t *himem_cache)
{
    log_trace("deinit"); 
    fast_cache_free(himem_cache->cache);
    himem_allocator_deinit(himem_cache->himem);
    free(himem_cache);
}
