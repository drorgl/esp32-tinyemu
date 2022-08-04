#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _VMM_t;
typedef struct _VMM_t VMM_t;

VMM_t *vmm_create(
    const char *pagefile, size_t maximum_size,
    size_t himem_page_size,
    size_t himem_pages,
    size_t psram_page_size,
    size_t psram_pages,
    size_t ram_page_size,
    size_t ram_pages);

void vmm_destroy(VMM_t *vmm);

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len);

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len);

void vmm_flush(VMM_t *vmm);


#ifdef __cplusplus
}
#endif