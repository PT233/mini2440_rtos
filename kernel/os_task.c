/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* TASK MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_TASK.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
* Task Description
* 任务 1: 改变任务优先级 (OSTaskChangePrio)
*
* 描述:
* 动态改变一个任务的优先级。
*
* 功能:
* 1. 检查优先级有效性。
* 2. 确保新优先级未被占用。
* 3. 查找旧任务的 TCB。
* 4. 从就绪表或事件等待表中移除旧优先级。
* 5. 更新 TCB 中的优先级信息。
* 6. 将任务以新优先级加入就绪表或事件等待表。
* 7. 触发调度。
*
* 移植要点:
* - 涉及复杂的位操作和链表操作，需确保原子性。
*********************************************************************************************************
*/
INT8U  OSTaskChangePrio (INT8U oldprio, INT8U newprio)
{
    OS_CPU_SR    cpu_sr;
    OS_EVENT    *pevent;
    OS_TCB      *ptcb;
    INT8U        x;
    INT8U        y;
    INT8U        bitx;
    INT8U        bity;

    if ((oldprio >= OS_LOWEST_PRIO && oldprio != OS_PRIO_SELF)  ||
         newprio >= OS_LOWEST_PRIO) {
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    if (OSTCBPrioTbl[newprio] != (OS_TCB *)0) {           
        OS_EXIT_CRITICAL();
        return (OS_PRIO_EXIST);
    } else {
        OSTCBPrioTbl[newprio] = (OS_TCB *)1;        
        OS_EXIT_CRITICAL();
        y    = newprio >> 3;                           
        bity = OSMapTbl[y];
        x    = newprio & 0x07;
        bitx = OSMapTbl[x];
        OS_ENTER_CRITICAL();
        if (oldprio == OS_PRIO_SELF) {                
            oldprio = OSTCBCur->OSTCBPrio;      
        }
        ptcb = OSTCBPrioTbl[oldprio];
        if (ptcb != (OS_TCB *)0) {            
            OSTCBPrioTbl[oldprio] = (OS_TCB *)0;           
            if ((OSRdyTbl[ptcb->OSTCBY] & ptcb->OSTCBBitX) != 0x00) { 
                if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) {
                    OSRdyGrp &= ~ptcb->OSTCBBitY;
                }
                OSRdyGrp    |= bity;                          
                OSRdyTbl[y] |= bitx;
            } 
            else {
                pevent = ptcb->OSTCBEventPtr;
                if (pevent != (OS_EVENT *)0) {                
                    if ((pevent->OSEventTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0) {
                        pevent->OSEventGrp &= ~ptcb->OSTCBBitY;
                    }
                    pevent->OSEventGrp    |= bity;             
                    pevent->OSEventTbl[y] |= bitx;
                }
            }
            OSTCBPrioTbl[newprio] = ptcb;                    
            ptcb->OSTCBPrio       = newprio;                  
            ptcb->OSTCBY          = y;
            ptcb->OSTCBX          = x;
            ptcb->OSTCBBitY       = bity;
            ptcb->OSTCBBitX       = bitx;
            OS_EXIT_CRITICAL();
            OS_Sched();                                        
            return (OS_NO_ERR);
        } 
        else {
            OSTCBPrioTbl[newprio] = (OS_TCB *)0;                
            OS_EXIT_CRITICAL();
            return (OS_PRIO_ERR);                               
        }
    }
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建任务 (OSTaskCreate)
*
* 描述:
* 创建一个新任务，初始化其堆栈和 TCB。
*
* 功能:
* 1. 检查优先级有效性。
* 2. 确保优先级未被占用。
* 3. 调用 OSTaskStkInit() 初始化堆栈结构。
* 4. 调用 OS_TCBInit() 获取并初始化 TCB。
* 5. 触发调度。
*
* 移植要点:
* - 关键依赖 OSTaskStkInit (在 OS_CPU_C.C 中)。
* - 对于 Mini2440，传递的 ptos 应该是堆栈的高地址（因为是满递减堆栈）。
*********************************************************************************************************
*/
INT8U OSTaskCreate(void (*task)(void *pd), void *pdata1, OS_STK *ptos, INT8U prio){
    //不考虑ext的情况
    OS_CPU_SR   cpu_sr;
    OS_STK      *psp; //pointer to stack pointer
    INT8U       err;

    if(prio > OS_LOWEST_PRIO){
        return OS_PRIO_INVALID;
    }
    OS_ENTER_CRITICAL();
    if(OSTCBPrioTbl[prio] == (OS_TCB *)0){
        OSTCBPrioTbl[prio] = (OS_TCB *)1;
        OS_EXIT_CRITICAL();
        psp = (OS_STK *)OSTaskStkInit(task, pdata1, ptos, 0);
        err = OS_TCBInit(prio, psp, (OS_STK *)0,0,0,(void *)0,0);
        if(err == OS_NO_ERR){
            OS_ENTER_CRITICAL();
            OSTaskCtr++;
            OS_EXIT_CRITICAL();
            if(OSRunning == TRUE){
                OS_Sched();
            }
        }
        else{
                OS_ENTER_CRITICAL();
                OSTCBPrioTbl[prio] = (OS_TCB *)0;
                OS_EXIT_CRITICAL();
            }
        return (err);
    }
    OS_EXIT_CRITICAL();
    return (OS_PRIO_EXIST);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 创建任务扩展版 (OSTaskCreateExt)
*
* 描述:
* 创建一个新任务，允许更多选项（如堆栈检查、清除、扩展数据）。
*
* 功能:
* 1. 类似 OSTaskCreate。
* 2. 支持堆栈清除（如果设置了 OS_TASK_OPT_STK_CLR）。
* 3. 将额外信息 (id, stack size, pext) 传递给 OS_TCBInit。
*
* 移植要点:
* - 如果启用堆栈检查，需要确保堆栈底 (pbos) 和堆栈大小在 Mini2440 内存映射中是有效的。
*********************************************************************************************************
*/
INT8U  OSTaskCreateExt (void (*task)(void *pd), void *pdata1, OS_STK *ptos, INT8U prio, INT16U id, OS_STK *pbos, INT32U stk_size, void *pext, INT16U opt)
{
    //不实现
    return (OS_NO_ERR);
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 删除任务 (OSTaskDel)
*
* 描述:
* 删除指定优先级的任务。
*
* 功能:
* 1. 禁止删除空闲任务。
* 2. 将任务从就绪表、事件等待表移除。
* 3. 如果任务持有事件标志，解除链接。
* 4. 将 TCB 返回给空闲列表。
* 5. 触发调度。
*
* 移植要点:
* - 删除当前任务时，调用 OS_Dummy() 确保在开中断后、上下文切换前 CPU 有指令可执行（防止流水线问题，虽在 ARM 上较少见，但为了兼容性保留）。
*********************************************************************************************************
*/
INT8U  OSTaskDel (INT8U prio)
{
    OS_CPU_SR       cpu_sr;
    OS_EVENT        *pevent;
    OS_FLAG_NODE    *pnode;
    OS_TCB          *ptcb;
    
    if (OSIntNesting > 0) {    
        return (OS_TASK_DEL_ISR);
    }
    if (prio == OS_IDLE_PRIO) {                               
        return (OS_TASK_DEL_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {       
        return (OS_PRIO_INVALID);
    }

    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                                
        prio = OSTCBCur->OSTCBPrio;                            
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb != (OS_TCB *)0) {                                  
        if ((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00) { 
            OSRdyGrp &= ~ptcb->OSTCBBitY;
        }
        pevent = ptcb->OSTCBEventPtr;
        if (pevent != (OS_EVENT *)0) {                  
            if ((pevent->OSEventTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0) {
                pevent->OSEventGrp &= ~ptcb->OSTCBBitY;       
            }
        }
        pnode = ptcb->OSTCBFlagNode;
        if (pnode != (OS_FLAG_NODE *)0) {                    
            OS_FlagUnlink(pnode);
        }
        ptcb->OSTCBDly  = 0;                               
        ptcb->OSTCBStat = OS_STAT_RDY;                          
        if (OSLockNesting < 255) {
            OSLockNesting++;
        }
        OS_EXIT_CRITICAL();                                    
        OS_Dummy();                                             
        OS_ENTER_CRITICAL();                                    
        if (OSLockNesting > 0) {
            OSLockNesting--;
        }
        OSTaskDelHook(ptcb);
        OSTaskCtr--;                                           
        OSTCBPrioTbl[prio] = (OS_TCB *)0;                       
        if (ptcb->OSTCBPrev == (OS_TCB *)0) {                   
            ptcb->OSTCBNext->OSTCBPrev = (OS_TCB *)0;
            OSTCBList                  = ptcb->OSTCBNext;
        } 
        else {
            ptcb->OSTCBPrev->OSTCBNext = ptcb->OSTCBNext;
            ptcb->OSTCBNext->OSTCBPrev = ptcb->OSTCBPrev;
        }
        ptcb->OSTCBNext = OSTCBFreeList;                        
        OSTCBFreeList   = ptcb;
        OS_EXIT_CRITICAL();
        OS_Sched();
        return (OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_DEL_ERR);
}


/*
*********************************************************************************************************
* Task Description
* 任务 5: 请求删除任务 (OSTaskDelReq)
*
* 描述:
* 请求另一个任务删除自身，或查询是否被请求删除。
*
* 功能:
* 1. 如果是 OS_PRIO_SELF，返回当前 TCB 的删除请求标志。
* 2. 否则，设置目标任务 TCB 的删除请求标志。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSTaskDelReq (INT8U prio)
{
    OS_CPU_SR  cpu_sr;
    BOOLEAN    stat;
    INT8U      err;
    OS_TCB    *ptcb;

    if (prio == OS_IDLE_PRIO) {                                 
        return (OS_TASK_DEL_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {   
        return (OS_PRIO_INVALID);
    }
    if (prio == OS_PRIO_SELF) {                               
        OS_ENTER_CRITICAL();                           
        stat = OSTCBCur->OSTCBDelReq;                         
        OS_EXIT_CRITICAL();
        return (stat);
    }
    OS_ENTER_CRITICAL();
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb != (OS_TCB *)0) {                                
        ptcb->OSTCBDelReq = OS_TASK_DEL_REQ;                    
        err = OS_NO_ERR;
    } 
    else {
        err = OS_TASK_NOT_EXIST;                 
    }
    OS_EXIT_CRITICAL();
    return (err);
}

/*
*********************************************************************************************************
* Task Description
* 任务 6: 恢复挂起的任务 (OSTaskResume)
*
* 描述:
* 恢复一个被 OSTaskSuspend 挂起的任务。
*
* 功能:
* 1. 清除 TCB 状态中的 OS_STAT_SUSPEND 位。
* 2. 如果任务没有其他等待条件（延时为 0），将其加入就绪表。
* 3. 触发调度。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSTaskResume (INT8U prio)
{
    // 在此输入代码
    OS_CPU_SR cpu_sr;
    OS_TCB *ptcb;
    if(prio >= OS_LOWEST_PRIO){
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    ptcb = OSTCBPrioTbl[prio];
    if(ptcb == (OS_TCB *)0){
        OS_EXIT_CRITICAL();
        return (OS_TASK_RESUME_PRIO);
    }
    if((ptcb->OSTCBStat & OS_STAT_SUSPEND) != OS_STAT_RDY){
        if(((ptcb->OSTCBStat &= ~OS_STAT_SUSPEND) == OS_STAT_RDY) && (ptcb->OSTCBDly == 0)){
            OSRdyGrp |= ptcb->OSTCBBitY;
            OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
            OS_EXIT_CRITICAL();
            OS_Sched();
        }
        else{
            OS_EXIT_CRITICAL();
        }
        return(OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_TASK_NOT_SUSPENDED);
}

/*
*********************************************************************************************************
* Task Description
* 任务 7: 堆栈检查 (OSTaskStkChk)
*
* 描述:
* 计算任务堆栈的空闲和已用空间。
*
* 功能:
* 1. 验证任务存在且启用了堆栈检查选项。
* 2. 从堆栈底部向上扫描，统计 0 值（空闲）的数量。
* 3. 计算字节数。
*
* 移植要点:
* - 依赖于堆栈增长方向 (OS_STK_GROWTH)。在 Mini2440 (ARM) 上，OS_STK_GROWTH 为 1，扫描需适应从低地址向高地址检查空闲区域。
*********************************************************************************************************
*/
INT8U  OSTaskStkChk (INT8U prio, OS_STK_DATA *p_data)
{
    // 不实现
    return (OS_NO_ERR);
}

/*
*********************************************************************************************************
* Task Description
* 任务 8: 挂起任务 (OSTaskSuspend)
*
* 描述:
* 无条件挂起一个任务。
*
* 功能:
* 1. 禁止挂起空闲任务。
* 2. 将任务从就绪表移除。
* 3. 在 TCB 状态中设置 OS_STAT_SUSPEND 位。
* 4. 触发调度。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSTaskSuspend (INT8U prio)
{
    OS_CPU_SR cpu_sr;
    BOOLEAN self;
    OS_TCB *ptcb;

    if(prio == OS_IDLE_PRIO){
        return (OS_TASK_SUSPEND_IDLE);
    }
    if(prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF){
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    if(prio == OS_PRIO_SELF){
        prio = OSTCBCur->OSTCBPrio;
        self = TRUE;
    }
    else if(prio == OSTCBCur->OSTCBPrio){
        self = TRUE;
    }
    else{
        self = TRUE;
    }
    ptcb = OSTCBPrioTbl[prio];
    if(ptcb == (OS_TCB *)0){
        OS_EXIT_CRITICAL();
        return (OS_TASK_SUSPEND_PRIO);
    }
    if((OSRdyTbl[ptcb->OSTCBY] &= ~ptcb->OSTCBBitX) == 0x00){
        OSRdyGrp &= ~ptcb->OSTCBBitY;
    }
    ptcb->OSTCBStat |= OS_STAT_SUSPEND;
    OS_EXIT_CRITICAL();
    if(self == TRUE){
        OS_Sched();
    }
    return (OS_NO_ERR);
}

/*
*********************************************************************************************************
* Task Description
* 任务 9: 查询任务信息 (OSTaskQuery)
*
* 描述:
* 复制任务的 TCB 内容到用户提供的结构中。
*
* 功能:
* 1. 复制 TCB 内容。
*
* 移植要点:
* - 涉及内存拷贝，注意结构体对齐。
*********************************************************************************************************
*/
INT8U  OSTaskQuery (INT8U prio, OS_TCB *pdata)
{
    OS_CPU_SR  cpu_sr;
    OS_TCB    *ptcb;

    if (prio > OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {   
        return (OS_PRIO_INVALID);
    }
    OS_ENTER_CRITICAL();
    if (prio == OS_PRIO_SELF) {                            
        prio = OSTCBCur->OSTCBPrio;
    }
    ptcb = OSTCBPrioTbl[prio];
    if (ptcb == (OS_TCB *)0) {                             
        OS_EXIT_CRITICAL();
        return (OS_PRIO_ERR);
    }
    memcpy(pdata, ptcb, sizeof(OS_TCB));                   
    return (OS_NO_ERR);
}