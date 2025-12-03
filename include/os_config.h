//对应OS_CFG.H文件
#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__





/*
*********************************************************************************************************
*                                         uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define OS_MAX_EVENTS             1    /*应用程序中事件控制块的最大数量*/
                                       /*必须 > 0*/
#define OS_MAX_FLAGS              1    /*应用程序中事件标志组的最大数量*/
                                       /*必须 > 0*/
#define OS_MAX_MEM_PART           1    /*内存分区的最大数量*/
                                       /*必须 > 0*/
#define OS_MAX_QS                 1    /*应用程序中队列控制块的最大数量*/
                                       /*必须 > 0*/
#define OS_MAX_TASKS              3   /*应用程序中任务的最大数量*/
									

#define OS_LOWEST_PRIO            8    /*定义可以分配的最低优先级*/
                                       /*绝不能高于 63*/

#define OS_TASK_IDLE_STK_SIZE    64    /*空闲任务堆栈大小*/

#define OS_TASK_STAT_STK_SIZE   512    /*统计任务堆栈大小*/

#define OS_TICKS_PER_SEC        50    /*设置一秒钟内的节拍数*/

#endif // __OS_CONFIG_H__
