#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <vmm_config.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Structure to hold statistics about the VMM storage.
 *
 * This structure tracks various metrics related to read and write operations performed on the storage,
 * including the number of operations, the amount of data transferred, and the time spent.
 */
struct VMM_storage_statistics_t {
    /** @brief Number of bytes read directly from the disk. */
    size_t store_read_disk_bytes;
    
    /** @brief Total time spent on read operations (implementation dependent unit). */
    size_t store_read_time;

    /** @brief Number of read operations performed. */
    size_t store_reads;

    /** @brief Total number of bytes read from the storage (potentially from in-memory cache). */
    size_t store_read_bytes;
    
    /** @brief Number of bytes written directly to the disk. */
    size_t store_write_disk_bytes;
    
    /** @brief Total time spent on write operations (implementation dependent unit). */
    size_t store_write_time;

    /** @brief Number of write operations performed. */
    size_t store_writes;

    /** @brief Total number of bytes written to the storage (potentially to in-memory cache). */
    size_t store_write_bytes;
};

/**
 * @brief Structure representing the VMM storage.
 *
 * This structure contains information about the storage, such as the page size, maximum size,
 * filename of the backing store, and pointers to data structures used for tracking dirty pages.
 */
struct VMM_storage_t {
    /** @brief The size of each page in bytes. Must be a power of 2. */
    size_t page_size;

    /** @brief The maximum size of the storage in bytes. */
    size_t maximum_size;

    /** @brief The filename of the backing store file. */
    char filename[255];

    /** @brief A pointer to the FILE object representing the backing store. */
    FILE *backing_store;
    
    /** @brief The number of bits to shift to get the section index. Used for dividing pages into smaller sections for dirty tracking */
    uint8_t section_shift;

    /** @brief A bit array indicating whether a page is dirty. */
    uint8_t *page_dirty;

    /** @brief An array indicating which sections within a page are dirty.  Each byte represents a page, and bits indicate dirty sections within that page. */
    uint8_t *dirty_write_sections;

    /** @brief Statistics about the VMM storage. */
    struct VMM_storage_statistics_t statistics;
};


/**
 * @brief Reads data from the backing store into the provided buffer.
 *
 * @param vmm_storage A pointer to the VMM_storage_t structure representing the storage.
 * @param page_number The page number to read from.
 * @param buf A pointer to the buffer where the data should be stored.  The buffer must be large enough to hold a full page.
 */
void vmm_storage_read(struct VMM_storage_t *vmm_storage, size_t page_number, uint8_t *buf);

/**
 * @brief Writes data from the provided buffer into the backing store.
 *
 * @param vmm_storage A pointer to the VMM_storage_t structure representing the storage.
 * @param page_number The page number to write to.
 * @param buf A pointer to the buffer containing the data to write.  The buffer must be at least `vmm_storage->page_size` bytes long.
 */
void vmm_storage_write(struct VMM_storage_t *vmm_storage, size_t page_number, const void *buf);

/**
 * @brief Destroys the VMM storage, releasing any allocated resources.
 *
 * This function closes the backing store file and frees any allocated memory.  It's essential to call this function to avoid memory leaks.
 * @param vmm_storage A pointer to the VMM_storage_t structure to destroy.
 */
void vmm_storage_destroy(struct VMM_storage_t *vmm_storage);

/**
 * @brief Marks a section of a page as dirty.
 *
 * This function is used to track which parts of a page have been modified, enabling optimizations for writing back data to the backing store.  This information can be used to avoid writing unmodified sections to disk.
 *
 * @param vmm_storage A pointer to the VMM_storage_t structure representing the storage.
 * @param page_number The page number containing the dirty section.
 * @param offset The offset within the page where the dirty section starts.
 * @param length The length of the dirty section in bytes.
 */
void vmm_storage_set_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number, size_t offset, size_t length);

uint8_t vmm_storage_get_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number);
uint8_t vmm_storage_get_page_write_dirty(struct VMM_storage_t *vmm_storage, size_t page_number);

void vmm_storage_clear_page_dirty(struct VMM_storage_t *vmm_storage, size_t page_number);

/**
 * @brief Creates and initializes a VMM storage instance.
 *
 * This function allocates memory for a `VMM_storage_t` structure, opens the backing store file,
 * initializes the storage parameters (page size, maximum size, filename), and allocates memory
 * for tracking dirty pages and sections.
 *
 * @param pagefile The path to the backing store file.
 * @param page_size The size of each page in bytes. Must be a power of 2.
 * @param maximum_size The maximum size of the storage in bytes.
 * @return A pointer to the newly created `VMM_storage_t` structure, or NULL if an error occurred.
 */
struct VMM_storage_t *vmm_storage_create(const char *pagefile, size_t page_size, size_t maximum_size);

/**
 * @brief Destroys the VMM storage, releasing any allocated resources.
 *
 * This function closes the backing store file and frees any allocated memory associated with the given
 * `VMM_storage_t` structure. It's essential to call this function when the storage is no longer needed
 * to prevent memory leaks and ensure proper resource cleanup.
 *
 * @param vmm_storage A pointer to the `VMM_storage_t` structure to destroy.
 */
void vmm_storage_destroy(struct VMM_storage_t *vmm_storage);


void vmm_storage_flush(struct VMM_storage_t *vmm_storage);

#ifdef __cplusplus
}
#endif