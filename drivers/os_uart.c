#include "2440addr.h"

// 初始化 UART0
void Uart_Init(int pclk, int baud)
{
    int i;
    // 1. 配置 GPIO (GPH2=TXD0, GPH3=RXD0)
    rGPHCON &= ~((3<<4) | (3<<6));
    rGPHCON |=  ((2<<4) | (2<<6));
    rGPHUP  &= ~((1<<2) | (1<<3));  // 使能上拉

    // 2. 配置 UART 控制寄存器
    rULCON0 = 0x3;  // 8N1: 8数据位, 无校验, 1停止位
    rUCON0  = 0x5;  // 查询模式 (Polling mode)
    rUFCON0 = 0x0;  // 禁用 FIFO
    rUMCON0 = 0x0;  // 禁用流控

    // 3. 设置波特率 115200
    // UBRDIVn = (int)(PCLK / (baud * 16)) - 1
    rUBRDIV0 = (int)(pclk / (baud * 16)) - 1;

    // 简单的延时，等待设置稳定
    for(i=0; i<100; i++);
}

// 发送一个字符
void Uart_SendByte(char c)
{
    if(c == '\n')
    {
        while(!(rUTRSTAT0 & 0x2)); // 等待发送缓冲区为空
        rUTXH0 = '\r';             // 发送回车
    }
    while(!(rUTRSTAT0 & 0x2));     // 等待发送缓冲区为空
    rUTXH0 = c;                    // 发送字符
}

// 发送字符串
void Uart_SendString(char *pt)
{
    while(*pt)
        Uart_SendByte(*pt++);
}
