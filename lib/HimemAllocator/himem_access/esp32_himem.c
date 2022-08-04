#include "himem_base.h"

#ifdef ESP32
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <esp32/himem.h>

#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_log.h"

static const char *TAG = "himem";

struct _himem_allocator_t
{
    uint32_t _block_size;
    bool mapped ;//= false;
    void *ptr;// = NULL;
    uint32_t current_superblock_id;
    esp_himem_rangehandle_t rh; // Handle for the actual RAM.
    esp_himem_handle_t mh;      // Handle for the address space we're using
};

static void map_superblock(himem_allocator_t * allocator, size_t superblock_id);

static bool esp32_himem_check_valid_block_size(uint16_t block_size)
{
    if ((block_size % 1024) != 0)
    {
        // throw exception(block size should be 1024 aligned);
        ESP_LOGE(TAG, "block size must be 1024 bytes aligned");
        return false;
    }

    if (block_size > ESP_HIMEM_BLKSZ)
    {
        ESP_LOGE(TAG, "block size can be at most %d bytes", ESP_HIMEM_BLKSZ);
        return false;
    }

    return true;
}

static uint16_t esp32_himem_get_maximum_blocks(uint16_t block_size)
{
    if (!esp32_himem_check_valid_block_size(block_size))
    {
        return 0;
    }

    size_t memcnt = esp_himem_get_phys_size();
    size_t memfree = esp_himem_get_free_size();
    uint16_t maximum_blocks = memfree / block_size;

    ESP_LOGD(TAG, "Himem has %dKiB of memory, %dKiB of which is free, can allocate %d blocks of %d bytes.", (int)memcnt / 1024, (int)memfree / 1024, maximum_blocks, block_size);

    return maximum_blocks;
}

static bool esp32_himem_validate_reserved_space()
{
    size_t reserved_space = esp_himem_reserved_area_size();
    if (reserved_space < (32 * 1024))
    {
        ESP_LOGE(TAG, "Himem could not find a minimum of reserved memory for bank switching");
        return false;
    }
    return true;
}

static uint16_t esp32_himem_get_superblocks_needed(uint16_t block_size, uint16_t blocks)
{
    size_t required_space_unaligned = block_size * blocks;
    if ((required_space_unaligned % ESP_HIMEM_BLKSZ) != 0)
    {
        uint16_t partial_superblocks = (required_space_unaligned / ESP_HIMEM_BLKSZ);
        uint16_t aligned_superblocks = partial_superblocks + 1;
        return aligned_superblocks;
    }
    return required_space_unaligned / ESP_HIMEM_BLKSZ;
}

// initialize and return number of available blocks
static himem_allocator_t *esp32_himem_init(uint16_t block_size, uint16_t blocks)
{
    himem_allocator_t *allocator = (himem_allocator_t *)malloc(sizeof(himem_allocator_t));
    assert(allocator);
    allocator->mapped =false;
    allocator->ptr = NULL;

    if (!esp32_himem_check_valid_block_size(block_size))
    {
        return 0;
    }

    allocator->_block_size = block_size;


    uint16_t available_blocks = esp32_himem_get_maximum_blocks(block_size);


    if (blocks > available_blocks)
    {
        // too many blocks requested
        ESP_LOGE(TAG, "too many blocks,  requested %d but can only allocate %d blocks", blocks, available_blocks);
        return 0;
    }

    if (!esp32_himem_validate_reserved_space())
    {
        ESP_LOGE(TAG, "Himem could not find a minimum of reserved memory for bank switching");
        return 0;
    }

    uint16_t required_superblocks = esp32_himem_get_superblocks_needed(block_size, blocks);

    size_t required_space = required_superblocks * ESP_HIMEM_BLKSZ;

    uint16_t possible_blocks = (required_space / block_size);

    ESP_LOGD(TAG, "Himem will use %dKiB of memory for %d superblocks hosting %d blocks", (int)required_space / 1024, (int)required_superblocks, (int)possible_blocks);

    // Allocate the memory we're going to check.
    ESP_ERROR_CHECK(esp_himem_alloc(required_space, &allocator->mh));

    // Allocate a block of address range
    ESP_ERROR_CHECK(esp_himem_alloc_map_range(ESP_HIMEM_BLKSZ, &allocator->rh));

    for (uint16_t i = 0; i < required_superblocks; i++)
    {
        map_superblock(allocator, i);
        memset(allocator->ptr, 0, ESP_HIMEM_BLKSZ);
    }

    return allocator;
}

static void map_superblock(himem_allocator_t * allocator, size_t superblock_id)
{
    if (allocator->mapped)
    {
        // check if currently mapped is the block we need, if not, remap the new block
        if (superblock_id != allocator->current_superblock_id)
        {
            ESP_LOGD(TAG, "unmapping %d as %d", allocator->current_superblock_id, allocator->current_superblock_id * ESP_HIMEM_BLKSZ);
            ESP_ERROR_CHECK(esp_himem_unmap(allocator->rh, allocator->ptr, ESP_HIMEM_BLKSZ));
            ESP_LOGD(TAG, "mapping %d as %d", superblock_id, superblock_id * ESP_HIMEM_BLKSZ);
            ESP_ERROR_CHECK(esp_himem_map(allocator->mh, allocator->rh, superblock_id * ESP_HIMEM_BLKSZ, 0, ESP_HIMEM_BLKSZ, 0, (void **)&allocator->ptr));
            allocator->current_superblock_id = superblock_id;
            allocator->mapped = true;
        }
    }
    else
    {
        ESP_LOGD(TAG, "mapping %d", superblock_id * ESP_HIMEM_BLKSZ);
        ESP_ERROR_CHECK(esp_himem_map(allocator->mh, allocator->rh, superblock_id * ESP_HIMEM_BLKSZ, 0, ESP_HIMEM_BLKSZ, 0, (void **)&allocator->ptr));
        allocator->current_superblock_id = superblock_id;
        allocator->mapped = true;
    }
}

static size_t esp32_himem_read(himem_allocator_t * allocator,const uint16_t block_id, size_t read_offset, void *buf, const size_t len)
{
    if (len > allocator->_block_size)
    {
        // throw exeption(out of bound);
        return 0;
    }

    if ((len + read_offset ) > allocator->_block_size){
        //show error
        return 0;
    }

    uint32_t superblock_id = (block_id * allocator->_block_size) / ESP_HIMEM_BLKSZ;
    uint32_t offset_in_superblock = (block_id * allocator->_block_size) % ESP_HIMEM_BLKSZ;

    map_superblock(allocator,superblock_id);
    ESP_LOGD(TAG, "reading offset %d in 0x%x %d bytes, ptr 0x%x", offset_in_superblock, (uintptr_t)allocator->ptr + offset_in_superblock, len, (uintptr_t)allocator->ptr);
    memcpy(buf, allocator->ptr + offset_in_superblock + read_offset, len);
    return len;
}
static size_t esp32_himem_write(himem_allocator_t * allocator,const uint16_t block_id, size_t write_offset, const void *buf, const size_t len)
{
    if (len > allocator->_block_size)
    {
        // throw exeption(out of bound);
        return 0;
    }

    if ((len + write_offset ) > allocator->_block_size){
        //show error
        return 0;
    }

    uint32_t superblock_id = (block_id * allocator->_block_size) / ESP_HIMEM_BLKSZ;
    uint32_t offset_in_superblock = (block_id * allocator->_block_size) % ESP_HIMEM_BLKSZ;

    map_superblock(allocator,superblock_id);
    ESP_LOGD(TAG, "writing offset %d 0x%x %d bytes, ptr 0x%x", offset_in_superblock, (uintptr_t)allocator->ptr + offset_in_superblock, len, (uintptr_t)allocator->ptr);
    memcpy(allocator->ptr + offset_in_superblock + write_offset, buf, len);
    return len;
}
static void esp32_himem_deinit(himem_allocator_t * allocator)
{
    if (allocator->mapped)
    {
        ESP_ERROR_CHECK(esp_himem_unmap(allocator->rh, allocator->ptr, ESP_HIMEM_BLKSZ));
        allocator->mapped = false;
    }

    ESP_ERROR_CHECK(esp_himem_free(allocator->mh));
    ESP_ERROR_CHECK(esp_himem_free_map_range(allocator->rh));

    free(allocator);
}

const himem_base esp32_himem = {
    .get_maximum_blocks = esp32_himem_get_maximum_blocks,
    .init = esp32_himem_init,
    .read = esp32_himem_read,
    .write = esp32_himem_write,
    .deinit = esp32_himem_deinit};

#endif