#include "includes.h"

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
    rUCON0  = 0x5;  // 查询模式 
    rUFCON0 = 0x0;  // 禁用 FIFO
    rUMCON0 = 0x0;  // 禁用流控

    // 3. 设置波特率 115200
    rUBRDIV0 = (int)(pclk / (baud * 16)) - 1;

    // 等待设置稳定
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

// 简单的整数转字符串
static void int_to_str(int num, char *str, int pad_width)
{
    char buf[16];
    int i = 0, is_neg = 0;
    
    if (num < 0) {
        is_neg = 1;
        num = -num;
    }
    
    if (num == 0) {
        buf[i++] = '0';
    } 
    else {
        while (num > 0) {
            buf[i++] = '0' + (num % 10);
            num /= 10;
        }
    }
    
    if (is_neg) {
        str[0] = '-';
        for (int j = 0; j < i; j++) {
            str[j + 1] = buf[i - 1 - j];
        }
        str[i + 1] = '\0';
    } 
    else {
        for (int j = 0; j < i; j++) {
            str[j] = buf[i - 1 - j];
        }
        str[i] = '\0';
    }
}

// 简单的十六进制转字符串
static void hex_to_str(unsigned int num, char *str)
{
    char buf[16];
    int i = 0;
    const char *hex = "0123456789abcdef";
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    while (num > 0) {
        buf[i++] = hex[num & 0xf];
        num >>= 4;
    }
    
    for (int j = 0; j < i; j++) {
        str[j] = buf[i - 1 - j];
    }
    str[i] = '\0';
}

// 简单的格式化打印函数，支持 %d, %x, %c, %s, %%
void Uart_Printf(const char *fmt, ...)
{
    va_list args;
    char buf[256];
    char tmp[32];
    char *p = buf;
    const char *s = fmt;
    int len = 0;
    
    va_start(args, fmt);
    
    while (*s && len < 255) {
        if (*s == '%') {
            s++;
            if (*s == '%') {
                // %% -> %
                *p++ = '%';
                len++;
            } else if (*s == 'd') {
                // 整数
                int val = va_arg(args, int);
                int_to_str(val, tmp, 0);
                int i = 0;
                while (tmp[i] && len < 255) {
                    *p++ = tmp[i++];
                    len++;
                }
            } else if (*s == 'x') {
                // 十六进制
                unsigned int val = va_arg(args, unsigned int);
                hex_to_str(val, tmp);
                int i = 0;
                while (tmp[i] && len < 255) {
                    *p++ = tmp[i++];
                    len++;
                }
            } else if (*s == 'c') {
                // 字符
                char c = va_arg(args, int);
                *p++ = c;
                len++;
            } else if (*s == 's') {
                // 字符串
                const char *str = va_arg(args, const char *);
                while (*str && len < 255) {
                    *p++ = *str++;
                    len++;
                }
            } else {
                *p++ = *s;
                len++;
            }
            s++;
        } else if (*s == '\n') {
            *p++ = '\r';
            len++;
            if (len < 255) {
                *p++ = '\n';
                len++;
            }
            s++;
        } else {
            *p++ = *s++;
            len++;
        }
    }
    
    *p = '\0';
    va_end(args);
    
    // 输出缓冲区中的内容
    int i = 0;
    while (buf[i] != '\0') {
        Uart_SendByte(buf[i++]);
    }
}
