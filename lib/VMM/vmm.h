#pragma once

#include "vmm_config.h"
#include "vmm_statistics.h"
#include "vmm_storage.h"

#include <stdio.h>
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

#define VMM_FASTPAGE_LENGTH 32

/**
 * @brief Represents a fast page for read/write operations.
 */
typedef struct VMM_fastpage_t
{
    int32_t page_number;   /**< The page number. */
    size_t page_start;     /**< The starting address of the page. */
    size_t page_end;       /**< The ending address of the page. */
    uint8_t *buffer;       /**< Pointer to the page buffer. */
    bool write_enabled;    /**< Flag indicating if write is enabled for the page. */
} VMM_fastpage_t;

/**
 * @brief Represents the Virtual Memory Manager.
 */
typedef struct VMM_t
{
    size_t maximum_size;    /**< Maximum size of the virtual memory. */
    size_t page_size;        /**< Size of a single page. */
    size_t number_of_pages; /**< Total number of pages. */
    size_t page_shift;      /**< Bit shift value for page address calculation. */

    size_t pagetable_size;   /**< Current size of the page table. */

    cache_t *lru_cache;         /**< LRU cache for page table entries. */
    direct_cache_t *direct_cache; /**< Direct cache for frequently accessed pages. */
    page_cache_t *page_cache;   /**< Page cache for storing page data. */

    size_t last_page_number; /**< The last accessed page number. */
    vmTable_t *last_page;    /**< Pointer to the last accessed page table entry. */

    struct VMM_storage_t *vmm_storage; /**< Storage module for handling disk operations. */

    struct VMM_statistics_t statistics; /**< Statistics for VMM operations. */

    uint8_t *empty_page;       /**< Pointer to an empty page buffer. */
    VMM_fastpage_t last_fastpage[VMM_FASTPAGE_LENGTH]; /**< The last accessed fast page. */
} VMM_t;

/**
 * @brief Creates a new VMM instance.
 * @param pagefile The path to the backing store file.
 * @param maximum_size The maximum size of the virtual memory.
 * @param page_size The size of a single page.
 * @param number_of_pages The total number of pages.
 * @param maximum_himem_blocks The maximum number of himem blocks for page cache
 * @return A pointer to the newly created VMM instance.
 */
VMM_t *vmm_create(const char *pagefile, size_t maximum_size, size_t page_size, size_t number_of_pages, size_t maximum_himem_blocks);

/**
 * @brief Destroys a VMM instance.
 * @param vmm The VMM instance to destroy.
 */
void vmm_destroy(VMM_t *vmm);

/**
 * @brief Reads data from the virtual memory.
 * @param vmm The VMM instance.
 * @param addr The address to read from.
 * @param target The buffer to store the read data.
 * @param len The number of bytes to read.
 */
void vmm_read(VMM_t *vmm, size_t addr, void *target, size_t len);

/**
 * @brief Preloads a fast page for reading.
 * @param vmm The VMM instance.
 * @param addr The address within the page to preload.
 * @return A pointer to the preloaded fast page.
 */
VMM_fastpage_t *vmm_read_preload_fastpage(VMM_t *vmm, size_t addr);

/**
 * @brief Preloads a fast page for writing.
 * @param vmm The VMM instance.
 * @param addr The address within the page to preload.
 * @return A pointer to the preloaded fast page.
 */
VMM_fastpage_t *vmm_write_preload_fastpage(VMM_t *vmm, size_t addr);

/**
 * @brief Writes data to the virtual memory.
 * @param vmm The VMM instance.
 * @param addr The address to write to.
 * @param source The buffer containing the data to write.
 * @param len The number of bytes to write.
 */
void vmm_write(VMM_t *vmm, const size_t addr, const void *source, const size_t len);

/**
 * @brief Flushes the VMM, writing all dirty pages to disk.
 * @param vmm The VMM instance.
 */
void vmm_flush(VMM_t *vmm);

#ifdef __cplusplus
}
#endif