#include <unity.h>
#include <vmm.h>
#include <inttypes.h>
#include <string.h>
#include "vmm_memory_test.h"

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
    VMM_t *vmm = vmm_create("pagefile3.bin", 1024 * 1024 * 20, 128, 10, 10);

    printf("check sequential write/read\r\n\r\n");
    for (uint32_t i = 0; i < 100; i += 10)//1024 * 1024 * 10
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i, buffer, strlen(buffer));

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_flush(vmm);

    printf("check sequential read\r\n\r\n");
    for (uint32_t i = 0; i <100 ; i += 10)//1024 * 1024 * 10
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_flush(vmm);

    printf("Resetting to 0x00\r\n\r\n");

    //check behavior after set to 0
    for (uint32_t i = 0; i < 4096; i += 128)//1024 * 1024 *20
    {

        uint8_t buffer[128] = {0};

        vmm_write(vmm, i, buffer, sizeof(buffer));


        uint8_t zero_buffer[128] = {0};
        
        uint8_t read_buffer[128] = {0};
        
        vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
    }

    vmm_flush(vmm);

    //check the error...

    printf("Verifying reset to 0x00\r\n\r\n");

    for (uint32_t i = 0; i < 4096; i +=128) //1024 * 1024 * 20
    {
        printf("Testing %d\r\n\r\n", i);
        uint8_t zero_buffer[128] = {0};
        
        uint8_t read_buffer[128] = {0};
        
        vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
    }

    vmm_flush(vmm);

    //check sequential
    printf("check sequential write/read large range\r\n\r\n");

    for (uint32_t i = 0; i < 15*1025; i += 3000) //1024 * 1024 * 10
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i, buffer, strlen(buffer));

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_flush(vmm);

    printf("check sequential read large range\r\n\r\n");

    for (uint32_t i = 0; i < 15*1025; i += 3000) //1024 * 1024 * 10
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_flush(vmm);

    printf("check sequential write 128 range\r\n\r\n");


    for (uint32_t i = 0; i < 1024 * 1024 *20; i += 128)
    {

        char buffer[128] = {0};

        vmm_write(vmm, i, buffer, sizeof(buffer));
    }

    vmm_flush(vmm);

    printf("check sequential read 128 range\r\n\r\n");

    for (uint32_t i = 0; i < 1024 * 1024 * 20; i +=128)
    {
        uint8_t zero_buffer[128] = {0};
        
        uint8_t read_buffer[128] = {0};
        
        vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
    }

    vmm_flush(vmm);

    printf("check sequential write/read 3000 range\r\n\r\n");


    //check sequential
    for (uint32_t i = 0; i < 1024 * 1024 * 10; i += 3000)
    {
        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        vmm_write(vmm, i, buffer, strlen(buffer));

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        // printf("Testing %d\r\n\r\n", i);
        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_flush(vmm);

    printf("check sequential read 3000 range\r\n\r\n");

    for (uint32_t i = 0; i < 1024 * 1024 * 10; i += 3000)
    {

        char buffer[50];
        sprintf(buffer, "%" PRIX32, i);

        char tmpbuffer[50];
        memset(tmpbuffer, 0, sizeof(tmpbuffer));
        vmm_read(vmm, i, tmpbuffer, strlen(buffer));

        TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
    }

    vmm_destroy(vmm);
}

// void printBitSetByteWise(uint8_t bitSet) {
//     for(int i = 0; i< sizeof(bitSet) * 8; i++) {
//         if(i>0 && i%4==0) {
//             printf('|');
//         }
//         printf(bitSet.get(i) ? 1 : 0);
//     }
//     System.out.println();
// }






int main()
{
    UNITY_BEGIN(); // IMPORTANT LINE!
    // log_set_level(LOG_INFO);
    log_set_level(LOG_TRACE);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    vmm_memory_tests();

    RUN_TEST(initialize);
    RUN_TEST(write_1st_page);
    RUN_TEST(write_1st_page_at_start_write_2nd_page_at_1MB);
    RUN_TEST(write_sequential_and_verify);
   
    UNITY_END(); // stop unit testing
}