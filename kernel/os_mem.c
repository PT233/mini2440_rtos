/*
*********************************************************************************************************
* uC/OS-II
* The Real-Time Kernel
* MEMORY MANAGEMENT
*
* (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
* All Rights Reserved
*
* File : OS_MEM.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#include "includes.h"



/*
*********************************************************************************************************
* Task Description
* 任务 1: 创建内存分区 (OSMemCreate)
*
* 描述:
* 从一块连续内存区域创建一个固定大小块的内存分区。
*
* 功能:
* 1. 申请一个空闲内存控制块 (OS_MEM)。
* 2. 将原始内存区域初始化为单向链表（将每个块的前 4 字节指向下一个块）。
* 3. 初始化 OS_MEM 结构：FreeList, BlkSize, NBlks 等。
*
* 移植要点:
* - Mini2440 (ARM9) 要求指针访问对齐。
* - `blksize` 必须至少是指针的大小 (4字节)，且最好是 4 字节对齐，否则在链表指针解引用时可能导致 Data Abort 异常。
* - 建议在代码中增加对 `addr` 和 `blksize` 的对齐检查。
*********************************************************************************************************
*/
OS_MEM  *OSMemCreate (void *addr, INT32U nblks, INT32U blksize, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 2: 获取内存块 (OSMemGet)
*
* 描述:
* 从分区中申请一个内存块。
*
* 功能:
* 1. 检查是否有空闲块 (OSMemNFree > 0)。
* 2. 如果有，从 FreeList 头部取出一个块。
* 3. 更新 FreeList 指向下一个空闲块。
* 4. 递减空闲计数。
*
* 移植要点:
* - 纯逻辑操作。
*********************************************************************************************************
*/
void  *OSMemGet (OS_MEM *pmem, INT8U *err)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 3: 释放内存块 (OSMemPut)
*
* 描述:
* 将内存块归还给分区。
*
* 功能:
* 1. 检查分区是否已满（防止错误释放）。
* 2. 将释放的块插入 FreeList 头部。
* 3. 递增空闲计数。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSMemPut (OS_MEM  *pmem, void *pblk)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 4: 查询分区状态 (OSMemQuery)
*
* 描述:
* 获取内存分区的使用情况。
*
* 功能:
* 1. 复制 OS_MEM 信息到 OS_MEM_DATA。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
INT8U  OSMemQuery (OS_MEM *pmem, OS_MEM_DATA *p_data)
{
    // 在此输入代码
}

/*
*********************************************************************************************************
* Task Description
* 任务 5: 初始化内存模块 (OS_MemInit)
*
* 描述:
* 内部函数。初始化内存控制块空闲列表。
*
* 功能:
* 1. 链接 OSMemTbl。
*
* 移植要点:
* - 无。
*********************************************************************************************************
*/
void  OS_MemInit (void)
{
    // 在此输入代码
#if OS_MAX_MEM_PART == 1
    OSMemFreeList = (OS_MEM *)&OSMemTbl[0];
    OSMemFreeList->OSMemAddr = (void *)0;
    OSMemFreeList->OSMemFreeList = (void *)0;
    OSMemFreeList->OSMemBlkSize = 0;
    OSMemFreeList->OSMemNBlks = 0;
    OSMemFreeList->OSMemNFree = 0;
#elif OS_MAX_MEM_PART >= 2
    OS_MEM *pmem;
    pmem = (OS_MEM *)&OSMemTbl[0];
    for(INT16U i = 0; i < (OS_MAX_MEM_PART - 1); i++){
        pmem->OSMemAddr = (void *)0;
        pmem->OSMemFreeList = (void *)&OSMemTbl[i+1];
        pmem->OSMemBlkSize = 0;
        pmem->OSMemNBlks = 0;
        pmem->OSMemNFree = 0;
        pmem++;
    }
    pmem->OSMemFreeList = (void *)0;
    pmem->OSMemAddr = (void *)0;
    pmem->OSMemBlkSize = 0;
    pmem->OSMemNBlks = 0;
    pmem->OSMemNFree = 0;

    OSMemFreeList =(OS_MEM *)&OSMemTbl[0];
#endif
}

