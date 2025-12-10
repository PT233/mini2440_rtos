
    .include "2440addr.inc" 
    .include "memcfg.inc"

    .equ NO_IRQ_FIQ, 0xC0        // 禁止 IRQ 和 FIQ
    .equ SVC_MODE,   0x13        // 管理模式 (Supervisor Mode)
    .equ IRQ_MODE,   0x12        // 中断模式 (IRQ Mode)

    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore
    .global OSStartHighRdy
    .global OSCtxSw
    .global OSIntCtxSw
    .global OSTickISR           // 系统滴答中断入口

    .extern OSTCBCur
    .extern OSTCBHighRdy
    .extern OSPrioCur
    .extern OSPrioHighRdy
    .extern OSIntNesting
    .extern OSRunning
    .extern OSTimeTick
    .extern OSIntEnter
    .extern OSIntExit
    .extern OSTaskSwHook

    .text
    .code 32
    .align 2



// OS_CPU_SR OS_CPU_SR_Save(void);
OS_CPU_SR_Save:
    mrs     r0, cpsr            // 读取 CPSR 到 R0
    orr     r1, r0, #NO_IRQ_FIQ 
    msr     cpsr_c, r1          // 写回 CPSR
    mov     pc, lr              // 返回

// void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
OS_CPU_SR_Restore:
    msr     cpsr_c, r0          // 将传入的 cpu_sr (R0) 恢复给 CPSR
    mov     pc, lr              // 返回


OSStartHighRdy:
    // 1. 钩子函数
    // bl OSTaskSwHook

    // 2. OSRunning = true
    ldr     r0, =OSRunning
    mov     r1, #1
    strb    r1, [r0]

    // 3. 获取最高优先级任务的 TCB 地址
    ldr     r0, =OSTCBHighRdy
    ldr     r0, [r0]            // R0 = ptcb
    ldr     sp, [r0]            // SP = ptcb->OSTCBStackPtr

    // 4. 恢复全部现场
    ldmfd   sp!, {r0}           // 弹出 SPSR 
    msr     cpsr_c, r0          // 恢复 CPSR 
    
    ldmfd   sp!, {r0-r12, lr, pc} // 弹出寄存器并跳转到任务入口


OSCtxSw:
    // 1. 保存当前任务现场
    stmfd   sp!, {lr}           // 入栈 PC
    stmfd   sp!, {lr}           // 入栈 LR
    stmfd   sp!, {r0-r12}       // 入栈通用寄存器
    mrs     r0, cpsr            // 读取 CPSR
    stmfd   sp!, {r0}           // 入栈 CPSR

    // 2. 保存 SP 到当前 TCB
    ldr     r0, =OSTCBCur       // 加载 OSTCBCur 地址
    ldr     r1, [r0]            // R1 = OSTCBCur
    str     sp, [r1]            // OSTCBCur->OSTCBStackPtr = SP

    // 3. 钩子函数
    // bl OSTaskSwHook

    // 4. 切换 TCB 指针: OSTCBCur = OSTCBHighRdy
    ldr     r2, =OSTCBHighRdy
    ldr     r3, [r2]            // R3 = OSTCBHighRdy
    str     r3, [r0]            // OSTCBCur = R3

    // 5. 切换优先级变量
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    // 6. 获取新任务 SP
    ldr     sp, [r3]            // SP = OSTCBHighRdy->OSTCBStackPtr

    // 7. 恢复新任务现场
    ldmfd   sp!, {r0}           // 弹出 CPSR
    msr     cpsr_c, r0          // 恢复状态
    ldmfd   sp!, {r0-r12, lr, pc} // 切换任务


OSIntCtxSw:
    // 1. 调用钩子
    // bl OSTaskSwHook

    // 2. 切换 TCB 指针
    ldr     r0, =OSTCBCur
    ldr     r1, =OSTCBHighRdy
    ldr     r2, [r1]
    str     r2, [r0]            // OSTCBCur = OSTCBHighRdy

    // 3. 切换优先级变量
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    // 4. 切换堆栈指针
    ldr     sp, [r2]            // SP = OSTCBHighRdy->OSTCBStackPtr

    // 5. 恢复新任务现场
    ldmfd   sp!, {r0}           // 弹出 CPSR
    msr     cpsr_c, r0          
    ldmfd   sp!, {r0-r12, lr, pc} // 切换任务


OSTickISR:
    
    // 1. 修正返回地址
    sub     lr, lr, #4

    // 2. 临时保存寄存器到 IRQ 堆栈
    stmfd   sp!, {r0-r3, r12, lr}
    
    // 3. 保存 SPSR (这是被中断之前的 CPSR)
    mrs     r0, spsr
    
    // 4. 准备切换到 SVC 模式
    add     sp, sp, #(6*4)      // 恢复 IRQ 堆栈指针
    
    // 5. 切换到 SVC 模式
    msr     cpsr_c, #(SVC_MODE | NO_IRQ_FIQ) 
    
    // 6. 重新压入刚才那些寄存器
    stmfd   sp!, {lr}           // 保存 SVC 模式的 LR
    stmfd   sp!, {lr}           // 占位
    stmfd   sp!, {r0-r12}       // 保存通用寄存器
    stmfd   sp!, {r0}           // 保存 SPSR

    
    ldr     r0, =OSIntNesting   // OSIntNesting++
    ldrb    r1, [r0]
    add     r1, r1, #1
    strb    r1, [r0]

    cmp     r1, #1              // 如果嵌套层数 > 1，说明是嵌套中断
    bne     _Int_Nested

    // 如果是第一层中断，保存当前 SP 到 TCB
    ldr     r0, =OSTCBCur
    ldr     r1, [r0]
    str     sp, [r1]

_Int_Nested:
    // 清除 Timer4 中断
    ldr     r0, =SRCPND     // rSRCPND
    ldr     r1, =(1<<14)
    str     r1, [r0]
    ldr     r0, =INTPND     // rINTPND
    str     r1, [r0]

    bl      OSTimeTick
    bl      OSIntExit
    
    ldmfd   sp!, {r0}              // 恢复 SPSR
    msr     spsr_cxsf, r0       
    ldmfd   sp!, {r0-r12, lr, pc}^ // 异常返回