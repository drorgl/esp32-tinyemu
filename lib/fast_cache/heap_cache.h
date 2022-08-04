#include "fast_cache.h"

struct _heap_cache_t;
typedef struct _heap_cache_t heap_cache_t;

heap_cache_t *heap_cache_init(const char * name,size_t page_size, size_t number_of_pages,
                        void * (*memory_alloc)(size_t size),
                        void (*memory_free)(void * ptr),
                        void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length),
                        void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length),
                        void *backend_context);

void heap_cache_read(heap_cache_t *heap_cache, size_t address, void *buffer, size_t length);
void heap_cache_write(heap_cache_t *heap_cache, size_t address, const void *buffer, size_t length);
void heap_cache_flush(heap_cache_t * heap_cache);
void heap_cache_deinit(heap_cache_t * heap_cache);