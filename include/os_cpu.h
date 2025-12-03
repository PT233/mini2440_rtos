//对应OS_CPU.H文件
#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#include "os_datatype.h"

OS_CPU_SR  OS_CPU_SR_Save(void);
void       OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);

void       OSCtxSw(void);
void       OSIntCtxSw(void);
void       OSStartHighRdy(void);

OS_STACK  *OSTaskStkInit(void (*task)(void *p_arg), void *p_arg, OS_STACK *ptos);

//临界段
#define  OS_ENTER_CRITICAL()   (cpu_sr = OS_CPU_SR_Save())// 关闭中断
#define  OS_EXIT_CRITICAL()    (OS_CPU_SR_Restore(cpu_sr))// 恢复中断

#endif // __OS_CPU_H__
