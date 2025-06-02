#pragma once

#include "vmm_config.h"
#include "vmm_statistics.h"
#include "vmm_storage.h"

#include <stdio.h>
// #include <llist.h>
// #include <splaytree.h>

#include <lru_cache.h>
#include <direct_cache.h>
#include <page_cache.h>

#include "vmtable.h"
#include <log.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef __cplusplus
extern "C" {
#endif



typedef struct VMM_fastpage_t
{
    int32_t page_number;
    size_t page_start;
    size_t page_end;
    uint8_t *buffer;
    bool write_enabled;
} VMM_fastpage_t;


typedef struct VMM_t
{
    size_t maximum_size;
    size_t page_size;
    size_t number_of_pages;
    size_t page_shift;

    size_t pagetable_size;
    // FILE *backing_store;

    // char filename[255];

    cache_t * lru_cache;
    direct_cache_t * direct_cache;
    page_cache_t * page_cache;

    size_t last_page_number;
    vmTable_t * last_page;

    // uint8_t section_shift;
    // uint8_t * page_dirty;
    // uint8_t * dirty_write_sections;

    struct VMM_storage_t *vmm_storage;

    struct VMM_statistics_t statistics;

    uint8_t * empty_page;
    VMM_fastpage_t last_fastpage;
} VMM_t;

VMM_t *vmm_create(const char *pagefile, size_t maximum_size, size_t page_size, size_t number_of_pages, size_t maximum_himem_blocks);

void vmm_destroy(VMM_t *vmm);

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len);

VMM_fastpage_t * vmm_read_preload_fastpage(VMM_t *vmm, size_t addr);
VMM_fastpage_t * vmm_write_preload_fastpage(VMM_t *vmm, size_t addr);

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len);

void vmm_flush(VMM_t *vmm);


#ifdef __cplusplus
}
#endif