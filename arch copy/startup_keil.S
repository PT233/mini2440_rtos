        AREA    STARTUP, CODE, READONLY
        ARM
        CODE32
        PRESERVE8

; ========================================================
; NAND Boot + SDRAM Init + Vector Table at 0x00000000
; Target: mini2440 (S3C2440) - Hardware Mode
; Stack: 0x33FF0000 based layout
; ========================================================

        EXPORT  _start
        EXPORT  Reset_Handler

        IMPORT  main
        IMPORT  OSTickISR
        IMPORT  OSStartHighRdy

; ========================================================
; Register Definitions
; ========================================================
WTCON       EQU     0x53000000
BWSCON      EQU     0x48000000
BANKCON6    EQU     0x4800001C
REFRESH     EQU     0x48000024
BANKSIZE    EQU     0x48000028
MRSRB6      EQU     0x4800002C
LOCKTIME    EQU     0x4C000000
MPLLCON     EQU     0x4C000004
UPLLCON     EQU     0x4C000008
CLKCON      EQU     0x4C00000C
CLKDIVN     EQU     0x4C000014
INTMSK      EQU     0x4A000008

; ========================================================
; Exception Vector Table
; ========================================================
_start
        B       Reset_Handler
        B       Undefined_Handler
        B       SWI_Handler
        B       Prefetch_Handler
        B       Abort_Handler
        B       .
        B       IRQ_Handler
        B       FIQ_Handler

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

IRQ_Handler
        B       OSTickISR

; ========================================================
; Reset Handler (Hardware Mode)
; ========================================================
Reset_Handler

; Disable Watchdog
        LDR     r0, =WTCON
        MOV     r1, #0
        STR     r1, [r0]

; Disable All Interrupts
        LDR     r0, =INTMSK
        LDR     r1, =0xFFFFFFFF
        STR     r1, [r0]

; Install Vector Table at 0x00000000
        LDR     r0, =0x00000000
        LDR     r1, =0xE59FF018
        MOV     r2, #8
VecLoop
        STR     r1, [r0], #4
        SUBS    r2, r2, #1
        BGT     VecLoop

        LDR     r0, =0x00000020
        LDR     r1, =Reset_Handler
        STR     r1, [r0], #4
        LDR     r1, =Undefined_Handler
        STR     r1, [r0], #4
        LDR     r1, =SWI_Handler
        STR     r1, [r0], #4
        LDR     r1, =Prefetch_Handler
        STR     r1, [r0], #4
        LDR     r1, =Abort_Handler
        STR     r1, [r0], #4
        LDR     r1, =IRQ_Handler
        STR     r1, [r0], #4
        LDR     r1, =FIQ_Handler
        STR     r1, [r0], #4

; Disable CP15 Cache/TLB
        MRC     p15, 0, r0, c1, c0, 0
        BIC     r0, r0, #(1<<12)
        BIC     r0, r0, #(1<<2)
        BIC     r0, r0, #(1<<0)
        MCR     p15, 0, r0, c1, c0, 0
        MOV     r0, #0
        MCR     p15, 0, r0, c8, c7, 0

; ========================================================
; Clock Configuration
; ========================================================
        LDR     r0, =LOCKTIME
        LDR     r1, =0xFFFFFFFF
        STR     r1, [r0]

        LDR     r0, =CLKDIVN
        MOV     r1, #5
        STR     r1, [r0]

        LDR     r0, =UPLLCON
        LDR     r1, =((56<<12)+(2<<4)+2)
        STR     r1, [r0]

        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP

        LDR     r0, =MPLLCON
        LDR     r1, =((127<<12)+(2<<4)+1)
        STR     r1, [r0]

; ========================================================
; SDRAM Initialization
; ========================================================
        LDR     r0, =BWSCON
        LDR     r1, =0x22000000
        STR     r1, [r0]

        LDR     r0, =BANKCON6
        LDR     r1, =0x18001
        STR     r1, [r0]

        LDR     r0, =BANKSIZE
        LDR     r1, =0xB1
        STR     r1, [r0]

        LDR     r0, =REFRESH
        LDR     r1, =0x8404F5
        STR     r1, [r0]

        LDR     r0, =MRSRB6
        LDR     r1, =0x20
        STR     r1, [r0]

; ========================================================
; Check if copy needed: NAND -> SDRAM
; ========================================================
        MOV     r0, pc
        LDR     r1, =0x30000000
        CMP     r0, r1
        BHS     CodeInSDRAM

; Copy code
        LDR     r0, =_start
        LDR     r1, =0x30000000
        LDR     r2, =Image_End
        SUB     r2, r2, r0

CopyLoop
        LDR     r3, [r0], #4
        STR     r3, [r1], #4
        SUBS    r2, r2, #4
        BGT     CopyLoop

        LDR     pc, =CodeInSDRAM

; ========================================================
; Continue in SDRAM
; ========================================================
CodeInSDRAM

STACK_BASE      EQU     0x33FF0000
SVCStack        EQU     0x33FEEC00
USERMODE        EQU     0x10
SVCMODE         EQU     0x13
MODEMASK        EQU     0x1F

        MRS     r0, cpsr
        BIC     r0, r0, #MODEMASK
        ORR     r0, r0, #SVCMODE
        MSR     cpsr_c, r0
        LDR     sp, =SVCStack

        BL      main

Stop
        B       Stop

Image_End
        END
