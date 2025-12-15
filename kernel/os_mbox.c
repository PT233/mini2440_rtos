/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* MESSAGE MAILBOX MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_MBOX.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

void OS_EventWaitListInit(OS_EVENT *pevent);

/*
*********************************************************************************************************
* Task Description
* 任务 1: 无等待请求邮箱 (OSMboxAccept)
*
* 描述:
* 检查邮箱中是否有消息。如果有，取出并清空邮箱，立即返回；如果没有，也立即返回，不阻塞任务。
*
* 功能:
* 1. 验证事件类型是否为邮箱。
* 2. 进入临界区。
* 3. 读取 OSEventPtr 中的消息指针。
* 4. 如果非空，将 OSEventPtr 置为 NULL（清空邮箱）。
* 5. 退出临界区并返回消息指针。
*
* 移植要点:
* - 指针操作在 Mini2440 (32位 ARM) 上是原子的，但为了逻辑完整性仍需临界区保护。
*********************************************************************************************************
*/
void  *OSMboxAccept (OS_EVENT *pevent)
{
    OS_CPU_SR cpu_sr;
    void *msg;

    if(pevent == (OS_EVENT *)0){
        return ((void *)0);
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_MBOX){
        return ((void *)0);
    }
    OS_ENTER_CRITICAL();
    msg = pevent->OSEventPtr;
    pevent->OSEventPtr = (void *)0;
    OS_EXIT_CRITICAL();
    return msg;
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建邮箱 (OSMboxCreate)
*
* 描述:
* 创建一个消息邮箱，并可选择初始化一条消息。
*
* 功能:
* 1. 申请一个空闲的事件控制块 (ECB)。
* 2. 设置类型为 OS_EVENT_TYPE_MBOX。
* 3. 初始化 OSEventCnt 和 OSEventPtr (指向初始消息)。
* 4. 初始化等待列表。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSMboxCreate (void *msg)
{
    OS_CPU_SR cpu_sr;
    OS_EVENT *pevent;

    if(OSIntNesting > 0){
        return((OS_EVENT *)0);
    }
    OS_ENTER_CRITICAL();
    pevent = OSEventFreeList;
    if(OSEventFreeList != (OS_EVENT *)0){
        OSEventFreeList = (OS_EVENT *)OSEventFreeList->OSEventPtr;
    }
    OS_EXIT_CRITICAL();
    if(pevent != (OS_EVENT *)0){
        pevent->OSEventType = OS_EVENT_TYPE_MBOX;
        pevent->OSEventCnt = 0;    // 计数器没用，清零！
        pevent->OSEventPtr = msg;  // 把信件的地址存入指针槽
        OS_EventWaitListInit(pevent);
    }
    return(pevent);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 删除邮箱 (OSMboxDel)
*
* 描述:
* 删除一个邮箱，处理等待该邮箱的任务。
*
* 功能:
* 1. 检查是否有任务在等待。
* 2. 如果 opt == OS_DEL_NO_PEND 且有任务等待，则返回错误。
* 3. 如果 opt == OS_DEL_ALWAYS，唤醒所有等待任务 (OS_EventTaskRdy)，并回收 ECB。
* 4. 触发调度。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSMboxDel (OS_EVENT *pevent, INT8U opt, INT8U *err)
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
    if (pevent->OSEventType != OS_EVENT_TYPE_MBOX) {       /* Validate event block type                */
        *err = OS_ERR_EVENT_TYPE;
        return (pevent);
    }
    OS_ENTER_CRITICAL();
    if (pevent->OSEventGrp != 0x00) {                      /* See if any tasks waiting on mailbox      */
        tasks_waiting = TRUE;                              /* Yes                                      */
    } else {
        tasks_waiting = FALSE;                             /* No                                       */
    }
    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete mailbox only if no task waiting   */
             if (tasks_waiting == FALSE) {
                 pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
                 pevent->OSEventPtr  = OSEventFreeList;    /* Return Event Control Block to free list  */
                 OSEventFreeList     = pevent;             /* Get next free event control block        */
                 OS_EXIT_CRITICAL();
                 *err = OS_NO_ERR;
                 return ((OS_EVENT *)0);                   /* Mailbox has been deleted                 */
             } else {
                 OS_EXIT_CRITICAL();
                 *err = OS_ERR_TASK_WAITING;
                 return (pevent);
             }

        case OS_DEL_ALWAYS:                                /* Always delete the mailbox                */
             while (pevent->OSEventGrp != 0x00) {          /* Ready ALL tasks waiting for mailbox      */
                 OS_EventTaskRdy(pevent, (void *)0, OS_STAT_MBOX);
             }
             pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
             pevent->OSEventPtr  = OSEventFreeList;        /* Return Event Control Block to free list  */
             OSEventFreeList     = pevent;                 /* Get next free event control block        */
             OS_EXIT_CRITICAL();
             if (tasks_waiting == TRUE) {                  /* Reschedule only if task(s) were waiting  */
                 OS_Sched();                               /* Find highest priority task ready to run  */
             }
             *err = OS_NO_ERR;
             return ((OS_EVENT *)0);                       /* Mailbox has been deleted                 */

        default:
             OS_EXIT_CRITICAL();
             *err = OS_ERR_INVALID_OPT;
             return (pevent);
    }
}
/*
*********************************************************************************************************
* Task Description
* 任务 4: 等待邮箱 (OSMboxPend)
*
* 描述:
* 等待邮箱中的消息。如果邮箱为空，任务将被阻塞。
*
* 功能:
* 1. 检查邮箱 (OSEventPtr) 是否有消息。
* 2. 如果有，取出消息，清空邮箱，返回消息。
* 3. 如果没有，将任务加入等待列表，设置超时。
* 4. 触发调度。
* 5. 任务恢复后，检查是收到消息还是超时。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
void  *OSMboxPend (OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
    OS_CPU_SR  cpu_sr;
    void      *msg;

    if (OSIntNesting > 0) {                           /* See if called from ISR ...                    */
        *err = OS_ERR_PEND_ISR;                       /* ... can't PEND from an ISR                    */
        return ((void *)0);
    }
    if (pevent == (OS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *err = OS_ERR_PEVENT_NULL;
        return ((void *)0);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MBOX) {  /* Validate event block type                     */
        *err = OS_ERR_EVENT_TYPE;
        return ((void *)0);
    }
    OS_ENTER_CRITICAL();
    msg = pevent->OSEventPtr;
    if (msg != (void *)0) {                           /* See if there is already a message             */
        pevent->OSEventPtr = (void *)0;               /* Clear the mailbox                             */
        OS_EXIT_CRITICAL();
        *err = OS_NO_ERR;
        return (msg);                                 /* Return the message received (or NULL)         */
    }
    OSTCBCur->OSTCBStat |= OS_STAT_MBOX;              /* Message not available, task will pend         */
    OSTCBCur->OSTCBDly   = timeout;                   /* Load timeout in TCB                           */
    OS_EventTaskWait(pevent);                         /* Suspend task until event or timeout occurs    */
    OS_EXIT_CRITICAL();
    OS_Sched();                                       /* Find next highest priority task ready to run  */
    OS_ENTER_CRITICAL();
    msg = OSTCBCur->OSTCBMsg;
    if (msg != (void *)0) {                           /* See if we were given the message              */
        OSTCBCur->OSTCBMsg      = (void *)0;          /* Yes, clear message received                   */
        OSTCBCur->OSTCBStat     = OS_STAT_RDY;
        OSTCBCur->OSTCBEventPtr = (OS_EVENT *)0;      /* No longer waiting for event                   */
        OS_EXIT_CRITICAL();
        *err                    = OS_NO_ERR;
        return (msg);                                 /* Return the message received                   */
    }
    OS_EventTO(pevent);                               /* Timed out, Make task ready                    */
    OS_EXIT_CRITICAL();
    *err = OS_TIMEOUT;                                /* Indicate that a timeout occured               */
    return ((void *)0);                               /* Return a NULL message                         */
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 发送消息到邮箱 (OSMboxPost)
*
* 描述:
* 向邮箱发送一条消息。如果邮箱已满（非空），则返回错误。
*
* 功能:
* 1. 检查是否有任务在等待该邮箱。
* 2. 如果有，将消息直接交给最高优先级的等待任务，唤醒它，并触发调度。
* 3. 如果没有任务等待，检查邮箱是否已满。
* 4. 如果未满，将消息存入 OSEventPtr。
*
* 移植要点:
* - 在 Mini2440 上，指针赋值是 32 位操作。
*********************************************************************************************************
*/
INT8U  OSMboxPost (OS_EVENT *pevent, void *msg)
{
    // 在此输入代码
    OS_CPU_SR cpu_sr;
    if(pevent == (OS_EVENT *)0){
        return (OS_ERR_PEVENT_NULL);
    }
    if(msg == (void *)0){
        return (OS_ERR_POST_NULL_PTR);
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_MBOX){
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    if(pevent->OSEventGrp != 0x00){
        OS_EventTaskRdy(pevent, msg, OS_STAT_MBOX);
        OS_EXIT_CRITICAL();
        OS_Sched();
        return (OS_NO_ERR);
    }
    if(pevent->OSEventPtr != (void *)0){
        OS_EXIT_CRITICAL();
        return (OS_MBOX_FULL);
    }
    pevent->OSEventPtr = msg;
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}

/*
*********************************************************************************************************
* Task Description
* 任务 6: 发送消息到邮箱（扩展选项） (OSMboxPostOpt)
*
* 描述:
* 向邮箱发送消息，支持广播（唤醒所有等待任务）。
*
* 功能:
* 1. 如果 opt 包含 OS_POST_OPT_BROADCAST：
* a. 遍历等待列表，唤醒所有任务 (OS_EventTaskRdy)。
* b. 触发调度。
* 2. 否则，行为同 OSMboxPost。
*
* 移植要点:
* - 广播操作在循环唤醒所有任务时会一直处于临界区 (关中断)。如果等待任务很多，在 ARM9 上可能会增加中断延迟，需评估系统实时性要求。
*********************************************************************************************************
*/
INT8U  OSMboxPostOpt (OS_EVENT *pevent, void *msg, INT8U opt)
{
    //不实现
}

/*
*********************************************************************************************************
* Task Description
* 任务 7: 查询邮箱状态 (OSMboxQuery)
*
* 描述:
* 获取邮箱的当前状态（是否有消息，哪些任务在等待）。
*
* 功能:
* 1. 复制 ECB 信息到用户提供的 OS_MBOX_DATA 结构体。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSMboxQuery (OS_EVENT *pevent, OS_MBOX_DATA *pdata)
{
    OS_CPU_SR  cpu_sr;  
    INT8U     *psrc;
    INT8U     *pdest;

    if (pevent == (OS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (OS_ERR_PEVENT_NULL);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MBOX) {       /* Validate event block type                */
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
    pdata->OSMsg = pevent->OSEventPtr;                     /* Get message from mailbox                 */
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}

