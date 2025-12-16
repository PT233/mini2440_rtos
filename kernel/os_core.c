#include "includes.h"

INT8U OSMapTbl[]   = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

INT8U OSUnMapTbl[] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF */
};


static  void  OS_InitEventList(void);
static  void  OS_InitMisc(void);
static  void  OS_InitRdyList(void);
static  void  OS_InitTaskIdle(void);
static  void  OS_InitTaskStat(void);
static  void  OS_InitTCBList(void);


/*
*******************************************************************************
*                       OS_InitMisc()
*
* Description: Initialize miscellaneous variables
*
* Arguments  : none
*
* Returns    : none
*******************************************************************************
*/
static void OS_InitMisc(void)
{
    OSTime        = 0L;                     /* Clear system clock                    */
    OSIntNesting  = 0;                      /* Clear interrupt nesting counter       */
    OSLockNesting = 0;                      /* Clear scheduling lock counter         */
    OSTaskCtr     = 0;                      /* Clear task counter                    */
    OSRunning     = FALSE;                  /* Kernel not started                    */
    OSCtxSwCtr    = 0;                      /* Clear context switch counter          */
    OSIdleCtr     = 0L;                     /* Clear idle counter                    */
    OSIdleCtrRun  = 0L;                     /* Clear run idle counter                */
    OSIdleCtrMax  = 0L;                     /* Clear max idle counter                */
    OSStatRdy     = FALSE;                  /* Statistics task not ready             */
    OSCPUUsage    = 0;                      /* Clear CPU usage                       */
}

/*
*******************************************************************************
*                       OS_InitRdyList()
*
* Description: Initialize the ready list
*
* Arguments  : none
*
* Returns    : none
*******************************************************************************
*/
static void OS_InitRdyList(void)
{
    INT16U i;
    INT8U *prdytbl;

    OSRdyGrp  = 0x00;                       /* Clear ready list group                */
    prdytbl   = &OSRdyTbl[0];
    for (i = 0; i < OS_RDY_TBL_SIZE; i++) {
        *prdytbl++ = 0x00;                  /* Clear ready list table                */
    }
    OSPrioCur     = 0;                      /* Set current priority to lowest        */
    OSPrioHighRdy = 0;                      /* Set highest ready priority            */
    OSTCBHighRdy  = (OS_TCB *)0;            /* No task ready yet                     */
    OSTCBCur      = (OS_TCB *)0;            /* No current task yet                   */
}

/*
*******************************************************************************
*                       OS_InitEventList()
*
* Description: Initialize event control blocks
*
* Arguments  : none
*
* Returns    : none
*******************************************************************************
*/
static void OS_InitEventList(void)
{
    INT16U i;
    OS_EVENT *pevent;
    OS_EVENT *pevent2;

    /* Initialize event free list */
    pevent = &OSEventTbl[0];
    pevent2 = &OSEventTbl[1];
    for (i = 0; i < (OS_MAX_EVENTS - 1); i++) {
        pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
        pevent->OSEventPtr = (void *)pevent2;
        pevent++;
        pevent2++;
    }
    pevent->OSEventType = OS_EVENT_TYPE_UNUSED;
    pevent->OSEventPtr = (void *)0;
    OSEventFreeList = &OSEventTbl[0];

    /* Initialize flag free list */
    OS_FlagInit();
}

/*
*******************************************************************************
*                       OS_InitTaskIdle()
*
* Description: Create the idle task
*
* Arguments  : none
*
* Returns    : none
*******************************************************************************
*/
static void OS_InitTaskIdle(void)
{
    (void)OSTaskCreate(OS_TaskIdle,
                       (void *)0,
                       &OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE - 1],
                       OS_IDLE_PRIO);
}

void  OSInit(void)
{
    OSInitHookBegin();               /* 调用特定于端口的初始化代码 */
    OS_InitMisc();                   /* 初始化杂项变量 */
    OS_InitRdyList();                /* 初始化就绪列表 */
    OS_InitTCBList();                /* 初始化 OS_TCB 的空闲列表 */
    OS_InitEventList();              /* 初始化事件标志结构 */
    OS_MemInit();                    /* 初始化内存管理 */
    OS_QInit();                      /* 初始化消息队列结构 */
    OS_InitTaskIdle();               /* 创建空闲任务 */
    OS_InitTaskStat();               /* 创建统计任务 */
    OSInitHookEnd();                 /* 调用特定于端口的初始化代码 */
}

//进入ISR,需要在context switch中定义
void  OSIntEnter (void)
{
    if (OSRunning == TRUE) {
        if (OSIntNesting < 255) {
            OSIntNesting++;                      /* 递增 ISR 嵌套级别                        */
        }
    }
}

//退出ISR，需要在context switch中定义
void OSIntExit(void){
    OS_CPU_SR   cpu_sr;                                         //CPU 状态寄存器
    if(OSRunning == TRUE){
        OS_ENTER_CRITICAL();
       if(OSIntNesting > 0){
           OSIntNesting--; 
       }
       if((OSIntNesting == 0) && (OSLockNesting == 0)){
            OSIntExitY = OSUnMapTbl[OSRdyGrp];                     // 获取最高优先级任务
            OSPrioHighRdy = (INT8U)((OSIntExitY << 3) + OSUnMapTbl[OSRdyTbl[OSIntExitY]]);
            if (OSPrioHighRdy != OSPrioCur) {              /* 如果当前任务是最高优先级就绪任务，则不进行上下文切换 */
                OSTCBHighRdy  = OSTCBPrioTbl[OSPrioHighRdy];
                OSCtxSwCtr++;                              /* 跟踪上下文切换的次数 */
                OSIntCtxSw();                              /* 执行中断级上下文切换       */
            }
        }
    OS_EXIT_CRITICAL();
    }
}


//调度锁
void OSSchedLock(void){
    OS_CPU_SR   cpu_sr;                                         
    if(OSRunning == FALSE){
        return;
    }
    else{
        OS_ENTER_CRITICAL();
        if(OSLockNesting < 255){
            OSLockNesting++; 
        }
        OS_EXIT_CRITICAL();
    }
}

//关闭调度锁
void OSSchedUnlock(void){
    OS_CPU_SR   cpu_sr;                                         
    if(OSRunning == TRUE){
        OS_ENTER_CRITICAL();
        if(OSLockNesting > 0){
            OSLockNesting--; 
            if((OSLockNesting == 0) && (OSIntNesting == 0)){
                OS_EXIT_CRITICAL();
                OS_Sched();
            }
            else{
                OS_EXIT_CRITICAL();
            }
        }
        else{
            OS_EXIT_CRITICAL();
        }
    }
}

//多任务系统开始运行
void  OSStart (void)
{
    INT8U y;
    INT8U x;

    if (OSRunning == FALSE) {
        y             = OSUnMapTbl[OSRdyGrp];        
        x             = OSUnMapTbl[OSRdyTbl[y]];
        OSPrioHighRdy = (INT8U)((y << 3) + x);
        OSPrioCur     = OSPrioHighRdy;
        OSTCBHighRdy  = OSTCBPrioTbl[OSPrioHighRdy]; 
        OSTCBCur      = OSTCBHighRdy;
        OSStartHighRdy();                            
    }
}

//系统状态初始化
void  OSStatInit (void)
{
    OS_CPU_SR  cpu_sr;
  
    OSTimeDly(2);                                /* 与时钟滴答同步 */
    OS_ENTER_CRITICAL();
    OSIdleCtr    = 0L;                           /* 清除空闲计数器 */
    OS_EXIT_CRITICAL();
    OSTimeDly(OS_TICKS_PER_SEC);                 /* 确定 1 秒内的最大空闲计数器值*/
    OS_ENTER_CRITICAL();
    OSIdleCtrMax = OSIdleCtr;                    /* 存储 1 秒内的最大空闲计数器计数 */
    OSStatRdy    = TRUE;
    OS_EXIT_CRITICAL();
}

//系统滴答
void  OSTimeTick (void)
{
                              
    OS_CPU_SR  cpu_sr;
    OS_TCB    *ptcb;
    OSTimeTickHook();
                                          
    OS_ENTER_CRITICAL();                                   
    OSTime++;
    OS_EXIT_CRITICAL();

    if (OSRunning == TRUE) {    
        ptcb = OSTCBList;                                  
        while (ptcb->OSTCBPrio != OS_IDLE_PRIO) {          
            OS_ENTER_CRITICAL();
            if (ptcb->OSTCBDly != 0) {                     
                if (--ptcb->OSTCBDly == 0) {               
                    if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY) {
                        OSRdyGrp               |= ptcb->OSTCBBitY; 
                        OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
                    } else {  
                        ptcb->OSTCBDly = 1;  
                    }    
                }
            }
            ptcb = ptcb->OSTCBNext;
            OS_EXIT_CRITICAL();
        }
    }
}

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

    Y       = OSUnMapTbl[pevent->OSEventGrp];
    Bit_Y   = OSMapTbl[Y];
    X       = OSUnMapTbl[pevent->OSEventTbl[Y]];
    Bit_X   = OSMapTbl[X];
    prio    = (INT8U)((Y << 3) + X);
    if((pevent->OSEventTbl[Y] & ~Bit_X) == 0x00){ /* 检查事件表中是否有任务等待 */
        pevent->OSEventGrp &= ~Bit_Y; /* 清除就绪组 */
    }
    ptcb = OSTCBPrioTbl[prio]; /* 获取任务控制块 */
    ptcb->OSTCBDly      = 0; /* 清除延迟 */
    ptcb->OSTCBEventPtr = (OS_EVENT *)0; /* 清除事件指针 */
    ptcb->OSTCBMsg      = msg; /* 设置消息 */
    msg = msg; /* 防止未使用变量警告 */
    ptcb->OSTCBStat     &= ~msk; /* 清除任务状态 */
    if(ptcb->OSTCBStat == OS_STAT_RDY){ /* 如果任务是就绪状态 */
        OSRdyGrp    |= Bit_Y; /* 设置就绪组 */
        OSRdyTbl[Y] |= Bit_X; /* 设置就绪表 */
    }
    return(prio); /* 返回任务优先级 */
}

//任务等待事件发生
void OS_EventTaskWait(OS_EVENT *pevent){
    OSTCBCur->OSTCBEventPtr = pevent; /* 设置任务等待的事件 */
    if((OSRdyTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0x00){
        OSRdyGrp &= ~OSTCBCur->OSTCBBitY; /* 如果就绪表中没有任务，清除就绪组 */        
    }
    pevent->OSEventTbl[OSTCBCur->OSTCBY] |= OSTCBCur->OSTCBBitX; /* 设置事件表 */
    pevent->OSEventGrp |= OSTCBCur->OSTCBBitY;
}

//事件超时使任务就绪运行,TO: timeout
void OS_EventTO(OS_EVENT *pevent){
    if((pevent->OSEventTbl[OSTCBCur->OSTCBY] &= ~OSTCBCur->OSTCBBitX) == 0x00){ /* 检查事件表中是否有任务等待 */
        pevent->OSEventGrp &= ~OSTCBCur->OSTCBBitY; /* 清除就绪组 */
    }
    OSTCBCur->OSTCBStat = OS_STAT_RDY; /* 设置任务状态为就绪 */
    OSTCBCur->OSTCBEventPtr = (OS_EVENT *)0; /* 清除任务等待的事件 */
}

//事件控制块等待列表初始化
void OS_EventWaitListInit(OS_EVENT *pevent){
    pevent->OSEventType = 0;                                 /* 事件类型初始化为0 */
    memset(pevent->OSEventTbl, 0, sizeof(pevent->OSEventTbl));
}

//事件控制块空闲列表初始化
//其余变量初始化
//就绪列表初始化
//空闲任务初始化
//统计任务初始化
static  void  OS_InitTaskStat (void)
{
    (void)OSTaskCreate(OS_TaskStat,
                       (void *)0,                                      /* 没有参数传递给 OS_TaskStat()*/
                       &OSTaskStatStk[OS_TASK_STAT_STK_SIZE - 1],      /* 设置栈顶               */
                       OS_STAT_PRIO);                                  /* 比空闲任务高一级  */
}

//TCBList初始化
static void OS_InitTCBList(void){
    INT8U        i;
    OS_TCB      *ptcb1;
    OS_TCB      *ptcb2;

    OSTCBList = (OS_TCB *)0;
    for(i = 0; i < OS_LOWEST_PRIO + 1; i++){
        OSTCBPrioTbl[i] = (OS_TCB *)0;
    }
    ptcb1 = &OSTCBTbl[0];
    ptcb2 = &OSTCBTbl[1];
    for(i = 0; i < OS_MAX_TASKS + OS_N_SYS_TASKS - 1; i++){
        ptcb1->OSTCBNext = ptcb2;
        ptcb1++;
        ptcb2++;
    }
    ptcb1->OSTCBNext = (OS_TCB *)0;
    OSTCBFreeList = &OSTCBTbl[0];
}

void  OS_Sched (void)
{
    OS_CPU_SR  cpu_sr;
    INT8U      y;
    OS_ENTER_CRITICAL();
    if ((OSIntNesting == 0) && (OSLockNesting == 0)) { 
        y             = OSUnMapTbl[OSRdyGrp];      
        OSPrioHighRdy = (INT8U)((y << 3) + OSUnMapTbl[OSRdyTbl[y]]);
        if (OSPrioHighRdy != OSPrioCur) {   
            OSTCBHighRdy = OSTCBPrioTbl[OSPrioHighRdy];
            OSCtxSwCtr++;                            
            OS_TASK_SW(); 
        }
    }
    OS_EXIT_CRITICAL();
}

//空闲任务
void OS_TaskIdle(void *pdata1){
    OS_CPU_SR  cpu_sr;                                         //CPU 状态寄存器
    pdata1 = pdata1;                                           // 防止未使用变量警告
    while(1){
        OS_ENTER_CRITICAL();
        OSIdleCtr++;                                           // 空闲计数器加1
        OS_EXIT_CRITICAL();
        OSTaskIdleHook();                                     // 调用用户定义的空闲任务钩子函数,还没定义
    }
}

//统计任务
void OS_TaskStat(void *pdata1){
    OS_CPU_SR  cpu_sr;   
    INT32U     run;
    INT32U     max;
    INT8S      usage;

    pdata1 = pdata1;                               /* 防止未使用 'pdata' 的编译器警告     */
    while (OSStatRdy == FALSE) {
        OSTimeDly(2 * OS_TICKS_PER_SEC);         /* 等待统计任务就绪                 */
    }
    max = OSIdleCtrMax / 100L;
    for (;;) {
        OS_ENTER_CRITICAL();
        OSIdleCtrRun = OSIdleCtr;                /* 获取过去一秒的空闲计数器值 */
        run          = OSIdleCtr;
        OSIdleCtr    = 0L;                       /* 重置下一秒的空闲计数器         */
        OS_EXIT_CRITICAL();
        if (max > 0L) {
            usage = (INT8S)(100L - run / max);
            if (usage >= 0) {                    /* 确保我们没有负百分比      */
                OSCPUUsage = usage;
            } 
            else {
                OSCPUUsage = 0;
            }
        } 
        else {
            OSCPUUsage = 0;
            max        = OSIdleCtrMax / 100L;
        }
        OSTaskStatHook();                        /* 调用用户可定义的钩子                         */
        OSTimeDly(OS_TICKS_PER_SEC);             /* 为下一秒累积 OSIdleCtr           */
    }
}

//TCB初始化
INT8U  OS_TCBInit (INT8U  prio, OS_STK  * ptos, 
				   OS_STK * pbos, INT16U  id, 
				   INT32U  stk_size, void * pext, INT16U  opt)
{
    OS_CPU_SR  cpu_sr;

    OS_TCB   *ptcb;
    OS_ENTER_CRITICAL();
    ptcb = OSTCBFreeList;      
    if (ptcb != (OS_TCB *)0) {
        OSTCBFreeList        = ptcb->OSTCBNext;    
        OS_EXIT_CRITICAL();
        ptcb->OSTCBStkPtr    = (OS_STK *)ptos;
        ptcb->OSTCBPrio      = (INT8U)prio;                
        ptcb->OSTCBStat      = OS_STAT_RDY;                
        ptcb->OSTCBDly       = 0;                          
        ptcb->OSTCBDelReq    = OS_NO_ERR;
        ptcb->OSTCBY         = prio >> 3;                  
        ptcb->OSTCBBitY      = OSMapTbl[ptcb->OSTCBY];
        ptcb->OSTCBX         = prio & 0x07;
        ptcb->OSTCBBitX      = OSMapTbl[ptcb->OSTCBX];
        ptcb->OSTCBEventPtr  = (OS_EVENT *)0;            
        ptcb->OSTCBFlagNode  = (OS_FLAG_NODE *)0;      
        ptcb->OSTCBMsg       = (void *)0;     
        OSTCBInitHook(ptcb);

        OSTaskCreateHook(ptcb);                            
        OS_ENTER_CRITICAL();
        OSTCBPrioTbl[prio] = ptcb;
        ptcb->OSTCBNext    = OSTCBList;                    
        ptcb->OSTCBPrev    = (OS_TCB *)0;
        if (OSTCBList != (OS_TCB *)0) {
            OSTCBList->OSTCBPrev = ptcb;
        }
        OSTCBList               = ptcb;
        OSRdyGrp               |= ptcb->OSTCBBitY;         
        OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
        OS_EXIT_CRITICAL();
        return (OS_NO_ERR);
    }
    OS_EXIT_CRITICAL();
    return (OS_NO_MORE_TCB);
}
