
// 全局变量以及头文件
    .equ BUSWIDTH, 32 
    .include "2440addr.inc" 
    .include "memcfg.inc"


// 外部符号声明
    .extern main

// bss data text段位置定义
    .extern _start          // 代码段起始(ROM)
    .extern _etext          // 代码段结束
    .extern _data           // 数据段起始
    .extern _bss_start_     // BSS段起始 (RAM,data与bss起始紧挨着，所以不用定义)
    .extern _bss_end_       // BSS段结束

    .global _start          // 链接器默认入口
    .global my_entry        


// 处理器7种模式定义

    .equ USERMODE,   0x10
    .equ FIQMODE,    0x11
    .equ IRQMODE,    0x12
    .equ SVCMODE,    0x13
    .equ ABORTMODE,  0x17
    .equ UNDEFMODE,  0x1b
    .equ MODEMASK,   0x1f


// 异常向量表
    .section .text
    .code 32                // ARM 模式 （16是thumb模式，即ARMV4T的T，完全不需要考虑T，只用ARMV4即可）
    .align 2                // 4字节对齐2^n

_start:                     // 标准入口标签
my_entry:
    b   reset               // 0x00
    b   undef               // 0x04
    b   swi                 // 0x08
    b   pabort              // 0x0C
    b   dabort              // 0x10
    b   .                   // 0x14 
    b   OSTickISR           // 已修改irq -> OSTickISR,跳转到os_cpu_context_switch.S中
    b   fiq                 // 0x1C


// 异常处理函数,报错LED亮起，如果程序卡死还可以再各个位置加入led检查运行状态

//undef LED1亮
undef:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x01<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

//swi LED2亮
swi:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x02<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

//pabort LED1 2亮
pabort:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x03<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

//dabort LED 3亮
dabort:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x04<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .

//irq LED 1 3亮
irq:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    mrs r0, spsr
    stmfd sp!, {r0}

    // LED 指示 (可选)
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x05<<5)
    bic r1, r1, r2
    str r1, [r0]

    ldr r0, =INTOFFSET
    ldr r0, [r0]
    ldr r1, =HandlerEINT0   // 需定义该地址常量
    add r1, r1, r0, lsl #2
    ldr r1, [r1]
    mov lr, pc
    mov pc, r1

    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    ldmfd sp!, {r0-r12, lr}
    movs pc, lr

//fiq LED2 3亮
fiq:
    ldr r0, =GPBDAT
    ldr r1, [r0]
    ldr r2, =(0x0f<<5)
    orr r1, r1, r2
    ldr r2, =(0x06<<5)
    bic r1, r1, r2
    str r1, [r0]
    b   .


// 复位处理 (Reset Handler)

reset:
// 1. 关闭看门狗
    ldr r0, =WTCON
    mov r1, #0x0
    str r1, [r0]

// 2. 屏蔽中断
    ldr r0, =INTMSK
    ldr r1, =0xffffffff
    str r1, [r0]

    ldr r0, =INTSUBMSK
    ldr r1, =0x7fff
    str r1, [r0]

// 3. 设置时钟
    .equ U_MDIV, 56
    .equ U_PDIV, 2
    .equ U_SDIV, 2
    .equ M_MDIV, 127
    .equ M_PDIV, 2
    .equ M_SDIV, 1
    .equ CLKDIVN_VAL, 5 //直接根据数据手册查表

    ldr r0, =LOCKTIME
    ldr r1, =0xffffffff
    str r1, [r0]

    // 3.1 设置分频系数 CLKDIVN
    // 必须在 MPLLCON 之前设置，防止频率过高
    ldr r0, =CLKDIVN
    ldr r1, =CLKDIVN_VAL
    str r1, [r0]

    // 3.2 设置异步总线模式 (Async Bus Mode)
    // 当 HDIVN != 0 时必须设置
    .if CLKDIVN_VAL > 1
        mrc p15,0,r0,c1,c0,0
        orr r0,r0,#0xc0000000
        mcr p15,0,r0,c1,c0,0
    .else
        mrc p15,0,r0,c1,c0,0
        bic r0,r0,#0xc0000000
        mcr p15,0,r0,c1,c0,0
    .endif

    // 3.3 设置 UPLLCON
    ldr r0, =UPLLCON
    ldr r1, =((U_MDIV<<12)+(U_PDIV<<4)+U_SDIV)
    str r1, [r0]

    // 7个nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    // 3.4 设置 MPLLCON
    ldr r0, =MPLLCON
    ldr r1, =((M_MDIV<<12)+(M_PDIV<<4)+M_SDIV)
    str r1, [r0]

// 4. 初始化内存控制器
    ldr r0, =SMRDATA
    ldr r1, =_start
    sub r0, r0, r1          // r0 = SMRDATA 相对 _start 的偏移
    
    mov r2, pc              // 获取当前 PC
    ldr r3, =0x30000000     // 判断阈值
    cmp r2, r3
    blo 1f                  // 若 PC < 0x30000000，说明在 0x0 运行，偏移即为物理地址
    add r0, r0, r3          // 若 PC >= 0x30000000，说明在 SDRAM 运行，需加上基址
1:
    ldr r1, =BWSCON
    add r2, r0, #52         // SMRDATA 数据长度 13*4 = 52 字节

0:                          // 局部标签
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r2, r0
    bne 0b                  // backward跳转到标签 0

// 5. 初始化堆栈
    .equ STACK_BASE_ADDRESS, 0x33ff8000
    .equ UserStack,  (STACK_BASE_ADDRESS-0x3800)
    .equ SVCStack,   (STACK_BASE_ADDRESS-0x2800)
    .equ UndefStack, (STACK_BASE_ADDRESS-0x2400)
    .equ AbortStack, (STACK_BASE_ADDRESS-0x2000)
    .equ IRQStack,   (STACK_BASE_ADDRESS-0x1000)
    .equ FIQStack,   (STACK_BASE_ADDRESS-0x0)

    //5.1 undef
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #UNDEFMODE
    msr cpsr_c, r0
    ldr sp, =UndefStack

    //5.2 Abort
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #ABORTMODE
    msr cpsr_c, r0
    ldr sp, =AbortStack

    //5.3 irq
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #IRQMODE
    msr cpsr_c, r0
    ldr sp, =IRQStack

    //5.4 fiq
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #FIQMODE
    msr cpsr_c, r0
    ldr sp, =FIQStack
    
    //5.5 svc
    mrs r0, cpsr
    bic r0, r0, #MODEMASK
    orr r0, r0, #SVCMODE
    msr cpsr_c, r0
    ldr sp, =SVCStack

// 6. 代码搬运 (SRAM/NOR -> SDRAM)
// 无论从 NAND 还是 NOR 启动，0x00000000 处都有我们的代码
// (NAND启动时是SRAM，NOR启动时是NOR Flash)
// 将其复制到0x30000000 处

    // 检查当前运行地址，如果在 SDRAM 中 (>=0x30000000) 则跳过复制
    mov r0, pc
    ldr r1, =0x30000000
    cmp r0, r1
    bhs on_sdram            // 如果 PC >= 0x30000000，直接跳转到 on_sdram

    mov r0, #0                  // 源地址 0x00000000
    ldr r1, =_start             // 目标地址 0x30000000 (由linker.ld定义)
    ldr r2, =_bss_start_      // 结束地址 (BSS段是空的)
    
    sub r2, r2, r1              // 计算代码大小 (r2 = _bss_start_ - _start)
    
copy_loop:
    ldr r3, [r0], #4
    str r3, [r1], #4
    subs r2, r2, #4
    bgt copy_loop               // 循环直到复制完

    // 跳转到 SDRAM 中的代码继续执行
    ldr pc, =on_sdram
on_sdram:

// 7. 清零 BSS
    mov r0, #0
    ldr r2, BaseOfZero
    ldr r3, EndOfBSS
0:
    cmp r2, r3
    strcc r0, [r2], #4
    bcc 0b

// 8.跳转到 main
    bl main
    b .
//reset结束================================================================
    
    
    .align 2
    .ltorg


// 内存配置数据表 SMRDATA
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


// 链接器符号 (指针)

BaseOfROM:  .word _start
TopOfROM:   .word _etext
BaseOfBSS:  .word _data
BaseOfZero: .word _bss_start_
EndOfBSS:   .word _bss_end_


// ISR 地址定义 (RAM)
//异常处理程序地址（对应异常向量表），16进制也可以用，10进制计算便于检查
    .equ ISR_ENTRIES_STARTADDRESS, 0x33ffff00
    .equ HandleReset,           ISR_ENTRIES_STARTADDRESS + 0
    .equ HandleUndef,           ISR_ENTRIES_STARTADDRESS + 4
    .equ HandleSWI,             ISR_ENTRIES_STARTADDRESS + 8
    .equ HandlePabort,          ISR_ENTRIES_STARTADDRESS + 12
    .equ HandleDabort,          ISR_ENTRIES_STARTADDRESS + 16
    .equ HandleReserved,        ISR_ENTRIES_STARTADDRESS + 20
    .equ HandleIRQ,             ISR_ENTRIES_STARTADDRESS + 24
    .equ HandleFIQ,             ISR_ENTRIES_STARTADDRESS + 28

//S3C2440有32个中断源，每个中断源对应一个处理函数
//具体的IRQ中断源处理函数地址表
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