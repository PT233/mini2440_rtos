//对应OS_CFG.H文件
#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__





/*
*********************************************************************************************************
*                                         uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define OS_MAX_EVENTS             1    /* 应用程序中事件控制块的最大数量 ...  */
                                       /* ... 必须 > 0                                              */
#define OS_MAX_FLAGS              1    /* 应用程序中事件标志组的最大数量 ...  */
                                       /* ... 必须 > 0                                              */
#define OS_MAX_MEM_PART           1    /* 内存分区的最大数量 ...                         */
                                       /* ... 必须 > 0                                              */
#define OS_MAX_QS                 1    /* 应用程序中队列控制块的最大数量 ...  */
                                       /* ... 必须 > 0                                              */
#define OS_MAX_TASKS              3   /* 应用程序中任务的最大数量 ...                 */
									

#define OS_LOWEST_PRIO            8    /* 定义可以分配的最低优先级 ...         */
                                       /* ... 绝不能高于 63！                            */

#define OS_TASK_IDLE_STK_SIZE    64    /* 空闲任务堆栈大小 (OS_STK 宽条目的数量)              */

#define OS_TASK_STAT_EN           1    /* 启用 (1) 或禁用 (0) 统计任务                 */
#define OS_TASK_STAT_STK_SIZE   512    /* 统计任务堆栈大小 (OS_STK 宽条目的数量)        */

#define OS_ARG_CHK_EN             1    /* 启用 (1) 或禁用 (0) 参数检查                  */
#define OS_CPU_HOOKS_EN           1    /* uC/OS-II 钩子位于处理器移植文件中         */


                                       /* ----------------------- 事件标志 ------------------------ */
#define OS_FLAG_EN                1    /* 启用 (1) 或禁用 (0) 事件标志的代码生成    */
#define OS_FLAG_WAIT_CLR_EN       1    /* 包含等待清除事件标志的代码                   */
#define OS_FLAG_ACCEPT_EN         1   /*     包含 OSFlagAccept() 的代码                          */
#define OS_FLAG_DEL_EN            1   /*     包含 OSFlagDel() 的代码                             */
#define OS_FLAG_QUERY_EN          1   /*     包含 OSFlagQuery() 的代码                           */


                                       /* -------------------- 消息邮箱 --------------------- */
#define OS_MBOX_EN                1   /* 启用 (1) 或禁用 (0) 邮箱的代码生成      */
#define OS_MBOX_ACCEPT_EN         1   /*     包含 OSMboxAccept() 的代码                          */
#define OS_MBOX_DEL_EN            1   /*     包含 OSMboxDel() 的代码                             */
#define OS_MBOX_POST_EN           1   /*     包含 OSMboxPost() 的代码                            */
#define OS_MBOX_POST_OPT_EN       1   /*     包含 OSMboxPostOpt() 的代码                         */
#define OS_MBOX_QUERY_EN          1   /*     包含 OSMboxQuery() 的代码                           */


                                       /* --------------------- 内存管理 -------------------- */
#define OS_MEM_EN                 1   /* 启用 (1) 或禁用 (0) 内存管理器的代码生成 */
#define OS_MEM_QUERY_EN           1   /*     包含 OSMemQuery() 的代码                            */


                                       /* ---------------- 互斥型信号量 --------------- */
#define OS_MUTEX_EN               1  /* 启用 (1) 或禁用 (0) 互斥量的代码生成          */
#define OS_MUTEX_ACCEPT_EN        1   /*     包含 OSMutexAccept() 的代码                         */
#define OS_MUTEX_DEL_EN           1   /*     包含 OSMutexDel() 的代码                            */
#define OS_MUTEX_QUERY_EN         1   /*     包含 OSMutexQuery() 的代码                          */


                                       /* ---------------------- 消息队列 ---------------------- */
#define OS_Q_EN                   1   /* 启用 (1) 或禁用 (0) 队列的代码生成         */
#define OS_Q_ACCEPT_EN            1   /*     包含 OSQAccept() 的代码                             */
#define OS_Q_DEL_EN               1  /*     包含 OSQDel() 的代码                                */
#define OS_Q_FLUSH_EN             1   /*     包含 OSQFlush() 的代码                              */
#define OS_Q_POST_EN              1   /*     包含 OSQPost() 的代码                               */
#define OS_Q_POST_FRONT_EN        1   /*     包含 OSQPostFront() 的代码                          */
#define OS_Q_POST_OPT_EN          1   /*     包含 OSQPostOpt() 的代码                            */
#define OS_Q_QUERY_EN             1   /*     包含 OSQQuery() 的代码                              */


                                       /* ------------------------ 信号量 ------------------------ */
#define OS_SEM_EN                 1   /* 启用 (1) 或禁用 (0) 信号量的代码生成     */
#define OS_SEM_ACCEPT_EN          1   /*    包含 OSSemAccept() 的代码                            */
#define OS_SEM_DEL_EN             1   /*    包含 OSSemDel() 的代码                               */
#define OS_SEM_QUERY_EN           1   /*    包含 OSSemQuery() 的代码                             */


                                       /* --------------------- 任务管理 ---------------------- */
#define OS_TASK_CHANGE_PRIO_EN    1   /*     包含 OSTaskChangePrio() 的代码                      */
#define OS_TASK_CREATE_EN         1    /*     包含 OSTaskCreate() 的代码                          */
#define OS_TASK_CREATE_EXT_EN     1   /*     包含 OSTaskCreateExt() 的代码                       */
#define OS_TASK_DEL_EN            1   /*     包含 OSTaskDel() 的代码                             */
#define OS_TASK_SUSPEND_EN        1   /*     包含 OSTaskSuspend() 和 OSTaskResume() 的代码      */
#define OS_TASK_QUERY_EN          1   /*     包含 OSTaskQuery() 的代码                           */


                                       /* --------------------- 时间管理 ---------------------- */
#define OS_TIME_DLY_HMSM_EN       1    /*     包含 OSTimeDlyHMSM() 的代码                         */
#define OS_TIME_DLY_RESUME_EN     1    /*     包含 OSTimeDlyResume() 的代码                       */
#define OS_TIME_GET_SET_EN        1    /*     包含 OSTimeGet() 和 OSTimeSet() 的代码             */


                                       /* ---------------------- 其他 ----------------------- */
#define OS_SCHED_LOCK_EN          1    /*     包含 OSSchedLock() 和 OSSchedUnlock() 的代码       */


#define OS_TICKS_PER_SEC        50    /* 设置一秒钟内的节拍数                        */

#endif // __OS_CONFIG_H__
