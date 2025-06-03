#include "vmm.h"

#include <stdio.h>
#include <llist.h>
#include "vmtable.h"
#include <log.h>
#include <inttypes.h>

#include <string.h>
#include <assert.h>

#include <time.h>

#ifdef LINUX
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

#include <sys/time.h>
#include <math.h>
// #include <bitarray.h>

#include <bitmasker.h>
#include "vmm_memory.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static size_t get_page_number_by_address(VMM_t *vmm, size_t address);

static vmTable_t *get_TLB(VMM_t *vmm, size_t page_number);

static void backing_store_write(VMM_t *vmm, size_t page_number, const uint8_t *buf, const size_t buf_len);

static void backing_store_read(VMM_t *vmm, size_t page_number, uint8_t *buf, const size_t buf_len);

static void free_vmtable(VMM_t *vmm, vmTable_t *entry);

static vmTable_t *new_vmtable(VMM_t *vmm);

static vmTable_t *find_empty_TLB(VMM_t *vmm);

static void load_page(VMM_t *vmm, size_t page_number, vmTable_t *page);

VMM_t *vmm_create(const char *pagefile, size_t maximum_size, size_t page_size, size_t number_of_pages, size_t maximum_himem_blocks);

void vmm_destroy(VMM_t *vmm);

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len);

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len);

static vmTable_t *get_page(VMM_t *vmm, size_t addr);

void vmm_flush(VMM_t *vmm);

int compare_page_number(const void *e1, const void *e2)
{
    return (intptr_t)e1 - (intptr_t)e2;
}

static inline size_t get_page_number_by_address(VMM_t *vmm, size_t address)
{
    const size_t page_number = address >> vmm->page_shift;
    return page_number;
}

static inline vmTable_t *get_TLB(VMM_t *vmm, size_t page_number)
{
    log_trace("getting TLB for page %d", page_number);

    vmTable_t *value = direct_cache_get(vmm->direct_cache, (void *)page_number);
    if (value != NULL)
    {
        return value;
    }

    value = lru_cache_get(vmm->lru_cache, (void *)page_number);
    if (value != NULL)
    {
        direct_cache_set(vmm->direct_cache, (void *)page_number, value);
        return value;
    }
    log_trace("did not find TLB");
    return NULL;
}

struct enumerate_info_t{
    VMM_t *vmm;
    size_t page_number;
    uint8_t *buf;
};

static void on_page_cache_pre_flush(size_t page_number, void * buf, void * flush_context){
    VMM_t * vmm = flush_context;

    log_debug("page_cache pre-flush page %d", page_number);
    vmTable_t *page = get_TLB(vmm, page_number);
    if (page != NULL)
    {
        page_cache_set(vmm->page_cache, page->page_number, page->page_cache);
    }
}

// page_cache flush callback
static void on_page_cache_flush(size_t page_number, void * buf, void * flush_context){
    VMM_t * vmm = flush_context;

    
    
    if (vmm_storage_get_page_write_dirty(vmm->vmm_storage, page_number) != 0)
    {
        #ifndef LOG_DISABLE
        bool hasNonZero = has_non_zero_bytes(buf, vmm->page_size);
        log_debug("page_cache flush page %d nonzero %d", page_number, hasNonZero? 1 : 0);
        #endif
        vmm_storage_write(vmm->vmm_storage, page_number, buf);
    }else{
        #ifndef LOG_DISABLE
        bool hasNonZero = has_non_zero_bytes(buf, vmm->page_size);
        log_debug("page_cache flush page %d ignored nonzero %d", page_number, hasNonZero? 1 : 0);
        #endif
    }
}

// backing store write handler, called by lru_cache flush
static void backing_store_write(VMM_t *vmm, size_t page_number, const uint8_t *buf, const size_t buf_len)
{   
    #ifndef LOG_DISABLE
    bool hasNonZero = has_non_zero_bytes(buf, buf_len);
    log_debug("backing store write page %d nonzero %d", page_number, hasNonZero? 1 : 0);
    #endif

    page_cache_set(vmm->page_cache,page_number, buf);
}

// backing store read, called by page_load
static void backing_store_read(VMM_t *vmm, size_t page_number, uint8_t *buf, const size_t buf_len)
{
    if (!page_cache_get(vmm->page_cache, page_number, buf))
    {
        if (vmm_storage_get_page_dirty(vmm->vmm_storage, page_number) == 0)
        {
            log_debug("skipping read of empty page %d", page_number);
            memset(buf, 0, buf_len); 
        }else{
            vmm_storage_read(vmm->vmm_storage, page_number, buf);
        }
    }
}

static void free_vmtable(VMM_t *vmm, vmTable_t *entry)
{
    lru_cache_remove(vmm->lru_cache, (void *)entry->page_number);
    direct_cache_remove(vmm->direct_cache, (void *)entry->page_number);
    free(entry->page_cache);
    entry->page_cache = NULL;
    free(entry); 
}

static vmTable_t *new_vmtable(VMM_t *vmm)
{
    vmTable_t *entry = (vmTable_t *)malloc(sizeof(vmTable_t));
    assert(entry);
    entry->page_cache = (uint8_t *)malloc(sizeof(uint8_t) * vmm->page_size);
    assert(entry->page_cache);
    return entry;
}


static void print_page_fault(VMM_t *vmm)
{
    size_t total_dirty_sections = 0;
    size_t total_not_dirty_sections = 0;

    //TODO: add counts
    // count_bits_naive(vmm->page_dirty, vmm->maximum_size / vmm->page_size, &total_dirty_sections, &total_not_dirty_sections);

    size_t total_dirty_pages = 0;
    // for (size_t i = 0; i < vmm->maximum_size / vmm->page_size; i++)
    // {
    //     if (vmm->page_dirty[i] != 0)
    //     {
    //         total_dirty_pages++;
    //     }
    // }

    log_debug("page fault %zu(%zu), read %zu (%zu bytes), write %zu (%zu bytes), store read %zu (%zu bytes %zu ms), store write %zu (%zu bytes %zu ms), disk read %zu, disk write %zu, dirty pages %zu/%zu (%zu/%zu)",
           vmm->statistics.page_faults,
           vmm->statistics.page_faults_new,
           vmm->statistics.reads, vmm->statistics.read_bytes,
           vmm->statistics.writes, vmm->statistics.write_bytes,
           vmm->vmm_storage->statistics.store_reads, vmm->vmm_storage->statistics.store_read_bytes,
           vmm->vmm_storage->statistics.store_read_time,
           vmm->vmm_storage->statistics.store_writes, vmm->vmm_storage->statistics.store_write_bytes,
           vmm->vmm_storage->statistics.store_write_time,
           vmm->vmm_storage->statistics.store_read_disk_bytes,
           vmm->vmm_storage->statistics.store_write_disk_bytes,
           total_dirty_pages,
           vmm->maximum_size / vmm->page_size,
           total_dirty_sections, total_not_dirty_sections);
}

static vmTable_t *find_empty_TLB(VMM_t *vmm)
{
    log_trace("looking for empty TLB in pagetable (%d items)", vmm->pagetable_size);
    vmTable_t *last_page = NULL;
    if (vmm->pagetable_size >= vmm->number_of_pages)
    {
        const int max_retries = 5;
        int retry_index = -1;
        
        for (int i = 0; i < max_retries; i++)
        {
            last_page = lru_cache_get_least_recently_used(vmm->lru_cache, retry_index);
            if (last_page != NULL)
            {
                retry_index--;
                if (vmm_storage_get_page_write_dirty(vmm->vmm_storage, last_page->page_number) == 0)
                {
                    log_debug("found empty page %d on retry %d", last_page->page_number,i);
                    break;
                }
            }else{
                break;
            }
        }

        if (last_page != NULL)
        {
            lru_cache_remove(vmm->lru_cache, (void *)last_page->page_number);
            direct_cache_remove(vmm->direct_cache, (void *)last_page->page_number);
            vmm->pagetable_size--;
        }
    }

#ifdef VMM_DEBUG
    vmm->statistics.page_faults++;
#endif
    vmTable_t *page;
    if (last_page != NULL){
        page = last_page;
    }else{
        page = new_vmtable(vmm);
#ifdef VMM_DEBUG
        vmm->statistics.page_faults_new++;
#endif
    }
    vmm->pagetable_size++;
#ifdef VMM_DEBUG
    if ((vmm->statistics.page_faults % 1000 == 0))
    {
        print_page_fault(vmm);
    }
#endif
    return page;
}

static void load_page(VMM_t *vmm, size_t page_number, vmTable_t *page)
{

    backing_store_read(vmm, page_number, page->page_cache, vmm->page_size);
}

void on_page_flush(void *key, void *value, void *context)
{
    VMM_t *vmm = (VMM_t *)context;
    vmTable_t *page = value;

    const uint8_t fastpage_slot = page->page_number % VMM_FASTPAGE_LENGTH;
    vmm->last_fastpage[fastpage_slot].page_number = -1;

    uint8_t page_write_dirty = vmm_storage_get_page_write_dirty(vmm->vmm_storage,page->page_number);
    uint8_t page_dirty = vmm_storage_get_page_dirty(vmm->vmm_storage,page->page_number);
    log_debug("page flush for page %d, dirty %x", page->page_number, page_write_dirty);
    if (page_write_dirty != 0)
    {
        log_trace("flushing %d (0x%zx), at %d", page->page_number, page->page_address, page->page_number * vmm->page_size);
        
        if (!has_non_zero_bytes(page->page_cache, vmm->page_size))
        {
            log_debug("resetting page dirty %d, all zeroes (%zu) %x", page->page_number,  vmm->page_size,  page_dirty, page_write_dirty);
            vmm_storage_clear_page_dirty(vmm->vmm_storage, page->page_number); //reset dirty bit if the page is all zeroes
            log_debug("deactivating page %d, all zeroes", page->page_number);
            page_cache_deactivate(vmm->page_cache, page->page_number);
            // page_write_dirty = 0; //reset dirty write sections
            // return;
        }else{
            backing_store_write(vmm, page->page_number, page->page_cache, vmm->page_size);
        }
    }else{
        log_debug("ignoring flushing page %d, dirty %x(%x)", page->page_number, page_dirty,page_write_dirty);
    }
}

static bool is_power_of_2(size_t n){
    float i = log2(n);
    // check if n is a power of 2
    if (ceil(i) == floor(i)) {
      return true;
    } 
    return false;
}


VMM_t *vmm_create(const char *pagefile, size_t maximum_size, size_t page_size, size_t number_of_pages, size_t maximum_himem_blocks)
{
    log_info("creating vmm %s size: %zu, page: %zu, pages: %zu, total: %zu, page bits: %zu",
           pagefile, maximum_size, page_size, number_of_pages, page_size * number_of_pages, maximum_size / page_size);
    VMM_t *vmm = (VMM_t *)malloc(sizeof(VMM_t));
    assert(vmm);
    vmm->pagetable_size = 0;
    vmm->vmm_storage = vmm_storage_create(pagefile, page_size, maximum_size);

    vmm->maximum_size = maximum_size;
    vmm->page_size = page_size;
    vmm->number_of_pages = number_of_pages;
    assert(is_power_of_2(page_size) && "page size must be power of 2");
    vmm->page_shift = log2(page_size);
    
    vmm->last_page_number = -1;
    vmm->empty_page = (uint8_t *)malloc(sizeof(uint8_t) * vmm->page_size);
    memset(vmm->empty_page, 0, vmm->page_size);
    for (size_t i = 0; i < VMM_FASTPAGE_LENGTH; i++)
    {
        vmm->last_fastpage[i].page_number = -1;
        vmm->last_fastpage[i].page_start = 0x00;
        vmm->last_fastpage[i].page_end = 0x00;
    }

    
#ifdef VMM_DEBUG
    vmm->statistics.reads = 0;
    vmm->statistics.read_bytes = 0;
    vmm->statistics.writes = 0;
    vmm->statistics.write_bytes = 0;

    vmm->statistics.page_faults = 0;
    vmm->statistics.page_faults_new = 0;

    vmm->statistics.lru_cache_miss = 0;
    vmm->statistics.direct_cache_miss = 0;

    vmm->statistics.direct_cache_hit = 0;
    vmm->statistics.lru_cache_hit = 0;

    
    vmm->last_page = NULL;

    vmm->statistics.same_page_hit = 0;
#endif

    vmm->lru_cache = lru_cache_init(compare_page_number, on_page_flush, vmm);
    vmm->direct_cache = direct_cache_init(1024 * 4 );
    vmm->page_cache = page_cache_init(page_size,maximum_himem_blocks, on_page_cache_pre_flush, on_page_cache_flush, vmm);

    
    return vmm;
}

void vmm_destroy(VMM_t *vmm)
{
    log_debug("page size %d", vmm->page_size);

    for (size_t i = 0; i < lru_cache_count(vmm->lru_cache); i++)
    {
        vmTable_t *last_page = lru_cache_get_least_recently_used(vmm->lru_cache,-1);
        if (last_page != NULL)
        {
            free_vmtable(vmm, last_page);
            vmm->pagetable_size--;
        }
    }

    
    lru_cache_free(vmm->lru_cache);
    direct_cache_free(vmm->direct_cache);
    
    if (vmm->vmm_storage != NULL)
    {
        vmm_storage_destroy(vmm->vmm_storage);
        vmm->vmm_storage = NULL;
    }
    free(vmm);
}

// static inline void my_memcpy(void *dest, void *src, size_t n)
// { 
//     char *csrc = (char *)src; 
//     char *cdest = (char *)dest; 
  
//     for (size_t i=0; i<n; i++) {
//         cdest[i] = csrc[i]; 
//     } 
// }

static inline size_t vmm_read_internal(VMM_t *vmm, size_t addr, void *target, size_t len)
{
    log_trace("reading from address 0x%zx %d bytes", addr, len);
    const size_t page_number = get_page_number_by_address(vmm, addr);
    
    const size_t page_address = page_number * vmm->page_size;
    const size_t offset = addr - page_address;
    const size_t max_length = vmm->page_size - offset;
    len = MIN(len, max_length);
    
    if (vmm_storage_get_page_dirty(vmm->vmm_storage,page_number) == 0){
        log_debug("skipping read of page %d, empty (%zu)", page_number, len);
        memset(target, 0, len);
        return len;
    }


    vmTable_t *page = get_page(vmm, addr);



    memcpy(target, page->page_cache + offset, len);
    return len;
}

void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len)
{
    log_debug("reading %s from address 0x%zx %zu bytes", vmm->vmm_storage->filename, addr, len);

    size_t remain = len;
    size_t offset = 0;
    while (remain)
    {
        size_t toCpy = vmm_read_internal(vmm, addr + offset, (uint8_t *)target + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }

#ifdef VMM_DEBUG
    vmm->statistics.reads++;
    vmm->statistics.read_bytes += len;
#endif
}

VMM_fastpage_t * vmm_write_preload_fastpage(VMM_t *vmm, size_t addr){
    // vmm->last_fastpage.page_number = -1;
    // vmm->last_fastpage.page_start = 0x00;
    // vmm->last_fastpage.page_end = 0x00;
    // return &vmm->last_fastpage;

    const size_t page_number = get_page_number_by_address(vmm, addr);
    const uint8_t fastpage_slot = page_number % VMM_FASTPAGE_LENGTH;
    //TODO: possible bug if the page was unloaded but the fastpage is still used
    if (page_number == vmm->last_fastpage[fastpage_slot].page_number && vmm->last_fastpage[fastpage_slot].write_enabled){
        // printf("using last fastpage %d\r\n", page_number);
        return &vmm->last_fastpage[fastpage_slot];
    }

    const size_t page_address = page_number * vmm->page_size;
    // // const size_t max_length = vmm->page_size 
    
    vmm->last_fastpage[fastpage_slot].page_number = page_number;
    vmm->last_fastpage[fastpage_slot].page_start = page_address;
    vmm->last_fastpage[fastpage_slot].page_end = page_address + vmm->page_size;
    vmm->last_fastpage[fastpage_slot].write_enabled = true;

    vmTable_t *page = get_page(vmm, addr);
    vmm->last_fastpage[fastpage_slot].buffer = page->page_cache;
    vmm_storage_set_page_dirty(vmm->vmm_storage, page->page_number, 0, vmm->page_size);


    return &vmm->last_fastpage[fastpage_slot];
}


VMM_fastpage_t * vmm_read_preload_fastpage(VMM_t *vmm, size_t addr){
    // vmm->last_fastpage.page_number = -1;
    // vmm->last_fastpage.page_start = 0x00;
    // vmm->last_fastpage.page_end = 0x00;
    // return &vmm->last_fastpage;

    const size_t page_number = get_page_number_by_address(vmm, addr);
    const uint8_t fastpage_slot = page_number % VMM_FASTPAGE_LENGTH;

    //TODO: possible bug if the page was unloaded but the fastpage is still used
    if (page_number == vmm->last_fastpage[fastpage_slot].page_number){
        // printf("using last fastpage %d\r\n", page_number);
        return &vmm->last_fastpage[fastpage_slot];
    }

    const size_t page_address = page_number * vmm->page_size;
    // // const size_t max_length = vmm->page_size 
    
    vmm->last_fastpage[fastpage_slot].page_number = page_number;
    vmm->last_fastpage[fastpage_slot].page_start = page_address;
    vmm->last_fastpage[fastpage_slot].page_end = page_address + vmm->page_size;
    vmm->last_fastpage[fastpage_slot].write_enabled = false;
    
    if (vmm_storage_get_page_dirty(vmm->vmm_storage,page_number) == 0){
        // printf("skipping read of page %d, empty (%zu)\r\n", page_number, vmm->page_size);
        vmm->last_fastpage[fastpage_slot].buffer = vmm->empty_page;
    }else{
        vmTable_t *page = get_page(vmm, addr);
        vmm->last_fastpage[fastpage_slot].buffer = page->page_cache;
    }

    return &vmm->last_fastpage[fastpage_slot];
}

static inline size_t vmm_write_internal(VMM_t *vmm, size_t addr, void *source, size_t len)
{
    log_trace("internal writing to address 0x%zx %d bytes", addr, len);
    const vmTable_t *page = get_page(vmm, addr);
    assert(page);
    const size_t offset = addr - page->page_address;
    const size_t max_length = vmm->page_size - offset;

    
    len = MIN(len, max_length);
    
    log_trace("internal writing page %d offset 0x%zx len %d, max length %d", page->page_number, offset,len, max_length);

    vmm_storage_set_page_dirty(vmm->vmm_storage, page->page_number, offset, len);

    assert(page->page_number < vmm->maximum_size / vmm->page_size && "page number out of bounds");

    memcpy(page->page_cache + offset, source, len);

    #ifndef LOG_DISABLE
    bool hasNonZero = has_non_zero_bytes(page->page_cache, vmm->page_size);
    log_debug("writing to page %d, dirty %x, section mask %x %x", page->page_number, vmm_storage_get_page_dirty(vmm->vmm_storage,page->page_number), vmm_storage_get_page_write_dirty(vmm->vmm_storage,page->page_number), hasNonZero );
    #endif

    return len;
}

void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len)
{
    log_debug("writing %s to address 0x%zx %zu bytes", vmm->vmm_storage->filename, addr, len);

    size_t remain = len;
    size_t offset = 0;
    while (remain)
    {
        size_t toCpy = vmm_write_internal(vmm, addr + offset, (uint8_t *)source + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }

#ifdef VMM_DEBUG
    vmm->statistics.writes++;
    vmm->statistics.write_bytes += len;
#endif
    // TODO(dror): don't flush!
    // vmm_flush(vmm);
}

static inline vmTable_t *get_page(VMM_t *vmm, size_t addr)
{
    log_trace("getting page for 0x%zx", addr);
    const size_t page_number = get_page_number_by_address(vmm, addr);

    if (page_number == vmm->last_page_number){
        // vmm->same_page_hit++;
        return vmm->last_page;
    }

    vmTable_t *page = get_TLB(vmm, page_number);
    if (page != NULL)
    {
        log_trace("found existing TLB");
    }
    else if (page == NULL)
    {
        const size_t page_offset = page_number * vmm->page_size;
        page = find_empty_TLB(vmm);
        page->page_number = page_number;
        page->page_address = page_offset;
        load_page(vmm, page_number, page);
        lru_cache_add(vmm->lru_cache, (void *)page_number, page);
        direct_cache_set(vmm->direct_cache, (void *)page_number, page);
    }

    vmm->last_page_number = page_number;
    vmm->last_page = page;


    page->page_accesses++;
    if ((page->page_accesses > 0) && (((page->page_accesses % 10000) == 0)))
    {
        log_trace("page %d accesses %zu",page->page_number, page->page_accesses);
        lru_cache_get(vmm->lru_cache, (void *)page->page_number);
    }

    return page;
}

void vmm_flush(VMM_t *vmm)
{
    log_debug("flushing dirty pages");
    lru_cache_sync(vmm->lru_cache);
    page_cache_flush(vmm->page_cache);
    vmm_storage_flush(vmm->vmm_storage);
}