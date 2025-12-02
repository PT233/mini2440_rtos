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
INT8U OSTaskDelete(INT8U prio)
{
    OS_CPU_SR         cpu_sr;
    OS_EVENT         *pevent;
    OS_FLAG_NODE     *pnode;
    OS_TCB           *ptcb;
    BOOLEAN           self;

    if(OSIntNesting > 0){ //中断嵌套
        return (OS_ERR_DEL_ISR); //不能在中断中删除任务
    }
    if(prio == OS_IDLE_PRIO){
        return (OS_ERR_DEL_IDLE); //不能删除空闲任务
    }
    if(prio > OS_LOWEST_PRIO && prio != OS_PRIO_SELF){
        return (OS_PRIO_INVALID); //优先级错误
    }
    OS_ENTER_CRITICAL();
    if(prio == OS_PRIO_SELF){ //删除当前任务
        prio = OSTCBCurrent->OSTCBPrio; //获取当前任务的优先级
    }
    ptcb = OSTCBPrioTable[prio]; //获取任务控制块优先级
    if(ptcb != (OS_TCB *)0){ //任务存在
        if((OSReadyTabale[ptcb->OSTCB_Y] &= ptcb->OSTCBBit_X) == 0){
            OSReadyGroup &= ~ptcb->OSTCBBit_Y; //从任务就绪组中删除
        }
        pevent = ptcb->OSTCBEventPtr; //获取任务等待的事件控制块
        if(pevent != (OS_EVENT *)0){ //任务正在等待某个事件
            if((pevent->OSReadyTabale[ptcb->OSTCB_Y] &= ptcb->OSTCBBit_X) == 0){
                pevent->OSReadyGroup &= ~ptcb->OSTCBBit_Y; //从事件就绪组中删除
            }
        }
        pnode = ptcb->OSTCBFlagNode; //获取任务等待的事件标志节点
        if(pnode != (OS_FLAG_NODE *)0){ //任务正在等待事件标志
            OS_FlagUnlink(pnode); //还没定义OS_FlagUnlink, 从node表中删除
        }
        ptcb->OSTCBDelay = 0; //清除任务延迟
        ptcb->OSTCBStatus = OS_STAT_RDY; //设置任务状态为就绪
        if(OSLockNesting < 255){ //调度没有被锁定
            OSLockNesting++;
        }
        OS_EXIT_CRITICAL();
        OS_Dummy(); // 空指令
        OS_ENTER_CRITICAL();
		if (OSLockNesting > 0) {
            OSLockNesting--;
		}
        OSTaskDeleteHook(ptcb); //调用用户定义的钩子函数
        OSTaskCtr--;
        OSTCBPrioTable[prio] = (OS_TCB *)0; //从优先级表中删除该任务
        if (ptcb->OSTCBPrevPtr == (OS_TCB *)0) {                   /* 从 TCB 链表中移除               */
            ptcb->OSTCBNextPtr->OSTCBPrevPtr = (OS_TCB *)0;
            OSTCBList                  = ptcb->OSTCBNextPtr;
        } else {
            ptcb->OSTCBPrevPtr->OSTCBNextPtr = ptcb->OSTCBNextPtr;
            ptcb->OSTCBNextPtr->OSTCBPrevPtr = ptcb->OSTCBPrevPtr;
        }
        ptcb->OSTCBNextPtr = OSTCBFreeList;                      /* 将该 TCB 加入到空闲 TCB 链表中 */
        OSTCBFreeList     = ptcb;
        OS_EXIT_CRITICAL();
        OS_Sched(); //还没定义OS_Sched
        return (OS_NO_ERR); //没有错误
    }
    OS_EXIT_CRITICAL();
    return(OS_TASK_DEL_ERR); //任务删除错误
}
//请求任务删除
INT8U OSTaskDelReq(INT8U prio)
{
    OS_CPU_SR   cpu_sr;
    OS_TCB     *ptcb;
    BOOLEAN     stat;
    INT8U       err;

    if (prio == OS_IDLE_PRIO) {                                 /* 不允许删除空闲任务     */
        return (OS_TASK_DEL_IDLE);
    }
    if (prio >= OS_LOWEST_PRIO && prio != OS_PRIO_SELF) {       /* 任务优先级有效 ?               */
        return (OS_PRIO_INVALID);
    }
     if (prio == OS_PRIO_SELF) {                                 /* 查看是否有任务请求 ...  */
        OS_ENTER_CRITICAL();                                    /* ... 此任务删除自身      */
        stat = OSTCBCurrent->OSTCBDeleteReq;                           /* 将请求状态返回给调用者     */
        OS_EXIT_CRITICAL();
        return (stat);
    }
    OS_ENTER_CRITICAL();
    ptcb = OSTCBPrioTable[prio];
    if (ptcb != (OS_TCB *)0) {                                  /* 要删除的任务必须存在           */
        ptcb->OSTCBDeleteReq = OS_TASK_DEL_REQ;                    /* 设置标志指示任务将被删除 */
        err                  = OS_NO_ERR;
    } else {
        err                  = OS_TASK_NOT_EXIST;                  /* 任务必须已被删除                */
    }
    OS_EXIT_CRITICAL();
    return (err); 
}
//恢复挂起的任务

//任务堆栈检查
INT8U OSTaskStackCheck(INT8U prio, OS_STK_DATA *pata)
{
//不需要写检查，因为没有定义taskcreateext函数，没有相应的参数返回size
}
//任务挂起

//查询任务