#pragma once

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

// #define VMM_DEBUG

typedef struct VMM_t
{
    size_t page_size;
    size_t number_of_pages;

    // list_t *pagetable;  // vmTable_t
    size_t pagetable_size;
    FILE *backing_store;

    char filename[255];

    // splaytree_t * search_tree;
    cache_t * lru_cache;
    direct_cache_t * direct_cache;
    page_cache_t * page_cache;
#ifdef VMM_DEBUG
    size_t reads;
    size_t read_bytes;
    size_t writes;
    size_t write_bytes;

    size_t store_writes;
    size_t store_write_bytes;
    size_t store_write_time;
    size_t store_reads;
    size_t store_read_bytes;
    size_t store_read_time;

    size_t page_faults;
#endif
} VMM_t;

VMM_t *vmm_create(const char *pagefile, size_t maximum_size, size_t page_size, size_t number_of_pages, size_t maximum_himem_blocks);

void vmm_destroy(VMM_t *vmm);

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len);

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len);

void vmm_flush(VMM_t *vmm);


#ifdef __cplusplus
}
#endif