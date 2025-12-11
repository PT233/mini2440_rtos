/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* MESSAGE QUEUE MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_Q.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"


/*
*********************************************************************************************************
* Task Description
* 任务 1: 无等待请求队列 (OSQAccept)
*
* 描述:
* 检查队列是否有消息。如果有，取出并返回；否则立即返回 NULL。
*
* 功能:
* 1. 检查队列条目数 (OSQEntries)。
* 2. 如果 > 0，从 OSQOut 指向的位置取出消息。
* 3. 移动 OSQOut 指针，处理环形缓冲区回绕。
* 4. 递减条目计数。
*
* 移植要点:
* - 指针算术运算：在 Mini2440 上，`void **` 指针增减步长为 4 字节。
*********************************************************************************************************
*/
void  *OSQAccept (OS_EVENT *pevent)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 创建消息队列 (OSQCreate)
*
* 描述:
* 创建一个消息队列，需要用户提供存储消息指针的数组。
*
* 功能:
* 1. 申请一个空闲 ECB 和一个空闲队列控制块 (OS_Q)。
* 2. 初始化 OS_Q 结构：Start, End, In, Out 指针及 Size。
* 3. 将 ECB 类型设为 OS_EVENT_TYPE_Q，并指向该 OS_Q。
*
* 移植要点:
* - 确保传入的 `start` 数组地址在 Mini2440 的有效 RAM 空间内。
*********************************************************************************************************
*/
OS_EVENT  *OSQCreate (void **start, INT16U size)
{
    // 在此输入代码
    OS_CPU_SR cpu_sr;
    OS_EVENT  *pevent;
    OS_Q      *pq;

    if(OSIntNesting > 0){
        return ((OS_EVENT *)0);
    }
    OS_ENTER_CRITICAL();
    pevent = OSEventFreeList;
    if(OSEventFreeList != (OS_EVENT *)0){
        OSEventFreeList = (OS_EVENT *)OSEventFreeList->OSEventPtr;
    }
    OS_EXIT_CRITICAL();

    if(pevent != (OS_EVENT *)0){
        OS_ENTER_CRITICAL();
        pq = OSQFreeList;
        if(pq != (OS_Q *)0){
            OSQFreeList = (OS_Q *)OSQFreeList->OSQPtr;
            OS_EXIT_CRITICAL();
            pq->OSQStart = start;
            pq->OSQEnd = &start[size];
            pq->OSQIn = start;
            pq->OSQOut = start;
            pq->OSQSize = size;
            pq->OSQEntries = 0;

            pevent->OSEventType = OS_EVENT_TYPE_Q;
            pevent->OSEventCnt = 0;
            pevent->OSEventPtr = pq;//二重指针
            OS_EventWaitListInit(pevent);
        }
        else{
            pevent->OSEventPtr = (void *)OSEventFreeList;
            OSEventFreeList = pevent;
            OS_EXIT_CRITICAL();
            pevent = (OS_EVENT *)0;
        }
    }
    return (pevent);
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 删除消息队列 (OSQDel)
*
* 描述:
* 删除队列，回收 ECB 和 OS_Q 块。
*
* 功能:
* 1. 处理等待任务（同信号量/邮箱删除逻辑）。
* 2. 回收 OS_Q 结构体到 OSQFreeList。
* 3. 回收 ECB 到 OSEventFreeList。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
OS_EVENT  *OSQDel (OS_EVENT *pevent, INT8U opt, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 清空队列 (OSQFlush)
*
* 描述:
* 清除队列中的所有消息，重置指针。
*
* 功能:
* 1. 重置 OSQIn = OSQOut = OSQStart。
* 2. OSQEntries = 0。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSQFlush (OS_EVENT *pevent)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 等待队列消息 (OSQPend)
*
* 描述:
* 等待队列中的消息。
*
* 功能:
* 1. 如果 OSQEntries > 0，取出消息，移动 OSQOut 指针，返回消息。
* 2. 如果队列为空，挂起当前任务。
* 3. 触发调度。
*
* 移植要点:
* - 环形缓冲区指针回绕逻辑必须正确。
*********************************************************************************************************
*/
void  *OSQPend (OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 6: 发送消息到队列 (OSQPost)
*
* 描述:
* 向队列（FIFO）发送一条消息。
*
* 功能:
* 1. 如果有任务等待，直接交给最高优先级的等待任务。
* 2. 否则，检查队列是否已满。
* 3. 如果未满，存入 OSQIn 位置，移动 OSQIn 指针，递增计数。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
INT8U  OSQPost (OS_EVENT *pevent, void *msg)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 7: 发送消息到队列前端 (OSQPostFront)
*
* 描述:
* 向队列前端（LIFO）发送一条消息，用于紧急消息。
*
* 功能:
* 1. 类似于 OSQPost，但操作的是 OSQOut 指针（反向移动）。
* 2. 需处理 OSQOut 回绕到 End 的情况。
*
* 移植要点:
* - 确保指针减法操作在 ARM 寻址模式下是正确的。
*********************************************************************************************************
*/
INT8U  OSQPostFront (OS_EVENT *pevent, void *msg)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 8: 发送消息（扩展选项） (OSQPostOpt)
*
* 描述:
* 支持广播和 LIFO/FIFO 选择的发送函数。
*
* 功能:
* 1. 处理广播 (OS_POST_OPT_BROADCAST)：唤醒所有等待任务。
* 2. 处理 LIFO (OS_POST_OPT_FRONT)。
* 3. 处理 FIFO (默认)。
*
* 移植要点:
* - 同样注意广播时的临界区时长。
*********************************************************************************************************
*/
INT8U  OSQPostOpt (OS_EVENT *pevent, void *msg, INT8U opt)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 9: 查询队列状态 (OSQQuery)
*
* 描述:
* 获取队列详情。
*
* 功能:
* 1. 复制 OS_Q 内容到 OS_Q_DATA。
* 2. 返回当前消息数等信息。
*
* 移植要点:
* - 内存拷贝。
*********************************************************************************************************
*/
INT8U  OSQQuery (OS_EVENT *pevent, OS_Q_DATA *p_data)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 10: 初始化队列模块 (OS_QInit)
*
* 描述:
* 内部函数。初始化空闲队列控制块链表。
*
* 功能:
* 1. 链接 OSQTbl 数组。
* 2. 初始化 OSQFreeList。
*
* 移植要点:
* - 必须在 OSInit 中调用。
*********************************************************************************************************
*/
void  OS_QInit (void)
{
    // 在此输入代码
#if OS_MAX_QS == 1
    OSQFreeList = &OSQTbl[0];
    OSQFreeList->OSQPtr = (OS_Q *)0;

#elif OS_MAX_QS >= 2
    OS_Q *pq1, *pq2;
    pq1 = &OSQTbl[0];
    pq2 = &OSQTbl[1]; 
    for(INT16U i = 0; i < (OS_MAX_QS - 1); i++){
        pq1->OSQPtr = pq2;
        pq1++;
        pq2++;
    }
    pq1->OSQPtr = (OS_Q *)0;
    OSQFreeList = &OSQTbl[0];
#endif
}

