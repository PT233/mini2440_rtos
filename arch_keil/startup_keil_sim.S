        AREA    STARTUP, CODE, READONLY
        ARM
        CODE32
        PRESERVE8

; ========================================================
; Mini2440 RTOS - Keil Simulator Startup Only
; No CP15, No Clock, No SDRAM init (simulator emulates it)
; Vector Table at 0x00000000
; Code linked to 0x00000000 (simulator address space)
; ========================================================

        EXPORT  _start
        EXPORT  Reset_Handler

        IMPORT  main
        IMPORT  OSTickISR
        IMPORT  OSStartHighRdy

; ========================================================
; Exception Vector Table (at 0x00000000)
; ========================================================

_start
        B       Reset_Handler        ; Reset
        B       Undefined_Handler
        B       SWI_Handler
        B       Prefetch_Handler
        B       Abort_Handler
        B       .
        B       IRQ_Handler          ; IRQ
        B       FIQ_Handler

; ========================================================
; Exception Handlers
; ========================================================
Undefined_Handler
        B       Undefined_Handler
SWI_Handler
        B       SWI_Handler
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
FIQ_Handler
        B       FIQ_Handler

; ========================================================
; IRQ Entry to OSTickISR
; ========================================================
IRQ_Handler
        B       OSTickISR

; ========================================================
; Reset Handler (Simulator - No Hardware Init)
; ========================================================
Reset_Handler

; Mode Masks
USERMODE        EQU     0x10
FIQMODE         EQU     0x11
IRQMODE         EQU     0x12
SVCMODE         EQU     0x13
ABORTMODE       EQU     0x17
UNDEFMODE       EQU     0x1B
MODEMASK        EQU     0x1F

; Stack Layout (for simulator, use IRAM: 0x40000000-0x40001000, 4KB total)
STACK_BASE      EQU     0x40000FFF   ; Top of 4KB IRAM (0x40000FFF is end)

UndefStack      EQU     0x40000F00
AbortStack      EQU     0x40000E00
FIQStack        EQU     0x40000D00
IRQStack        EQU     0x40000C00
SVCStack        EQU     0x40000B00   ; Main stack
UserStack       EQU     0x40000A00

; ========================
; Set SVC Stack (main)
; ========================
        MRS     r0, cpsr
        BIC     r0, r0, #MODEMASK
        ORR     r0, r0, #SVCMODE
        MSR     cpsr_c, r0
        LDR     sp, =SVCStack

; ========================
; Jump to main()
; ========================
        BL      main

Stop_Here
        B       Stop_Here

        END
