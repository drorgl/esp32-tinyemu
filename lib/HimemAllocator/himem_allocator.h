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
size_t himem_read(himem_t * himem_instance,const uint16_t block_id, void *buf, const size_t len);
size_t himem_write(himem_t * himem_instance,const uint16_t block_id, const void *buf, const size_t len);
#ifdef __cplusplus
}
#endif