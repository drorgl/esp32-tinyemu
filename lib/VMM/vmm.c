#include <vmm.h>

#include <malloc.h>
#include <file_backend.h>
#include <heap_cache.h>
#include <himem_cache.h>
#include <assert.h>
#ifdef ESP32
#include <esp_heap_caps.h>
#endif

struct _VMM_t
{
    const char *pagefile;

    file_backend_t *file_backend;

    heap_cache_t *ram_cache;
    heap_cache_t *psram_cache;
    himem_cache_t *himem_cache;
};

void *psram_malloc(size_t bytes)
{
#ifdef ESP32
    void *ptr = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM);
#else
    void *ptr = malloc(bytes);
#endif

    assert(ptr);
    return ptr;
}

void psram_free(void *ptr)
{
    free(ptr);
}

void *ram_malloc(size_t bytes)
{
#ifdef ESP32
    void *ptr = heap_caps_malloc(bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
    void *ptr = malloc(bytes);
#endif

    assert(ptr);
    return ptr;
}

void ram_free(void *ptr)
{
    free(ptr);
}

void ram_write_backend(void *context, const size_t address, const void *buffer, const size_t length)
{
    // ram writes to psram
    VMM_t *vmm = context;
    heap_cache_write(vmm->psram_cache, address, buffer, length);
}
void ram_read_backend(void *context, const size_t address, void *buffer, const size_t length)
{
    // ram reads from psram
    VMM_t *vmm = context;
    heap_cache_read(vmm->psram_cache, address, buffer, length);
}

void psram_write_backend(void *context, const size_t address, const void *buffer, const size_t length)
{
    // ram writes to himem
    VMM_t *vmm = context;
    himem_cache_write(vmm->himem_cache, address, buffer, length);
}
void psram_read_backend(void *context, const size_t address, void *buffer, const size_t length)
{
    // ram reads from himem
    VMM_t *vmm = context;
    himem_cache_read(vmm->himem_cache, address, buffer, length);
}

void himem_write_backend(void *context, const size_t address, const void *buffer, const size_t length)
{
    // himem writes to file
    VMM_t *vmm = context;
    backend_write(vmm->file_backend, address, buffer, length);
}
void himem_read_backend(void *context, const size_t address, void *buffer, const size_t length)
{
    // himem reads from file
    VMM_t *vmm = context;
    backend_read(vmm->file_backend, address, buffer, length);
}

VMM_t *vmm_create(
    const char *pagefile, size_t maximum_size,
    size_t himem_page_size,
    size_t himem_pages,
    size_t psram_page_size,
    size_t psram_pages,
    size_t ram_page_size,
    size_t ram_pages)
{
    VMM_t *vmm = (VMM_t *)malloc(sizeof(VMM_t));

    vmm->pagefile = pagefile;

    vmm->file_backend = backend_open(pagefile, himem_page_size, maximum_size);
    vmm->ram_cache = heap_cache_init("ram", ram_page_size, ram_pages, ram_malloc, ram_free, ram_write_backend, ram_read_backend, vmm);
    vmm->psram_cache = heap_cache_init("psram", psram_page_size, psram_pages, psram_malloc, psram_free, psram_write_backend, psram_read_backend, vmm);
    vmm->himem_cache = himem_cache_init(himem_page_size, himem_pages, himem_write_backend, himem_read_backend, vmm);

    return vmm;
}

void vmm_destroy(VMM_t *vmm)
{
    heap_cache_deinit(vmm->ram_cache);
    heap_cache_deinit(vmm->psram_cache);
    himem_cache_deinit(vmm->himem_cache);
    backend_close(vmm->file_backend);
    free(vmm);
}

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len)
{
    // reads from ram
    heap_cache_read(vmm->ram_cache, addr, target, len);
}

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len)
{
    // writes to ram
    heap_cache_write(vmm->ram_cache, addr, source, len);
}

void vmm_flush(VMM_t *vmm)
{
    heap_cache_flush(vmm->ram_cache);
    heap_cache_flush(vmm->psram_cache);
    himem_cache_flush(vmm->himem_cache);
    backend_flush(vmm->file_backend);
}