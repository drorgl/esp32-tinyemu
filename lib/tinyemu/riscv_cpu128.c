#include <inttypes.h>
#if defined(__SIZEOF_INT128__)
#define MAX_XLEN 128
#include "riscv_cpu.c"
#endif
