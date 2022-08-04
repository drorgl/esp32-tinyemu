#include <unity.h>
#include <vmm.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

void setUp()
{
}
void tearDown()
{
}

void initialize()
{
    VMM_t *vmm = vmm_create("pagefile.bin", 1024, 128, 128, 128, 128, 128, 128);
    vmm_destroy(vmm);
}

void write_1st_page()
{
    VMM_t *vmm = vmm_create("pagefile1.bin", 1024, 128, 128, 128, 128, 128, 128);
    char *buf = "test";
    vmm_write(vmm, 0x00, buf, sizeof(buf));
    vmm_destroy(vmm);
}



void write_1_page_at_start_then_write_before_it()
{
    uint8_t buffer[1024];

    VMM_t *vmm = vmm_create("pagefile2.bin", 1024, 32, 32, 32, 32, 32, 32);

    vmm_write(vmm, 0x05, "1", strlen("1"));

    vmm_write(vmm, 0x04, "2", strlen("2"));

    vmm_write(vmm, 0x03, "3", strlen("3"));

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 0, buffer, 10);
    TEST_ASSERT_EQUAL(buffer[3], '3');
    TEST_ASSERT_EQUAL(buffer[4], '2');
    TEST_ASSERT_EQUAL(buffer[5], '1');

    vmm_destroy(vmm);
}


void write_1st_page_at_start_write_2nd_page_at_1MB()
{
    uint8_t buffer[1024];

    VMM_t *vmm = vmm_create("pagefile2.bin", 1024 * 1024 * 2, 128, 128, 128, 128, 128, 128);
    char *start = "start";

    vmm_write(vmm, 0x00, start, strlen(start));

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 0x00, buffer, strlen(start));
    TEST_ASSERT_EQUAL_STRING(start, buffer);

    char *end = "end";
    vmm_write(vmm, 1024 * 1024, end, strlen(end));

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 1024 * 1024, buffer, strlen(end));
    TEST_ASSERT_EQUAL_STRING(end, buffer);

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 0x00, buffer, strlen(start));
    TEST_ASSERT_EQUAL_STRING(start, buffer);

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 1024 * 1024, buffer, strlen(end));
    TEST_ASSERT_EQUAL_STRING(end, buffer);

    vmm_destroy(vmm);
}

void write_sequential_and_verify(
    size_t maximum_size,
    size_t himem_page_size,
    size_t himem_pages,
    size_t psram_page_size,
    size_t psram_pages,
    size_t ram_page_size,
    size_t ram_pages)
{
    VMM_t *vmm = vmm_create("pagefile3.bin", maximum_size, himem_page_size, himem_pages, psram_page_size, psram_pages, ram_page_size, ram_pages);

    for (uint32_t i = 0; i < maximum_size; i += 10)
    {
        
        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);
        printf("Writing %d %s\r\n", i, buffer);

        vmm_write(vmm, i, buffer, strlen(buffer));
        // char tmpbuffer[50];
        // memset(tmpbuffer, 0, sizeof(tmpbuffer));
        // vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Verifying %d\r\n", i);
        // TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    // vmm_flush(vmm);

    for (uint32_t i = 0; i < maximum_size; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        printf("Testing %d %s\r\n", i, buffer);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    for (uint32_t i = 0; i < maximum_size; i += 10)
    {
        printf("Writing 2nd %d\r\n", i);
        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i + 5, buffer, strlen(buffer));
    }

    for (uint32_t i = 0; i < maximum_size; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        printf("Testing 2nd %d\r\n", i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));

        vmm_read(vmm, i + 5, tmpbuffer, strlen(buffer));


        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_destroy(vmm);
}

void write_sequential_and_verify_128_16()
{
    write_sequential_and_verify(128, 16, 16, 16, 16, 16, 16);
}

void write_sequential_and_verify_128k_32()
{
    write_sequential_and_verify(1024 * 128, 32, 32, 32, 32, 32, 32);
}

void write_sequential_and_verify_1024k_32()
{
    write_sequential_and_verify(1024 * 1024, 32, 32, 32, 32, 32, 32);
}

int main()
{
    UNITY_BEGIN(); // IMPORTANT LINE!
    // log_set_level(LOG_INFO);
    RUN_TEST(initialize);
    RUN_TEST(write_1st_page);
    RUN_TEST(write_1_page_at_start_then_write_before_it);
    RUN_TEST(write_1st_page_at_start_write_2nd_page_at_1MB);
    RUN_TEST(write_sequential_and_verify_128_16);
    RUN_TEST(write_sequential_and_verify_128k_32);
    RUN_TEST(write_sequential_and_verify_1024k_32);
    UNITY_END(); // stop unit testing
}