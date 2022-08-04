#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _himem_t;
typedef struct _himem_t himem_t;

uint16_t himem_allocator_get_maximum_blocks(uint16_t block_size);
himem_t * himem_allocator_init(uint16_t block_size, uint16_t blocks);
void himem_allocator_deinit(himem_t * himem_instance);

uint16_t himem_free_blocks(himem_t * himem_instance);
int16_t himem_allocate_block(himem_t * himem_instance);
void himem_free_block(himem_t * himem_instance,uint16_t block_id);

size_t himem_read(himem_t * himem_instance,const uint16_t block_id, size_t offset, void *buf, const size_t len);
size_t himem_write(himem_t * himem_instance,const uint16_t block_id, size_t offset, const void *buf, const size_t len);
#ifdef __cplusplus
}
#endif