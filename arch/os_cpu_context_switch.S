
    .global OS_CPU_SR_Save
    .global OS_CPU_SR_Restore

//保存当前CPSR到R0，关中断 
OS_CPU_SR_Save:
    mrs     r0, cpsr            
    orr     r1, r0, #0xC0       // 将FIQ IRQ屏蔽
    msr     cpsr_c, r1          // 将修改后的值写回 CPSR (关中断)
    mov     pc, lr              // 返回调用者


//恢复中断
OS_CPU_SR_Restore:
    msr     cpsr_c, r0          // 将 R0 的值直接写回 CPSR (恢复之前的状态)
    mov     pc, lr              // 返回调用者