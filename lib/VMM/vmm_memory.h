#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Checks if a buffer contains any non-zero bytes.
 *
 * This function efficiently checks if a given buffer contains any byte that is not zero.
 * It optimizes the process by checking 4 bytes (uint32_t) at a time where possible.
 *
 * @param buffer A pointer to the beginning of the buffer (uint8_t array).
 * @param size The size of the buffer in bytes.
 * @return true if the buffer contains at least one non-zero byte, false otherwise.
 */
bool has_non_zero_bytes(const uint8_t *buffer, size_t size);