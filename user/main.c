#include "include.h"

OS_STACK TaskStack1[TASK_STACK_SIZE];
OS_STACK TaskStack2[TASK_STACK_SIZE];
OS_STACK TaskStack3[TASK_STACK_SIZE];

//函数声明,设计三个任务才会用到互斥锁，并且实现优先级反转功能实现
//中优先级把低优先级堵门，导致高优先级任务无法运行
void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
//============================================================
void main(void)
{
    OS_init();//还没定义
    InitTimer();//还没定义
    OS_TASK_CREATE(Task1, (void *)0, &TaskStack1[TASK_STACK_SIZE - 1], 5);//还没定义
    OS_TASK_CREATE(Task2, (void *)0, &TaskStack2[TASK_STACK_SIZE - 1], 6);//还没定义
    OS_TASK_CREATE(Task3, (void *)0, &TaskStack3[TASK_STACK_SIZE - 1], 7);//还没定义
    OS_start();//还没定义
}
//============================================================ 
//任务1:
void Task1(void *pdata)
{
    int a = 1, b = 2;
    long c = 3;
    pdata = pdata; // 防止编译器警告
    for(;;) { 
        ReentrantTest(&a, &b, c);
    }
}

 
void EINT0_Handler(void) {
    rSRCPND |= (1<<0);  // EINT0 对应 位0
    rINTPND |= (1<<0);
}
//============================================================ 
//任务2：LED流水灯
void Task2(void *pdata)
{
    int a = 1, b = 2;
    long c = 3;
    pdata = pdata; // 防止编译器警告
    // 初始化 LED (GPB5, GPB6, GPB7, GPB8)
    rGPBCON &= ~(0xFF << 10); 
    rGPBCON |= (0x55 << 10); 
    pISR_EINT0 = (unsigned)EINT0_Handler;
    // 全灭 
    rGPBDAT |= (0xF << 5);
    for(;;) { 
        ReentrantTest(&a, &b, c);
        // 流水灯
        for(int i = 5; i <= 8; i++) {
             rGPBDAT &= ~(1 << i); // 点亮 LED i
             OSTimeDlyHMSM(0, 0, 1, 0);
             rGPBDAT |= (1 << i);  // 熄灭 LED i
        }
    }
}
//============================================================
//任务3：
void Task3(void *pdata)
{
    int a = 1, b = 2;
    long c = 3;
    pdata = pdata; // 防止编译器警告
    for(;;) {
        ReentrantTest(&a, &b, c);
        OSFlagPend();
        OSMailBoxPend();
        OSMutexPend();
        OSQueuePend();
        OSSemPend();
        OSTaskDelete(OS_PRIO_SELF);
        OSTaskSuspend(OS_PRIO_SELF);
        OSTimeDelay();
        OSTimeDelayHMSM();//HMSM:小时,分钟,秒,毫秒
    }
}
