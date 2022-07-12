
#include <unity.h>
#include <runner.h>

#include <direct_cache.h>

void setUp()
{
}
void tearDown()
{
}


void test_init_free(){
    direct_cache_t * cache = direct_cache_init(10);
    direct_cache_free(cache);
}

void test_different_cells(){
    direct_cache_t * cache = direct_cache_init(4);

    direct_cache_set(cache, (void*)0, (void*)10);
    direct_cache_set(cache, (void*)1, (void*)11);
    direct_cache_set(cache, (void*)2, (void*)12);
    direct_cache_set(cache, (void*)3, (void*)13);

    TEST_ASSERT_EQUAL(10, direct_cache_get(cache,0));
    TEST_ASSERT_EQUAL(11, direct_cache_get(cache,1));
    TEST_ASSERT_EQUAL(12, direct_cache_get(cache,2));
    TEST_ASSERT_EQUAL(13, direct_cache_get(cache,3));

    direct_cache_free(cache);
}

void test_same_cells(){
    direct_cache_t * cache = direct_cache_init(3);

    direct_cache_set(cache, 0, 10);
    direct_cache_set(cache, 1, 11);
    direct_cache_set(cache, 2, 12);
    direct_cache_set(cache, 3, 13);

    TEST_ASSERT_EQUAL(NULL, direct_cache_get(cache,0));
    TEST_ASSERT_EQUAL(13, direct_cache_get(cache,3));
    TEST_ASSERT_EQUAL(11, direct_cache_get(cache,1));
    TEST_ASSERT_EQUAL(12, direct_cache_get(cache,2));

    direct_cache_free(cache);
}

void test_clear_cell(){
    direct_cache_t * cache = direct_cache_init(4);

    direct_cache_set(cache, 0, 10);
    direct_cache_set(cache, 1, 11);

    TEST_ASSERT_EQUAL(10, direct_cache_get(cache,0));
    TEST_ASSERT_EQUAL(11, direct_cache_get(cache,1));

    direct_cache_remove(cache, 0);
    TEST_ASSERT_EQUAL(NULL, direct_cache_get(cache,0));

    direct_cache_free(cache);
}


void process()
{
    UNITY_BEGIN();
    RUN_TEST(test_init_free);
    RUN_TEST(test_different_cells);
    RUN_TEST(test_same_cells);
    RUN_TEST(test_clear_cell);

    UNITY_END();
}

MAIN()
{
    process();
}