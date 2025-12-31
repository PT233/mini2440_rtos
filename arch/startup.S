
// Global constants and headers
    .equ BUSWIDTH, 32 
    .include "2440addr.inc" 
    .include "memcfg.inc"


// External symbol declarations
    .extern main

// Section locations for text/data/bss
    .extern _start          // Start of text section (ROM)
    .extern _etext          // End of text section
    .extern _data           // Start of data section
    .extern _bss_start_     // BSS segment start (RAM; data and bss starts are adjacent, so no extra symbol)
    .extern _bss_end_       // BSS segment end

    .global _start          // Linker default entry
    .global my_entry        


// Processor mode definitions

    .equ USERMODE,   0x10
    .equ FIQMODE,    0x11
    .equ IRQMODE,    0x12
    .equ SVCMODE,    0x13
    .equ ABORTMODE,  0x17
    .equ UNDEFMODE,  0x1b
    .equ MODEMASK,   0x1f


// Exception vector table
    .section .text
    .code 32                // ARM state (16 is Thumb; using ARMv4 only)
    .align 2                // 4-byte alignment (2^n)

_start:                     // Standard entry label
my_entry:
    b   reset               // 0x00
    b   undef               // 0x04
    b   swi                 // 0x08
    b   pabort              // 0x0C
    b   dabort              // 0x10
    b   .                   // 0x14 
    b   OSTickISR           // Changed IRQ to OSTickISR; jumps to os_cpu_context_switch.S
    b   fiq                 // 0x1C


// Exception handlers: turn on LEDs on errors; if the program hangs, add LED checks at various points to inspect runtime state

// undef: LED1 on
undef:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x01<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

// swi: LED2 on
swi:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x02<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

// pabort: LED1 and LED2 on
pabort:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x03<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

// dabort: LED3 on
dabort:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x04<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

// irq: LED1 and LED3 on
irq:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    mrs r0, spsr
    stmfd sp!, {r0}

    // LED indicator (optional)
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x05<<5)
    bic r1, r1, r2
    str r1, [r0]

    ldr r0, =INTOFFSET
    ldr r0, [r0]
    ldr r1, =HandlerEINT0   // Address constant must be defined
    add r1, r1, r0, lsl #2
    ldr r1, [r1]
    mov lr, pc
    mov pc, r1

    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    ldmfd sp!, {r0-r12, lr}
    movs pc, lr

// fiq: LED2 and LED3 on
fiq:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x06<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .


// Reset handler

reset:
// 1. Disable watchdog
    ldr r0, =WTCON
    mov r1, #0x0
    str r1, [r0]

// 2. Mask interrupts
    ldr r0, =INTMSK
    ldr r1, =0xffffffff
    str r1, [r0]

    ldr r0, =INTSUBMSK
    ldr r1, =0x7fff
    str r1, [r0]

// 3. Configure clock
    .equ U_MDIV, 56
    .equ U_PDIV, 2
    .equ U_SDIV, 2
    .equ M_MDIV, 127
    .equ M_PDIV, 2
    .equ M_SDIV, 1
    .equ CLKDIVN_VAL, 5 // Directly from datasheet table

    ldr r0, =LOCKTIME
    ldr r1, =0xffffffff
    str r1, [r0]

    // 3.1 Set divider CLKDIVN
    // Must be set before MPLLCON to prevent excessive frequency
    ldr r0, =CLKDIVN
    ldr r1, =CLKDIVN_VAL
    str r1, [r0]

    // 3.2 Set asynchronous bus mode (Async Bus Mode)
    // Must be set when HDIVN != 0
    .if CLKDIVN_VAL > 1
        mrc p15,0,r0,c1,c0,0
        orr r0,r0,#0xc0000000
        mcr p15,0,r0,c1,c0,0
    .else
        mrc p15,0,r0,c1,c0,0
        bic r0,r0,#0xc0000000
        mcr p15,0,r0,c1,c0,0
    .endif

    // 3.3 Configure UPLLCON
    ldr r0, =UPLLCON
    ldr r1, =((U_MDIV<<12)+(U_PDIV<<4)+U_SDIV)
    str r1, [r0]

    // 7 NOPs
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    // 3.4 Configure MPLLCON
    ldr r0, =MPLLCON
    ldr r1, =((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV)
    str r1, [r0]

// 4. Initialize memory controller
    ldr r0, =SMRDATA
    ldr r1, =_start
    sub r0, r0, r1          // r0 = offset of SMRDATA relative to _start
    
    mov r2, pc              // Get current PC
    ldr r3, =0x30000000     // Threshold
    cmp r2, r3
    blo 1f                  // If PC < 0x30000000, running at 0x0; offset equals physical address
    add r0, r0, r3          // If PC >= 0x30000000, running in SDRAM; add base address
1:
    ldr r1, =BWSCON
    add r2, r0, #52         // SMRDATA length 13*4 = 52 bytes

0:                          // Local label
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r2, r0
    bne 0b                  // Backward branch to label 0

// 5. Initialize stacks
    .equ STACK_BASE_ADDRESS, 0x33ff8000
    .equ UserStack,  (STACK_BASE_ADDRESS-0x3800)
    .equ SVCStack,   (STACK_BASE_ADDRESS-0x2800)
    .equ UndefStack, (STACK_BASE_ADDRESS-0x2400)
    .equ AbortStack, (STACK_BASE_ADDRESS-0x2000)
    .equ IRQStack,   (STACK_BASE_ADDRESS-0x1000)
    .equ FIQStack,   (STACK_BASE_ADDRESS-0x0)

    // 5.1 Undef
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #UNDEFMODE
    msr cpsr_c, r0
    ldr sp, =UndefStack

    // 5.2 Abort
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #ABORTMODE
    msr cpsr_c, r0
    ldr sp, =AbortStack

    // 5.3 IRQ
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #IRQMODE
    msr cpsr_c, r0
    ldr sp, =IRQStack

    // 5.4 FIQ
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #FIQMODE
    msr cpsr_c, r0
    ldr sp, =FIQStack
    
    // 5.5 SVC
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #SVCMODE
    msr cpsr_c, r0
    ldr sp, =SVCStack

// 6. Code relocation (SRAM/NOR -> SDRAM)
// Regardless of NAND or NOR boot, our code is at 0x00000000
// (NAND boot uses SRAM; NOR boot uses NOR Flash)
// Copy it to 0x30000000

    // Check current address; if in SDRAM (>=0x30000000), skip copying
    mov r0, pc
    ldr r1, =0x30000000
    cmp r0, r1
    bhs on_sdram            // If PC >= 0x30000000, jump directly to on_sdram

    mov r0, #0                  // Source address 0x00000000
    ldr r1, =_start             // Destination address 0x30000000 (defined in linker.ld)
    ldr r2, =_bss_start_      // End address (BSS segment is empty)
    
    sub r2, r2, r1              // Compute code size (r2 = _bss_start_ - _start)
    
copy_loop:
    ldr r3, [r0], #4
    str r3, [r1], #4
    subs r2, r2, #4
    bgt copy_loop               // Loop until copy completes

    // Jump to code in SDRAM to continue
    ldr pc, =on_sdram
on_sdram:

// 7. Copy initialized data (.data) from ROM image to RAM
    ldr r0, TopOfROM        // r0 = &_etext
    ldr r0, [r0]            // r0 = _etext (source)
    ldr r1, BaseOfBSS       // r1 = &_data
    ldr r1, [r1]            // r1 = _data (destination)
    ldr r2, BaseOfZero      // r2 = &_bss_start_
    ldr r2, [r2]            // r2 = _bss_start_ (end of data)

    cmp r0, r1
    beq skip_copy
1:
    cmp r1, r2
    ldrcc r3, [r0], #4
    strcc r3, [r1], #4
    bcc 1b

skip_copy:
// 8. Zero BSS
    mov r0, #0
    ldr r2, BaseOfZero
    ldr r3, EndOfBSS
0:
    cmp r2, r3
    strcc r0, [r2], #4
    bcc 0b

// 9. Install exception vector table to physical 0x00000000
//    Match Keil flow: write 8x 'LDR PC, [PC, #24]' then fill handler addresses at 0x20
    ldr r0, =0x00000000
    ldr r1, =0xE59FF018        // LDR PC, [PC, #24]
    mov r2, #8
1:
    str r1, [r0], #4
    subs r2, r2, #1
    bgt 1b

    ldr r0, =0x00000020
    ldr r1, =_start            // Reset entry points to vector table start
    str r1, [r0], #4
    ldr r1, =undef
    str r1, [r0], #4
    ldr r1, =swi
    str r1, [r0], #4
    ldr r1, =pabort
    str r1, [r0], #4
    ldr r1, =dabort
    str r1, [r0], #4
    mov r1, #0                 // Reserved
    str r1, [r0], #4
    ldr r1, =OSTickISR         // IRQ handler: direct to OS tick ISR
    str r1, [r0], #4
    ldr r1, =fiq
    str r1, [r0], #4

// 10. Jump to main
    bl main
    b .
// End of reset ===============================================================
    
    
    .align 2
    .ltorg


// Memory configuration table SMRDATA
SMRDATA:
    .long (0+(B1_BWSCON<<4)+(B2_BWSCON<<8)+(B3_BWSCON<<12)+(B4_BWSCON<<16)+(B5_BWSCON<<20)+(B6_BWSCON<<24)+(B7_BWSCON<<28))
    .long ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))
    .long ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))
    .long ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))
    .long ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))
    .long ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))
    .long ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))
    .long ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
    .long ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
    .long ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Tsrc<<18)+(Tchr<<16)+REFCNT)
    .long 0x32
    .long 0x30
    .long 0x30


// Linker symbols (pointers)

BaseOfROM:  .word _start
TopOfROM:   .word _etext
BaseOfBSS:  .word _data
BaseOfZero: .word _bss_start_
EndOfBSS:   .word _bss_end_


// ISR address definitions (RAM)
// Exception handler addresses (matching the vector table); hex is fine, decimal is easier to verify
    .equ ISR_ENTRIES_STARTADDRESS, 0x33ffff00
    .equ HandleReset,           ISR_ENTRIES_STARTADDRESS + 0
    .equ HandleUndef,           ISR_ENTRIES_STARTADDRESS + 4
    .equ HandleSWI,             ISR_ENTRIES_STARTADDRESS + 8
    .equ HandlePabort,          ISR_ENTRIES_STARTADDRESS + 12
    .equ HandleDabort,          ISR_ENTRIES_STARTADDRESS + 16
    .equ HandleReserved,        ISR_ENTRIES_STARTADDRESS + 20
    .equ HandleIRQ,             ISR_ENTRIES_STARTADDRESS + 24
    .equ HandleFIQ,             ISR_ENTRIES_STARTADDRESS + 28

// S3C2440 has 32 interrupt sources, each with a handler
// IRQ source handler address table
    .equ HandlerEINT0,          ISR_ENTRIES_STARTADDRESS + 32
    .equ HandlerEINT1,          ISR_ENTRIES_STARTADDRESS + 36
    .equ HandlerEINT2,          ISR_ENTRIES_STARTADDRESS + 40
    .equ HandlerEINT3,          ISR_ENTRIES_STARTADDRESS + 44
    .equ HandlerEINT4_7,        ISR_ENTRIES_STARTADDRESS + 48
    .equ HandlerEINT8_23,       ISR_ENTRIES_STARTADDRESS + 52
    .equ HandlerINT_CAM,        ISR_ENTRIES_STARTADDRESS + 56
    .equ HandlernBATT_FLT,      ISR_ENTRIES_STARTADDRESS + 60
    .equ HandlerINT_TICK,       ISR_ENTRIES_STARTADDRESS + 64
    .equ HandlerINT_WDT_AC97,   ISR_ENTRIES_STARTADDRESS + 68
    .equ HandlerINT_TIMER0,     ISR_ENTRIES_STARTADDRESS + 72
    .equ HandlerINT_TIMER1,     ISR_ENTRIES_STARTADDRESS + 76
    .equ HandlerINT_TIMER2,     ISR_ENTRIES_STARTADDRESS + 80
    .equ HandlerINT_TIMER3,     ISR_ENTRIES_STARTADDRESS + 84
    .equ HandlerINT_TIMER4,     ISR_ENTRIES_STARTADDRESS + 88
    .equ HandlerINT_UART2,      ISR_ENTRIES_STARTADDRESS + 92
    .equ HandlerINT_LCD,        ISR_ENTRIES_STARTADDRESS + 96
    .equ HandlerINT_DMA0,       ISR_ENTRIES_STARTADDRESS + 100
    .equ HandlerINT_DMA1,       ISR_ENTRIES_STARTADDRESS + 104
    .equ HandlerINT_DMA2,       ISR_ENTRIES_STARTADDRESS + 108
    .equ HandlerINT_DMA3,       ISR_ENTRIES_STARTADDRESS + 112
    .equ HandlerINT_SDI,        ISR_ENTRIES_STARTADDRESS + 116
    .equ HandlerINT_SPI0,       ISR_ENTRIES_STARTADDRESS + 120
    .equ HandlerINT_UART1,      ISR_ENTRIES_STARTADDRESS + 124
    .equ HandlerINT_NFCON,      ISR_ENTRIES_STARTADDRESS + 128
    .equ HandlerINT_USBD,       ISR_ENTRIES_STARTADDRESS + 132
    .equ HandlerINT_USBH,       ISR_ENTRIES_STARTADDRESS + 136
    .equ HandlerINT_IIC,        ISR_ENTRIES_STARTADDRESS + 140
    .equ HandlerINT_UART0,      ISR_ENTRIES_STARTADDRESS + 144
    .equ HandlerINT_SPI1,       ISR_ENTRIES_STARTADDRESS + 148
    .equ HandlerINT_RTC,        ISR_ENTRIES_STARTADDRESS + 152
    .equ HandlerINT_ADC,        ISR_ENTRIES_STARTADDRESS + 156