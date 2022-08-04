
#include <unity.h>
#include <runner.h>

#include <heap_cache.h>
#include <himem_cache.h>
#include <file_backend.h>
#include <malloc.h>

void setUp()
{
}
void tearDown()
{
}

static void *memory_allocate(size_t bytes)
{
    return malloc(bytes);
}

static void memory_free(void *ptr)
{
    free(ptr);
}

void write_backend(void *context, const size_t address, const void *buffer, const size_t length)
{
    file_backend_t *backend_file = context;
    backend_write(backend_file, address, buffer, length);
}
void read_backend(void *context, const size_t address, void *buffer, const size_t length)
{
    file_backend_t *backend_file = context;
    backend_read(backend_file, address, buffer, length);
}

void heap_cache_initAndDeInit()
{
    file_backend_t *backend_file = backend_open("testpagefile1.bin", 128, 1024);

    heap_cache_t *cache = heap_cache_init("test", 128, 10, memory_allocate, memory_free, write_backend, read_backend, backend_file);
    heap_cache_deinit(cache);
    backend_close(backend_file);
}

void heap_cache_writeSequential()
{
    file_backend_t *backend_file = backend_open("testpagefile2.bin", 128, 1024);

    heap_cache_t *cache = heap_cache_init("test", 32, 3, memory_allocate, memory_free, write_backend, read_backend, backend_file);

    for (int i = 0; i < 128; i+= 4){
        heap_cache_write(cache,i, &i, sizeof(int) );
    }

    for (int i = 0; i < 128; i+= 4){
        int val;
        heap_cache_read(cache,i, &val, sizeof(int) );
        TEST_ASSERT_EQUAL(i, val);
    }

    heap_cache_deinit(cache);
    backend_close(backend_file);
}













void himem_cache_initAndDeInit()
{
    file_backend_t *backend_file = backend_open("testpagefile3.bin", 128, 1024);

    himem_cache_t *cache = himem_cache_init(128, 10, write_backend, read_backend, backend_file);
    himem_cache_deinit(cache);
    backend_close(backend_file);
}

void himem_cache_writeSequential()
{
    file_backend_t *backend_file = backend_open("testpagefile4.bin", 128, 1024);

    himem_cache_t *cache = himem_cache_init(32, 3, write_backend, read_backend, backend_file);

    for (int i = 0; i < 128; i+= 4){
        himem_cache_write(cache,i, &i, sizeof(int) );

        int val;
        himem_cache_read(cache,i, &val, sizeof(int) );
        TEST_ASSERT_EQUAL(i, val);
    }

    for (int i = 0; i < 128; i+= 4){
        int val;
        himem_cache_read(cache,i, &val, sizeof(int) );
        TEST_ASSERT_EQUAL(i, val);
    }

    himem_cache_deinit(cache);
    backend_close(backend_file);
}










void process()
{
    UNITY_BEGIN();
    RUN_TEST(heap_cache_initAndDeInit);
    RUN_TEST(heap_cache_writeSequential);
    RUN_TEST(himem_cache_initAndDeInit);
    RUN_TEST(himem_cache_writeSequential);
    UNITY_END();
}

MAIN()
{
    process();
    return 0;
}