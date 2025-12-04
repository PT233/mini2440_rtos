#include "includes.h"

void OSTimeDly(INT16U ticks) {
    OS_CPU_SR cpu_sr;
    
    if (ticks > 0) {
        OS_ENTER_CRITICAL();
        if ((OSRdyTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0) {
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }
        OSTCBCur->OSTCBDly = ticks;
        OS_EXIT_CRITICAL();
        OS_Sched();
    }
}

INT8U  OSTimeDlyHMSM (INT8U hours, INT8U minutes, INT8U seconds, INT16U milli)
{
    INT32U ticks;
    INT16U loops;


    if (hours > 0 || minutes > 0 || seconds > 0 || milli > 0) {
        if (minutes > 59) {
            return (OS_TIME_INVALID_MINUTES);    /* 验证参数是否在范围内              */
        }
        if (seconds > 59) {
            return (OS_TIME_INVALID_SECONDS);
        }
        if (milli > 999) {
            return (OS_TIME_INVALID_MILLI);
        }
                                                 /* 计算所需的总时钟节拍数.. */
                                                 /* .. (四舍五入到最接近的节拍)                   */
        ticks = ((INT32U)hours * 3600L + (INT32U)minutes * 60L + (INT32U)seconds) * OS_TICKS_PER_SEC
              + OS_TICKS_PER_SEC * ((INT32U)milli + 500L / OS_TICKS_PER_SEC) / 1000L;
        loops = (INT16U)(ticks / 65536L);        /* 计算 65536 节拍延迟的整数倍   */
        ticks = ticks % 65536L;                  /* 获取节拍的小数部分             */
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

INT8U  OSTimeDlyResume (INT8U prio)
{
    OS_CPU_SR  cpu_sr;
    OS_TCB    *ptcb;

    if (prio >= OS_LOWEST_PRIO) {
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    ptcb = (OS_TCB *)OSTCBPrioTbl[prio];                   /* 确保任务存在                */
    if (ptcb != (OS_TCB *)0) {
        if (ptcb->OSTCBDly != 0) {                         /* 查看任务是否被延迟                   */
            ptcb->OSTCBDly  = 0;                           /* 清除时间延迟                     */
            if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY) {  /* 查看任务是否准备好运行  */
                OSRdyGrp               |= ptcb->OSTCBBitY;             /* 使任务准备好运行       */
                OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
                OS_EXIT_CRITICAL();
                OS_Sched();                                /* 查看这是否是新的最高优先级      */
            } else {
                OS_EXIT_CRITICAL();                        /* 任务可能被挂起                    */
            }
            return (OS_NO_ERR);
        } else {
            OS_EXIT_CRITICAL();
            return (OS_TIME_NOT_DLY);                      /* 指示任务未被延迟       */
        }
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_NOT_EXIST);                            /* 任务不存在                  */
}

INT32U  OSTimeGet (void)
{
    OS_CPU_SR  cpu_sr;
    INT32U     ticks;


    OS_ENTER_CRITICAL();
    ticks = OSTime;
    OS_EXIT_CRITICAL();
    return (ticks);
}

void  OSTimeSet (INT32U ticks)
{
    OS_CPU_SR  cpu_sr;
 
    OS_ENTER_CRITICAL();
    OSTime = ticks;
    OS_EXIT_CRITICAL();
}

