#include "himem_base.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <log.h>

struct _himem_allocator_t
{
    void *himem_container;
    uint16_t _block_size;
    uint16_t _maximum_blocks;

} ;

const uint32_t MAXIMUM_MEMORY = 1024 * 1024 * 4; // 4MB

uint16_t emulated_himem_get_maximum_blocks(uint16_t block_size)
{
    return MAXIMUM_MEMORY / block_size;
}

// initialize and return number of available blocks
static himem_allocator_t* emulated_himem_init(uint16_t block_size, uint16_t maximum_blocks)
{
    log_debug("initializing himem allocator %d bytes x %d blocks", block_size, maximum_blocks);
    himem_allocator_t * allocator = (himem_allocator_t*)malloc(sizeof(himem_allocator_t));
    assert(allocator);
    allocator->_block_size = block_size;
    allocator->_maximum_blocks = maximum_blocks;
    allocator->himem_container = malloc(block_size * maximum_blocks);
    assert(allocator->himem_container);

    return allocator;
}

static size_t emulated_himem_read(himem_allocator_t* allocator, const uint16_t block_id, size_t read_offset, void *buf, const size_t len)
{
    if (block_id >= allocator->_maximum_blocks)
    {
        log_error("block %d is out of range %d", block_id, allocator->_maximum_blocks);
        return 0;
    }

    if (len > allocator->_block_size)
    {
        log_error("block read %d is out of range %d", len, allocator->_block_size);
        return 0;
    }

    if ((len + read_offset ) > allocator->_block_size){
        log_error("block read %d is out of range %d", (len + read_offset ), allocator->_block_size);
        return 0;
    }

    uint32_t offset = (block_id * allocator->_block_size) +  read_offset;
    log_trace("reading offset %d", offset);

    memcpy(buf, allocator->himem_container + offset, len);

    return len;
}

static size_t emulated_himem_write(himem_allocator_t* allocator, const uint16_t block_id, size_t write_offset, const void *buf, const size_t len)
{
    if (block_id >= allocator->_maximum_blocks)
    {
        log_error("block %d is out of range %d", block_id, allocator->_maximum_blocks);
        return 0;
    }

    if (len > allocator->_block_size)
    {
        log_error("block write %d is out of range %d", len, allocator->_block_size);
        return 0;
    }

    if ((len + write_offset ) > allocator->_block_size){
        log_error("block write %d is out of range %d", (len + write_offset ), allocator->_block_size);
        
        return 0;
    }

    uint32_t offset = (block_id * allocator->_block_size) + write_offset;
    log_trace("writing offset %d", offset);
    memcpy(allocator->himem_container + offset, buf, len);
    return len;
}

static void emulated_himem_deinit(himem_allocator_t* allocator)
{
    free(allocator->himem_container);
    free(allocator);
}

const himem_base emulated_himem = {
    .get_maximum_blocks = emulated_himem_get_maximum_blocks,
    .init = emulated_himem_init,
    .read = emulated_himem_read,
    .write = emulated_himem_write,
    .deinit = emulated_himem_deinit};