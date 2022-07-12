#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

int vd_chdir(const char *path);

void vd_cwd(char *cwd_buffer, size_t cwd_buffer_length);