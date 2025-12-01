#include "include.h"

//改变任务优先级

//创建任务
INT8U OSTaskCreate(void task(void *pdata),
                   void *pdata1,
                   OS_STACK *ptos,
                   INT8U prio)
{
    OS_CPU_SR    cpu_sr;
    OS_STACK    *psp;//point to stack pointer
    INT8U        err;
    if(prio > OS_LOWEST_PRIO){
        return (OS_PRIO_INVALID); //优先级错误
    }
    OS_ENTER_CRITICAL();
    if(OSTCBPrioTable[prio] == (OS_TCB *)0){ //优先级不存在
        OSTCBPrioTable[prio] = (OS_TCB *)1; //先占位，防止重复创建同一优先级任务
        OS_EXIT_CRITICAL();
        psp = (OS_STACK *)OSTaskStackInit(task, pdata1, ptos, 0);//还没定义OSTaskStackInit
        err = OS_TCBInit(prio, psp);
        if(err == OS_NO_ERR){
            OS_ENTER_CRITICAL();
            OSTaskCtr++;
            OS_EXIT_CRITICAL();
            if(OSRunning == TRUE){//还没定义OSRunning
                OS_Sched(); //还没定义OS_Sched
            }
        }
        else {
            OS_ENTER_CRITICAL();
            OSTCBPrioTable[prio] = (OS_TCB *)0; //释放优先级占位
            OS_EXIT_CRITICAL();
        }
        return (err);
    }
    OS_EXIT_CRITICAL();
    return (OS_PRIO_EXIST); //优先级已经存在
}
//删除任务

//请求任务删除自身

//恢复挂起的任务

//任务堆栈检查

//任务挂起

//查询任务