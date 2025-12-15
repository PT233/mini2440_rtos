    .syntax unified
    .cpu arm920t
    .thumb
    .thumb_func

    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore
    .global OSStartHighRdy
    .global OSCtxSw
    .global OSIntCtxSw
    .global OSTickISR

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

    #include "2440addr.inc"
    #include "memcfg.inc"

    .text
    .p2align 2

// ===========================================================
//   OS_CPU_SR_Save
// ===========================================================
OS_CPU_SR_Save:
    mrs     r0, cpsr
    orr     r1, r0, #0xC0
    msr     cpsr_c, r1
    bx      lr

// ===========================================================
//   OS_CPU_SR_Restore
// ===========================================================
OS_CPU_SR_Restore:
    msr     cpsr_c, r0
    bx      lr

// ===========================================================
//   OSStartHighRdy
// ===========================================================
OSStartHighRdy:

    ldr     r0, =OSRunning
    mov     r1, #1
    strb    r1, [r0]

    ldr     r0, =OSTCBHighRdy
    ldr     r0, [r0]
    ldr     sp, [r0]

    ldmfd   sp!, {r1}
    msr     cpsr_c, r1

    ldmfd   sp!, {r0-r12, lr}
    subs    pc, lr, #0

// ===========================================================
//   OSCtxSw
// ===========================================================
OSCtxSw:

    stmfd   sp!, {lr}
    stmfd   sp!, {lr}
    stmfd   sp!, {r0-r12}

    mrs     r0, cpsr
    stmfd   sp!, {r0}

    ldr     r0, =OSTCBCur
    ldr     r1, [r0]
    str     sp, [r1]

    ldr     r2, =OSTCBHighRdy
    ldr     r3, [r2]
    str     r3, [r0]

    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    ldr     sp, [r3]

    ldmfd   sp!, {r0}
    msr     cpsr_c, r0

    ldmfd   sp!, {r0-r12, lr}
    subs    pc, lr, #0

// ===========================================================
//   OSIntCtxSw
// ===========================================================
OSIntCtxSw:

    ldr     r0, =OSTCBCur
    ldr     r1, =OSTCBHighRdy
    ldr     r2, [r1]
    str     r2, [r0]

    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]

    ldr     sp, [r2]

    ldmfd   sp!, {r0}
    msr     cpsr_c, r0

    ldmfd   sp!, {r0-r12, lr}
    subs    pc, lr, #0

// ===========================================================
//   OSTickISR
// ===========================================================
OSTickISR:

    sub     lr, lr, #4
    stmfd   sp!, {r0-r3, r12, lr}

    mrs     r0, spsr
    stmfd   sp!, {r0}

    add     sp, sp, #(6*4)

    msr     cpsr_c, #(0x13 | 0xC0)

    stmfd   sp!, {lr}
    stmfd   sp!, {lr}
    stmfd   sp!, {r0-r12}
    stmfd   sp!, {r0}

    ldr     r0, =OSIntNesting
    ldrb    r1, [r0]
    add     r1, r1, #1
    strb    r1, [r0]

    cmp     r1, #1
    bne     _int_nested

    ldr     r0, =OSTCBCur
    ldr     r1, [r0]
    str     sp, [r1]

_int_nested:

    ldr     r0, =SRCPND
    ldr     r1, =(1<<14)
    str     r1, [r0]

    ldr     r0, =INTPND
    str     r1, [r0]

    bl      OSTimeTick
    bl      OSIntExit

    ldmfd   sp!, {r0}
    msr     spsr_cxsf, r0

    ldmfd   sp!, {r0-r12, lr}
    subs    pc, lr, #0

