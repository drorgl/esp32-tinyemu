#include "himem_allocator.h"

#include <malloc.h>

#include "himem_access/himem_base.h"

#ifdef ESP32
#include "himem_access/esp32_himem.h"
const himem_base *himem = &esp32_himem;
#else
#include "himem_access/emulated_himem.h"
const himem_base *himem = &emulated_himem;
#endif

#include <llist.h>

#include <log.h>

#include <assert.h>

struct _himem_t
{
    uint16_t block_size;
    uint16_t blocks;
    himem_allocator_t *allocator;

    list_t *allocation_list;
    uint16_t allocation_list_count;
    uint16_t allocation_last_block;
};

uint16_t himem_allocator_get_maximum_blocks(uint16_t block_size)
{
    return himem->get_maximum_blocks(block_size);
}

himem_t *himem_allocator_init(uint16_t block_size, uint16_t blocks)
{
    log_debug("creating new himem allocator %d bytes x %d blocks", block_size, blocks);
    himem_t *himem_instance = (himem_t *)malloc(sizeof(himem_t));
    assert(himem_instance);
    himem_instance->block_size = block_size;
    himem_instance->blocks = blocks;
    himem_instance->allocator = himem->init(block_size, blocks);
    himem_instance->allocation_list = list_new();
    himem_instance->allocation_list_count = 0;
    himem_instance->allocation_last_block =0;
    return himem_instance;
}

void himem_allocator_deinit(himem_t *himem_instance)
{
    log_debug("freeing himem allocator");
    list_destroy(himem_instance->allocation_list);
    himem->deinit(himem_instance->allocator);
    free(himem_instance);
}

size_t himem_read(himem_t *himem_instance, const uint16_t block_id, size_t offset, void *buf, const size_t len)
{
    log_debug("%p reading block %d offset %d, %d bytes", himem_instance, block_id, offset, len);
    return himem->read(himem_instance->allocator, block_id, offset, buf, len);
}

size_t himem_write(himem_t *himem_instance, const uint16_t block_id, size_t offset, const void *buf, const size_t len)
{
    log_debug("%p writing block %d offset %d, %d bytes", himem_instance, block_id, offset, len);
    return himem->write(himem_instance->allocator, block_id, offset, buf, len);
}

uint16_t himem_free_blocks(himem_t *himem_instance)
{
    uint16_t unallocated = himem_instance->blocks - himem_instance->allocation_last_block;
    uint16_t available = himem_instance->allocation_list_count;
    return available + unallocated;
}
int16_t himem_allocate_block(himem_t *himem_instance)
{
    list_node_t *last = list_rpop(himem_instance->allocation_list);
    if (last != NULL){
        himem_instance->allocation_list_count--;
        int16_t block_id = last->val;
        LIST_FREE(last);
        log_debug("allocated %d", block_id);
        return block_id;
    }

    if (himem_instance->allocation_last_block >= himem_instance->blocks){
        // no more blocks available
        return -1;
    }

    int16_t block_id = himem_instance->allocation_last_block;
    log_debug("allocated %d", block_id);
    himem_instance->allocation_last_block++;
    return block_id;


}
void himem_free_block(himem_t *himem_instance, uint16_t block_id)
{
    log_debug("free %d", block_id);
    list_lpush(himem_instance->allocation_list, list_node_new(block_id));
    himem_instance->allocation_list_count++;
}