#pragma once

#include "stdint.h"
#include "stdbool.h"

typedef struct vmTable_t
{
    size_t page_number;
    size_t page_address;
    uint8_t *page_cache;
    bool dirty;
} vmTable_t;
