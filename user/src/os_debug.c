#include "includes.h"

// 手动定义变长参数宏，不依赖 <stdarg.h>
typedef __builtin_va_list va_list;

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

//因为taks1和task2会同时调用此函数，测试编译器对可重入函数的支持情况
int ReentrantTest(int* a, int* b, long c)
{
	int va = *a;
	int vb = *b;
	long lc = c;
	a = b;
	return a;	
}

// 将整数转换为字符串
static void print_dec(uint32_t n)
{
    if (n == 0) {
        Uart_SendByte('0');
        return;
    }
    
    char buf[16];
    int i = 0;
    
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    
    while (--i >= 0) {
        Uart_SendByte(buf[i]);
    }
}

static void print_hex(uint32_t n)
{
    int i;
    int started = 0;
    Uart_SendString("0x");
    if (n == 0) {
        Uart_SendByte('0');
        return;
    }
    for (i = 28; i >= 0; i -= 4) {
        int digit = (n >> i) & 0xF;
        if (digit != 0 || started || i == 0) {
            if (digit < 10)
                Uart_SendByte(digit + '0');
            else
                Uart_SendByte(digit - 10 + 'a');
            started = 1;
        }
    }
}

void printf(const char *fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            Uart_SendByte(*fmt++);
            continue;
        }
        
        fmt++; // 跳过 '%'
        switch (*fmt) {
            case 'd':
            case 'u': // 简化处理，都按无符号十进制打印
                print_dec(va_arg(ap, unsigned int));
                break;
            case 'x':
            case 'X':
            case 'p':
                print_hex(va_arg(ap, unsigned int));
                break;
            case 's':
                Uart_SendString(va_arg(ap, char *));
                break;
            case 'c':
                Uart_SendByte((char)va_arg(ap, int));
                break;
            case '%':
                Uart_SendByte('%');
                break;
            default:
                Uart_SendByte('%');
                Uart_SendByte(*fmt);
                break;
        }
        fmt++;
    }
    va_end(ap);
}
