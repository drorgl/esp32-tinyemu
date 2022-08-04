#include "heap_cache.h"

// uses fast_cache
// allocates very small pages on internal ram
// includes dirty bit for saves
// on fault, retrives from l2 cache
// on flush, saves to l2 cache

#include <stdbool.h>
#include <inttypes.h>

#include "fast_cache.h"
#include <malloc.h>
#include <log.h>
#include <string.h>
#include <assert.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct _heap_page_item
{
    size_t page_number;
    void *page_bytes;
    int dirty_start;
    int dirty_length;
};

typedef struct _heap_page_item heap_page_item;

struct _heap_cache_t
{
    const char *name;
    size_t page_size;
    size_t number_of_pages;
    fast_cache_t *cache;
    void *backend_context;
    void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length);
    void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length);

    void *(*memory_alloc)(size_t size);
    void (*memory_free)(void *ptr);
};

static void heap_cache_on_flush(void *key, void *value, void *context)
{
    heap_cache_t *heap_cache = context;
    log_debug("%s flushing page %d", heap_cache->name, (intptr_t)key);

    heap_page_item *item = value;
    if ((item->dirty_start != -1) && (item->dirty_length != -1))
    {
        log_debug("page %d dirty from %d to %d", item->page_number, item->dirty_start, item->dirty_length);
        heap_cache->write_backend(heap_cache->backend_context, item->page_number * heap_cache->page_size + item->dirty_start, item->page_bytes + item->dirty_start, item->dirty_length);
    }
}

static void heap_cache_on_remove(void *key, void *value, void *context)
{
    heap_cache_t *heap_cache = context;
    log_debug("%s removing page %d", heap_cache->name, (intptr_t)key);

    heap_page_item *item = value;
    heap_cache->memory_free(item->page_bytes);
    free(item);
}

static inline uint64_t get_page_number_by_address(heap_cache_t *heap_cache, size_t address)
{
    uint64_t page_number = address / heap_cache->page_size;
    return page_number;
}

static void *heap_cache_on_fault(void *key, void *context)
{
    heap_cache_t *heap_cache = context;
    log_trace("%s page fault %d", heap_cache->name, (intptr_t)key);

    heap_page_item *page;

    if (fast_cache_count(heap_cache->cache) >= heap_cache->number_of_pages)
    {
        page = fast_cache_get_least_recently_used(heap_cache->cache);
        log_trace("reusing existing page %d (%d - %d)", page->page_number, page->dirty_start, page->dirty_length);
        // todo: optimize remove so it can be reused, in this case page_bytes is reallocated!!
        fast_cache_remove(heap_cache->cache, (void *)page->page_number);

        page = (heap_page_item *)malloc(sizeof(heap_page_item));
        assert(page);
        page->page_bytes = (heap_page_item *)heap_cache->memory_alloc(heap_cache->page_size);
    }
    else
    {
        log_trace("creating new page");
        page = (heap_page_item *)malloc(sizeof(heap_page_item));
        assert(page);
        page->page_bytes = (heap_page_item *)heap_cache->memory_alloc(heap_cache->page_size);
    }

    page->page_number = (intptr_t)key;
    page->dirty_start = -1;
    page->dirty_length = -1;
    log_trace("reading into page %d", page->page_number);
    heap_cache->read_backend(heap_cache->backend_context, page->page_number * heap_cache->page_size, page->page_bytes, heap_cache->page_size);
    log_trace("returning page %d (%d - %d) from fault", page->page_number, page->dirty_start, page->dirty_length);
    return page;
}

heap_cache_t *heap_cache_init(const char *name, size_t page_size, size_t number_of_pages,
                              void *(*memory_alloc)(size_t size),
                              void (*memory_free)(void *ptr),
                              void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length),
                              void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length),
                              void *backend_context)
{
    heap_cache_t *heap_cache = (heap_cache_t *)malloc(sizeof(heap_cache_t));
    assert(heap_cache);
    log_debug("%s init %d bytes x %d pages", name, page_size, number_of_pages);

    heap_cache->name = name;
    heap_cache->page_size = page_size;
    heap_cache->number_of_pages = number_of_pages;
    heap_cache->cache = fast_cache_init(number_of_pages, heap_cache_on_fault, heap_cache_on_flush, heap_cache_on_remove, heap_cache);
    heap_cache->write_backend = write_backend;
    heap_cache->read_backend = read_backend;
    heap_cache->backend_context = backend_context;

    heap_cache->memory_alloc = memory_alloc;
    heap_cache->memory_free = memory_free;

    return heap_cache;
}

static inline size_t heap_cache_read_internal(heap_cache_t *heap_cache, size_t address, void *target, size_t length)
{
    log_trace("%s internal reading from address 0x%zx %zu bytes", heap_cache->name, address, length);
    size_t page_number = get_page_number_by_address(heap_cache, address);
    heap_page_item *page = fast_cache_get(heap_cache->cache, (void *)page_number);
    log_trace("asked for page %d got %d, %d-%d", page_number, page->page_number, page->dirty_start, page->dirty_length);
    uint64_t page_address = page->page_number * heap_cache->page_size;
    size_t offset = address - page_address;
    size_t max_length = heap_cache->page_size - offset;
    log_trace("reading internal page %zu(%zu) offset 0x%zx, max length %zu", page_number, page->page_number, offset, max_length);
    assert(page_number == page->page_number);
    length = MIN(length, max_length);
    memcpy(target, page->page_bytes + offset, length);
    return length;
}

void heap_cache_read(heap_cache_t *heap_cache, size_t address, void *buffer, size_t length)
{
    log_trace("%s reading from address 0x%zx zu bytes", heap_cache->name, address, length);
    uint64_t remain = length;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = heap_cache_read_internal(heap_cache, address + offset, (uint8_t *)buffer + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }
}

static inline size_t heap_cache_write_internal(heap_cache_t *heap_cache, size_t address, void *target, size_t length)
{
    log_trace("%s internal writing to address 0x%zx %zu bytes", heap_cache->name, address, length);

    size_t page_number = get_page_number_by_address(heap_cache, address);
    heap_page_item *page = fast_cache_get(heap_cache->cache, (void *)page_number);

    log_trace("asked for page %zu got %zu, %d-%d", page_number, page->page_number, page->dirty_start, page->dirty_length);

    uint64_t page_address = page->page_number * heap_cache->page_size;

    size_t offset = address - page_address;

    size_t max_length = heap_cache->page_size - offset;

    log_trace("writing internal page %zu(%zu) offset 0x%zx, max length %zu", page_number, page->page_number, offset, max_length);
    assert(page_number == page->page_number);

    length = MIN(length, max_length);

    memcpy(page->page_bytes + offset, target, length);

    if ((page->dirty_start == -1) && (page->dirty_length == -1))
    {
        page->dirty_start = offset;
        page->dirty_length = length;
    }
    else
    {
        size_t range_start = offset;
        size_t range_end = length + offset;

        size_t previous_range_start = page->dirty_start;
        size_t previous_range_end = previous_range_start + page->dirty_length;

        size_t dirty_start = MIN(range_start, previous_range_start);
        size_t dirty_end = MAX(range_end, previous_range_end);

        size_t dirty_length = dirty_end - dirty_start;

        page->dirty_start = dirty_start;
        page->dirty_length = dirty_length;

        log_trace("page %d info range (%d - %d), previous (%d - %d), dirty (%d - %d, len %d)", page->page_number, range_start, range_end, previous_range_start, previous_range_end, dirty_start, dirty_end, dirty_length);
    }

    if ((page->dirty_start + page->dirty_length) > heap_cache->page_size)
    {
        log_error("Error processing dirty length start %d - length %d, offset %d length %d", page->dirty_start, page->dirty_length, offset, length);
        assert(false);
    }

    log_trace("page %d dirty %d - %d", page->page_number, page->dirty_start, page->dirty_length);
    return length;
}

void heap_cache_write(heap_cache_t *heap_cache, size_t address, const void *buffer, size_t length)
{
    log_trace("%s writing to address 0x%zx %zu bytes", heap_cache->name, address, length);
    uint64_t remain = length;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = heap_cache_write_internal(heap_cache, address + offset, (uint8_t *)buffer + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }
}
void heap_cache_flush(heap_cache_t *heap_cache)
{
    log_trace("%s flush", heap_cache->name);
    fast_cache_sync(heap_cache->cache);
}
void heap_cache_deinit(heap_cache_t *heap_cache)
{
    log_trace("%s deinit", heap_cache->name);
    fast_cache_free(heap_cache->cache);
    free(heap_cache);
}
