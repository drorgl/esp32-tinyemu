
#include <unity.h>
#include <runner.h>

#include <stdio.h>
#include <stdlib.h>

#include <himem_allocator.h>

void setUp()
{
}
void tearDown()
{
}

// Fill memory with pseudo-random data generated from the given seed.
// Fills the memory in 32-bit words for speed.
static void fill_mem_seed(int seed, void *mem, int len)
{
    uint32_t *p = (uint32_t *)mem;
    unsigned int rseed = seed ^ 0xa5a5a5a5;
    srand(rseed);
    for (int i = 0; i < len / 4; i++)
    {
        // *p++ = rand_r(&rseed);
        *p++ = rand();
    }
}

// Check the memory filled by fill_mem_seed. Returns true if the data matches the data
// that fill_mem_seed wrote (when given the same seed).
// Returns true if there's a match, false when the region differs from what should be there.
static bool check_mem_seed(int seed, void *mem, int len, int phys_addr)
{
    uint32_t *p = (uint32_t *)mem;
    unsigned int rseed = seed ^ 0xa5a5a5a5;
    srand(rseed);
    for (int i = 0; i < len / 4; i++)
    {
        // uint32_t ex = rand_r(&rseed);
        uint32_t ex = rand();
        if (ex != *p)
        {
            printf("check_mem_seed: %x has 0x%08x expected 0x%08x\n", phys_addr + ((char *)p - (char *)mem), *p, ex);
            return false;
        }
        p++;
    }
    return true;
}

void test_himem(size_t block_size = 1024, size_t desired_blocks = 1000)
{
    bool ret = true;
    int seed = 0xaaaa;

    uint16_t total_blocks = desired_blocks;
    himem_t * allocator = himem_allocator_init(block_size, desired_blocks);
    void *buf = malloc(block_size);
    for (int i = 0; i < total_blocks; i++)
    {
        // printf("writing block %d\n", i);
        fill_mem_seed(i ^ seed, buf, block_size);
        himem_write(allocator,i, buf, block_size);
    }

    for (int i = 0; i < total_blocks; i++)
    {
        // printf("verifying block %d\n", i);
        himem_read(allocator,i, buf, block_size);
        if (!check_mem_seed(i ^ seed, buf, block_size, i * block_size))
        {
            printf("Error in block %d\n", i);
            ret = false;
        }
        TEST_ASSERT_TRUE_MESSAGE(ret, "block verification error");
    }

    free(buf);
    himem_allocator_deinit(allocator);
}

void test_himem_1024_1000()
{
    test_himem(1024, 1000);
}


void test_himem_4096_1000()
{
    test_himem(1024, 1000);
}

void test_himem_16384_1000()
{
    test_himem(16384, 1000);
}


void process()
{
    UNITY_BEGIN();
    RUN_TEST(test_himem_1024_1000);
    RUN_TEST(test_himem_4096_1000);
    RUN_TEST(test_himem_16384_1000);
    UNITY_END();
}

MAIN()
{
    process();
}