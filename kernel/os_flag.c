/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* EVENT FLAG MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_FLAG.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

static  void     OS_FlagBlock(OS_FLAG_GRP *pgrp, OS_FLAG_NODE *pnode, OS_FLAGS flags, INT8U wait_type, INT16U timeout);
static  BOOLEAN  OS_FlagTaskRdy(OS_FLAG_NODE *pnode, OS_FLAGS flags_rdy);

/*
*********************************************************************************************************
* Task Description
* 任务 1: 检查事件标志 (OSFlagAccept)
*
* 描述:
* 检查标志组是否满足条件，不阻塞。
*
* 功能:
* 1. 根据 wait_type (AND/OR, SET/CLR) 检查标志位。
* 2. 如果满足条件且包含 CONSUME 标志，则清除/设置相应位。
* 3. 返回当前标志值。
*
* 移植要点:
* - OS_FLAGS 通常定义为 8位、16位或32位。在 Mini2440 上，建议定义为 32 位 (INT32U) 以最大化性能和容量。
*********************************************************************************************************
*/
OS_FLAGS  OSFlagAccept (OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT8U *err)
{
    OS_CPU_SR     cpu_sr;
    OS_FLAGS      flags_cur;
    OS_FLAGS      flags_rdy;
    BOOLEAN       consume;

    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *err = OS_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
    if (pgrp->OSFlagType != OS_EVENT_TYPE_FLAG) {          /* Validate event block type                */
        *err = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    if (wait_type & OS_FLAG_CONSUME) {                     /* See if we need to consume the flags      */
        wait_type &= ~OS_FLAG_CONSUME;
        consume    = TRUE;
    } 
    else {
        consume    = FALSE;
    }
    *err = OS_NO_ERR;                                      /* Assume NO error until proven otherwise.  */
    OS_ENTER_CRITICAL();
    switch (wait_type) {
        case OS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
             flags_rdy = pgrp->OSFlagFlags & flags;        /* Extract only the bits we want            */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we wanted      */
                 }
             } else {
                 *err  = OS_FLAG_ERR_NOT_RDY;
             }
             flags_cur = pgrp->OSFlagFlags;                /* Will return the state of the group       */
             OS_EXIT_CRITICAL();
             break;

        case OS_FLAG_WAIT_SET_ANY:
             flags_rdy = pgrp->OSFlagFlags & flags;        /* Extract only the bits we want            */
             if (flags_rdy != (OS_FLAGS)0) {               /* See if any flag set                      */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we got         */
                 }
             } else {
                 *err  = OS_FLAG_ERR_NOT_RDY;
             }
             flags_cur = pgrp->OSFlagFlags;                /* Will return the state of the group       */
             OS_EXIT_CRITICAL();
             break;

        case OS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
             flags_rdy = ~pgrp->OSFlagFlags & flags;       /* Extract only the bits we want            */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags |= flags_rdy;       /* Set ONLY the flags that we wanted        */
                 }
             } else {
                 *err  = OS_FLAG_ERR_NOT_RDY;
             }
             flags_cur = pgrp->OSFlagFlags;                /* Will return the state of the group       */
             OS_EXIT_CRITICAL();
             break;

        case OS_FLAG_WAIT_CLR_ANY:
             flags_rdy = ~pgrp->OSFlagFlags & flags;       /* Extract only the bits we want            */
             if (flags_rdy != (OS_FLAGS)0) {               /* See if any flag cleared                  */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags |= flags_rdy;       /* Set ONLY the flags that we got           */
                 }
             } else {
                 *err  = OS_FLAG_ERR_NOT_RDY;
             }
             flags_cur = pgrp->OSFlagFlags;                /* Will return the state of the group       */
             OS_EXIT_CRITICAL();
             break;
        default:
             OS_EXIT_CRITICAL();
             flags_cur = (OS_FLAGS)0;
             *err      = OS_FLAG_ERR_WAIT_TYPE;
             break;
    }
    return (flags_cur);
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建事件标志组 (OSFlagCreate)
*
* 描述:
* 创建并初始化一个事件标志组。
*
* 功能:
* 1. 从 OSFlagFreeList 申请一个节点。
* 2. 初始化标志值。
* 3. 初始化等待列表 (OSFlagWaitList)。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_FLAG_GRP  *OSFlagCreate (OS_FLAGS flags, INT8U *err)
{
    // 在此输入代码
    OS_CPU_SR    cpu_sr;
    OS_FLAG_GRP  *pflag;

    if(OSIntNesting > 0){
        *err = OS_ERR_CREATE_ISR;
        return ((OS_FLAG_GRP *)0);
    }
    OS_ENTER_CRITICAL();
    pflag = OSFlagFreeList;
    if(pflag != (OS_FLAG_GRP *)0){
        OSFlagFreeList = (OS_FLAG_GRP *)OSFlagFreeList->OSFlagWaitList;
        OS_EXIT_CRITICAL();
        pflag->OSFlagType = OS_EVENT_TYPE_FLAG;
        pflag->OSFlagFlags = flags;
        pflag->OSFlagWaitList = (void *)0;
        *err = OS_NO_ERR;
    }
    else{
        OS_EXIT_CRITICAL();
        *err =OS_FLAG_GRP_DEPLETED;
    }
    return(pflag);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 删除事件标志组 (OSFlagDel)
*
* 描述:
* 删除标志组。
*
* 功能:
* 1. 唤醒所有等待该组的任务。
* 2. 回收节点到空闲列表。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_FLAG_GRP  *OSFlagDel (OS_FLAG_GRP *pgrp, INT8U opt, INT8U *err)
{
    OS_CPU_SR     cpu_sr;
    BOOLEAN       tasks_waiting;
    OS_FLAG_NODE *pnode;


    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *err = OS_ERR_DEL_ISR;                             /* ... can't DELETE from an ISR             */
        return (pgrp);
    }
    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *err = OS_FLAG_INVALID_PGRP;
        return (pgrp);
    }
    if (pgrp->OSFlagType != OS_EVENT_TYPE_FLAG) {          /* Validate event group type                */
        *err = OS_ERR_EVENT_TYPE;
        return (pgrp);
    }
    OS_ENTER_CRITICAL();
    if (pgrp->OSFlagWaitList != (void *)0) {               /* See if any tasks waiting on event flags  */
        tasks_waiting = TRUE;                              /* Yes                                      */
    } else {
        tasks_waiting = FALSE;                             /* No                                       */
    }
    switch (opt) {
        case OS_DEL_NO_PEND:                               /* Delete group if no task waiting          */
             if (tasks_waiting == FALSE) {
                 pgrp->OSFlagType     = OS_EVENT_TYPE_UNUSED;
                 pgrp->OSFlagWaitList = (void *)OSFlagFreeList; /* Return group to free list           */
                 OSFlagFreeList       = pgrp;
                 OS_EXIT_CRITICAL();
                 *err                 = OS_NO_ERR;
                 return ((OS_FLAG_GRP *)0);                /* Event Flag Group has been deleted        */
             } else {
                 OS_EXIT_CRITICAL();
                 *err                 = OS_ERR_TASK_WAITING;
                 return (pgrp);
             }

        case OS_DEL_ALWAYS:                                /* Always delete the event flag group       */
             pnode = (OS_FLAG_NODE *)pgrp->OSFlagWaitList;
             while (pnode != (OS_FLAG_NODE *)0) {          /* Ready ALL tasks waiting for flags        */
                 OS_FlagTaskRdy(pnode, (OS_FLAGS)0);
                 pnode = (OS_FLAG_NODE *)pnode->OSFlagNodeNext;
             }
             pgrp->OSFlagType     = OS_EVENT_TYPE_UNUSED;
             pgrp->OSFlagWaitList = (void *)OSFlagFreeList;/* Return group to free list                */
             OSFlagFreeList       = pgrp;
             OS_EXIT_CRITICAL();
             if (tasks_waiting == TRUE) {                  /* Reschedule only if task(s) were waiting  */
                 OS_Sched();                               /* Find highest priority task ready to run  */
             }
             *err = OS_NO_ERR;
             return ((OS_FLAG_GRP *)0);                    /* Event Flag Group has been deleted        */

        default:
             OS_EXIT_CRITICAL();
             *err = OS_ERR_INVALID_OPT;
             return (pgrp);
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 等待事件标志 (OSFlagPend)
*
* 描述:
* 等待特定的标志位组合。
*
* 功能:
* 1. 检查当前标志是否满足 wait_type 条件。
* 2. 如果满足，消耗标志（如果需要）并返回。
* 3. 如果不满足，创建 OS_FLAG_NODE 节点，将其挂入标志组的等待链表。
* 4. 挂起任务并调度。
*
* 移植要点:
* - OS_FLAG_NODE 是临时分配在任务堆栈上的局部变量吗？不，通常它是一个结构体，这里 uC/OS-II 的实现比较特殊，
* 它使用 `OS_FLAG_NODE` 结构，任务被挂起时，这个节点被链接到标志组的链表中。
*********************************************************************************************************
*/
OS_FLAGS  OSFlagPend (OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT16U timeout, INT8U *err)
{
    OS_CPU_SR     cpu_sr;
    OS_FLAG_NODE  node;
    OS_FLAGS      flags_cur;
    OS_FLAGS      flags_rdy;
    BOOLEAN       consume;

    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *err = OS_ERR_PEND_ISR;                            /* ... can't PEND from an ISR               */
        return ((OS_FLAGS)0);
    }

    if (pgrp == (OS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *err = OS_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
    if (pgrp->OSFlagType != OS_EVENT_TYPE_FLAG) {          /* Validate event block type                */
        *err = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    if (wait_type & OS_FLAG_CONSUME) {                     /* See if we need to consume the flags      */
        wait_type &= ~OS_FLAG_CONSUME;
        consume    = TRUE;
    } else {
        consume    = FALSE;
    }
    OS_ENTER_CRITICAL();
    switch (wait_type) {
        case OS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
             flags_rdy = pgrp->OSFlagFlags & flags;        /* Extract only the bits we want            */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we wanted      */
                 }
                 flags_cur = pgrp->OSFlagFlags;            /* Will return the state of the group       */
                 OS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *err      = OS_NO_ERR;
                 return (flags_cur);
             } else {                                      /* Block task until events occur or timeout */
                 OS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 OS_EXIT_CRITICAL();
             }
             break;

        case OS_FLAG_WAIT_SET_ANY:
             flags_rdy = pgrp->OSFlagFlags & flags;        /* Extract only the bits we want            */
             if (flags_rdy != (OS_FLAGS)0) {               /* See if any flag set                      */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we got         */
                 }
                 flags_cur = pgrp->OSFlagFlags;            /* Will return the state of the group       */
                 OS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *err      = OS_NO_ERR;
                 return (flags_cur);
             } else {                                      /* Block task until events occur or timeout */
                 OS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 OS_EXIT_CRITICAL();
             }
             break;

        case OS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
             flags_rdy = ~pgrp->OSFlagFlags & flags;       /* Extract only the bits we want            */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags |= flags_rdy;       /* Set ONLY the flags that we wanted        */
                 }
                 flags_cur = pgrp->OSFlagFlags;            /* Will return the state of the group       */
                 OS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *err      = OS_NO_ERR;
                 return (flags_cur);
             } else {                                      /* Block task until events occur or timeout */
                 OS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 OS_EXIT_CRITICAL();
             }
             break;

        case OS_FLAG_WAIT_CLR_ANY:
             flags_rdy = ~pgrp->OSFlagFlags & flags;       /* Extract only the bits we want            */
             if (flags_rdy != (OS_FLAGS)0) {               /* See if any flag cleared                  */
                 if (consume == TRUE) {                    /* See if we need to consume the flags      */
                     pgrp->OSFlagFlags |= flags_rdy;       /* Set ONLY the flags that we got           */
                 }
                 flags_cur = pgrp->OSFlagFlags;            /* Will return the state of the group       */
                 OS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *err      = OS_NO_ERR;
                 return (flags_cur);
             } else {                                      /* Block task until events occur or timeout */
                 OS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 OS_EXIT_CRITICAL();
             }
             break;

        default:
             OS_EXIT_CRITICAL();
             flags_cur = (OS_FLAGS)0;
             *err      = OS_FLAG_ERR_WAIT_TYPE;
             return (flags_cur);
    }
    OS_Sched();                                            /* Find next HPT ready to run               */
    OS_ENTER_CRITICAL();
    if (OSTCBCur->OSTCBStat & OS_STAT_FLAG) {              /* Have we timed-out?                       */
        OS_FlagUnlink(&node);
        OSTCBCur->OSTCBStat = OS_STAT_RDY;                 /* Yes, make task ready-to-run              */
        OS_EXIT_CRITICAL();
        flags_cur           = (OS_FLAGS)0;
        *err                = OS_TIMEOUT;                  /* Indicate that we timed-out waiting       */
    } 
    else {
        if (consume == TRUE) {                             /* See if we need to consume the flags      */
            switch (wait_type) {
                case OS_FLAG_WAIT_SET_ALL:
                case OS_FLAG_WAIT_SET_ANY:                 /* Clear ONLY the flags we got              */
                     pgrp->OSFlagFlags &= ~OSTCBCur->OSTCBFlagsRdy;
                     break;

                case OS_FLAG_WAIT_CLR_ALL:
                case OS_FLAG_WAIT_CLR_ANY:                 /* Set   ONLY the flags we got              */
                     pgrp->OSFlagFlags |= OSTCBCur->OSTCBFlagsRdy;
                     break;
            }
        }
        flags_cur = pgrp->OSFlagFlags;
        OS_EXIT_CRITICAL();
        *err      = OS_NO_ERR;                             /* Event(s) must have occurred              */
    }
    return (flags_cur);
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 发布事件标志 (OSFlagPost)
*
* 描述:
* 设置或清除标志位，并检查是否触发了等待任务。
*
* 功能:
* 1. 更新标志组的值 (SET 或 CLR)。
* 2. 遍历等待链表中的每个节点。
* 3. 检查每个任务的等待条件是否因本次更新而满足。
* 4. 如果满足，唤醒该任务 (OS_FlagTaskRdy)。
* 5. 如果有任务被唤醒，触发调度。
*
* 移植要点:
* - 这里的链表遍历在临界区内，如果是长链表，在 ARM9 上需注意中断延迟。
*********************************************************************************************************
*/
OS_FLAGS  OSFlagPost (OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U opt, INT8U *err)
{
    OS_CPU_SR cpu_sr;
    OS_FLAG_NODE *pnode;
    BOOLEAN sched;
    OS_FLAGS flags_cur;
    OS_FLAGS flags_rdy;

    if (pgrp == (OS_FLAG_GRP *)0) {                        
        *err = OS_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
    if (pgrp->OSFlagType != OS_EVENT_TYPE_FLAG) {   
        *err = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    switch (opt) {
        case OS_FLAG_CLR:
            pgrp->OSFlagFlags &= ~flags;            /* Clear the flags specified in the group         */
            break;

        case OS_FLAG_SET:
            pgrp->OSFlagFlags |=  flags;            /* Set   the flags specified in the group         */
            break;

        default:                                    /* INVALID option                                 */
            *err = OS_FLAG_INVALID_OPT;
            return ((OS_FLAGS)0);
    }
    OS_ENTER_CRITICAL();
    sched = FALSE;                                   /* Indicate that we don't need rescheduling       */
    pnode = (OS_FLAG_NODE *)pgrp->OSFlagWaitList;
    while (pnode != (OS_FLAG_NODE *)0) {             /* Go through all tasks waiting on event flag(s)  */
        switch (pnode->OSFlagNodeWaitType) {
            case OS_FLAG_WAIT_SET_ALL:               /* See if all req. flags are set for current node */
                flags_rdy = pgrp->OSFlagFlags & pnode->OSFlagNodeFlags;
                if (flags_rdy == pnode->OSFlagNodeFlags) {
                    if (OS_FlagTaskRdy(pnode, flags_rdy) == TRUE) { /* Make task RTR, event(s) Rx'd   */
                        sched = TRUE;                               /* When done we will reschedule   */
                    }
                }
                break;

            case OS_FLAG_WAIT_SET_ANY:               /* See if any flag set                            */
                flags_rdy = pgrp->OSFlagFlags & pnode->OSFlagNodeFlags;
                if (flags_rdy != (OS_FLAGS)0) {
                    if (OS_FlagTaskRdy(pnode, flags_rdy) == TRUE) { /* Make task RTR, event(s) Rx'd   */
                        sched = TRUE;                               /* When done we will reschedule   */
                    }
                }
                break;

            case OS_FLAG_WAIT_CLR_ALL:               /* See if all req. flags are set for current node */
                flags_rdy = ~pgrp->OSFlagFlags & pnode->OSFlagNodeFlags;
                if (flags_rdy == pnode->OSFlagNodeFlags) {
                    if (OS_FlagTaskRdy(pnode, flags_rdy) == TRUE) { /* Make task RTR, event(s) Rx'd   */
                        sched = TRUE;                               /* When done we will reschedule   */
                    }
                }
                break;

            case OS_FLAG_WAIT_CLR_ANY:               /* See if any flag set                            */
                flags_rdy = ~pgrp->OSFlagFlags & pnode->OSFlagNodeFlags;
                if (flags_rdy != (OS_FLAGS)0) {
                    if (OS_FlagTaskRdy(pnode, flags_rdy) == TRUE) { /* Make task RTR, event(s) Rx'd   */
                        sched = TRUE;                               /* When done we will reschedule   */
                    }
                }
                break;
        }
        pnode = (OS_FLAG_NODE *)pnode->OSFlagNodeNext; /* Point to next task waiting for event flag(s) */
    }
    OS_EXIT_CRITICAL();
    if (sched == TRUE) {
        OS_Sched();
    }
    OS_ENTER_CRITICAL();
    flags_cur = pgrp->OSFlagFlags;
    OS_EXIT_CRITICAL();
    *err      = OS_NO_ERR;
    return (flags_cur);
}        

/*
*********************************************************************************************************
* Task Description
* 任务 6: 查询标志组 (OSFlagQuery)
*
* 描述:
* 获取当前标志值。
*
* 功能:
* 1. 返回 OSEventFlags。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_FLAGS  OSFlagQuery (OS_FLAG_GRP *pgrp, INT8U *err)
{
    OS_CPU_SR cpu_sr;
    OS_FLAGS flags;

    if(pgrp == (OS_FLAG_GRP *)0){
        *err = OS_FLAG_INVALID_PGRP;
        return ((OS_FLAGS)0);
    }
    if(pgrp->OSFlagType != OS_EVENT_TYPE_FLAG){
        *err = OS_ERR_EVENT_TYPE;
        return ((OS_FLAGS)0);
    }
    OS_ENTER_CRITICAL();
    flags = pgrp->OSFlagFlags;
    OS_EXIT_CRITICAL();
    *err = OS_NO_ERR;
    return (flags);
}

/*
*********************************************************************************************************
* Task Description
* 任务 7: 内部函数 - 初始化 (OS_FlagInit)
*
* 描述:
* 初始化标志组空闲列表。
*
* 功能:
* 1. 初始化OSFlagTbl。
* 2. 
* 移植要点:
* - 无。
*********************************************************************************************************
*/
void  OS_FlagInit (void)
{
#if OS_MAX_FLAGS ==1
    OSFlagFreeList = (OS_FLAG_GRP *)&OSFlagTbl[0];
    OSFlagFreeList->OSFlagType = OS_EVENT_TYPE_UNUSED;
    OSFlagFreeList->OSFlagWaitList = (void *)0;
#elif OS_MAX_FLAGS >= 2
    OS_FLAG_GRP *pgrp1, *pgrp2;
    pgrp1 = &OSFlagTbl[0];
    pgrp2 = &OSFlagTbl[1];
    for(INT16U i = 0; i < (OS_MAX_FLAGS - 1); i++){
        pgrp1->OSFlagType = OS_EVENT_TYPE_UNUSED;
        pgrp1->OSFlagWaitList = (void *)pgrp2;
        pgrp1++;
        pgrp2++;
    }
    pgrp1->OSFlagWaitList = (void *)0;
    OSFlagFreeList = (OS_FLAG_GRP *)&OSFlagTbl[0];
#endif
}

/*
*********************************************************************************************************
* Task Description
* 任务 8: 内部函数 - 任务就绪 (OS_FlagTaskRdy)
*
* 描述:
* 将等待标志的任务变为就绪态。
*
* 功能:
* 1. 从标志等待列表中解链 (OS_FlagUnlink)。
* 2. 清除 TCB 状态。
* 3. 加入就绪表。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
static  BOOLEAN  OS_FlagTaskRdy (OS_FLAG_NODE *pnode, OS_FLAGS flags_rdy)
{
    OS_TCB   *ptcb;
    BOOLEAN   sched;

    ptcb                = (OS_TCB *)pnode->OSFlagNodeTCB;  /* Point to TCB of waiting task             */
    ptcb->OSTCBDly      = 0;
    ptcb->OSTCBFlagsRdy = flags_rdy;
    ptcb->OSTCBStat    &= ~OS_STAT_FLAG;
    if (ptcb->OSTCBStat == OS_STAT_RDY) {                  /* Put task into ready list                 */
        OSRdyGrp               |= ptcb->OSTCBBitY;
        OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
        sched                   = TRUE;
    } else {
        sched                   = FALSE;
    }
    OS_FlagUnlink(pnode);
    return (sched);
}

/*
*********************************************************************************************************
* Task Description
* 任务 9: 内部函数 - 阻塞任务 (OS_FlagBlock)
*
* 描述:
* 将任务加入标志组的等待列表。
*
* 功能:
* 1. 初始化 pnode 节点。
* 2. 将节点插入等待链表头部。
* 3. 挂起任务。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
static  void  OS_FlagBlock (OS_FLAG_GRP *pgrp, OS_FLAG_NODE *pnode, OS_FLAGS flags, INT8U wait_type, INT16U timeout)
{
    OS_FLAG_NODE  *pnode_next;

    OSTCBCur->OSTCBStat      |= OS_STAT_FLAG;
    OSTCBCur->OSTCBDly        = timeout;              /* Store timeout in task's TCB                   */
    OSTCBCur->OSTCBFlagNode   = pnode;                /* TCB to link to node                           */
    pnode->OSFlagNodeFlags    = flags;                /* Save the flags that we need to wait for       */
    pnode->OSFlagNodeWaitType = wait_type;            /* Save the type of wait we are doing            */
    pnode->OSFlagNodeTCB      = (void *)OSTCBCur;     /* Link to task's TCB                            */
    pnode->OSFlagNodeNext     = pgrp->OSFlagWaitList; /* Add node at beginning of event flag wait list */
    pnode->OSFlagNodePrev     = (void *)0;
    pnode->OSFlagNodeFlagGrp  = (void *)pgrp;         /* Link to Event Flag Group                      */
    pnode_next                = (OS_FLAG_NODE *)pgrp->OSFlagWaitList;
    if (pnode_next != (void *)0) {                    /* Is this the first NODE to insert?             */
        pnode_next->OSFlagNodePrev = pnode;           /* No, link in doubly linked list                */
    }
    pgrp->OSFlagWaitList = (void *)pnode;
                                                      /* Suspend current task until flag(s) received   */
    if ((OSRdyTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0) {
        OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 10: 内部函数 - 解链节点 (OS_FlagUnlink)
*
* 描述:
* 从双向链表中移除节点。
*
* 功能:
* 1. 修改 Prev 和 Next 指针。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
void  OS_FlagUnlink (OS_FLAG_NODE *pnode)
{
    OS_TCB *ptcb;
    OS_FLAG_GRP *pflag;
    OS_FLAG_NODE *pnode_prev;
    OS_FLAG_NODE *pnode_next;

    pnode_prev = (OS_FLAG_NODE *)pnode->OSFlagNodePrev;
    pnode_next = (OS_FLAG_NODE *)pnode->OSFlagNodeNext;
    if(pnode_prev == (OS_FLAG_NODE *)0){
        pflag = (OS_FLAG_GRP *)pnode->OSFlagNodeFlagGrp;
        pflag->OSFlagWaitList = (void *)pnode_next;
        if(pnode_next != (OS_FLAG_NODE *)0){
            pnode_next->OSFlagNodePrev = (OS_FLAG_NODE *)0;
        }
    }
    else{
        pnode_prev->OSFlagNodeNext = pnode_next;
        if(pnode_next != (OS_FLAG_NODE *)0){
            pnode_next->OSFlagNodePrev = pnode_prev;
        }
    }
    ptcb = (OS_TCB *)pnode->OSFlagNodeTCB;
    ptcb->OSTCBFlagNode = (OS_FLAG_NODE *)0;
}

