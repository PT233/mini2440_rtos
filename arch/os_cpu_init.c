/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* ARM9 / Mini2440 Port
*
* File : OS_CPU_C.C
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
* Task Description
* 任务 1: 初始化任务堆栈 (OSTaskStkInit)
*
* 描述:
* 初始化任务的堆栈帧，使其看起来像是刚发生过中断一样。
* 当任务第一次运行时，OSStartHighRdy 或 OSCtxSw 将恢复这些寄存器，从而使 CPU 跳转到任务入口。
*
* 功能:
* 1. 在堆栈顶端模拟 ARM 寄存器上下文。
* 2. 通常包括：R0-R12, LR, PC, CPSR。
* 3. 对于 Mini2440 (ARM9):
* - PC = task (任务入口地址)
* - LR = 0 (或指向任务返回错误处理函数)
* - R0 = pdata (传递给任务的参数)
* - CPSR = 系统模式/管理模式，开启中断 (IRQ)。
* 4. 堆栈增长方向为高到低 (Full Descending)。
*
* 移植要点:
* - 这里的堆栈布局必须与 OS_CPU_A.ASM 中的恢复上下文汇编代码完全匹配。
* - ARM 处理器通常有 16 个可见寄存器 + CPSR。
* - 确保堆栈指针对齐（通常 8 字节对齐以支持 AAPCS）。
*********************************************************************************************************
*/
OS_STK *OSTaskStkInit (void (*task)(void *pdata), void *pdata1, OS_STK *ptos, INT16U opt)
{
    OS_STK * stk;

    stk = ptos;                         /* Load stack pointer */

    /* Registers: PC, LR, R12-R0, CPSR */
    *stk = (OS_STK)task;              /* PC */
    *(--stk) = (OS_STK)task;          /* LR */
    *(--stk) = 0;                       /* R12 */
    *(--stk) = 0;                       /* R11 */
    *(--stk) = 0;                       /* R10 */
    *(--stk) = 0;                       /* R9 */
    *(--stk) = 0;                       /* R8 */
    *(--stk) = 0;                       /* R7 */
    *(--stk) = 0;                       /* R6 */
    *(--stk) = 0;                       /* R5 */
    *(--stk) = 0;                       /* R4 */
    *(--stk) = 0;                       /* R3 */
    *(--stk) = 0;                       /* R2 */
    *(--stk) = 0;                       /* R1 */
    *(--stk) = (OS_STK )pdata1;       /* R0 */
    *(--stk) = (OS_STK)0x00000013;    /* CPSR (SVC mode, Enable IRQ/FIQ) */

    return (stk);   
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 系统相关钩子函数 (Hooks)
*
* 描述:
* 提供 OS 运行时的回调点。
*
* 功能:
* - OSTaskCreateHook: 任务创建时调用。
* - OSTaskDelHook: 任务删除时调用。
* - OSTaskSwHook: 任务切换时调用。
* - OSTaskStatHook: 统计任务运行时调用。
* - OSTimeTickHook: 每个 Tick 中断时调用。
*
* 移植要点:
* - 在 Mini2440 上，可以在 OSTaskIdleHook 中调用电源管理指令。
* - OSTimeTickHook 可用于处理非 OS 定时事件。
*********************************************************************************************************
*/


void  OSTaskCreateHook (OS_TCB *ptcb)
{
    // 在此输入代码
    ptcb = ptcb;
}

void  OSTaskDelHook (OS_TCB *ptcb)
{
    // 在此输入代码
    INT8U val;
    val = ptcb->OSTCBPrio;
    Uart_Printf("/n task :",val," has been deleted /n");
}

void  OSTaskSwHook (void)
{
    // 在此输入代码
}

void  OSTaskStatHook (void)
{
    // 在此输入代码
}

void  OSTimeTickHook (void)
{
    // 在此输入代码
}



void  OSInitHookBegin (void)
{
    // 在此输入代码
}

void  OSInitHookEnd (void)
{
    // 在此输入代码
}

void  OSTCBInitHook (OS_TCB *ptcb)
{
    // 在此输入代码
    ptcb = ptcb;
}
