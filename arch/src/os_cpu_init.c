#include "includes.h"

//OS 初始化钩子开始
void  OSInitHookBegin (void)
{
}


//OS 初始化钩子结束
void  OSInitHookEnd (void)
{
}

//任务创建钩子
void  OSTaskCreateHook (OS_TCB *ptcb)
{
    printf("Task %d Created successfully!\n", ptcb->OSTCBPrio);
}

//任务删除钩子
void  OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning*/
    printf("Task %d Deleted successfully!\n", ptcb->OSTCBPrio);
}


//空闲任务钩子
void  OSTaskIdleHook (void)
{
}


//统计任务钩子
void  OSTaskStatHook (void)
{
}

OS_STACK * OSTaskStkInit (void (* task)(void *p_arg) , void * p_arg, OS_STACK *  ptos)
{
    OS_STACK * stk;

    stk = ptos;                         /* Load stack pointer */

    /* Registers: PC, LR, R12-R0, CPSR */
    *stk = (OS_STACK)task;              /* PC */
    *(--stk) = (OS_STACK)task;          /* LR */
    *(--stk) = 0;                       /* R12 */
    *(--stk) = 0;                       /* R11 */
    *(--stk) = 0;                       /* R10 */
    *(--stk) = 0;                       /* R9 */
    *(--stk) = 0;                       /* R8 */
    *(--stk) = 0;                       /* R7 */
    *(--stk) = 0;                       /* R6 */
    *(--stk) = 0;                       /* R5 */
    *(--stk) = 0;                       /* R4 */
    *(--stk) = 0;                       /* R3 */
    *(--stk) = 0;                       /* R2 */
    *(--stk) = 0;                       /* R1 */
    *(--stk) = (OS_STACK)p_arg;         /* R0 */
    *(--stk) = (OS_STACK)0x00000013;    /* CPSR (SVC mode, Enable IRQ/FIQ) */

    return (stk);
}



//TASK SWITCH HOOK
void  OSTaskSwHook (void)
{
}


//OSTCBInit() HOOK
void  OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}



//TICK HOOK
void  OSTimeTickHook (void)
{
}

