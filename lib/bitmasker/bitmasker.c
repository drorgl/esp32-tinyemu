#include "bitmasker.h"
#include <assert.h>
#include <log.h>

uint8_t get_section_mask(int section_shift, size_t offset, size_t length){
    assert(section_shift > 0 && "section shift must be greater than 0");
    int start_idx = offset >> section_shift;
    int end_idx = (offset + length - 1) >> section_shift;
    assert(start_idx <=end_idx && "start index must be less than or equal to end index");
    log_debug("start_idx %d end_idx %d", start_idx, end_idx);
    // uint8_t mask = 0;
    // for (int i = start_idx; i <= end_idx; ++i) {
    //     mask |= (1 << i); 
    // }

    uint8_t mask = ((length != 0) ? ((1 << (end_idx - start_idx + 1)) - 1) : 0) << start_idx;

    return mask;
}

void count_bits_naive(const uint8_t* arr, size_t size, size_t* total_ones, size_t* total_zeros) {
    *total_ones = 0;
    *total_zeros = 0;

    for (size_t i = 0; i < size; ++i) {
        uint8_t byte = arr[i];
        for (int j = 0; j < 8; ++j) {
            if ((byte >> j) & 1) { // Check the j-th bit
                (*total_ones)++;
            } else {
                (*total_zeros)++;
            }
        }
    }
}



void enumerate_bit_ranges(uint8_t bitset, void (*callback)(int start, int end, void* user_data), void* user_data) {
    int start = -1;
    int write_calls = 0;
    for (int i = 0; i < 8; i++) {
        if ((bitset >> i) & 1) {  // Check LSB first
            if (start == -1) {
                start = i;  // Mark the start of a new range
            }
        } else {
            if (start != -1) {
                callback(start, i - 1, user_data);  // End of a range
                write_calls++;
                start = -1;  // Reset start
            }
        }
    }
    if (start != -1) {
        callback(start, 7, user_data);  // Handle case where the last bit is set
        write_calls++;
    }
}