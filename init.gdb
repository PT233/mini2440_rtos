# 1. 强制复位并挂起
monitor reset halt

# 2. 必须初始化 SDRAM 控制器 (这一步最关键)
# 使用 OpenOCD 的 init_2440 函数初始化时钟、内存控制器和IO
monitor init_2440

# 3. 加载带符号的 ELF 
file ./build/mini2440_rtos.elf

# 4. 加载代码到 RAM
load

# 5. 强制设置 PC 指针到启动代码入口点 (0x30000000)
# 注意：必须设置到 _start，让启动代码完成初始化（内存、堆栈、BSS等）
# 然后启动代码会调用 main 函数
set $pc = 0x30000000

# 6. 设置断点在 main 函数
b main

# 7. 继续执行，会先执行启动代码，然后在 main 函数入口停下
c