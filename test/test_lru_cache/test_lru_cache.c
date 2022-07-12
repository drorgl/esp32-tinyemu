
#include <unity.h>
#include <runner.h>

#include <lru_cache.h>

void setUp()
{
}
void tearDown()
{
}

int compare_number(const void *e1, const void *e2)
{
    return (intptr_t)e1 - (intptr_t)e2;
}

static uint8_t flush_called;
static void * flush_key;
static void * flush_value;
static void * flush_context;

void on_flush(void *key, void *value, void *context)
{
    flush_called ++;
    flush_key = key;
    flush_value = value;
    flush_context = context;

}

const char * CTX = "context";

void test_init_free(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );
    lru_cache_free(cache);
}

void test_add_and_check_count(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key", "value");

    TEST_ASSERT_EQUAL(1, lru_cache_count(cache));

    lru_cache_free(cache);
}

void test_add_and_get(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key", "value");

    char * value = lru_cache_get(cache,"key");
    TEST_ASSERT_EQUAL_STRING("value", value);

    lru_cache_free(cache);
}

void test_add_and_get_least_recently_used(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key", "value");

    char * value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("value", value);

    lru_cache_free(cache);
}


void test_add_and_remove(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key", "value");

    char * value = lru_cache_get(cache, "key");
    TEST_ASSERT_EQUAL_STRING("value", value);

    lru_cache_remove(cache, "key");
    value = lru_cache_get(cache, "key");
    TEST_ASSERT_NULL(value);

    lru_cache_free(cache);
}

void test_add_and_sync(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key", "value");

    flush_called =0;
    flush_key = NULL;
    flush_value = NULL;
    flush_context = NULL;

    char * value = lru_cache_get(cache,"key");
    
    lru_cache_sync(cache);

    TEST_ASSERT_EQUAL(1, flush_called);
    TEST_ASSERT_EQUAL("key", flush_key);
    TEST_ASSERT_EQUAL_STRING("value", flush_value);
    TEST_ASSERT_EQUAL(CTX, flush_context);

    lru_cache_free(cache);
}

void test_add_and_get_least_recently_used_with_2_keys(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key1", "value1");
    lru_cache_add(cache, "key2", "value2");

    char * value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("value1", value);

    lru_cache_free(cache);
}

void test_add_and_get_least_recently_used_with_3_keys(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key1", "value1");
    lru_cache_add(cache, "key2", "value2");
    lru_cache_add(cache, "key3", "value3");

    char * value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("value1", value);

    lru_cache_remove(cache, "key1");

    value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("value2", value);

    lru_cache_free(cache);
}

void test_add_and_get_then_check_least_recently_used_is_the_unused_key(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "key1", "value1");
    lru_cache_add(cache, "key2", "value2");

    lru_cache_get(cache,"key2");

    char * value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("value1", value);

    lru_cache_free(cache);
}



void test_add_and_sync_with_2_keys(){
    cache_t * cache = lru_cache_init(compare_number, on_flush,CTX );

    lru_cache_add(cache, "kv1", "kv1");
    lru_cache_add(cache, "kv2", "kv2");

    flush_called =0;
    flush_key = NULL;
    flush_value = NULL;
    flush_context = NULL;

    char * value = lru_cache_get_least_recently_used(cache);
    TEST_ASSERT_EQUAL_STRING("kv1", value);
    lru_cache_remove(cache,value );

    TEST_ASSERT_EQUAL(1, flush_called);
    TEST_ASSERT_EQUAL("kv1", flush_key);
    TEST_ASSERT_EQUAL_STRING("kv1", flush_value);
    TEST_ASSERT_EQUAL(CTX, flush_context);

    lru_cache_free(cache);
}



void process()
{
    UNITY_BEGIN();
    RUN_TEST(test_init_free);
    RUN_TEST(test_add_and_check_count);
    RUN_TEST(test_add_and_get);
    RUN_TEST(test_add_and_get_least_recently_used);
    RUN_TEST(test_add_and_remove);
    RUN_TEST(test_add_and_sync);
    RUN_TEST(test_add_and_get_least_recently_used_with_2_keys);
    RUN_TEST(test_add_and_get_least_recently_used_with_3_keys);
    RUN_TEST(test_add_and_get_then_check_least_recently_used_is_the_unused_key);
    RUN_TEST(test_add_and_sync_with_2_keys);

    UNITY_END();
}

MAIN()
{
    process();
}