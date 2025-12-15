/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* SEMAPHORE MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_SEM.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
* Task Description
* 任务 1: 无等待请求信号量 (OSSemAccept)
*
* 描述:
* 检查信号量是否可用，如果不可用立即返回，不阻塞。
*
* 功能:
* 1. 检查事件类型。
* 2. 如果计数器 > 0，递减计数器并返回当前计数值。
* 3. 否则返回 0。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT16U  OSSemAccept (OS_EVENT *pevent)
{
    OS_CPU_SR cpu_sr;
    INT16U cnt;
    if(pevent == (OS_EVENT *)0){
        return 0;
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_SEM){
        return 0;
    }
    OS_ENTER_CRITICAL();
    cnt = pevent->OSEventCnt;
    if(cnt > 0){
        pevent->OSEventCnt--;
    }
    OS_EXIT_CRITICAL();
    return cnt;
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建信号量 (OSSemCreate)
*
* 描述:
* 创建并初始化一个信号量。
*
* 功能:
* 1. 从空闲 ECB 列表获取一个事件控制块。
* 2. 设置类型为 OS_EVENT_TYPE_SEM。
* 3. 设置初始计数值。
* 4. 初始化等待列表。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSSemCreate (INT16U cnt)
{
    OS_CPU_SR cpu_sr;
    OS_EVENT  *pevent;
    //如果在中断中无法关中断
    if(OSIntNesting > 0){
        return((OS_EVENT *)0);
    }
    OS_ENTER_CRITICAL();
    pevent = OSEventFreeList;                                     //把 OSEventFreeList 指向的地址（节点A）保存到pevent中
    if(OSEventFreeList != (OS_EVENT *)0){
        OSEventFreeList = (OS_EVENT *)OSEventFreeList->OSEventPtr;//把 OSEventFreeList 更新为节点A指向的下一个节点（节点B）
    }
    OS_EXIT_CRITICAL();
    if(pevent != (OS_EVENT *)0){
        pevent->OSEventType = OS_EVENT_TYPE_SEM;
        pevent->OSEventCnt = cnt;       // 存入计数值（比如 5，表示有5个资源）
        pevent->OSEventPtr = (void *)0; // 指针没用，必须清空！
        OS_EventWaitListInit(pevent);//pevent->OSEventGrp\pevent->OSEventTbl在这里面初始化
    }
    return (pevent);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 删除信号量 (OSSemDel)
*
* 描述:
* 删除一个信号量。
*
* 功能:
* 1. 检查是否有任务在等待。
* 2. 根据 opt 选项（OS_DEL_NO_PEND 或 OS_DEL_ALWAYS）决定是否删除。
* 3. 如果删除，唤醒所有等待任务，并回收 ECB。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSSemDel (OS_EVENT *pevent, INT8U opt, INT8U *err)
{
    OS_CPU_SR  cpu_sr;  
    BOOLEAN    tasks_waiting;


    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *err = OS_ERR_DEL_ISR;                             /* ... can't DELETE from an ISR             */
        return (pevent);
    }

    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *err = OS_ERR_PEVENT_NULL;
        return (pevent);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {        /* Validate event block type                */
        *err = OS_ERR_EVENT_TYPE;
        return (pevent);
    }

    OS_ENTER_CRITICAL();
    if (pevent->OSEventGrp != 0x00) {                      /* See if any tasks waiting on semaphore    */
        tasks_waiting = TRUE;                              /* Yes                                      */
    } else {
        tasks_waiting = FALSE;                             /* No                                       */
    }
    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete semaphore only if no task waiting */
             if (tasks_waiting == FALSE) {
                 pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
                 pevent->OSEventPtr  = OSEventFreeList;    /* Return Event Control Block to free list  */
                 OSEventFreeList     = pevent;             /* Get next free event control block        */
                 OS_EXIT_CRITICAL();
                 *err = OS_NO_ERR;
                 return ((OS_EVENT *)0);                   /* Semaphore has been deleted               */
             } else {
                 OS_EXIT_CRITICAL();
                 *err = OS_ERR_TASK_WAITING;
                 return (pevent);
             }

        case OS_DEL_ALWAYS:                                /* Always delete the semaphore              */
             while (pevent->OSEventGrp != 0x00) {          /* Ready ALL tasks waiting for semaphore    */
                 OS_EventTaskRdy(pevent, (void *)0, OS_STAT_SEM);
             }
             pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
             pevent->OSEventPtr  = OSEventFreeList;        /* Return Event Control Block to free list  */
             OSEventFreeList     = pevent;                 /* Get next free event control block        */
             OS_EXIT_CRITICAL();
             if (tasks_waiting == TRUE) {                  /* Reschedule only if task(s) were waiting  */
                 OS_Sched();                               /* Find highest priority task ready to run  */
             }
             *err = OS_NO_ERR;
             return ((OS_EVENT *)0);                       /* Semaphore has been deleted               */

        default:
             OS_EXIT_CRITICAL();
             *err = OS_ERR_INVALID_OPT;
             return (pevent);
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 等待信号量 (OSSemPend)
*
* 描述:
* 请求信号量，如果不可用则阻塞任务。
*
* 功能:
* 1. 如果计数器 > 0，递减并返回。
* 2. 否则，设置 TCB 状态为等待信号量。
* 3. 设置超时时间。
* 4. 调用 OS_EventTaskWait 将任务加入等待列表。
* 5. 触发调度。
* 6. 检查返回原因（获得信号量还是超时）。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
void  OSSemPend (OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
    OS_CPU_SR  cpu_sr;  
    if (OSIntNesting > 0) {                           /* See if called from ISR ...                    */
        *err = OS_ERR_PEND_ISR;                       /* ... can't PEND from an ISR                    */
        return;
    }
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *err = OS_ERR_PEVENT_NULL;
        return;
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {   /* Validate event block type                     */
        *err = OS_ERR_EVENT_TYPE;
        return;
    }
    OS_ENTER_CRITICAL();
    if (pevent->OSEventCnt > 0) {                     /* If sem. is positive, resource available ...   */
        pevent->OSEventCnt--;                         /* ... decrement semaphore only if positive.     */
        OS_EXIT_CRITICAL();
        *err = OS_NO_ERR;
        return;
    }
                                                      /* Otherwise, must wait until event occurs       */
    OSTCBCur->OSTCBStat |= OS_STAT_SEM;               /* Resource not available, pend on semaphore     */
    OSTCBCur->OSTCBDly   = timeout;                   /* Store pend timeout in TCB                     */
    OS_EventTaskWait(pevent);                         /* Suspend task until event or timeout occurs    */
    OS_EXIT_CRITICAL();
    OS_Sched();                                       /* Find next highest priority task ready         */
    OS_ENTER_CRITICAL();
    if (OSTCBCur->OSTCBStat & OS_STAT_SEM) {          /* Must have timed out if still waiting for event*/
        OS_EventTO(pevent);
        OS_EXIT_CRITICAL();
        *err = OS_TIMEOUT;                            /* Indicate that didn't get event within TO      */
        return;
    }
    OSTCBCur->OSTCBEventPtr = (OS_EVENT *)0;
    OS_EXIT_CRITICAL();
    *err = OS_NO_ERR;
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 发送信号量 (OSSemPost)
*
* 描述:
* 释放一个信号量。
*
* 功能:
* 1. 检查是否有任务在等待。
* 2. 如果有，唤醒最高优先级的等待任务 (OS_EventTaskRdy)。
* 3. 触发调度。
* 4. 如果没有任务等待，递增计数器（防止溢出）。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSSemPost (OS_EVENT *pevent)
{
    OS_CPU_SR cpu_sr;
    if(pevent == (OS_EVENT *)0){
        return (OS_ERR_PEVENT_NULL);
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_SEM){
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    if(pevent->OSEventGrp != 0x00){
        OS_EventTaskRdy(pevent, (void *)0, OS_STAT_SEM);
        OS_EXIT_CRITICAL();
        OS_Sched();
        return (OS_NO_ERR);
    }
    if(pevent->OSEventCnt < 65535){
        pevent->OSEventCnt++;
        OS_EXIT_CRITICAL();
        return (OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_SEM_OVF);
}

/*
*********************************************************************************************************
* Task Description
* 任务 6: 查询信号量 (OSSemQuery)
*
* 描述:
* 获取信号量的当前状态信息。
*
* 功能:
* 1. 复制 ECB 中的信息到用户结构体。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSSemQuery (OS_EVENT *pevent, OS_SEM_DATA *pdata)
{
    OS_CPU_SR  cpu_sr; 
    INT8U     *psrc;
    INT8U     *pdest;

    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (OS_ERR_PEVENT_NULL);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_SEM) {        /* Validate event block type                */
        return (OS_ERR_EVENT_TYPE);
    }
    OS_ENTER_CRITICAL();
    pdata->OSEventGrp = pevent->OSEventGrp;                /* Copy message mailbox wait list           */
    psrc              = &pevent->OSEventTbl[0];
    pdest             = &pdata->OSEventTbl[0];
#if OS_EVENT_TBL_SIZE > 0
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 1
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 2
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 3
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 4
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 5
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 6
    *pdest++          = *psrc++;
#endif

#if OS_EVENT_TBL_SIZE > 7
    *pdest            = *psrc;
#endif
    pdata->OSCnt      = pevent->OSEventCnt;                /* Get semaphore count                      */
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}