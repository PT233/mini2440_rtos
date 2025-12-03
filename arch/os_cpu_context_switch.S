
    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore
    .global OSCtxSw(void);
    .global OSTickISR(void);//还没实现

//保存当前CPSR到R0，关中断
OS_CPU_SR_Save:
    mrs     r0, cpsr            
    orr     r1, r0, #0xC0       // 保留低6位状态，将FIQ IRQ屏蔽
    msr     cpsr_c, r1          // 将修改后的值写回 CPSR (关中断)
    mov     pc, lr              // 返回调用者地址


//恢复中断
OS_CPU_SR_Restore:
    msr     cpsr_c, r0          // 将 R0 的值直接写回 CPSR (恢复之前的状态)
    mov     pc, lr              // 返回调用者地址
    
//任务级上下文切换
OSCtxSw:
    // 1. 保存当前任务现场 
    stmfd   sp!, {lr}           // 入栈返回地址
    stmfd   sp!, {lr}           // 入栈 LR
    stmfd   sp!, {r0-r12}       // 入栈通用寄存器 
    mrs     r0, cpsr            // 读取 CPSR 
    stmfd   sp!, {r0}           // 入栈 CPSR 

    // 2. 保存 SP 到当前 TCB 
    ldr     r0, =OSTCBCur       // 加载 OSTCBCur 的地址 
    ldr     r1, [r0]            // 读取 OSTCBCur 的值 (指向 TCB) 
    str     sp, [r1]            // 将 SP 保存到 TCB 的第一个成员 (OSTCBStackPtr) 

    // 3. 切换 TCB 指针: OSTCBCur = OSTCBHighRdy 
    ldr     r2, =OSTCBHighRdy   // 加载 OSTCBHighRdy 的地址 
    ldr     r3, [r2]            // 读取 OSTCBHighRdy 的值 
    str     r3, [r0]            // OSTCBCur = OSTCBHighRdy 

    // 4. 切换优先级变量 (可选，为了调试方便) 
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    // 5. 获取新任务的 SP 
    ldr     sp, [r3]            // 从新 TCB 中读取 SP 

    // 6. 恢复新任务现场 
    ldmfd   sp!, {r0}           // 弹出 CPSR
    msr     spsr_cxsf, r0       // 准备恢复 SPSR
    msr     cpsr_cxsf, r0       
    
    ldmfd   sp!, {r0-r12, lr, pc} // 弹出所有寄存器并跳转 
