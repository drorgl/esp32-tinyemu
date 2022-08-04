#include "fast_cache.h"

struct _himem_cache_t;
typedef struct _himem_cache_t himem_cache_t;

himem_cache_t *himem_cache_init(size_t page_size, size_t number_of_pages,
                        void (*write_backend)(void *context, const size_t address, const void *buffer, const size_t length),
                        void (*read_backend)(void *context, const size_t address, void *buffer, const size_t length),
                        void *backend_context);

void himem_cache_read(himem_cache_t *himem_cache, size_t address, void *buffer, size_t length);
void himem_cache_write(himem_cache_t *himem_cache, size_t address, const void *buffer, size_t length);
void himem_cache_flush(himem_cache_t * himem_cache);
void himem_cache_deinit(himem_cache_t * himem_cache);