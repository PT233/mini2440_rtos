/*
*********************************************************************************************************
* MINI LIBC FOR uC/OS-II
* (Required when using -nostdlib)
*********************************************************************************************************
*/

#include "includes.h" 

/*
*********************************************************************************************************
* MEMSET
* 描述: 将内存块填充为指定的值
* 用途: OS_EventWaitListInit 中清零等待表
*********************************************************************************************************
*/
void *memset(void *s, int c, unsigned int count)
{
    char *xs = (char *)s; 
    while (count--) {
        *xs++ = c;
    }
    return s;
}

/*
*********************************************************************************************************
* MEMCPY
* 描述: 内存拷贝
* 用途: OSTaskQuery 中复制 TCB
* 优化: 针对 ARM 32位特性，尝试按 4 字节搬运以提高效率
*********************************************************************************************************
*/
void *memcpy(void *dest, const void *src, unsigned int count)
{
    char *tmp = (char *)dest;
    const char *s = (const char *)src;

    // 1. 如果源和目的地址都是 4 字节对齐的，且长度 >= 4，则按 int 拷贝 (速度快4倍)
    if ((((unsigned int)dest & 3) == 0) && 
        (((unsigned int)src  & 3) == 0) && 
        (count >= 4)) 
    {
        unsigned int *d_32 = (unsigned int *)dest;
        const unsigned int *s_32 = (const unsigned int *)src;
        
        while (count >= 4) {
            *d_32++ = *s_32++;
            count -= 4;
        }
        
        // 更新指针位置，处理剩余字节
        tmp = (char *)d_32;
        s   = (char *)s_32;
    }

    // 2. 处理剩余的字节 (或者处理不对齐的情况)
    while (count--) {
        *tmp++ = *s++;
    }

    return dest;
}
