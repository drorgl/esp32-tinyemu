
#include <unity.h>
#include <runner.h>

#include <memory_indexer.h>

void setUp()
{
}
void tearDown()
{
}


void test_init_free(){
    memory_indexer_t * indexer = memory_indexer_init();
    size_t number_of_deleted_elements = memory_indexer_free(indexer);
    TEST_ASSERT_EQUAL(0, number_of_deleted_elements);
}

void test_add_element(){
    memory_indexer_t * indexer = memory_indexer_init();

    memory_indexer_set(indexer, 1, "1");

    size_t number_of_deleted_elements = memory_indexer_free(indexer);
    TEST_ASSERT_EQUAL(1, number_of_deleted_elements);
}

void test_add_element_and_search(){
    memory_indexer_t * indexer = memory_indexer_init();

    memory_indexer_set(indexer, 1, "1");

    char * value = memory_indexer_search(indexer, 1);
    TEST_ASSERT_EQUAL_STRING("1", value);

    size_t number_of_deleted_elements = memory_indexer_free(indexer);
    TEST_ASSERT_EQUAL(1, number_of_deleted_elements);
}

void test_add_element_search_and_remove(){
    memory_indexer_t * indexer = memory_indexer_init();

    memory_indexer_set(indexer, 1, "1");

    char * value = memory_indexer_search(indexer, 1);
    TEST_ASSERT_EQUAL_STRING("1", value);

    memory_indexer_remove(indexer, 1);

    size_t number_of_deleted_elements = memory_indexer_free(indexer);
    TEST_ASSERT_EQUAL(0, number_of_deleted_elements);
}

void test_add_element_update_and_search(){
    memory_indexer_t * indexer = memory_indexer_init();

    memory_indexer_set(indexer, 1, "1");

    char * value = memory_indexer_search(indexer, 1);
    TEST_ASSERT_EQUAL_STRING("1", value);

    memory_indexer_set(indexer, 1, "11");
    value = memory_indexer_search(indexer, 1);
    TEST_ASSERT_EQUAL_STRING("11", value);

    size_t number_of_deleted_elements = memory_indexer_free(indexer);
    TEST_ASSERT_EQUAL(1, number_of_deleted_elements);
}


void process()
{
    UNITY_BEGIN();
    RUN_TEST(test_init_free);
    RUN_TEST(test_add_element);
    RUN_TEST(test_add_element_and_search);
    RUN_TEST(test_add_element_search_and_remove);
    RUN_TEST(test_add_element_update_and_search);
    UNITY_END();
}

MAIN()
{
    process();
}