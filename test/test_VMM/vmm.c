#include <unity.h>
#include <vmm.h>
#include <inttypes.h>

void setUp()
{
}
void tearDown()
{
}

void initialize()
{
    VMM_t *vmm = vmm_create("pagefile.bin", 1024, 128, 128, 128);
    vmm_destroy(vmm);
}

void write_1st_page()
{
    VMM_t *vmm = vmm_create("pagefile1.bin", 1024, 128, 128, 128);
    char *buf = "test";
    vmm_write(vmm, 0x00, buf, sizeof(buf));
    vmm_destroy(vmm);
}

void write_1st_page_at_start_write_2nd_page_at_1MB()
{
    VMM_t *vmm = vmm_create("pagefile2.bin", 1024 * 1024 * 2, 128, 128, 128);
    char *start = "start";
    vmm_write(vmm, 0x00, start, strlen(start));

    char *end = "end";
    vmm_write(vmm, 1024 * 1024, end, strlen(end));

    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 0x00, buffer, strlen(start));
    TEST_ASSERT_EQUAL_STRING(start, buffer);

    memset(buffer, 0, sizeof(buffer));
    vmm_read(vmm, 1024 * 1024, buffer, strlen(end));
    TEST_ASSERT_EQUAL_STRING(end, buffer);

    vmm_destroy(vmm);
}

void write_sequential_and_verify()
{
    VMM_t *vmm = vmm_create("pagefile3.bin", 1024 * 1024 * 2, 128, 128, 128);

    for (uint32_t i = 0; i < 1024 * 1024; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i, buffer, strlen(buffer));
    }

    for (uint32_t i = 0; i < 1024 * 1024; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    for (uint32_t i = 0; i < 1024 * 1024; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i + 5, buffer, strlen(buffer));
    }

    for (uint32_t i = 0; i < 1024 * 1024; i += 10)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i + 5, tmpbuffer, strlen(buffer));

        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_destroy(vmm);
}

int main()
{
    UNITY_BEGIN(); // IMPORTANT LINE!
    log_set_level(LOG_INFO);
    RUN_TEST(initialize);
    RUN_TEST(write_1st_page);
    RUN_TEST(write_1st_page_at_start_write_2nd_page_at_1MB);
    RUN_TEST(write_sequential_and_verify);
    UNITY_END(); // stop unit testing
}