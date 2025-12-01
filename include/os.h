#ifndef __OS_H__
#define __OS_H__

#include "include.h"
#include "os_config.h"

#define  OS_VERSION              252                    /* Version of uC/OS-II (Vx.yy mult. by 100)    */


#define  OS_EXT  extern


#ifndef  FALSE
#define  FALSE                     0
#endif

#ifndef  TRUE
#define  TRUE                      1
#endif

#define  OS_PRIO_SELF           0xFF                    /* Indicate SELF priority                      */


#define  OS_N_SYS_TASKS            3                    /* Number of system tasks                      */


#define  OS_STAT_PRIO       (OS_LOWEST_PRIO - 1)        /* Statistic task priority                     */
#define  OS_IDLE_PRIO       (OS_LOWEST_PRIO)            /* IDLE      task priority                     */

#define  OS_EVENT_TBL_SIZE ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of event table                         */
#define  OS_RDY_TBL_SIZE   ((OS_LOWEST_PRIO) / 8 + 1)   /* Size of ready table                         */

#define  OS_TASK_IDLE_ID       65535                    /* I.D. numbers for Idle and Stat tasks        */
#define  OS_TASK_STAT_ID       65534

#define  OS_EVENT_EN       (((OS_Q_EN > 0) && (OS_MAX_QS > 0)) || (OS_MBOX_EN > 0) || (OS_SEM_EN > 0) || (OS_MUTEX_EN > 0))

/*$PAGE*/
/*
*********************************************************************************************************
*                              TASK STATUS (Bit definition for OSTCBStat)
*********************************************************************************************************
*/
#define  OS_STAT_RDY            0x00        /* Ready to run                                            */
#define  OS_STAT_SEM            0x01        /* Pending on semaphore                                    */
#define  OS_STAT_MBOX           0x02        /* Pending on mailbox                                      */
#define  OS_STAT_Q              0x04        /* Pending on queue                                        */
#define  OS_STAT_SUSPEND        0x08        /* Task is suspended                                       */
#define  OS_STAT_MUTEX          0x10        /* Pending on mutual exclusion semaphore                   */
#define  OS_STAT_FLAG           0x20        /* Pending on event flag group                             */

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
#define  OS_FLAG_WAIT_CLR_ALL      0        /* Wait for ALL    the bits specified to be CLR (i.e. 0)   */ 
#define  OS_FLAG_WAIT_CLR_AND      0

#define  OS_FLAG_WAIT_CLR_ANY      1        /* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_OR       1

#define  OS_FLAG_WAIT_SET_ALL      2        /* Wait for ALL    the bits specified to be SET (i.e. 1)   */ 
#define  OS_FLAG_WAIT_SET_AND      2

#define  OS_FLAG_WAIT_SET_ANY      3        /* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_OR       3


#define  OS_FLAG_CONSUME        0x80        /* Consume the flags if condition(s) satisfied             */


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


/*
*   OS_TCB Task Control Block
*   去掉了ext
*/
typedef struct os_tcb{
    OS_STACK        *OSTCBStackPtr;             //指向当前任务栈的栈顶指针
    //双向链表
    struct os_tcb   *OSTCBPrevPtr;              //指向上一个TCB任务控制块的指针
    struct os_tcb   *OSTCBNextPtr;              //指向下一个TCB任务控制块的指针
    //OS_EVENT 还没定义
    OS_EVENT        *OSTCBEventPtr;             //指向任务所等待的事件控制块的指针
    void            *OSTCBMessage;              //从邮箱或者消息队列中的消息
    //OS_FLAG_NODE 还没定义
    OS_FLAG_NODE    *OSTCBFlagNode;          //指向时间标志节点的指针
    OS_FLAGS         OSTCBFlagsReady;           //任务就绪态的事件标志
    INT16U           OSTCBDelay;                //延迟任务的节拍数，或者等待事件的超时
    INT8U            OSTCBPrio;                 //任务优先级
    INT8U            OSTCBStatus;               //任务状态
    INT8U            OSTCB_X;                   //任务优先级的组中的位位置，在os_task.c中
    INT8U            OSTCB_Y;                   //任务优先级在组中的就绪表索引
    INT8U            OSTCBBit_X;             //访问就绪表中的位位置的位掩码
    INT8U            OSTCBBit_Y;             //访问就绪表中的位位置的位掩码
    BOOLEAN          OSTCBDeleteReq;            //任务等待事件的返回值
}OS_TCB;

extern INT8U         OSTaskCtr;                       //已创建的任务数

extern INT8U         OSReadyGroup;                        //就绪列表组
extern INT8U         OSReadyTable[OS_RDY_TBL_SIZE];       //准备运行的任务表

extern OS_TCB       *OSTCBCurrent; //指向当前任务的TCB
extern OS_TCB       *OSTCBHighRdy; //指向最高优先级就绪任务的TCB
extern OS_TCB       *OSTCBFreeList; //指向空闲TCB列表的头指针
extern OS_TCB       *OSTCBList;     //指向所有任务TCB列表的头指针
extern OS_TCB       *OSTCBPrioTable[OS_LOWEST_PRIO + 1]; //任务控制块指针数组
extern OS_TCB        OSTCBTable[OS_MAX_TASKS + OS_N_SYS_TASKS]; //任务控制块数组






#endif // __OS_H__
