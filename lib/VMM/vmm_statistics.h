#include <stddef.h>

#include "vmm_config.h"

struct VMM_statistics_t
{
#ifdef VMM_DEBUG
    size_t reads;
    size_t read_bytes;
    size_t writes;
    size_t write_bytes;

    // size_t store_writes;
    // size_t store_write_bytes;
    // size_t store_write_disk_bytes;
    // size_t store_write_time;
    // size_t store_reads;
    // size_t store_read_bytes;
    // size_t store_read_disk_bytes;
    // size_t store_read_time;

    size_t page_faults;
    size_t page_faults_new;

    size_t direct_cache_hit;
    size_t direct_cache_miss;
    size_t lru_cache_hit;
    size_t lru_cache_miss;

    size_t same_page_hit;
    #endif
};
