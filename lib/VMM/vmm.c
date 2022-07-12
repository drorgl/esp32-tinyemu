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


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static uint64_t get_page_number_by_address(VMM_t *vmm, uint64_t address);

static vmTable_t *get_TLB(VMM_t *vmm, uint64_t page_number);

static void backing_store_write(VMM_t *vmm, uint64_t page_number, const uint8_t *buf, const uint64_t buf_len);

static void backing_store_read(VMM_t *vmm, uint64_t page_number, uint8_t *buf, const uint64_t buf_len);

static void free_vmtable(VMM_t *vmm, vmTable_t *entry);

static vmTable_t *new_vmtable(VMM_t *vmm);

static vmTable_t *find_empty_TLB(VMM_t *vmm);

static void load_page(VMM_t *vmm, uint64_t page_number, vmTable_t *page);

VMM_t *vmm_create(const char *pagefile, uint64_t maximum_size, uint64_t page_size, uint64_t number_of_pages, size_t maximum_himem_blocks);

void vmm_destroy(VMM_t *vmm);

void vmm_read(VMM_t *vmm, uint64_t addr, void *target, uint64_t len);

void vmm_write(VMM_t *vmm, const uint64_t addr, const void *source, const uint64_t len);

static vmTable_t *get_page(VMM_t *vmm, uint64_t addr);

void vmm_flush(VMM_t *vmm);

int compare_page_number(const void *e1, const void *e2)
{
    return (intptr_t)e1 - (intptr_t)e2;
}

static uint64_t get_page_number_by_address(VMM_t *vmm, uint64_t address)
{
    uint64_t page_number = address / vmm->page_size;
    return page_number;
}

static vmTable_t *get_TLB(VMM_t *vmm, uint64_t page_number)
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
        return value;
    }
    log_trace("did not find TLB");
    return NULL;
}

static void on_page_cache_flush(uint64_t page_number, void * buf, void * flush_context){
    VMM_t * vmm = flush_context;
    if (fseek(vmm->backing_store, page_number * vmm->page_size, SEEK_SET) != 0)
    {
        log_warn("Error seeking in backing store");
    }

    if (fwrite(buf, sizeof(uint8_t), vmm->page_size, vmm->backing_store) != vmm->page_size)
    {
        log_error("Error writing to backing store\n");
    }
}

static void backing_store_write(VMM_t *vmm, uint64_t page_number, const uint8_t *buf, const uint64_t buf_len)
{
#ifdef VMM_DEBUG
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#endif
    // if (fseek(vmm->backing_store, page_number * vmm->page_size, SEEK_SET) != 0)
    // {
    //     log_warn("Error seeking in backing store");
    // }

    // // now vmm_read PAGE_READ_SIZE bytes from the backing store to the fileReadBuffer
    // if (fwrite(buf, sizeof(uint8_t), buf_len, vmm->backing_store) != buf_len)
    // {
    //     log_error("Error writing to backing store\n");
    // }

    page_cache_set(vmm->page_cache,page_number, buf);
#ifdef VMM_DEBUG
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    float t_s = tv_end.tv_sec - tv_start.tv_sec + 1e-6f * (tv_end.tv_usec - tv_start.tv_usec);
    vmm->store_write_time += (t_s * 1e3);

    vmm->store_writes++;
    vmm->store_write_bytes += buf_len;
#endif
}

static void backing_store_read(VMM_t *vmm, uint64_t page_number, uint8_t *buf, const uint64_t buf_len)
{
#ifdef VMM_DEBUG
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
#endif

    if (!page_cache_get(vmm->page_cache, page_number, buf))
    {

        if (fseek(vmm->backing_store, page_number * vmm->page_size, SEEK_SET) != 0)
        {
            log_warn("Error seeking in backing store, page %d\n", page_number);
            *(uint8_t *)(0) = 1;
        }

        if (fread(buf, sizeof(uint8_t), buf_len, vmm->backing_store) == 0)
        {
            memset(buf, 0, buf_len);
            log_error("Error reading from backing store\n");
        }
    }
#ifdef VMM_DEBUG
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    float t_s = tv_end.tv_sec - tv_start.tv_sec + 1e-6f * (tv_end.tv_usec - tv_start.tv_usec);
    vmm->store_read_time += (t_s * 1e3);

    vmm->store_reads++;
    vmm->store_read_bytes += buf_len;
#endif
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

static vmTable_t *find_empty_TLB(VMM_t *vmm)
{
    log_trace("looking for empty TLB in pagetable (%d items)", vmm->pagetable_size);
    if (vmm->pagetable_size >= vmm->number_of_pages)
    {
        vmTable_t *last_page = lru_cache_get_least_recently_used(vmm->lru_cache);
        if (last_page != NULL)
        {
            free_vmtable(vmm, last_page);
            vmm->pagetable_size--;
        }
    }

#ifdef VMM_DEBUG
    vmm->page_faults++;
#endif
    vmTable_t *page = new_vmtable(vmm);
    vmm->pagetable_size++;
#ifdef VMM_DEBUG
    if ((vmm->page_faults % 1000 == 0))
    {
        printf("page fault %" PRIu64 ", read %" PRIu64 " (%" PRIu64 " bytes), write %" PRIu64 " (%" PRIu64 " bytes), store read %" PRIu64 " (%" PRIu64 " bytes %" PRIu64 " ms), store write %" PRIu64 " (%" PRIu64 " bytes %" PRIu64 " ms)\r\n",
               vmm->page_faults,
               vmm->reads, vmm->read_bytes,
               vmm->writes, vmm->write_bytes,
               vmm->store_reads, vmm->store_read_bytes,
               vmm->store_read_time,
               vmm->store_writes, vmm->store_write_bytes,
               vmm->store_write_time);
    }
#endif
    return page;
}

static void load_page(VMM_t *vmm, uint64_t page_number, vmTable_t *page)
{
    backing_store_read(vmm, page_number, page->page_cache, vmm->page_size);
}

void on_page_flush(void *key, void *value, void *context)
{
    VMM_t *vmm = (VMM_t *)context;
    vmTable_t *page = value;
    if (page->dirty)
    {
        log_trace("flushing 0x%" PRIx64 ", at %d", page->page_address, page->page_number * vmm->page_size);
        backing_store_write(vmm, page->page_number, page->page_cache, vmm->page_size);
        page->dirty = false;
    }
}



VMM_t *vmm_create(const char *pagefile, uint64_t maximum_size, uint64_t page_size, uint64_t number_of_pages, size_t maximum_himem_blocks)
{
    printf("creating vmm %s size: %" PRIu64 ", page: %" PRIu64 ", pages: %" PRIu64 ", total: %" PRIu64 "\r\n",
           pagefile, maximum_size, page_size, number_of_pages, page_size * number_of_pages);
    VMM_t *vmm = (VMM_t *)malloc(sizeof(VMM_t));
    assert(vmm);
    vmm->pagetable_size = 0;
    strcpy((char *)&vmm->filename, pagefile);

    vmm->page_size = page_size;
    vmm->number_of_pages = number_of_pages;

#ifdef VMM_DEBUG
    vmm->reads = 0;
    vmm->read_bytes = 0;
    vmm->writes = 0;
    vmm->write_bytes = 0;

    vmm->store_writes = 0;
    vmm->store_write_bytes = 0;
    vmm->store_reads = 0;
    vmm->store_read_bytes = 0;

    vmm->page_faults = 0;
#endif

    vmm->lru_cache = lru_cache_init(compare_page_number, on_page_flush, vmm);
    vmm->direct_cache = direct_cache_init(1024);
    vmm->page_cache = page_cache_init(page_size,maximum_himem_blocks, on_page_cache_flush, vmm);

    log_trace("creating page file %s of %d", pagefile, maximum_size);
    FILE *create_f = fopen(pagefile, "wb");
    if (!create_f)
    {
        log_error("fopen() failed");
    }
    if (setvbuf(create_f, NULL, _IOFBF, 1024 * 8) != 0)
    {
        perror("setvbuf");
    }
    if (fseek(create_f, maximum_size, SEEK_SET) != 0)
    {
        log_error("fseek() failed");
    }
    if (fputc('\0', create_f) != '\0')
    {
        log_error("fputc() failed");
    }
    fclose(create_f);

    log_trace("opening for rw");

    vmm->backing_store = fopen(pagefile, "r+b");
    if (!vmm->backing_store)
    {
        log_error("error opening file");
        vmm_destroy(vmm);
        return NULL;
    }
    if (setvbuf(vmm->backing_store, NULL, _IOFBF, 1024 * 8) != 0)
    {
        perror("setvbuf");
    }

    log_debug("opened");
    return vmm;
}

void vmm_destroy(VMM_t *vmm)
{
    log_debug("page size %d", vmm->page_size);

    for (size_t i = 0; i < lru_cache_count(vmm->lru_cache); i++)
    {
        vmTable_t *last_page = lru_cache_get_least_recently_used(vmm->lru_cache);
        if (last_page != NULL)
        {
            free_vmtable(vmm, last_page);
            vmm->pagetable_size--;
        }
    }

    if (vmm->backing_store != NULL)
    {
        vmm_flush(vmm);

        // TODO: free each item in list pagetable and its page_cache
        log_trace("closing");
        fclose(vmm->backing_store);
        log_debug("closed");
    }

    lru_cache_free(vmm->lru_cache);
    direct_cache_free(vmm->direct_cache);
    free(vmm);
}

static uint64_t vmm_read_internal(VMM_t *vmm, uint64_t addr, void *target, uint64_t len)
{
    log_trace("reading from address 0x%" PRIx64 " %d bytes", addr, len);
    vmTable_t *page = get_page(vmm, addr);
    uint64_t offset = addr - page->page_address;
    uint64_t max_length = vmm->page_size - offset;
    len = MIN(len, max_length);
    memcpy(target, page->page_cache + offset, len);
    return len;
}

void vmm_read(VMM_t *vmm, uint64_t addr, void *target, uint64_t len)
{
    log_debug("reading %s from address 0x%" PRIx64 " %" PRIu64 " bytes", vmm->filename, addr, len);

    uint64_t remain = len;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = vmm_read_internal(vmm, addr + offset, (uint8_t *)target + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }

#ifdef VMM_DEBUG
    vmm->reads++;
    vmm->read_bytes += len;

    if ((vmm->reads % 1000000 == 0) || (vmm->read_bytes % 1000000 == 0))
    {
        // printf("reads %" PRIu64 " bytes %" PRIu64 "\r\n", vmm->reads, vmm->read_bytes);
    }
#endif
}

static uint64_t vmm_write_internal(VMM_t *vmm, uint64_t addr, void *source, uint64_t len)
{
    log_trace("internal writing to address 0x%" PRIx64 " %d bytes", addr, len);
    vmTable_t *page = get_page(vmm, addr);
    assert(page);
    uint64_t offset = addr - page->page_address;
    uint64_t max_length = vmm->page_size - offset;
    log_trace("internal page offset 0x%" PRIx64 ", max length %d", offset, max_length);
    len = MIN(len, max_length);
    memcpy(page->page_cache + offset, source, len);
    page->dirty = true;
    return len;
}

void vmm_write(VMM_t *vmm, const uint64_t addr, const void *source, const uint64_t len)
{
    log_debug("writing %s to address 0x%" PRIx64 " %" PRIu64 " bytes", vmm->filename, addr, len);

    uint64_t remain = len;
    uint64_t offset = 0;
    while (remain)
    {
        uint64_t toCpy = vmm_write_internal(vmm, addr + offset, (uint8_t *)source + offset, remain);
        offset += toCpy;
        remain -= toCpy;
    }

#ifdef VMM_DEBUG
    vmm->writes++;
    vmm->write_bytes += len;

    if ((vmm->writes % 1000000 == 0) || (vmm->write_bytes % 1000000 == 0))
    {
        // printf("writes %" PRIu64 " bytes %" PRIu64 "\r\n", vmm->writes, vmm->write_bytes);
    }
#endif
    // TODO(dror): don't flush!
    // vmm_flush(vmm);
}

static vmTable_t *get_page(VMM_t *vmm, uint64_t addr)
{
    log_trace("getting page for 0x%" PRIx64 "", addr);
    uint64_t page_number = get_page_number_by_address(vmm, addr);
    uint64_t page_offset = page_number * vmm->page_size;

    log_trace("page number %d, offset 0x%" PRIx64 "", page_number, page_offset);

    vmTable_t *page = get_TLB(vmm, page_number);
    if (page != NULL)
    {
        log_trace("found existing TLB");
    }
    else if (page == NULL)
    {
        page = find_empty_TLB(vmm);
        page->page_number = page_number;
        page->page_address = page_offset;
        load_page(vmm, page_number, page);
        lru_cache_add(vmm->lru_cache, (void *)page_number, page);
        direct_cache_set(vmm->direct_cache, (void *)page_number, page);
        // splaytree_put(vmm->search_tree, page_number, page);
    }

    return page;
}

void vmm_flush(VMM_t *vmm)
{
    log_debug("flushing dirty pages");

    printf("sync\n");
    lru_cache_sync(vmm->lru_cache);
    printf("flush\n");
    fflush(vmm->backing_store);
}