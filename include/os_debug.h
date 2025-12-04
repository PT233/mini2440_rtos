#ifndef OS_DEBUG_H
#define OS_DEBUG_H
#include "os_uart.h"

int ReentrantTest(int* a, int* b, long c);

static void print_dec(unsigned int n);
static void print_hex(unsigned int n);
void printf(const char *fmt, ... );
#endif // OS_DEBUG_H
