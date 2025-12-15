/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* TIME MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_TIME.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
* Task Description
* 任务 1: 任务延时 (OSTimeDly)
*
* 描述:
* 将当前任务延时指定的系统节拍数 (ticks)。
*
* 功能:
* 1. 如果 ticks > 0：
* a. 从就绪表移除当前任务。
* b. 设置 TCB 中的 OSTCBDly。
* c. 触发调度。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
void  OSTimeDly (INT16U ticks)
{
    OS_CPU_SR cpu_sr;
    if(ticks > 0){
        OS_ENTER_CRITICAL();
        if((OSRdyTbl[OSTCBCur->OSTCBY] &= ~(OSTCBCur->OSTCBBitX)) == 0){
            OSRdyGrp &= ~(OSTCBCur->OSTCBBitY);
        }
        OSTCBCur->OSTCBDly = ticks;
        OS_EXIT_CRITICAL();
        OS_Sched();
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 按时分秒延时 (OSTimeDlyHMSM)
*
* 描述:
* 将当前任务延时指定的时间（小时、分、秒、毫秒）。
*
* 功能:
* 1. 验证参数有效性。
* 2. 将时间转换为系统节拍数 (ticks)。
* 3. 调用 OSTimeDly。
*
* 移植要点:
* - 计算涉及 32 位乘法和除法。Mini2440 (ARM9) 硬件支持乘法，但除法需软件库支持，可能会增加些许开销。
*********************************************************************************************************
*/
INT8U  OSTimeDlyHMSM (INT8U hours, INT8U minutes, INT8U seconds, INT16U milli)
{
    // 在此输入代码
    INT32U ticks;
    INT16U loops;

    if (hours > 0 || minutes > 0 || seconds > 0 || milli > 0) {
        if (minutes > 59) {
            return (OS_TIME_INVALID_MINUTES);    /* Validate arguments to be within range              */
        }
        if (seconds > 59) {
            return (OS_TIME_INVALID_SECONDS);
        }
        if (milli > 999) {
            return (OS_TIME_INVALID_MILLI);
        }
                                                /* Compute the total number of clock ticks required.. */
                                                /* .. (rounded to the nearest tick)                   */
        ticks = ((INT32U)hours * 3600L + (INT32U)minutes * 60L + (INT32U)seconds) * OS_TICKS_PER_SEC
            + OS_TICKS_PER_SEC * ((INT32U)milli + 500L / OS_TICKS_PER_SEC) / 1000L;
        loops = (INT16U)(ticks / 65536L);        /* Compute the integral number of 65536 tick delays   */
        ticks = ticks % 65536L;                  /* Obtain  the fractional number of ticks             */
        OSTimeDly((INT16U)ticks);
        while (loops > 0) {
            OSTimeDly(32768);
            OSTimeDly(32768);
            loops--;
        }
        return (OS_NO_ERR);
    }
    return (OS_TIME_ZERO_DLY);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 恢复延时任务 (OSTimeDlyResume)
*
* 描述:
* 唤醒一个正在延时的任务。
*
* 功能:
* 1. 检查任务是否存在且正在延时。
* 2. 清除 TCB 的 OSTCBDly。
* 3. 如果任务未被挂起，将其加入就绪表。
* 4. 触发调度。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSTimeDlyResume (INT8U prio)
{
    OS_CPU_SR  cpu_sr; 
    OS_TCB    *ptcb;

    if (prio >= OS_LOWEST_PRIO) {
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    ptcb = (OS_TCB *)OSTCBPrioTbl[prio];                   /* Make sure that task exist                */
    if (ptcb != (OS_TCB *)0) {
        if (ptcb->OSTCBDly != 0) {                         /* See if task is delayed                   */
            ptcb->OSTCBDly  = 0;                           /* Clear the time delay                     */
            if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY) {  /* See if task is ready to run  */
                OSRdyGrp               |= ptcb->OSTCBBitY;             /* Make task ready to run       */
                OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
                OS_EXIT_CRITICAL();
                OS_Sched();                                /* See if this is new highest priority      */
            } else {
                OS_EXIT_CRITICAL();                        /* Task may be suspended                    */
            }
            return (OS_NO_ERR);
        } else {
            OS_EXIT_CRITICAL();
            return (OS_TIME_NOT_DLY);                      /* Indicate that task was not delayed       */
        }
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_NOT_EXIST);                            /* The task does not exist                  */
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 获取系统时间 (OSTimeGet)
*
* 描述:
* 获取自启动以来的总节拍数。
*
* 功能:
* 1. 进入临界区。
* 2. 读取 OSTime。
* 3. 退出临界区。
*
* 移植要点:
* - 在 32 位系统 (Mini2440) 上，读取 32 位整数通常是原子的，但为了可移植性和防止编译器优化问题，仍建议使用临界区。
*********************************************************************************************************
*/
INT32U  OSTimeGet (void)
{
    // 在此输入代码
    OS_CPU_SR cpu_sr;
    INT32U ticks;
    OS_ENTER_CRITICAL();
    ticks = OSTime;
    OS_EXIT_CRITICAL();
    return (ticks);
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 设置系统时间 (OSTimeSet)
*
* 描述:
* 设置系统节拍计数器。
*
* 功能:
* 1. 进入临界区。
* 2. 写入 OSTime。
* 3. 退出临界区。
*
* 移植要点:
* - 同 OSTimeGet。
*********************************************************************************************************
*/
void  OSTimeSet (INT32U ticks)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    OSTime = ticks;
    OS_EXIT_CRITICAL();
}