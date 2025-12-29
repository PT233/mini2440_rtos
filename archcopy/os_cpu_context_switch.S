.equ BUSWIDTH, 32            // Memory bus width
    .include "2440addr.inc" 
    .include "memcfg.inc"

    .equ NO_IRQ_FIQ, 0xC0        // Disable IRQ and FIQ
    .equ SVC_MODE,   0x13        // Supervisor mode
    .equ IRQ_MODE,   0x12        // Interrupt mode (IRQ mode)

    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore
    .global OSStartHighRdy
    .global OSCtxSw
    .global OSIntCtxSw
    .global OSTickISR           // System tick interrupt entry

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
    mrs     r0, cpsr            // Read CPSR into r0
    orr     r1, r0, #NO_IRQ_FIQ 
    msr     cpsr_c, r1          // Write back CPSR
    mov     pc, lr              // Return

//OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
OS_CPU_SR_Restore:
    msr     cpsr_c, r0          // Restore passed cpu_sr (r0) to CPSR
    mov     pc, lr              // Return


OSStartHighRdy:
    // 1. Hook function
    // bl OSTaskSwHook

    // 2. OSRunning = true
    ldr     r0, =OSRunning
    mov     r1, #1
    strb    r1, [r0]

    // 3. Get TCB address of highest-priority task
    ldr     r0, =OSTCBHighRdy
    ldr     r0, [r0]            // r0 = ptcb
    ldr     sp, [r0]            // sp = ptcb->OSTCBStackPtr

    // 4. Restore full context
    ldmfd   sp!, {r0}           // Pop SPSR 
    msr     cpsr_c, r0          // Restore CPSR 
    
    ldmfd   sp!, {r0-r12, lr, pc} // Pop registers and jump to task entry


OSCtxSw:
    // 1. Save current task context
    stmfd   sp!, {lr}           // Push PC
    stmfd   sp!, {lr}           // Push LR
    stmfd   sp!, {r0-r12}       // Push general-purpose registers
    mrs     r0, cpsr            // Read CPSR
    stmfd   sp!, {r0}           // Push CPSR

    // 2. Save SP to current TCB
    ldr     r0, =OSTCBCur       // Load address of OSTCBCur
    ldr     r1, [r0]            // r1 = OSTCBCur
    str     sp, [r1]            // OSTCBCur->OSTCBStackPtr = sp

    // 3. Hook function
    // bl OSTaskSwHook

    // 4. Switch TCB pointer: OSTCBCur = OSTCBHighRdy
    ldr     r2, =OSTCBHighRdy
    ldr     r3, [r2]            // r3 = OSTCBHighRdy
    str     r3, [r0]            // OSTCBCur = r3

    // 5. Switch priority variables
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    // 6. Get new task SP
    ldr     sp, [r3]            // sp = OSTCBHighRdy->OSTCBStackPtr

    // 7. Restore new task context
    ldmfd   sp!, {r0}           // Pop CPSR
    msr     cpsr_c, r0          // Restore state
    ldmfd   sp!, {r0-r12, lr, pc} // Switch to task


OSIntCtxSw:
    // 1. Call hook
    // bl OSTaskSwHook

    // 2. Switch TCB pointer
    ldr     r0, =OSTCBCur
    ldr     r1, =OSTCBHighRdy
    ldr     r2, [r1]
    str     r2, [r0]            // OSTCBCur = OSTCBHighRdy

    // 3. Switch priority variables
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    // 4. Switch stack pointer
    ldr     sp, [r2]            // sp = OSTCBHighRdy->OSTCBStackPtr

    // 5. Restore new task context
    ldmfd   sp!, {r0}           // Pop CPSR
    msr     cpsr_c, r0          
    ldmfd   sp!, {r0-r12, lr, pc} // Switch to task


OSTickISR:
    
    // 1. Adjust return address
    sub     lr, lr, #4

    // 2. Temporarily save registers to IRQ stack
    stmfd   sp!, {r0-r3, r12, lr}
    
    // 3. Save SPSR (the CPSR before interrupt)
    mrs     r0, spsr
    
    // 4. Prepare to switch to SVC mode
    add     sp, sp, #(6*4)      // Restore IRQ stack pointer
    
    // 5. Switch to SVC mode
    msr     cpsr_c, #(SVC_MODE | NO_IRQ_FIQ) 
    
    // 6. Re-push those registers
    stmfd   sp!, {lr}           // Save LR in SVC mode
    stmfd   sp!, {lr}           // Placeholder
    stmfd   sp!, {r0-r12}       // Save general-purpose registers
    stmfd   sp!, {r0}           // Save SPSR

    
    ldr     r0, =OSIntNesting   // OSIntNesting++
    ldrb    r1, [r0]
    add     r1, r1, #1
    strb    r1, [r0]

    cmp     r1, #1              // If nesting level > 1, it's a nested interrupt
    bne     _Int_Nested

    // If it's the first-level interrupt, save current SP to TCB
    ldr     r0, =OSTCBCur
    ldr     r1, [r0]
    str     sp, [r1]

_Int_Nested:
    // Clear Timer4 interrupt
    ldr     r0, =SRCPND     // rSRCPND
    ldr     r1, =(1<<14)
    str     r1, [r0]
    ldr     r0, =INTPND     // rINTPND
    str     r1, [r0]

    bl      OSTimeTick
    bl      OSIntExit
    
    ldmfd   sp!, {r0}              // Restore SPSR
    msr     spsr_cxsf, r0       
    ldmfd   sp!, {r0-r12, lr, pc}^ // Exception return