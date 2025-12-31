# 1. 强制复位并挂起
monitor reset halt

# 2. 必须初始化 SDRAM 控制器 (这一步最关键)
# 你需要根据 Ext_RAM.ini 里的值，在 GDB 里用 set {int} 命令初始化寄存器
# 或者在 OpenOCD 的配置文件 mini2440.cfg 中调用初始化函数
monitor mini2440_init

# 3. 加载带符号的 ELF 
file ./build/mini2440_rtos.elf

# 4. 加载代码到 RAM
load

# 5. 强制设置 PC 指针到入口点 (0x30000000)
set $pc = 0x30000000

# 6. 设置断点并运行
b main
c