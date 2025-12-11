/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_MUTEX.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                            LOCAL CONSTANTS
*********************************************************************************************************
*/

#define  OS_MUTEX_KEEP_LOWER_8   0x00FF
#define  OS_MUTEX_KEEP_UPPER_8   0xFF00

#define  OS_MUTEX_AVAILABLE      0x00FF

/*
*********************************************************************************************************
* Task Description
* 任务 1: 无等待请求互斥量 (OSMutexAccept)
*
* 描述:
* 尝试获取互斥量，如果被占用则立即返回。
*
* 功能:
* 1. 检查 OSEventCnt 的值。
* 2. 如果高 8 位（PIP）和低 8 位（Value）指示可用：
* a. 获取互斥量：设置所有者优先级，链接 TCB。
* b. 返回 1。
* 3. 否则返回 0。
*
* 移植要点:
* - 互斥量使用了 OSEventCnt 的高 8 位存储 PIP (Priority Inheritance Priority)，低 8 位存储状态。
* - Mini2440 是小端模式 (Little Endian)，位操作 `>> 8` 和 `& 0xFF` 是标准的，不受端序影响，但直接内存访问需小心。
*********************************************************************************************************
*/
INT8U  OSMutexAccept (OS_EVENT *pevent, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建互斥量 (OSMutexCreate)
*
* 描述:
* 创建一个互斥信号量，用于解决优先级反转问题。
*
* 功能:
* 1. 验证优先级继承优先级 (prio) 是否有效且未被占用。
* 2. 在优先级表中保留该 PIP。
* 3. 申请 ECB。
* 4. 初始化 OSEventCnt：高 8 位为 PIP，低 8 位为 OS_MUTEX_AVAILABLE (0xFF)。
*
* 移植要点:
* - PIP 必须是系统独有的优先级，不能被其他任务使用。
*********************************************************************************************************
*/
OS_EVENT  *OSMutexCreate (INT8U prio, INT8U *err)
{
    OS_CPU_SR  cpu_sr;
    OS_EVENT  *pevent;

    // 1. 参数检查：不能在中断中调用
    if (OSIntNesting > 0) {
        *err = OS_ERR_CREATE_ISR;
        return ((OS_EVENT *)0);
    }
    if (prio >= OS_LOWEST_PRIO) { // 注意：通常Mutex优先级不能等于最低优先级
        *err = OS_PRIO_INVALID;
        return ((OS_EVENT *)0);
    }
    OS_ENTER_CRITICAL();
    if (OSTCBPrioTbl[prio] != (OS_TCB *)0) { 
        OS_EXIT_CRITICAL();
        *err = OS_PRIO_EXIST;  // 报错：优先级已被占用
        return ((OS_EVENT *)0);
    }
    pevent = OSEventFreeList;
    if (pevent == (OS_EVENT *)0) {
        OS_EXIT_CRITICAL();
        *err = OS_ERR_PEVENT_NULL; 
        return ((OS_EVENT *)0);
    }
    OSEventFreeList = (OS_EVENT *)OSEventFreeList->OSEventPtr;
    OSTCBPrioTbl[prio] = (OS_TCB *)1; 
    OS_EXIT_CRITICAL();
    pevent->OSEventType = OS_EVENT_TYPE_MUTEX;
    
    // 互斥锁的 Cnt 比较特殊：
    // 高8位 = PIP (优先级继承优先级)
    // 低8位 = 0xFF (表示锁是可用的，Available)
    pevent->OSEventCnt  = (prio << 8) | OS_MUTEX_AVAILABLE; 
    
    pevent->OSEventPtr  = (void *)0; // 互斥锁不需要 Ptr 指向别的
    
    OS_EventWaitListInit(pevent);    // 清空等待列表

    *err = OS_NO_ERR;
    return (pevent);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 删除互斥量 (OSMutexDel)
*
* 描述:
* 删除互斥量。
*
* 功能:
* 1. 释放保留的 PIP (在优先级表中置空)。
* 2. 处理等待任务。
* 3. 回收 ECB。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSMutexDel (OS_EVENT *pevent, INT8U opt, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 等待互斥量 (OSMutexPend)
*
* 描述:
* 请求互斥量，支持优先级继承。
*
* 功能:
* 1. 如果互斥量可用，占有之。
* 2. 如果不可用：
* a. 检查持有者优先级。
* b. 如果持有者优先级低于当前任务，提升持有者优先级到 PIP (优先级继承)。
* c. 挂起当前任务。
* 3. 触发调度。
*
* 移植要点:
* - 优先级继承涉及修改持有任务的 TCB 和就绪表位置，逻辑复杂，需确保操作原子性。
*********************************************************************************************************
*/
void  OSMutexPend (OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
    OS_CPU_SR   cpu_sr;
    INT8U       pip; //Priority Inheritance Priority
    INT8U       mprio;//Mutex owner Priority
    BOOLEAN     rdy;
    OS_TCB      *ptcb;

    if(OSIntNesting > 0){
        *err = OS_ERR_PEND_ISR;
        return;
    }
    if(pevent == (OS_EVENT *)0){
        *err = OS_ERR_PEVENT_NULL;
        return;
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_MUTEX){
        *err = OS_ERR_EVENT_TYPE;
        return;
    }

    OS_ENTER_CRITICAL();
    if((INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8) == OS_MUTEX_AVAILABLE){
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;
        pevent->OSEventCnt |= OSTCBCur->OSTCBPrio;
        pevent->OSEventPtr = (void *)OSTCBCur;
        OS_EXIT_CRITICAL();
        *err = OS_NO_ERR;
        return;
    }
    pip = (INT8U)(pevent->OSEventCnt >> 8);
    mprio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);
    ptcb = (OS_TCB *)(pevent->OSEventPtr);
    //解决三者堵门问题(优先级A>B>C, C占用cpu, B想抢占C, 但堵了A的门)
    if((ptcb->OSTCBPrio != pip) && (mprio > OSTCBCur->OSTCBPrio)){
        if((OSRdyTbl[ptcb->OSTCBY]) & (ptcb->OSTCBBitX) != 0x00){
            if((OSRdyTbl[ptcb->OSTCBY]) & ~(ptcb->OSTCBBitX) == 0x00){
                OSRdyGrp &= ~(ptcb->OSTCBBitY);
            }
            rdy = TRUE;
        }
        else{
            rdy = FALSE;
        }
        ptcb->OSTCBPrio = pip;
        ptcb->OSTCBY = (ptcb->OSTCBPrio) >> 3;
        ptcb->OSTCBBitY = OSMapTbl[ptcb->OSTCBY];
        ptcb->OSTCBX = (ptcb->OSTCBPrio) & 7;
        ptcb->OSTCBBitX = OSMapTbl[ptcb->OSTCBX];
        if(rdy == TRUE){
            OSRdyGrp |= ptcb->OSTCBBitY;
            OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX; 
        }
        OSTCBPrioTbl[pip] = (OS_TCB *)ptcb;
    }
    OSTCBCur->OSTCBStat |= OS_STAT_MUTEX;
    OSTCBCur->OSTCBDly = timeout;
    OS_EventTaskWait(pevent);//还没完成
    OS_EXIT_CRITICAL();
    OS_Sched();
    OS_ENTER_CRITICAL();
    if(OSTCBCur->OSTCBStat & OS_STAT_MUTEX){
        OS_EventTO(pevent);//还没完成
        OS_EXIT_CRITICAL();
        *err = OS_TIMEOUT;
        return;
    }
    OSTCBCur->OSTCBEventPtr = (OS_EVENT *)0;
    OS_EXIT_CRITICAL();
    *err = OS_NO_ERR;
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 释放互斥量 (OSMutexPost)
*
* 描述:
* 释放互斥量，并恢复可能的优先级提升。
*
* 功能:
* 1. 确认当前任务是持有者。
* 2. 如果任务优先级被提升过 (当前优先级 == PIP)，将其恢复为原始优先级。
* 3. 检查是否有任务等待：
* a. 如果有，唤醒最高优先级等待者，将所有权转交给它。
* 4. 如果无等待，标记互斥量可用。
*
* 移植要点:
* - 恢复优先级时需要重新计算在就绪表中的位置。
*********************************************************************************************************
*/
INT8U  OSMutexPost (OS_EVENT *pevent)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 6: 查询互斥量 (OSMutexQuery)
*
* 描述:
* 获取互斥量状态，包括 PIP 和当前持有者。
*
* 功能:
* 1. 提取 PIP 和 OwnerPrio。
* 2. 填充 OS_MUTEX_DATA。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSMutexQuery (OS_EVENT *pevent, OS_MUTEX_DATA *p_data)
{
    // 在此输入代码
}

