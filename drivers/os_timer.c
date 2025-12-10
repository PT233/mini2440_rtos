/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
*
* S3C2440 Timer Driver
*
* Filename : os_timer.c
*********************************************************************************************************
*/
#include "os_cfg.h"
#include "os_cpu.h"
#include "2440addr.h"

/* 中断向量表基址 (根据 startup.S 定义) */
#define ISR_START_ADDR  0x33ffff00

/* 外部汇编中断服务程序声明 */
extern void OSTickISR(void);

/*
*********************************************************************************************************
* TIMER 4 INITIALIZATION
*
* Description: 初始化 Timer 4 作为 uC/OS-II 的系统时钟节拍源 (Tick Source)
*
* Note(s)    : 1. 假设 PCLK = 50MHz (Mini2440 标准配置: FCLK=400, HCLK=100, PCLK=50)
* 2. 目标节拍率 = OS_TICKS_PER_SEC (通常为 100Hz, 即 10ms)
* 3. 计算公式:
* Timer Input Clock = PCLK / (Prescaler + 1) / Divider
* Timer Input Clock = 50MHz / (249 + 1) / 4 = 50kHz
* TCNTB4 = 50000 / 100 = 500
*********************************************************************************************************
*/
void InitTimer4(void)
{
    // 1. 设置 Timer 4 的中断向量
    // 将汇编写好的 OSTickISR 地址填入中断向量表，这样硬件产生中断时就知道跳去哪里
    pISR_TIMER4 = (INT32U)OSTickISR;

    // 2. 清除可能存在的旧中断标志 (防止一开中断就误触发)
    rSRCPND |= (1 << 14); // 写 1 清除 Timer4 标志
    rINTPND |= (1 << 14); 

    // 3. 配置预分频器 (Prescaler 1 for Timer 2,3,4)
    // rTCFG0 [15:8] = 249
    rTCFG0 &= ~(0xFF << 8);
    rTCFG0 |=  (249 << 8);

    // 4. 配置分频器 (Divider MUX for Timer 4)
    // rTCFG1 [19:16] = 0001 (1/4)
    rTCFG1 &= ~(0x0F << 16);
    rTCFG1 |=  (1 << 16);

    // 5. 设置计数初值 (TCNTB4)
    // 频率 50kHz, 周期 10ms (100Hz) => 500
    rTCNTB4 = (50000000 / 250 / 4) / OS_TICKS_PER_SEC;

    // 6. 加载初值 (Manual Update)
    // TCON Bit 21: Timer 4 Manual Update
    rTCON |=  (1 << 21);
    
    // 7. 清除手动更新位 (必须做，否则 Timer 不会开始计数)
    rTCON &= ~(1 << 21);

    // 8. 启动定时器 & 开启自动重装载 (Start + Auto Reload)
    // TCON Bit 20: Start, Bit 22: Auto Reload
    rTCON |=  (1 << 20) | (1 << 22);

    // 9. 解除中断屏蔽 (Unmask Timer 4 Interrupt)
    // INTMSK Bit 14: Timer 4
    rINTMSK &= ~(1 << 14);
}