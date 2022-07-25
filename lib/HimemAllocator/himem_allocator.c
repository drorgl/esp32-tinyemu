#include "himem_allocator.h"

#include <malloc.h>

#include "himem_access/himem_base.h"

#if (defined(ESP32) && !defined(ESP32S3))
#include "himem_access/esp32_himem.h"
const himem_base *himem = &esp32_himem;
#else
#include "himem_access/emulated_himem.h"
const himem_base *himem = &emulated_himem;
#endif

struct _himem_t{
    uint16_t block_size;
    uint16_t blocks;
    himem_allocator_t * allocator;
};

uint16_t himem_allocator_get_maximum_blocks(uint16_t block_size){
    return himem->get_maximum_blocks(block_size);
}

himem_t * himem_allocator_init(uint16_t block_size, uint16_t blocks)
{
    himem_t * himem_instance =(himem_t *)malloc(sizeof(himem_t));
    himem_instance->block_size = block_size;
    himem_instance->blocks = blocks;
    himem_instance->allocator = himem->init(block_size, blocks);
    return himem_instance;
}

void himem_allocator_deinit(himem_t * himem_instance)
{
    himem->deinit(himem_instance->allocator);
}

size_t himem_read(himem_t * himem_instance,const uint16_t block_id, void *buf, const size_t len)
{
    return himem->read(himem_instance->allocator,block_id, buf, len);
}

size_t himem_write(himem_t * himem_instance,const uint16_t block_id, const void *buf, const size_t len)
{
    return himem->write(himem_instance->allocator,block_id, buf, len);
}
