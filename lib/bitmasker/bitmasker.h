#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Generates a bitmask representing sections within a larger data structure.
 *
 * This function calculates a bitmask where each set bit corresponds to a section
 * within a data structure.  The sections are determined by dividing the address space
 * based on the `section_shift` value.  The resulting bitmask indicates which sections
 * are covered by the specified `offset` and `length`.
 *
 * @param section_shift The number of bits to shift the offset and length by to determine the section index.
 *                      Must be greater than 0.  Determines the size of each section (2^section_shift).
 * @param offset The starting offset within the data structure.
 * @param length The length of the region of interest.
 * @return An 8-bit mask where each bit represents a section.  A set bit indicates that the
 *         corresponding section is covered by the offset and length.
 */
uint8_t get_section_mask(int section_shift, size_t offset, size_t length);

/**
 * @brief Counts the number of set and unset bits (1s and 0s) in a byte array.
 *
 * This function iterates through the provided byte array and counts the number
 * of bits that are set to 1 and the number of bits that are set to 0. The results
 * are stored in the `total_ones` and `total_zeros` pointers.
 *
 * @param arr A pointer to the byte array to analyze.
 * @param size The size of the byte array in bytes.
 * @param total_ones A pointer to a size_t variable where the total number of ones will be stored.
 * @param total_zeros A pointer to a size_t variable where the total number of zeros will be stored.
 */
void count_bits_naive(const uint8_t* arr, size_t size, size_t* total_ones, size_t* total_zeros);

/**
 * @brief Enumerates ranges of consecutive set bits within a byte.
 *
 * This function iterates through an 8-bit value (`bitset`) and identifies ranges
 * of consecutive bits that are set to 1.  For each such range, it calls the
 * provided `callback` function with the starting and ending indices of the range.
 *
 * @param bitset The 8-bit value to analyze.
 * @param callback A function pointer to a callback function that will be called for each range of set bits.
 *                 The callback function should accept the starting index, ending index, and a user-defined
 *                 data pointer as arguments.
 * @param user_data A pointer to user-defined data that will be passed to the callback function.
 */
void enumerate_bit_ranges(uint8_t bitset, void (*callback)(int start, int end, void* user_data), void* user_data);


#ifdef __cplusplus
}
#endif