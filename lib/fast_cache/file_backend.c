#include "file_backend.h"

#include <log.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <inttypes.h>
#include <assert.h>

struct _file_backend_t
{
    char pagefile[256];
    FILE *store_file;
};

file_backend_t *backend_open(const char *pagefile,const size_t buffer_size,const size_t maximum_size)
{
    log_trace("creating page file %s of %d", pagefile, maximum_size);
    file_backend_t *backend = (file_backend_t *)malloc(sizeof(file_backend_t));
    assert(backend);
    strncpy(backend->pagefile, pagefile, sizeof(backend->pagefile) -1);

    FILE *create_f = fopen(pagefile, "wb");
    if (!create_f)
    {
        log_error("fopen() failed");
    }
    if (setvbuf(create_f, NULL, _IOFBF, 1024 * 8) != 0)
    {
        perror("setvbuf");
    }
    if (fseek(create_f, maximum_size, SEEK_SET) != 0)
    {
        log_error("fseek() failed");
    }
    if (fputc('\0', create_f) != '\0')
    {
        log_error("fputc() failed");
    }
    fclose(create_f);

    log_trace("opening for rw");

    backend->store_file = fopen(pagefile, "r+b");
    if (!backend->store_file)
    {
        log_error("error opening file");
        free(backend);
        return NULL;
    }
    if (setvbuf(backend->store_file, NULL, _IOFBF, buffer_size) != 0)
    {
        perror("setvbuf");
    }

    return backend;
}

void backend_close(file_backend_t *backend)
{
    log_trace("closing page file %s", backend->pagefile);
    fclose(backend->store_file);
    free(backend);
}

void backend_read(file_backend_t *backend, const long address, void *buffer, const size_t length)
{
    log_trace("reading page file %s,into %p 0x%ld %zu bytes", backend->pagefile, buffer, address, length);
    if (fseek(backend->store_file, address, SEEK_SET) != 0)
    {
        log_warn("Error seeking in backend store, address %d\n", address);
        assert(false);
    }

    if (fread(buffer, sizeof(uint8_t), length, backend->store_file) == 0)
    {
        memset(buffer, 0, length);
        log_error("Error reading from backing store\n");
    }
}

void backend_write(file_backend_t *backend, const long address, const void *buffer, const size_t length)
{
    log_trace("writing page file %s, 0x%" PRIx64 " %d bytes", backend->pagefile, address, length);
    if (fseek(backend->store_file, address, SEEK_SET) != 0)
    {
        log_warn("Error seeking in backing store");
    }

    if (fwrite(buffer, sizeof(uint8_t), length, backend->store_file) != length)
    {
        log_error("Error writing to backing store\n");
    }
}

void backend_flush(file_backend_t *backend){
    log_trace("flushing page file %s", backend->pagefile);
    fflush(backend->store_file);
}