        AREA    OS_CPU_CODE, CODE, READONLY
        ARM
        CODE32
        PRESERVE8

; ===================================================
; Export all global symbols
; ===================================================
        EXPORT  OS_CPU_SR_Save
        EXPORT  OS_CPU_SR_Restore
        EXPORT  OSStartHighRdy
        EXPORT  OSCtxSw
        EXPORT  OSIntCtxSw
        EXPORT  OSTickISR

; ===================================================
; Import external symbols (from uCOS-II core)
; ===================================================
        IMPORT  OSTCBCur
        IMPORT  OSTCBHighRdy
        IMPORT  OSPrioCur
        IMPORT  OSPrioHighRdy
        IMPORT  OSIntNesting
        IMPORT  OSRunning
        IMPORT  OSTimeTick
        IMPORT  OSIntEnter
        IMPORT  OSIntExit
        IMPORT  OSTaskSwHook

; ============================================
; S3C2440 Register Address EQU Definitions (ARMASM recognizable)
; ============================================

BWSCON      EQU     0x48000000
BANKCON0    EQU     0x48000004
BANKCON1    EQU     0x48000008
BANKCON2    EQU     0x4800000C
BANKCON3    EQU     0x48000010
BANKCON4    EQU     0x48000014
BANKCON5    EQU     0x48000018
BANKCON6    EQU     0x4800001C
BANKCON7    EQU     0x48000020

REFRESH     EQU     0x48000024
BANKSIZE    EQU     0x48000028
MRSRB6      EQU     0x4800002C
MRSRB7      EQU     0x48000030

; CLOCK
LOCKTIME    EQU     0x4C000000
MPLLCON     EQU     0x4C000004
UPLLCON     EQU     0x4C000008
CLKCON      EQU     0x4C00000C
CLKSLOW     EQU     0x4C000010
CLKDIVN     EQU     0x4C000014

; INTERRUPT
SRCPND      EQU     0x4A000000
INTMOD      EQU     0x4A000004
INTMSK      EQU     0x4A000008
PRIORITY    EQU     0x4A00000C
INTPND      EQU     0x4A000010
INTOFFSET   EQU     0x4A000014
SUBSRCPND   EQU     0x4A000018
INTSUBMSK   EQU     0x4A00001C

; IO PORT
GPFCON      EQU     0x56000050
GPFDAT      EQU     0x56000054
GPBDAT      EQU     0x56000014



; ===================================================
; OS_CPU_SR_Save
; ===================================================
OS_CPU_SR_Save
        MRS     r0, cpsr
        ORR     r1, r0, #0xC0
        MSR     cpsr_c, r1
        BX      lr


; ===================================================
; OS_CPU_SR_Restore
; ===================================================
OS_CPU_SR_Restore
        MSR     cpsr_c, r0
        BX      lr


; ===================================================
; OSStartHighRdy
; ===================================================
OSStartHighRdy

        ; OSRunning = 1
        LDR     r0, =OSRunning
        MOV     r1, #1
        STRB    r1, [r0]

        ; Load highest priority task stack
        LDR     r0, =OSTCBHighRdy
        LDR     r0, [r0]
        LDR     sp, [r0]

        ; Restore SPSR
        LDMFD   sp!, {r1}
        MSR     cpsr_c, r1

        LDMFD   sp!, {r0-r12, lr, pc}


; ===================================================
; OSCtxSw (Task Context Switch)
; ===================================================
OSCtxSw

        STMFD   sp!, {lr}
        STMFD   sp!, {lr}
        STMFD   sp!, {r0-r12}

        MRS     r0, cpsr
        STMFD   sp!, {r0}

        ; Save current SP
        LDR     r0, =OSTCBCur
        LDR     r1, [r0]
        STR     sp, [r1]

        ; Switch TCB
        LDR     r2, =OSTCBHighRdy
        LDR     r3, [r2]
        STR     r3, [r0]

        ; Switch priority variables
        LDR     r0, =OSPrioCur
        LDR     r1, =OSPrioHighRdy
        LDRB    r2, [r1]
        STRB    r2, [r0]

        ; Switch SP to new task
        LDR     sp, [r3]

        ; Restore task CPSR
        LDMFD   sp!, {r0}
        MSR     cpsr_c, r0

        LDMFD   sp!, {r0-r12, lr}
        SUBS    pc, lr, #0


; ===================================================
; OSIntCtxSw (Context Switch in Interrupt)
; ===================================================
OSIntCtxSw

        LDR     r0, =OSTCBCur
        LDR     r1, =OSTCBHighRdy
        LDR     r2, [r1]
        STR     r2, [r0]

        LDR     r0, =OSPrioCur
        LDR     r1, =OSPrioHighRdy
        LDRB    r2, [r1]
        STRB    r2, [r0]

        LDR     sp, [r2]

        LDMFD   sp!, {r0}
        MSR     cpsr_c, r0

        LDMFD   sp!, {r0-r12, lr}
        SUBS    pc, lr, #0


; ===================================================
; OSTickISR (System Clock Tick)
; ===================================================
OSTickISR

        SUB     lr, lr, #4
        STMFD   sp!, {r0-r3, r12, lr}

        MRS     r0, spsr
        STMFD   sp!, {r0}

        ADD     sp, sp, #(6*4)

        MSR     cpsr_c, #(0x13 :OR: 0xC0)

        STMFD   sp!, {lr}
        STMFD   sp!, {lr}
        STMFD   sp!, {r0-r12}
        STMFD   sp!, {r0}

        ; OSIntNesting++
        LDR     r0, =OSIntNesting
        LDRB    r1, [r0]
        ADD     r1, r1, #1
        STRB    r1, [r0]

        CMP     r1, #1
        BNE     _int_nested

        ; Save task SP before interrupt
        LDR     r0, =OSTCBCur
        LDR     r1, [r0]
        STR     sp, [r1]

_int_nested

        ; Clear interrupt flags
        LDR     r0, =SRCPND
        LDR     r1, =(1<<14)
        STR     r1, [r0]

        LDR     r0, =INTPND
        STR     r1, [r0]

        BL      OSTimeTick
        BL      OSIntExit

        ; Restore SPSR
        LDMFD   sp!, {r0}
        MSR     spsr_cxsf, r0

        LDMFD   sp!, {r0-r12, lr}
        SUBS    pc, lr, #0


        LTORG
        END
