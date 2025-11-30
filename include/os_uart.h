#ifndef OS_UART_H
#define OS_UART_H

#include "2440addr.h"

void Uart_Init(int pclk, int baud);
void Uart_SendByte(char c);
void Uart_SendString(char *pt);
#endif // OS_UART_H