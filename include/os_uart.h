#ifndef OS_UART_H
#define OS_UART_H

#include "2440addr.h"
#include <stdarg.h>

void Uart_Init(int pclk, int baud);
void Uart_SendByte(char c);
void Uart_Printf(const char *fmt, ...);

#endif // OS_UART_H
