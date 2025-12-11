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
INT8U OSTaskChangePrio(INT8U oldprio, INT8U newprio){
    OS_CPU_SR cpu_sr;
    OS_TCB    *ptcb;
    if(oldprio >= OS_LOWEST_PRIO || newprio >= OS_LOWEST_PRIO){
        return OS_PRIO_INVALID;
    }
    if(oldprio != OS_PRIO_SELF){
        return OS_PRIO_ERR;
    }
    if(newprio == OS_PRIO_EXIST){
        return OS_PRIO_EXIST;
    }
    OS_ENTER_CRITICAL();
    ptcb->OSTCBPrio = newprio;
    OS_EXIT_CRITICAL();//检查
    ptcb->OSTCBY = newprio >> 3;
    ptcb->OSTCBX = newprio & 7;
    ptcb->OSTCBBitY = OSMapTbl[ptcb->OSTCBY];
    ptcb->OSTCBBitX = OSMapTbl[ptcb->OSTCBX];


    if(ptcb->OSTCBEventPtr != 0){
        oseventwaitlist;
        return //直接看
    }
    return OS_NO_ERR;
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
INT8U OSTaskCreate(int (*task)(int* pdata), void *pata1, void *ptos, INT8U prio){
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
        psp = (OS_STK *)OSTaskStkInit(task, pata1, ptos, 0);
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
    // 在此输入代码

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
    // 在此输入代码
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
    // 在此输入代码
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
    // 在此输入代码
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
    // 在此输入代码
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
INT8U  OSTaskQuery (INT8U prio, OS_TCB *p_data)
{
    // 在此输入代码
}