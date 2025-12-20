/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : uCOS_II.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/


#include "includes.h"


// #define  OS_MASTER_FILE                       // Prevent the following files from including includes.h
// #include "os_core.c"
// #include "os_flag.c"
// #include "os_mbox.c"
// #include "os_mem.c"
// #include "os_mutex.c"
// #include "os_q.c"
// #include "os_sem.c"
// #include "os_task.c"
// #include "os_time.c"

// Global Variables

volatile INT32U  OSTime           = 0L;                          // System time in clock ticks

INT8U            OSIntNesting     = 0;                           // ISR nesting level
INT8U            OSIntExitY       = 0;                           // Interrupt exit Y value

INT8U            OSLockNesting    = 0;                           // Scheduler lock nesting level

INT8U            OSPrioCur        = 0;                           // Current task priority
INT8U            OSPrioHighRdy    = 0;                           // Highest ready task priority

INT8U            OSRdyGrp         = 0;                           // Ready list group
INT8U            OSRdyTbl[OS_RDY_TBL_SIZE];                      // Ready list table

BOOLEAN          OSRunning        = FALSE;                       // Kernel running flag

INT8U            OSTaskCtr        = 0;                           // Number of tasks created
INT32U           OSCtxSwCtr       = 0;                           // Context switch counter

volatile INT32U  OSIdleCtr        = 0L;                          // Idle counter
INT32U           OSIdleCtrRun     = 0L;                          // Idle counter run value
INT32U           OSIdleCtrMax     = 0L;                          // Idle counter max value

BOOLEAN          OSStatRdy        = FALSE;                       // Statistics task ready flag

INT8U            OSCPUUsage       = 0;                           // CPU usage percentage

OS_STK           OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];           // Idle task stack
OS_STK           OSTaskStatStk[OS_TASK_STAT_STK_SIZE];           // Statistics task stack

OS_TCB          *OSTCBCur         = (OS_TCB *)0;                 // Pointer to current task
OS_TCB          *OSTCBFreeList    = (OS_TCB *)0;                 // Pointer to free TCB list
OS_TCB          *OSTCBHighRdy     = (OS_TCB *)0;                 // Pointer to highest ready TCB
OS_TCB          *OSTCBList        = (OS_TCB *)0;                 // Pointer to TCB list
OS_TCB          *OSTCBPrioTbl[OS_LOWEST_PRIO + 1];               // Priority table of TCBs
OS_TCB           OSTCBTbl[OS_MAX_TASKS + OS_N_SYS_TASKS];        // TCB table

OS_EVENT        *OSEventFreeList  = (OS_EVENT *)0;               // Pointer to free event list
OS_EVENT         OSEventTbl[OS_MAX_EVENTS];                      // Event control block table

OS_FLAG_GRP     *OSFlagFreeList   = (OS_FLAG_GRP *)0;            // Pointer to free flag group list
OS_FLAG_GRP      OSFlagTbl[OS_MAX_FLAGS];                        // Flag group table

OS_MEM          *OSMemFreeList    = (OS_MEM *)0;                 // Pointer to free memory list
OS_MEM           OSMemTbl[OS_MAX_MEM_PART];                      // Memory partition table

OS_Q            *OSQFreeList      = (OS_Q *)0;                   // Pointer to free queue list
OS_Q             OSQTbl[OS_MAX_QS];                              // Queue table
