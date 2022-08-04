#pragma once

#include <stddef.h>

struct _file_backend_t;
typedef struct _file_backend_t file_backend_t;

file_backend_t *backend_open(const char *pagefile,const size_t buffer_size,const size_t maximum_size);

void backend_close(file_backend_t *backend);

void backend_read(file_backend_t *backend, const long address, void *buffer, const size_t length);

void backend_write(file_backend_t *backend, const long address, const void *buffer, const size_t length);
void backend_flush(file_backend_t *backend);