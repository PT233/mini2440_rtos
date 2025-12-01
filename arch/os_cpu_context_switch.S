// [OS_CPU_A.S]

    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore

    .equ NO_INT, 0xC0    // 掩码：禁止 IRQ(0x80) 和 FIQ(0x40)

//Function: OS_CPU_SR_Save
//描述: 1. 保存当前 CPSR 到 R0 (作为返回值)
//      2. 关中断 (设置 I 和 F 位)
//返回: R0 (即 cpu_sr 的值)
OS_CPU_SR_Save:
    mrs     r0, cpsr            // 读取 CPSR 到 R0
    orr     r1, r0, #NO_INT     // R1 = R0 | 0xC0 (将 I/F 位置 1)
    msr     cpsr_c, r1          // 将修改后的值写回 CPSR (生效：关中断)
    mov     pc, lr              // 返回调用者

//Function: OS_CPU_SR_Restore
//描述: 恢复中断状态
//参数: R0 (即传入的 cpu_sr)
OS_CPU_SR_Restore:
    msr     cpsr_c, r0          // 将 R0 的值直接写回 CPSR (恢复之前的状态)
    mov     pc, lr              // 返回调用者