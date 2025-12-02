#include "include.h"

const INT8U OSMapTable[]   = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
const INT8U OSUnMapTable[] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F                             */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F                             */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF                             */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF                             */
};

//函数声明
//初始化操作系统
void  OSInit(void)
{
    OSInitHookBegin();               /* 调用特定于端口的初始化代码*/
    OS_InitMisc();                   /* 初始化杂项变量 */
    OS_InitRdyList();                /* 初始化就绪列表 */
    OS_InitTCBList();                /* 初始化 OS_TCB 的空闲列表*/
    OS_InitEventList();              /* 初始化 OS_EVENT 的空闲列表 */
    OS_FlagInit();                   /* 初始化事件标志结构  */
    OS_MemInit();                    /* 初始化内存管理器*/
    OS_QueueInit();                  /* 初始化消息队列结构 */
    OS_InitTaskIdle();               /* 创建空闲任务*/
    OS_InitTaskStat();               /* 创建统计任务 */
    OSInitHookEnd();                 /* 调用特定于端口的初始化代码*/
}

//进入ISR
//退出ISR
//停止调度
//启动调度
//多任务系统开始运行
//系统状态初始化
//系统滴答
//空函数
void OS_Dummy(void){

}
//事件触发任务就绪
INT8U OS_EventTaskRdy(OS_EVENT *pevent, void *msg, INT8U msk){
    OS_TCB    *ptcb;
    INT8U      prio;
    INT8U      X;
    INT8U      Y;
    INT8U      Bit_X;
    INT8U      Bit_Y;

    Y       = OSUnMapTable[pevent->OSEventGrp];
    Bit_Y   = OSMapTable[Y];
    X       = OSUnMapTable[pevent->OSEventTable[Y]];
    Bit_X   = OSMapTable[X];
    prio    = (INT8U)((Y << 3) + X);
    if((pevent->OSEventTable[Y] & ~Bit_X) == 0x00){ /* 检查事件表中是否有任务等待 */
        pevent->OSEventGrp &= ~Bit_Y; /* 清除就绪组 */
    }
    ptcb = OSTCBPrioTable[prio]; /* 获取任务控制块 */
    ptcb->OSTCBDelay    = 0; /* 清除延迟 */
    ptcb->OSTCBEventPtr = (OS_EVENT *)0; /* 清除事件指针 */
    ptcb->OSTCBMessage  = msg; /* 设置消息 */
    msg = msg; /* 防止未使用变量警告 */
    ptcb->OSTCBStatus   &= ~msk; /* 清除任务状态 */
    if(ptcb->OSTCBStatus == OS_STAT_RDY){ /* 如果任务是就绪状态 */
        OSReadyGroup    |= Bit_Y; /* 设置就绪组 */
        OSReadyTable[Y] |= Bit_X; /* 设置就绪表 */
    }
    return(prio); /* 返回任务优先级 */
}

//任务等待事件发生
void OS_EventTaskWait(OS_EVENT *pevent){
    OSTCBCurrent->OSTCBEventPtr = pevent; /* 设置任务等待的事件 */
    if((OSReadyTable[OSTCBCurrent->OSTCB_Y] &= ~OSTCBCurrent->OSTCBBit_X) == 0x00){
        OSReadyGroup &= ~OSTCBCurrent->OSTCBBit_Y; /* 如果就绪表中没有任务，清除就绪组 */        
    }
    pevent->OSEventTable[OSTCBCurrent->OSTCB_Y] |= OSTCBCurrent->OSTCBBit_X; /* 设置事件表 */
    pevent->OSEventGrp |= OSTCBCurrent->OSTCBBit_Y;
}

//事件超时使任务就绪运行
void OS_EventTO(OS_EVENT *pevent){
    if((pevent->OSEventTable[OSTCBCurrent->OSTCB_Y] &= ~OSTCBCurrent->OSTCBBit_X) == 0x00){ /* 检查事件表中是否有任务等待 */
        pevent->OSEventGrp &= ~OSTCBCurrent->OSTCBBit_Y; /* 清除就绪组 */
    }
    OSTCBCurrent->OSTCBStatus = OS_STAT_RDY; /* 设置任务状态为就绪 */
    OSTCBCurrent->OSTCBEventPtr = (OS_EVENT *)0; /* 清除任务等待的事件 */
}

//事件控制块等待列表初始化
void OS_EventWaitListInit(OS_EVENT *pevent){
    pevent->OSEventType = 0;                                 /* 事件类型初始化为0 */
    memset(pevent->OSEventTable, 0, sizeof(pevent->OSEventTable));
}

//事件控制块空闲列表初始化
//其余变量初始化
//就绪列表初始化
//空闲任务初始化
//任务状态初始化
//TCB表初始化
//调度程序
//空闲任务
//任务状态
//TCB初始化

INT8U OS_TCBInit(INT8U prio, OS_STACK *ptos)
{
    OS_CPU_SR   cpu_sr;
    OS_TCB     *ptcb;    //point to TCB
    OS_ENTER_CRITICAL();
    ptcb = OSTCBFreeList;                                      /* 从空闲列表中取出一个TCB */
    if(ptcb != (OS_TCB *)0){
        OSTCBFreeList = ptcb->OSTCBNextPtr;                    /* 调整空闲列表指针 */
        OS_EXIT_CRITICAL();
        ptcb->OSTCBStackPtr    = (OS_STACK *)ptos;              /* 设置任务栈指针 */
        ptcb->OSTCBEventPtr    = (OS_EVENT *)0;                 /* 任务不等待任何事件 */
        ptcb->OSTCBMessage     = (void *)0;                     /* 任务没有收到任何消息 */
        ptcb->OSTCBFlagNode    = (OS_FLAG_NODE *)0;             /* 任务不等待任何事件标志 */
        ptcb->OSTCBFlagsReady  = (OS_FLAGS)0;
        ptcb->OSTCBPrio        = (INT8U)prio;                   /* 设置任务优先级 */
        ptcb->OSTCBStatus      = OS_STAT_RDY;                   /* 设置任务状态为就绪态 */
        ptcb->OSTCBDelay       = 0;                             /* 无延迟 */
        ptcb->OSTCB_Y          = prio >> 3;                     /* 计算任务优先级在组中的索引 */
        ptcb->OSTCBBit_Y       = OSMapTable[ptcb->OSTCB_Y];     /* 计算访问就绪表中的位位置的位掩码 */
        ptcb->OSTCB_X          = prio & 0x07;                   /* 计算任务优先级的组中的位位置 */
        ptcb->OSTCBBit_X       = OSMapTable[ptcb->OSTCB_X];     /* 计算访问就绪表中的位位置的位掩码 */
        ptcb->OSTCBDeleteReq   = OS_NO_ERR;                     /* 任务删除请求标志清零 */

        OSTCBInitHook(ptcb);                                    /* 调用用户定义的钩子函数 */
        OSTaskCreateHook(ptcb);                                 /* 调用用户定义的钩子函数 */
        OS_ENTER_CRITICAL();
        OSTCBPrioTable[prio] = ptcb;                            /* 在优先级表中注册该任务 */
        ptcb->OSTCBNextPtr = OSTCBList;
        ptcb->OSTCBPrevPtr = (OS_TCB *)0;
        if(OSTCBList != (OS_TCB *)0){                          /* 如果TCB列表为空 */
            OSTCBList->OSTCBPrevPtr = ptcb;
        } 
        OSTCBList                   = ptcb;
        OSReadyGroup                |= ptcb->OSTCBBit_Y;        /* 设置就绪组 */
        OSReadyTable[ptcb->OSTCB_Y] |= ptcb->OSTCBBit_X;        /* 设置就绪表 */
        OS_EXIT_CRITICAL();
        return (OS_NO_ERR); /* 返回成功 */
    }
    OS_EXIT_CRITICAL();
    return (OS_NO_MORE_TCB); /* 返回没有更多的TCB */
}
