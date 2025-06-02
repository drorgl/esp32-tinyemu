// tests/test_vmm_memory.c
#include "unity.h"
#include "vmm_memory.h"
#include <stdint.h>
#include <string.h>

void test_has_non_zero_bytes_single_byte(void)
{
    for (size_t i = 0; i < 1024; i++)
    {
        uint8_t buffer[1024] = {0};
        buffer[i] = 1;
        TEST_ASSERT_TRUE(has_non_zero_bytes(buffer, 1024));
    }
}

void test_has_non_zero_bytes_all_zeros(void)
{
    uint8_t buffer[1024] = {0};
    TEST_ASSERT_FALSE(has_non_zero_bytes(buffer, 1024));
}

int vmm_memory_tests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_has_non_zero_bytes_single_byte);
    RUN_TEST(test_has_non_zero_bytes_all_zeros);
    return UNITY_END();
}