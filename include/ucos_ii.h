#ifndef __UCOS_II_H__
#define __UCOS_II_H__

#include "os_cfg.h"
#include "os_cpu.h"

/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/
#ifndef  FALSE
#define  FALSE                     0
#endif

#ifndef  TRUE
#define  TRUE                      1
#endif

#define  OS_PRIO_SELF           0xFF                    // Indicate SELF priority                      


#define  OS_N_SYS_TASKS            3                    // Number of system tasks                      


#define  OS_STAT_PRIO       (OS_LOWEST_PRIO - 1)        // Statistic task priority                     
#define  OS_IDLE_PRIO       (OS_LOWEST_PRIO)            // IDLE      task priority                     

#define  OS_EVENT_TBL_SIZE ((OS_LOWEST_PRIO) / 8 + 1)   // Size of event table                         
#define  OS_RDY_TBL_SIZE   ((OS_LOWEST_PRIO) / 8 + 1)   // Size of ready table                         

#define  OS_TASK_IDLE_ID       65535                    // I.D. numbers for Idle and Stat tasks        
#define  OS_TASK_STAT_ID       65534

/*
*********************************************************************************************************
*                              TASK STATUS (Bit definition for OSTCBStat)
*********************************************************************************************************
*/
#define  OS_STAT_RDY            0x00        // Ready to run                                            
#define  OS_STAT_SEM            0x01        // Pending on semaphore                                    
#define  OS_STAT_MBOX           0x02        // Pending on mailbox                                      
#define  OS_STAT_Q              0x04        // Pending on queue                                        
#define  OS_STAT_SUSPEND        0x08        // Task is suspended                                       
#define  OS_STAT_MUTEX          0x10        // Pending on mutual exclusion semaphore                   
#define  OS_STAT_FLAG           0x20        // Pending on event flag group                             

/*
*********************************************************************************************************
*                                        OS_EVENT types
*********************************************************************************************************
*/
#define  OS_EVENT_TYPE_UNUSED      0
#define  OS_EVENT_TYPE_MBOX        1
#define  OS_EVENT_TYPE_Q           2
#define  OS_EVENT_TYPE_SEM         3
#define  OS_EVENT_TYPE_MUTEX       4
#define  OS_EVENT_TYPE_FLAG        5

/*
*********************************************************************************************************
*                                         EVENT FLAGS
*********************************************************************************************************
*/
#define  OS_FLAG_WAIT_CLR_ALL      0        // Wait for ALL    the bits specified to be CLR (i.e. 0)    
#define  OS_FLAG_WAIT_CLR_AND      0

#define  OS_FLAG_WAIT_CLR_ANY      1        // Wait for ANY of the bits specified to be CLR (i.e. 0)   
#define  OS_FLAG_WAIT_CLR_OR       1

#define  OS_FLAG_WAIT_SET_ALL      2        // Wait for ALL    the bits specified to be SET (i.e. 1)    
#define  OS_FLAG_WAIT_SET_AND      2

#define  OS_FLAG_WAIT_SET_ANY      3        // Wait for ANY of the bits specified to be SET (i.e. 1)   
#define  OS_FLAG_WAIT_SET_OR       3


#define  OS_FLAG_CONSUME        0x80        // Consume the flags if condition(s) satisfied             


#define  OS_FLAG_CLR               0
#define  OS_FLAG_SET               1

/*
*********************************************************************************************************
*       Possible values for 'opt' argument of OSSemDel(), OSMboxDel(), OSQDel() and OSMutexDel()
*********************************************************************************************************
*/
#define  OS_DEL_NO_PEND            0
#define  OS_DEL_ALWAYS             1

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define OS_NO_ERR                 0

#define OS_ERR_EVENT_TYPE         1
#define OS_ERR_PEND_ISR           2
#define OS_ERR_POST_NULL_PTR      3
#define OS_ERR_PEVENT_NULL        4
#define OS_ERR_POST_ISR           5
#define OS_ERR_QUERY_ISR          6
#define OS_ERR_INVALID_OPT        7
#define OS_ERR_TASK_WAITING       8

#define OS_TIMEOUT               10
#define OS_TASK_NOT_EXIST        11

#define OS_MBOX_FULL             20

#define OS_Q_FULL                30

#define OS_PRIO_EXIST            40
#define OS_PRIO_ERR              41
#define OS_PRIO_INVALID          42

#define OS_SEM_OVF               50

#define OS_TASK_DEL_ERR          60
#define OS_TASK_DEL_IDLE         61
#define OS_TASK_DEL_REQ          62
#define OS_TASK_DEL_ISR          63

#define OS_NO_MORE_TCB           70

#define OS_TIME_NOT_DLY          80
#define OS_TIME_INVALID_MINUTES  81
#define OS_TIME_INVALID_SECONDS  82
#define OS_TIME_INVALID_MILLI    83
#define OS_TIME_ZERO_DLY         84

#define OS_TASK_SUSPEND_PRIO     90
#define OS_TASK_SUSPEND_IDLE     91

#define OS_TASK_RESUME_PRIO     100
#define OS_TASK_NOT_SUSPENDED   101

#define OS_MEM_INVALID_PART     110
#define OS_MEM_INVALID_BLKS     111
#define OS_MEM_INVALID_SIZE     112
#define OS_MEM_NO_FREE_BLKS     113
#define OS_MEM_FULL             114
#define OS_MEM_INVALID_PBLK     115
#define OS_MEM_INVALID_PMEM     116
#define OS_MEM_INVALID_PDATA    117
#define OS_MEM_INVALID_ADDR     118

#define OS_ERR_NOT_MUTEX_OWNER  120

#define OS_TASK_OPT_ERR         130

#define OS_ERR_DEL_ISR          140
#define OS_ERR_CREATE_ISR       141

#define OS_FLAG_INVALID_PGRP    150
#define OS_FLAG_ERR_WAIT_TYPE   151
#define OS_FLAG_ERR_NOT_RDY     152
#define OS_FLAG_INVALID_OPT     153
#define OS_FLAG_GRP_DEPLETED    154

//EVENT CONTROL BLOCK===============================
typedef struct os_event {
    INT8U           OSEventType;        // 事件类型 。它可以是信号量(OS_EVENT-TYPE_SEM)、互斥型信号
                                        //(OS-EVENT-TYPE-MUTEX)邮箱(OS_EVENT-TYPE-MBOX)或消息队列
                                        //(OSEVENT-TYPE-Q)中的一种
    INT8U           OSEventGrp;         // 任务等待此事件的就绪组                            
    void           *OSEventPtr;         // 指向等待此事件的最高优先级任务的指针                
    INT32U          OSEventCnt;         // 信号量计数                                        
    INT8U           OSEventTbl[OS_EVENT_TBL_SIZE]; // 任务等待此事件的就绪表                
}OS_EVENT;

//EVENT FLAGS CONTROL BLOCK=========================
typedef struct {                            /* Event Flag Group                                        */
    INT8U         OSFlagType;               /* Should be set to OS_EVENT_TYPE_FLAG                     */
    void         *OSFlagWaitList;           /* Pointer to first NODE of task waiting on event flag     */
    OS_FLAGS      OSFlagFlags;              /* 8, 16 or 32 bit flags                                   */
} OS_FLAG_GRP; 

typedef struct {                            /* Event Flag Wait List Node                               */
    void         *OSFlagNodeNext;           /* Pointer to next     NODE in wait list                   */
    void         *OSFlagNodePrev;           /* Pointer to previous NODE in wait list                   */
    void         *OSFlagNodeTCB;            /* Pointer to TCB of waiting task                          */  
    void         *OSFlagNodeFlagGrp;        /* Pointer to Event Flag Group                             */  
    OS_FLAGS      OSFlagNodeFlags;          /* Event flag to wait on                                   */  
    INT8U         OSFlagNodeWaitType;       /* Type of wait:                                           */
                                            /*      OS_FLAG_WAIT_AND                                   */
                                            /*      OS_FLAG_WAIT_ALL                                   */
                                            /*      OS_FLAG_WAIT_OR                                    */
                                            /*      OS_FLAG_WAIT_ANY                                   */
} OS_FLAG_NODE;

//MESSAGE MAILBOX DATA==============================
typedef struct {
    void   *OSMsg;                         /* Pointer to message in mailbox                            */
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE]; /* List of tasks waiting for event to occur                 */
    INT8U   OSEventGrp;                    /* Group corresponding to tasks waiting for event to occur  */
} OS_MBOX_DATA;

//MEMORY PARTITION DATA STRUCTURES==================
typedef struct {                       /* MEMORY CONTROL BLOCK                                         */
    void   *OSMemAddr;                 /* Pointer to beginning of memory partition                     */
    void   *OSMemFreeList;             /* Pointer to list of free memory blocks                        */
    INT32U  OSMemBlkSize;              /* Size (in bytes) of each block of memory                      */
    INT32U  OSMemNBlks;                /* Total number of blocks in this partition                     */
    INT32U  OSMemNFree;                /* Number of memory blocks remaining in this partition          */
} OS_MEM;

typedef struct {
    void   *OSAddr;                    /* Pointer to the beginning address of the memory partition     */
    void   *OSFreeList;                /* Pointer to the beginning of the free list of memory blocks   */
    INT32U  OSBlkSize;                 /* Size (in bytes) of each memory block                         */
    INT32U  OSNBlks;                   /* Total number of blocks in the partition                      */
    INT32U  OSNFree;                   /* Number of memory blocks free                                 */
    INT32U  OSNUsed;                   /* Number of memory blocks used                                 */
} OS_MEM_DATA;

//MUTUAL EXCLUSION SEMAPHORE DATA===================
typedef struct {
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT8U   OSEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    INT8U   OSValue;                        /* Mutex value (0 = used, 1 = available)                   */
    INT8U   OSOwnerPrio;                    /* Mutex owner's task priority or 0xFF if no owner         */
    INT8U   OSMutexPIP;                     /* Priority Inheritance Priority or 0xFF if no owner       */
} OS_MUTEX_DATA;

//MESSAGE QUEUE DATA================================
typedef struct os_q {                   /* QUEUE CONTROL BLOCK                                         */
    struct os_q   *OSQPtr;              /* Link to next queue control block in list of free blocks     */
    void         **OSQStart;            /* Pointer to start of queue data                              */
    void         **OSQEnd;              /* Pointer to end   of queue data                              */
    void         **OSQIn;               /* Pointer to where next message will be inserted  in   the Q  */
    void         **OSQOut;              /* Pointer to where next message will be extracted from the Q  */
    INT16U         OSQSize;             /* Size of queue (maximum number of entries)                   */
    INT16U         OSQEntries;          /* Current number of entries in the queue                      */
} OS_Q;

typedef struct {
    void          *OSMsg;               /* Pointer to next message to be extracted from queue          */
    INT16U         OSNMsgs;             /* Number of messages in message queue                         */
    INT16U         OSQSize;             /* Size of message queue                                       */
    INT8U          OSEventTbl[OS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur         */
    INT8U          OSEventGrp;          /* Group corresponding to tasks waiting for event to occur     */
} OS_Q_DATA;

//SEMAPHORE DATA====================================
typedef struct {
    INT16U  OSCnt;                          /* 信号量计数                                         */
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE];  /* 等待事件发生的任务列表                */
    INT8U   OSEventGrp;                     /* 对应于等待事件发生的任务的组 */
} OS_SEM_DATA;

//TASK STACK DATA===================================
typedef struct {
    INT32U  OSFree;                    // 堆栈上的空闲字节数                            
    INT32U  OSUsed;                    // 堆栈上已使用的字节数                            
} OS_STK_DATA;

//TASK CONTROL BLOCK================================
typedef struct os_tcb{
    OS_STK          *OSTCBStkPtr;               //指向当前任务栈的栈顶指针
    //双向链表
    struct os_tcb   *OSTCBPrev;                 //指向上一个TCB任务控制块的指针
    struct os_tcb   *OSTCBNext;                 //指向下一个TCB任务控制块的指针
    OS_EVENT        *OSTCBEventPtr;             //指向任务所等待的事件控制块的指针
    void            *OSTCBMsg;                  //从邮箱或者消息队列中的消息
    OS_FLAG_NODE    *OSTCBFlagNode;             //指向时间标志节点的指针
    OS_FLAGS         OSTCBFlagsRdy;             //任务就绪态的事件标志
    INT16U           OSTCBDly;                  //延迟任务的节拍数，或者等待事件的超时
    INT8U            OSTCBPrio;                 //任务优先级
    INT8U            OSTCBStat;                 //任务状态
    INT8U            OSTCBX;                    //任务优先级的组中的位位置，在os_task.c中
    INT8U            OSTCBY;                    //任务优先级在组中的就绪表索引
    INT8U            OSTCBBitX;                 //访问就绪表中的位位置的位掩码
    INT8U            OSTCBBitY;                 //访问就绪表中的位位置的位掩码
    BOOLEAN          OSTCBDelReq;               //任务等待事件的返回值
}OS_TCB;

//GLOBAL VARIABLES==================================
extern INT32U            OSCtxSwCtr;               // 上下文切换次数计数器           

extern OS_EVENT         *OSEventFreeList;          // 指向空闲 EVENT 控制块列表的指针    
extern OS_EVENT          OSEventTbl[OS_MAX_EVENTS];// EVENT 控制块表                   

extern OS_FLAG_GRP       OSFlagTbl[OS_MAX_FLAGS];  // 包含事件标志组的表              
extern OS_FLAG_GRP      *OSFlagFreeList;           // 指向事件标志组空闲列表的指针       

extern INT8U             OSCPUUsage;               // CPU 使用率百分比                          
extern INT32U            OSIdleCtrMax;             // 空闲计数器在 1 秒内可以达到的最大值     
extern INT32U            OSIdleCtrRun;             // 运行时空闲计数器在 1 秒内达到的值  
extern BOOLEAN           OSStatRdy;                // 指示统计任务已就绪的标志  
extern OS_STK            OSTaskStatStk[OS_TASK_STAT_STK_SIZE];      // 统计任务堆栈          

extern INT8U             OSIntNesting;             // 中断嵌套级别                         
extern INT8U             OSIntExitY;

extern INT8U             OSLockNesting;            // 多任务锁定嵌套级别                 

extern INT8U             OSPrioCur;                    // 当前任务的优先级                        
extern INT8U             OSPrioHighRdy;            // 最高优先级任务的优先级               

extern INT8U             OSRdyGrp;                            // 就绪列表组                         
extern INT8U             OSRdyTbl[OS_RDY_TBL_SIZE];           // 准备运行的任务表    

extern BOOLEAN           OSRunning;                       // 指示内核正在运行的标志   

extern INT8U             OSTaskCtr;                       // 已创建的任务数                  

extern volatile  INT32U  OSIdleCtr;                                 // 空闲计数器                   

extern OS_STK            OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      // 空闲任务堆栈                


extern OS_TCB           *OSTCBCur;                            // 指向当前运行 TCB 的指针         
extern OS_TCB           *OSTCBFreeList;                   // 指向空闲 TCB 列表的指针             
extern OS_TCB           *OSTCBHighRdy;                    // 指向最高优先级就绪 TCB 的指针   
extern OS_TCB           *OSTCBList;                       // 指向 TCB 双向链表的指针    
extern OS_TCB           *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];// 指向已创建 TCB 的指针表        
extern OS_TCB            OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];   // TCB 表                  

extern OS_MEM           *OSMemFreeList;            // 指向内存分区空闲列表的指针       
extern OS_MEM            OSMemTbl[OS_MAX_MEM_PART];// 内存分区管理器的存储            


extern OS_Q             *OSQFreeList;              // 指向空闲队列控制块列表的指针    
extern OS_Q              OSQTbl[OS_MAX_QS];        // 队列控制块表                   

extern volatile  INT32U  OSTime;                   // 系统时间的当前值 (以节拍为单位)         

extern  INT8U            OSMapTbl[];                 // 优先级->位掩码 查找表                 
extern  INT8U            OSUnMapTbl[];               // 优先级->索引   查找表                 

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                     (Target Independent Functions)
*********************************************************************************************************
*/

//EVENT FLAGS MANAGEMENT=============================
OS_FLAGS      OSFlagAccept(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT8U *err);
OS_FLAG_GRP  *OSFlagCreate(OS_FLAGS flags, INT8U *err);
OS_FLAG_GRP  *OSFlagDel(OS_FLAG_GRP *pgrp, INT8U opt, INT8U *err);
OS_FLAGS      OSFlagPend(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U wait_type, INT16U timeout, INT8U *err);
OS_FLAGS      OSFlagPost(OS_FLAG_GRP *pgrp, OS_FLAGS flags, INT8U operation, INT8U *err);
OS_FLAGS      OSFlagQuery(OS_FLAG_GRP *pgrp, INT8U *err);

//MESSAGE MAILBOX MANAGEMENT=========================
void         *OSMboxAccept(OS_EVENT *pevent);
OS_EVENT     *OSMboxCreate(void *msg);

OS_EVENT     *OSMboxDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
void         *OSMboxPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSMboxPost(OS_EVENT *pevent, void *msg);
INT8U         OSMboxPostOpt(OS_EVENT *pevent, void *msg, INT8U opt);
INT8U         OSMboxQuery(OS_EVENT *pevent, OS_MBOX_DATA *pdata);


//MEMORY MANAGEMENT==================================
OS_MEM       *OSMemCreate(void *addr, INT32U nblks, INT32U blksize, INT8U *err);
void         *OSMemGet(OS_MEM *pmem, INT8U *err);
INT8U         OSMemPut(OS_MEM *pmem, void *pblk);
INT8U         OSMemQuery(OS_MEM *pmem, OS_MEM_DATA *pdata);

//MUTUAL EXCLUSION SEMAPHORE MANAGEMENT==============
INT8U         OSMutexAccept(OS_EVENT *pevent, INT8U *err);
OS_EVENT     *OSMutexCreate(INT8U prio, INT8U *err);
OS_EVENT     *OSMutexDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
void          OSMutexPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSMutexPost(OS_EVENT *pevent);
INT8U         OSMutexQuery(OS_EVENT *pevent, OS_MUTEX_DATA *pdata);

//MESSAGE QUEUE MANAGEMENT===========================
void         *OSQAccept(OS_EVENT *pevent);
OS_EVENT     *OSQCreate(void **start, INT16U size);
OS_EVENT     *OSQDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
INT8U         OSQFlush(OS_EVENT *pevent);
void         *OSQPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSQPost(OS_EVENT *pevent, void *msg);
INT8U         OSQPostFront(OS_EVENT *pevent, void *msg);
INT8U         OSQPostOpt(OS_EVENT *pevent, void *msg, INT8U opt);
INT8U         OSQQuery(OS_EVENT *pevent, OS_Q_DATA *pdata);


//SEMAPHORE MANAGEMENT===============================
INT16U        OSSemAccept(OS_EVENT *pevent);
OS_EVENT     *OSSemCreate(INT16U cnt);
OS_EVENT     *OSSemDel(OS_EVENT *pevent, INT8U opt, INT8U *err);
void          OSSemPend(OS_EVENT *pevent, INT16U timeout, INT8U *err);
INT8U         OSSemPost(OS_EVENT *pevent);
INT8U         OSSemQuery(OS_EVENT *pevent, OS_SEM_DATA *pdata);

//TASK MANAGEMENT====================================
INT8U         OSTaskChangePrio(INT8U oldprio, INT8U newprio);
INT8U         OSTaskCreate(void (* task)(void *pd), void *  pdata1, OS_STK *  ptos, INT8U  prio);
INT8U         OSTaskCreateExt(void  (*task)(void *pd), void   *pdata, OS_STK *ptos, INT8U   prio, INT16U  id, OS_STK *pbos, INT32U  stk_size, void   *pext, INT16U  opt);
INT8U         OSTaskDel(INT8U prio);
INT8U         OSTaskDelReq(INT8U prio);
INT8U         OSTaskResume(INT8U prio);
INT8U         OSTaskSuspend(INT8U prio);
INT8U         OSTaskStkChk(INT8U prio, OS_STK_DATA *pdata);
INT8U         OSTaskQuery(INT8U prio, OS_TCB *pdata);

//TIME MANAGEMENT====================================
void          OSTimeDly(INT16U ticks);
INT8U         OSTimeDlyHMSM(INT8U hours, INT8U minutes, INT8U seconds, INT16U milli);
INT8U         OSTimeDlyResume(INT8U prio);
INT32U        OSTimeGet(void);
void          OSTimeSet(INT32U ticks);
void          OSTimeTick(void);

//MISCELLANEOUS======================================
void          OSInit(void);
void          OSIntEnter(void);
void          OSIntExit(void);
void          OSSchedLock(void);
void          OSSchedUnlock(void);
void          OSStart(void);
void          OSStatInit(void);

//INTERNAL FUNCTION PROTOTYPES=======================
void          OS_Dummy(void);
INT8U         OS_EventTaskRdy(OS_EVENT *pevent, void *msg, INT8U msk);
void          OS_EventTaskWait(OS_EVENT *pevent);
void          OS_EventTO(OS_EVENT *pevent);
void          OS_EventWaitListInit(OS_EVENT *pevent);
void          OS_FlagInit(void);
void          OS_FlagUnlink(OS_FLAG_NODE *pnode);
void          OS_MemInit(void);
void          OS_QInit(void);
void          OS_Sched(void);
void          OS_TaskIdle(void *data1);
void          OS_TaskStat(void *data);
INT8U         OS_TCBInit (INT8U  prio, OS_STK  * ptos, OS_STK * pbos, INT16U  id, INT32U  stk_size, void * pext, INT16U  opt) ;

//FUNC Prototype=====================================
void          OSInitHookBegin(void);
void          OSInitHookEnd(void);
void          OSIntCtxSw(void);
void          OSStartHighRdy(void);
void          OSTaskCreateHook(OS_TCB *ptcb);
void          OSTaskDelHook(OS_TCB *ptcb);
void          OSTaskIdleHook(void);
void          OSTaskStatHook(void);
OS_STK       *OSTaskStkInit (void (* task)(void *pd) , void * pdata1, OS_STK *  ptos, INT16U  opt);
void          OSTaskSwHook(void);
void          OSTCBInitHook(OS_TCB *ptcb);
void          OSTimeTickHook(void);

void          OSCtxSw(void);
void          OSTickISR(void);

#endif // __UCOS_II_H__
