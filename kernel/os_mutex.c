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
    OS_CPU_SR  cpu_sr;  
      
    if (OSIntNesting > 0) {                            // Make sure it's not called from an ISR
        *err = OS_ERR_PEND_ISR;
        return (0);
    }
    if (pevent == (OS_EVENT *)0) {                     // Validate 'pevent'
        *err = OS_ERR_PEVENT_NULL;
        return (0);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {  // Validate event block type
        *err = OS_ERR_EVENT_TYPE;
        return (0);
    }                                                    
    OS_ENTER_CRITICAL();							   // Get value (0 or 1) of Mutex
    if ((pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8) == OS_MUTEX_AVAILABLE) {     
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;   // Mask off LSByte (Acquire Mutex)
        pevent->OSEventCnt |= OSTCBCur->OSTCBPrio;     // Save current task priority in LSByte
        pevent->OSEventPtr  = (void *)OSTCBCur;        // Link TCB of task owning Mutex
        OS_EXIT_CRITICAL();
        *err = OS_NO_ERR;
        return (1);
    }
    OS_EXIT_CRITICAL();
    *err = OS_NO_ERR;
    return (0);
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
    OS_CPU_SR  cpu_sr;   
    BOOLEAN    tasks_waiting;
    INT8U      pip;


    if (OSIntNesting > 0) {                                // See if called from ISR ...
        *err = OS_ERR_DEL_ISR;                             // ... can't DELETE from an ISR
        return (pevent);
    }
    if (pevent == (OS_EVENT *)0) {                         // Validate 'pevent'
        *err = OS_ERR_PEVENT_NULL;
        return ((OS_EVENT *)0);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {      // Validate event block type
        *err = OS_ERR_EVENT_TYPE;
        return (pevent);
    }
    OS_ENTER_CRITICAL();
    if (pevent->OSEventGrp != 0x00) {                      // See if any tasks waiting on mutex
        tasks_waiting = TRUE;                              // Yes
    } else {
        tasks_waiting = FALSE;                             // No
    }
    switch (opt) {
        case OS_DEL_NO_PEND:                               // Delete mutex only if no task waiting
             if (tasks_waiting == FALSE) {
                 pip                 = (INT8U)(pevent->OSEventCnt >> 8);
                 OSTCBPrioTbl[pip]   = (OS_TCB *)0;        // Free up the PIP
                 pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
                 pevent->OSEventPtr  = OSEventFreeList;    // Return Event Control Block to free list
                 OSEventFreeList     = pevent;
                 OS_EXIT_CRITICAL();
                 *err = OS_NO_ERR;
                 return ((OS_EVENT *)0);                   // Mutex has been deleted
             } else {
                 OS_EXIT_CRITICAL();
                 *err = OS_ERR_TASK_WAITING;
                 return (pevent);
             }

        case OS_DEL_ALWAYS:                                // Always delete the mutex
             while (pevent->OSEventGrp != 0x00) {          // Ready ALL tasks waiting for mutex
                 OS_EventTaskRdy(pevent, (void *)0, OS_STAT_MUTEX);
             }
             pip                 = (INT8U)(pevent->OSEventCnt >> 8);
             OSTCBPrioTbl[pip]   = (OS_TCB *)0;            // Free up the PIP
             pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
             pevent->OSEventPtr  = OSEventFreeList;        // Return Event Control Block to free list
             OSEventFreeList     = pevent;                 // Get next free event control block
             OS_EXIT_CRITICAL();
             if (tasks_waiting == TRUE) {                  // Reschedule only if task(s) were waiting
                 OS_Sched();                               // Find highest priority task ready to run
             }
             *err = OS_NO_ERR;
             return ((OS_EVENT *)0);                       // Mutex has been deleted

        default:
             OS_EXIT_CRITICAL();
             *err = OS_ERR_INVALID_OPT;
             return (pevent);
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 等待互斥量 (OSMutexPend)
*
* 描述:
* 请求互斥量，实现优先级反转。
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
    //解决三者堵门问题(优先级A>B>C, C占用cpu, B想抢占C, 但堵了A的门)
    //如果不做处理，A优先级10就会挂起等待。这时候，如果B优先级20突然出现，因为B比C优先级30优先级高，B就会抢占CPU。
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
    // OS_MUTEX_KEEP_LOWER_8 是 0x00FF，用来把高8位屏蔽掉，只看低8位
    if((INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8) == OS_MUTEX_AVAILABLE){
        // 情况 1：没人占用互斥锁
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;  // 保留高8位的 PIP 信息，清空低8位
        pevent->OSEventCnt |= OSTCBCur->OSTCBPrio;    // 把A的优先级写进低8位
        pevent->OSEventPtr = (void *)OSTCBCur;        // 在记录本上写下“持有者是A”
        OS_EXIT_CRITICAL();
        *err = OS_NO_ERR;
        return;
    }
    // 取出高8位：尚方宝剑优先级 (PIP = 5)
    pip = (INT8U)(pevent->OSEventCnt >> 8);
    // 取出低8位：当前持有者的优先级 (mprio = 30，即C)
    mprio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);
    // 找到持有者C的任务控制块 (TCB)
    ptcb = (OS_TCB *)(pevent->OSEventPtr);

    // 判断条件：
    // 1. ptcb->OSTCBPrio != pip：C还没被提拔过（如果已经被提拔就不浪费时间了）
    // 2. mprio > OSTCBCur->OSTCBPrio：持有者（30）比我（10）弱。
    if((ptcb->OSTCBPrio != pip) && (mprio > OSTCBCur->OSTCBPrio)){
        
        //  动作 1：把C从“旧家”赶出来 
        // 检查C是不是“就绪”状态（在 Rdy 表里）
        if(((OSRdyTbl[ptcb->OSTCBY]) & (ptcb->OSTCBBitX)) != 0x00){
            // 如果这一行只有C一个人，就把整行的标志位清零
            if(((OSRdyTbl[ptcb->OSTCBY]) & ~(ptcb->OSTCBBitX)) == 0x00){
                OSRdyGrp &= ~(ptcb->OSTCBBitY);
            }
            rdy = TRUE; // 标记：他本来是醒着的
        }
        else{
            rdy = FALSE; // 他本来可能在睡觉（比如在该别的锁），那就不用操作就绪表
        }

        //  动作 2：给C改名换姓 
        ptcb->OSTCBPrio = pip; // 现在的优先级变成 5 (PIP)！
        // 重新计算他在 8x8 任务表里的新坐标（数学逻辑）
        ptcb->OSTCBY = (ptcb->OSTCBPrio) >> 3;      // 新行号 = 5 / 8 = 0
        ptcb->OSTCBBitY = OSMapTbl[ptcb->OSTCBY];   // 查表得行掩码
        ptcb->OSTCBX = (ptcb->OSTCBPrio) & 7;       // 新列号 = 5 % 8 = 5
        ptcb->OSTCBBitX = OSMapTbl[ptcb->OSTCBX];   // 查表得列掩码

        //  动作 3：把C搬进“新豪宅” 
        if(rdy == TRUE){
            // 在就绪表的“新位置”把它置位
            OSRdyGrp |= ptcb->OSTCBBitY;
            OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX; 
        }
        // 记录一下：PIP 优先级 5 现在的代表人物是这个 ptcb
        OSTCBPrioTbl[pip] = (OS_TCB *)ptcb;
    }
    OSTCBCur->OSTCBStat |= OS_STAT_MUTEX; // 设置状态：A在等互斥量
    OSTCBCur->OSTCBDly = timeout;         // 设置最长等多久
    OS_EventTaskWait(pevent);             // 把A从就绪表里移除，放到这个互斥量的等待队列里
    OS_EXIT_CRITICAL();
    OS_Sched(); // 关键发起调度：A，优先级 10睡着了。B，优先级 20是醒着的。C，现优先级5是醒着的。
    OS_ENTER_CRITICAL();
    // 检查：如果A醒来时，状态里还标着“我在等锁”，说明A是被迫醒的（超时），而不是拿到了锁
    if(OSTCBCur->OSTCBStat & OS_STAT_MUTEX){
        OS_EventTO(pevent); // 处理超时逻辑（把任务从等待队列移除）
        OS_EXIT_CRITICAL();
        *err = OS_TIMEOUT;  // 告诉调用者A：超时了
        return;
    }
    
    // 如果走到这里，说明A正常拿到了锁
    OSTCBCur->OSTCBEventPtr = (OS_EVENT *)0;
    OS_EXIT_CRITICAL();
    *err = OS_NO_ERR; // 成功！
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
INT8U  OSMutexPost (OS_EVENT *pevent)//考虑加上err
{
    OS_CPU_SR cpu_sr;
    INT8U     pip;
    INT8U     prio;

    if(OSIntNesting > 0){
        return (OS_ERR_POST_ISR);
    }
    if(pevent == (OS_EVENT *)0){
        return (OS_ERR_PEVENT_NULL);
    }
    if(pevent->OSEventType != OS_EVENT_TYPE_MUTEX){
        return (OS_ERR_EVENT_TYPE);
    }

    OS_ENTER_CRITICAL();
    pip = (INT8U)(pevent->OSEventCnt >> 8);
    prio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);
    if(OSTCBCur->OSTCBPrio != pip && OSTCBCur->OSTCBPrio != prio){
        OS_EXIT_CRITICAL();
        return (OS_ERR_NOT_MUTEX_OWNER);
    }
    if(OSTCBCur->OSTCBPrio == pip){
        if((OSRdyTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0){
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }
        OSTCBCur->OSTCBPrio = prio;
        OSTCBCur->OSTCBY = prio >> 3;
        OSTCBCur->OSTCBBitY = OSMapTbl[OSTCBCur->OSTCBY];
        OSTCBCur->OSTCBX = prio & 0x07;
        OSTCBCur->OSTCBBitX = OSMapTbl[OSTCBCur->OSTCBX];
        OSRdyGrp |= OSTCBCur->OSTCBBitY;
        OSRdyTbl[OSTCBCur->OSTCBY] |= OSTCBCur->OSTCBBitX;
        OSTCBPrioTbl[prio] = (OS_TCB *)OSTCBCur;
    }
    OSTCBPrioTbl[pip] = (OS_TCB *)1;
    if(pevent->OSEventGrp != 0x00){
        prio = OS_EventTaskRdy(pevent, (void *)0, OS_STAT_MUTEX);//还没完成
        pevent->OSEventCnt &= OS_MUTEX_KEEP_UPPER_8;
        pevent->OSEventCnt |= prio;
        pevent->OSEventPtr = OSTCBPrioTbl[prio];
        OS_EXIT_CRITICAL();
        OS_Sched();
        return (OS_NO_ERR);
    }
    pevent->OSEventCnt |= OS_MUTEX_AVAILABLE;
    pevent->OSEventPtr = (void *)0;
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
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
INT8U  OSMutexQuery (OS_EVENT *pevent, OS_MUTEX_DATA *pdata)
{
    OS_CPU_SR  cpu_sr;

    if (OSIntNesting > 0) {                                // See if called from ISR ...
        return (OS_ERR_QUERY_ISR);                         // ... can't QUERY mutex from an ISR
    }
    if (pevent == (OS_EVENT *)0) {                         // Validate 'pevent'
        return (OS_ERR_PEVENT_NULL);
    }
    if (pevent->OSEventType != OS_EVENT_TYPE_MUTEX) {      // Validate event block type
        return (OS_ERR_EVENT_TYPE);
    }
    OS_ENTER_CRITICAL();
    pdata->OSMutexPIP  = (INT8U)(pevent->OSEventCnt >> 8);
    pdata->OSOwnerPrio = (INT8U)(pevent->OSEventCnt & OS_MUTEX_KEEP_LOWER_8);
    if (pdata->OSOwnerPrio == 0xFF) {
        pdata->OSValue = 1;
    } else {
        pdata->OSValue = 0;
    }
    pdata->OSEventGrp  = pevent->OSEventGrp;               // Copy wait list
    memcpy(&pdata->OSEventTbl[0], &pevent->OSEventTbl[0], sizeof(pdata->OSEventTbl));
    OS_EXIT_CRITICAL();
    return (OS_NO_ERR);
}
