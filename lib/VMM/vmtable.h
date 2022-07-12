#pragma once

#include "stdint.h"
#include "stdbool.h"

typedef struct vmTable_t
{
    uint64_t page_number;
    uint64_t page_address;
    uint8_t *page_cache;
    bool dirty;
} vmTable_t;
