
// should initialize himem to blocks of page_size
// should use splay-tree and direct-cache to store page indexes
// should evict unused pages
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct _page_cache_t;
typedef struct _page_cache_t page_cache_t;


page_cache_t *page_cache_init(size_t page_size, size_t maximum_himem_blocks, void (*on_flush)(size_t page_number, void * buf,void * flush_context), void* flush_context);

bool page_cache_get(page_cache_t *page_cache, size_t page_number, void *buff);

void page_cache_set(page_cache_t *page_cache, size_t page_number, void *buff);