#include <unity.h>
#include <bitmasker.h>
#include <inttypes.h>
#include <string.h>
#include <log.h>

#define UNUSED(x) ((void)(x))


void setUp()
{
}
void tearDown()
{
}

// void initialize()
// {
//     VMM_t *vmm = vmm_create("pagefile.bin", 1024, 128, 128, 128);
//     vmm_destroy(vmm);
// }

// void write_1st_page()
// {
//     VMM_t *vmm = vmm_create("pagefile1.bin", 1024, 128, 128, 128);
//     char *buf = "test";
//     vmm_write(vmm, 0x00, buf, sizeof(buf));
//     vmm_destroy(vmm);
// }

// void write_1st_page_at_start_write_2nd_page_at_1MB()
// {
//     VMM_t *vmm = vmm_create("pagefile2.bin", 1024 * 1024 * 2, 128, 128, 128);
//     char *start = "start";
//     vmm_write(vmm, 0x00, start, strlen(start));

//     char *end = "end";
//     vmm_write(vmm, 1024 * 1024, end, strlen(end));

//     uint8_t buffer[1024];
//     memset(buffer, 0, sizeof(buffer));
//     vmm_read(vmm, 0x00, buffer, strlen(start));
//     TEST_ASSERT_EQUAL_STRING(start, buffer);

//     memset(buffer, 0, sizeof(buffer));
//     vmm_read(vmm, 1024 * 1024, buffer, strlen(end));
//     TEST_ASSERT_EQUAL_STRING(end, buffer);

//     vmm_destroy(vmm);
// }

// void write_sequential_and_verify()
// {
//     VMM_t *vmm = vmm_create("pagefile3.bin", 1024 * 1024 * 20, 128, 10, 10);

//     //check sequential
//     for (uint32_t i = 0; i < 100; i += 10)//1024 * 1024 * 10
//     {

//         char buffer[50];
//         sprintf(buffer, "%" PRIX32, i);

//         vmm_write(vmm, i, buffer, strlen(buffer));

//         char tmpbuffer[50];
//         memset(tmpbuffer, 0, sizeof(tmpbuffer));
//         vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     }

//     vmm_flush(vmm);

//     for (uint32_t i = 0; i <100 ; i += 10)//1024 * 1024 * 10
//     {

//         char buffer[50];
//         sprintf(buffer, "%" PRIX32, i);

//         char tmpbuffer[50];
//         memset(tmpbuffer, 0, sizeof(tmpbuffer));
//         vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     }

//     vmm_flush(vmm);

//     printf("Resetting 0\r\n");

//     //check behavior after set to 0
//     for (uint32_t i = 0; i < 4096; i += 128)//1024 * 1024 *20
//     {

//         uint8_t buffer[128] = {0};

//         vmm_write(vmm, i, buffer, sizeof(buffer));


//         uint8_t zero_buffer[128] = {0};
        
//         uint8_t read_buffer[128] = {0};
        
//         vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
//     }

//     vmm_flush(vmm);

//     //check the error...

//     for (uint32_t i = 0; i < 4096; i +=128) //1024 * 1024 * 20
//     {
//         printf("Testing %d\r\n", i);
//         uint8_t zero_buffer[128] = {0};
        
//         uint8_t read_buffer[128] = {0};
        
//         vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
//     }

//     vmm_flush(vmm);

//     //check sequential
//     for (uint32_t i = 0; i < 15*1025; i += 3000) //1024 * 1024 * 10
//     {

//         char buffer[50];
//         sprintf(buffer, "%" PRIX32, i);

//         vmm_write(vmm, i, buffer, strlen(buffer));

//         char tmpbuffer[50];
//         memset(tmpbuffer, 0, sizeof(tmpbuffer));
//         vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     }

//     vmm_flush(vmm);

//     for (uint32_t i = 0; i < 15*1025; i += 3000) //1024 * 1024 * 10
//     {

//         char buffer[50];
//         sprintf(buffer, "%" PRIX32, i);

//         char tmpbuffer[50];
//         memset(tmpbuffer, 0, sizeof(tmpbuffer));
//         vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     }

//     vmm_flush(vmm);

//     for (uint32_t i = 0; i < 1024 * 1024 *20; i += 128)
//     {

//         char buffer[128] = {0};

//         vmm_write(vmm, i, buffer, sizeof(buffer));
//     }

//     vmm_flush(vmm);

//     for (uint32_t i = 0; i < 1024 * 1024 * 20; i +=128)
//     {
//         uint8_t zero_buffer[128] = {0};
        
//         uint8_t read_buffer[128] = {0};
        
//         vmm_read(vmm, i, read_buffer, sizeof(read_buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_HEX8_ARRAY(zero_buffer, read_buffer, 128);
//     }

//     vmm_flush(vmm);



//     //check sequential
//     for (uint32_t i = 0; i < 1024 * 1024 * 10; i += 3000)
//     {
//         char buffer[50];
//         sprintf(buffer, "%" PRIX32, i);

//         vmm_write(vmm, i, buffer, strlen(buffer));

//         char tmpbuffer[50];
//         memset(tmpbuffer, 0, sizeof(tmpbuffer));
//         vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//         // printf("Testing %d\r\n", i);
//         TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     }

//     vmm_flush(vmm);

//     // for (uint32_t i = 0; i < 1024 * 1024 * 10; i += 3000)
//     // {
//     //     for (uint32_t j = 0; j < 1024 * 1024 * 10; j += 3000)
//     //     {
//     //         {
//     //             char buffer[50];
//     //             sprintf(buffer, "%" PRIX32, i);

//     //             char tmpbuffer[50];
//     //             memset(tmpbuffer, 0, sizeof(tmpbuffer));
//     //             vmm_read(vmm, i, tmpbuffer, strlen(buffer));

//     //             // printf("Testing %d\r\n", i);
//     //             TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     //         }
//     //         {
//     //             char buffer[50];
//     //             sprintf(buffer, "%" PRIX32, j);

//     //             char tmpbuffer[50];
//     //             memset(tmpbuffer, 0, sizeof(tmpbuffer));
//     //             vmm_read(vmm, j, tmpbuffer, strlen(buffer));

//     //             // printf("Testing %d\r\n", i);
//     //             TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     //         }
//     //     }
//     // }

//     // for (uint32_t i = 0; i < 1024 * 1024; i += 50)
//     // {

//     //     char buffer[50];
//     //     sprintf(buffer, "%" PRIX32, i);

//     //     char tmpbuffer[50];
//     //     memset(tmpbuffer, 0, sizeof(tmpbuffer));
//     //     vmm_read(vmm, i + 5, tmpbuffer, strlen(buffer));

//     //     TEST_ASSERT_EQUAL_STRING(buffer, tmpbuffer);
//     // }

//     vmm_destroy(vmm);
// }

// void printBitSetByteWise(uint8_t bitSet) {
//     for(int i = 0; i< sizeof(bitSet) * 8; i++) {
//         if(i>0 && i%4==0) {
//             printf('|');
//         }
//         printf(bitSet.get(i) ? 1 : 0);
//     }
//     System.out.println();
// }



typedef struct {
    int start;
    int end;
    bool active; // To indicate if this record has been populated
} RangeRecord;

// Max 8 possible ranges for an 8-bit bitset (e.g., 0xAA -> [1-1], [3-3], [5-5], [7-7])
#define MAX_RANGE_RECORDS 8
static RangeRecord g_range_records[MAX_RANGE_RECORDS];
static int g_record_count;

void record_range(int start, int end, void *user_data) {
    UNUSED(user_data);
    if (g_record_count < MAX_RANGE_RECORDS) {
        g_range_records[g_record_count].start = start;
        g_range_records[g_record_count].end = end;
        g_range_records[g_record_count].active = true;
        g_record_count++;
    } else {
        // Handle error: too many ranges for the buffer.
        // In a real test scenario, you might fail the test here.
        TEST_FAIL_MESSAGE("Too many ranges recorded!");
    }
}

void clear_records(){
    for (int i = 0; i < MAX_RANGE_RECORDS; ++i) {
        g_range_records[i].start = 0;
        g_range_records[i].end = 0;
        g_range_records[i].active = false;
    }
    g_record_count = 0;
}



// static void enumerate_bit_ranges(uint8_t bitset, void (*callback)(uint32_t start, uint32_t end, void* user_data), void* user_data) {
//     int start = -1;
//     for (int i = 0; i < 8; i++) {
//         if ((bitset >> i) & 1) {  // Check LSB first
//             if (start == -1) {
//                 start = i;  // Mark the start of a new range
//             }
//         } else {
//             if (start != -1) {
//                 callback(start, i - 1, user_data);  // End of a range
//                 start = -1;  // Reset start
//             }
//         }
//     }
//     if (start != -1) {
//         callback(start, 7, user_data);  // Handle case where the last bit is set
//     }
// }

// static uint8_t get_section_mask(int section_shift, uint16_t offset, uint16_t length){
//     int start_idx = offset >> section_shift;
//     int end_idx = (offset + length - 1) >> section_shift;
//     // printf("start_idx %d end_idx %d\r\n", start_idx, end_idx);
//     uint8_t mask = 0;
//     for (int i = start_idx; i <= end_idx; ++i) {
//         mask |= (1 << i); 
//     }
//     return mask;
// }



void test_get_section_mask_section_9_single_bit_0(void) {
    // Offset 0, Length 1 -> Should hit only section 0
    // start_idx = 0 >> 9 = 0
    // end_idx   = (0 + 1 - 1) >> 9 = 0 >> 9 = 0
    // mask = 1 << 0 = 0b00000001
    TEST_ASSERT_EQUAL_HEX8(0b00000001, get_section_mask(9, 0, 1));
}

void test_get_section_mask_section_9_single_bit_mid_section(void) {
    // Offset 256 (in section 0), Length 1 -> Should hit only section 0
    // start_idx = 256 >> 9 = 0
    // end_idx   = (256 + 1 - 1) >> 9 = 256 >> 9 = 0
    // mask = 1 << 0 = 0b00000001
    TEST_ASSERT_EQUAL_HEX8(0b00000001, get_section_mask(9, 256, 1));
}

void test_get_section_mask_section_9_single_bit_end_section_0(void) {
    // Offset 511 (last byte of section 0), Length 1 -> Should hit only section 0
    // start_idx = 511 >> 9 = 0
    // end_idx   = (511 + 1 - 1) >> 9 = 511 >> 9 = 0
    // mask = 1 << 0 = 0b00000001
    TEST_ASSERT_EQUAL_HEX8(0b00000001, get_section_mask(9, 511, 1));
}

void test_get_section_mask_section_9_single_bit_section_1(void) {
    // Offset 512 (first byte of section 1), Length 1 -> Should hit only section 1
    // start_idx = 512 >> 9 = 1
    // end_idx   = (512 + 1 - 1) >> 9 = 512 >> 9 = 1
    // mask = 1 << 1 = 0b00000010
    TEST_ASSERT_EQUAL_HEX8(0b00000010, get_section_mask(9, 512, 1));
}

void test_get_section_mask_section_9_single_bit_end_section_7(void) {
    // Offset 4095 (last byte of section 7), Length 1 -> Should hit only section 7
    // start_idx = 4095 >> 9 = 7
    // end_idx   = (4095 + 1 - 1) >> 9 = 4095 >> 9 = 7
    // mask = 1 << 7 = 0b10000000
    TEST_ASSERT_EQUAL_HEX8(0b10000000, get_section_mask(9, 4095, 1));
}

void test_get_section_mask_section_9_two_contiguous_sections(void) {
    // Offset 0, Length 512 (section 0) + 1 (into section 1) = 513
    // start_idx = 0 >> 9 = 0
    // end_idx   = (0 + 513 - 1) >> 9 = 512 >> 9 = 1
    // mask = (1 << 0) | (1 << 1) = 0b00000011
    TEST_ASSERT_EQUAL_HEX8(0b00000011, get_section_mask(9, 0, 513));
}

void test_get_section_mask_section_9_span_multiple_sections(void) {
    // Offset 256 (mid section 0), Length to span section 0, 1, 2, 3
    // Length = (512 - 256) + 512 + 512 + 1 = 256 + 512 + 512 + 1 = 1281
    // start_idx = 256 >> 9 = 0
    // end_idx   = (256 + 1281 - 1) >> 9 = 1536 >> 9 = 3
    // mask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) = 0b00001111
    TEST_ASSERT_EQUAL_HEX8(0b00001111, get_section_mask(9, 256, 1281));
}

void test_get_section_mask_section_9_entire_range(void) {
    // Offset 0, Length 4096 (all 8 sections)
    // start_idx = 0 >> 9 = 0
    // end_idx   = (0 + 4096 - 1) >> 9 = 4095 >> 9 = 7
    // mask = 0b11111111
    TEST_ASSERT_EQUAL_HEX8(0b11111111, get_section_mask(9, 0, 4096));
}

void test_get_section_mask_section_9_empty_length(void) {
    // Offset 0, Length 0 -> should result in no sections
    // As discussed, this relies on (offset + length - 1) being negative and `end_idx < start_idx`.
    TEST_ASSERT_EQUAL_HEX8(0b00000000, get_section_mask(9, 0, 0));
}

void test_get_section_mask_section_9_specific_mask_0b00011000(void) {
    TEST_ASSERT_EQUAL_HEX8(0b00011000, get_section_mask(9, 1536, 1024));
}



void test_enumerate_bit_ranges_0b00000000(void) {
    clear_records();
    enumerate_bit_ranges(0b00000000, record_range, NULL);
    TEST_ASSERT_EQUAL(0, g_record_count); // No ranges expected
}

void test_enumerate_bit_ranges_0b00000001(void) {
    clear_records();
    enumerate_bit_ranges(0b00000001, record_range, NULL); // Bit 0 set
    TEST_ASSERT_EQUAL(1, g_record_count);
    TEST_ASSERT_EQUAL(0, g_range_records[0].start);
    TEST_ASSERT_EQUAL(0, g_range_records[0].end);
}

void test_enumerate_bit_ranges_0b00000011(void) {
    clear_records();
    enumerate_bit_ranges(0b00000011, record_range, NULL); // Bits 0, 1 set
    TEST_ASSERT_EQUAL(1, g_record_count);
    TEST_ASSERT_EQUAL(0, g_range_records[0].start);
    TEST_ASSERT_EQUAL(1, g_range_records[0].end);
}

void test_enumerate_bit_ranges_0b11000000(void) {
    clear_records();
    enumerate_bit_ranges(0b11000000, record_range, NULL); // Bits 6, 7 set
    TEST_ASSERT_EQUAL(1, g_record_count);
    TEST_ASSERT_EQUAL(6, g_range_records[0].start);
    TEST_ASSERT_EQUAL(7, g_range_records[0].end);
}

void test_enumerate_bit_ranges_0b01001100(void) {
    clear_records();
    enumerate_bit_ranges(0b01001100, record_range, NULL); // Bits 2, 3, 6 set
    TEST_ASSERT_EQUAL(2, g_record_count);
    TEST_ASSERT_EQUAL(2, g_range_records[0].start);
    TEST_ASSERT_EQUAL(3, g_range_records[0].end);
    TEST_ASSERT_EQUAL(6, g_range_records[1].start);
    TEST_ASSERT_EQUAL(6, g_range_records[1].end); // Single bit at index 6
}

void test_enumerate_bit_ranges_0b10000000(void) {
    clear_records();
    enumerate_bit_ranges(0b10000000, record_range, NULL); // Bit 7 set
    TEST_ASSERT_EQUAL(1, g_record_count);
    TEST_ASSERT_EQUAL(7, g_range_records[0].start);
    TEST_ASSERT_EQUAL(7, g_range_records[0].end);
}




void print_range(int start, int end, void *user_data) {
    UNUSED(user_data);

    printf(" [%d-%d]", start, end);
    printf("([%d-%d])", start << 9, (end + 1) << 9);
}

void bitset_tests(){
    printf("\r\n");
    uint8_t bitset =get_section_mask(9, 0, 100);

    for (int i = 0; i < 8; i++) {
        printf("%d", (bitset >> (7 - i)) & 1);
    }
    printf("\r\n");

    enumerate_bit_ranges(bitset,print_range, NULL);
      printf("\r\n");

}




int main()
{
    UNITY_BEGIN();
    log_set_level(LOG_TRACE);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    RUN_TEST(bitset_tests);

    RUN_TEST(test_get_section_mask_section_9_single_bit_0);
    RUN_TEST(test_get_section_mask_section_9_single_bit_mid_section);
    RUN_TEST(test_get_section_mask_section_9_single_bit_end_section_0);
    RUN_TEST(test_get_section_mask_section_9_single_bit_section_1);
    RUN_TEST(test_get_section_mask_section_9_single_bit_end_section_7);
    RUN_TEST(test_get_section_mask_section_9_two_contiguous_sections);
    RUN_TEST(test_get_section_mask_section_9_span_multiple_sections);
    RUN_TEST(test_get_section_mask_section_9_entire_range);
    RUN_TEST(test_get_section_mask_section_9_empty_length);
    RUN_TEST(test_get_section_mask_section_9_specific_mask_0b00011000);


    RUN_TEST(test_enumerate_bit_ranges_0b00000000);
    RUN_TEST(test_enumerate_bit_ranges_0b00000001);
    RUN_TEST(test_enumerate_bit_ranges_0b00000011);
    RUN_TEST(test_enumerate_bit_ranges_0b11000000);
    RUN_TEST(test_enumerate_bit_ranges_0b01001100);
    RUN_TEST(test_enumerate_bit_ranges_0b10000000);


    UNITY_END(); // stop unit testing
}