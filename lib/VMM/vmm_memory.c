#include "vmm_memory.h"


bool has_non_zero_bytes(const uint8_t *buffer, size_t size) {
    const size_t step = sizeof(uint32_t); // Check 4 bytes at a time
    size_t i;

    // Check 4 bytes at a time
    for (i = 0; i <= size - step; i += step) {
        if (*(const uint32_t *)(buffer + i) != 0) {
            return true; // Found a non-zero byte
        }
    }

    // Check remaining bytes
    for (; i < size; ++i) {
        if (buffer[i] != 0) {
            return true; // Found a non-zero byte
        }
    }
    // for (size_t i = 0; i < size; i++) {
    //     if (buffer[i] != 0) {
    //         return true; // Found a non-zero byte
    //     }
    // }

    return false; // No non-zero bytes found
}