#pragma once

#include <stddef.h>
#include <stdint.h>

struct _himem_allocator_t;
typedef struct _himem_allocator_t himem_allocator_t;

typedef struct {
    uint16_t (*get_maximum_blocks)(uint16_t block_size);
    himem_allocator_t * (*init)(uint16_t block_size, uint16_t blocks); //initialize and return number of allocated blocks
    size_t (*read)(himem_allocator_t * allocator, const uint16_t block_id, size_t offset, void *buf, const size_t len);
    size_t (*write)(himem_allocator_t * allocator,const uint16_t block_id, size_t offset, const void *buf, const size_t len);
    void (*deinit)(himem_allocator_t * allocator);
} himem_base;