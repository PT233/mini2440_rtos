#include "2440addr.h"
#include "os_cpu.h"
#include "os_uart.h"

// 1.蜂鸣器控制：1响，0停
void Beep_Ctrl(int on)
{
    if (on) {rGPBDAT |=  (1<<0); }
    else    {rGPBDAT &= ~(1<<0);}
}

void Beep_Init(void){
    rGPBCON &= ~(3<<0);
    rGPBCON |=  (1<<0);
    Beep_Ctrl(0);
}
// 2.LED 控制：mask 的 bit0-3 对应 LED1-4
// Mini2440 LED 是低电平点亮
void LED_Set(int mask)
{
    INT32U val = rGPBDAT;
    val |= (0xF << 5); // 先全部关 (置1)
    
    // 如果 bit0 是 1 (亮)，则 GPB5 置 0
    if (mask & 1) {val &= ~(1<<5);}
    if (mask & 2) {val &= ~(1<<6);}
    if (mask & 4) {val &= ~(1<<7);}
    if (mask & 8) {val &= ~(1<<8);}
    
    rGPBDAT = val;
}

void LED_Init(void){
    rGPBCON &= ~((3<<10)|(3<<12)|(3<<14)|(3<<16));
    rGPBCON |=  ((1<<10)|(1<<12)|(1<<14)|(1<<16));
    LED_Set(0);
}

// 3.按键扫描
int Key_Scan(void)
{
    INT32U val = rGPGDAT;
    if (!(val & (1<<0)))  {return 1;}
    if (!(val & (1<<3)))  {return 2;}
    if (!(val & (1<<5)))  {return 3;}
    if (!(val & (1<<6)))  {return 4;}
    if (!(val & (1<<7)))  {return 5;}
    if (!(val & (1<<11))) {return 6;}
    return 0;
}

void Key_Init(void){
    rGPGCON &= ~((3<<0)|(3<<6)|(3<<10)|(3<<12)|(3<<14)|(3<<22));
}

void BSP_Init(void)
{
    Uart_Init(50000000,115200);// PCLK=50000000, Baud=115200
    Beep_Init();
    LED_Init();
    Key_Init(); 
    InitTimer4();
}
