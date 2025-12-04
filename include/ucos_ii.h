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

#define  OS_EVENT_EN       (((OS_Q_EN > 0) && (OS_MAX_QS > 0)) || (OS_MBOX_EN > 0) || (OS_SEM_EN > 0) || (OS_MUTEX_EN > 0))


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
    INT8U           OSEventTable[OS_EVENT_TBL_SIZE]; // 任务等待此事件的就绪表                
}OS_EVENT;

//EVENT FLAGS CONTROL BLOCK=========================

//MESSAGE MAILBOX DATA==============================

//MEMORY PARTITION DATA STRUCTURES==================

//MUTUAL EXCLUSION SEMAPHORE DATA===================

//MESSAGE QUEUE DATA================================

//SEMAPHORE DATA====================================

//TASK STACK DATA===================================
typedef struct {
    INT32U  OSFree;                    // 堆栈上的空闲字节数                            
    INT32U  OSUsed;                    // 堆栈上已使用的字节数                            
} OS_STK_DATA;

//TASK CONTROL BLOCK================================
typedef struct os_tcb{
    OS_STACK        *OSTCBStkPtr;               //指向当前任务栈的栈顶指针
    //双向链表
    struct os_tcb   *OSTCBPrev;                 //指向上一个TCB任务控制块的指针
    struct os_tcb   *OSTCBNext;                 //指向下一个TCB任务控制块的指针
    OS_EVENT        *OSTCBEventPtr;             //指向任务所等待的事件控制块的指针
    void            *OSTCBMsg;                  //从邮箱或者消息队列中的消息
    //OS_FLAG_NODE 还没定义
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

extern INT8S             OSCPUUsage;               // CPU 使用率百分比                          
extern INT32U            OSIdleCtrMax;             // 空闲计数器在 1 秒内可以达到的最大值     
extern INT32U            OSIdleCtrRun;             // 运行时空闲计数器在 1 秒内达到的值  
extern BOOLEAN           OSStatRdy;                // 指示统计任务已就绪的标志  
extern OS_STACK          OSTaskStatStk[OS_TASK_STAT_STK_SIZE];      // 统计任务堆栈          

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

extern OS_STACK          OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];      // 空闲任务堆栈                


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


extern  INT8U             OSMapTbl[];                 // 优先级->位掩码 查找表                 
extern  INT8U             OSUnMapTbl[];               // 优先级->索引    查找表                 

//EVENT FLAGS MANAGEMENT=============================
//MESSAGE MAILBOX MANAGEMENT=========================
//MEMORY MANAGEMENT==================================
//MUTUAL EXCLUSION SEMAPHORE MANAGEMENT==============
//MESSAGE QUEUE MANAGEMENT===========================
//SEMAPHORE MANAGEMENT===============================
//TASK MANAGEMENT====================================
//TIME MANAGEMENT====================================
//MISCELLANEOUS======================================
//FUNC Prototype=====================================
void    OSCtxSw(void);
void    OSTickISR(void);//还没定义
//EVENT FLAGS========================================
//MESSAGE MAILBOXES==================================
//MEMORY MANAGEMENT==================================
//MUTUAL EXCLUSION SEMAPHORES========================
//MESSAGE QUEUES=====================================
//SEMAPHORES=========================================
//TASK MANAGEMENT====================================
//TIME MANAGEMENT====================================
//MISCELLANEOUS======================================

#endif // __OS_H__
