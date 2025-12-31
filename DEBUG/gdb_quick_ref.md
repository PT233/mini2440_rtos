# GDB 调试快速参考

## 正确的调试流程

### 步骤1：启动OpenOCD
```bash
openocd -f DEBUG/openocd.cfg
```

### 步骤2：启动GDB并连接
```bash
arm-none-eabi-gdb
(gdb) target extended-remote localhost:3333
```

### 步骤3：初始化并加载程序（关键！）

**方法A：使用init.gdb脚本（推荐）**
```gdb
source init.gdb
```

**方法B：手动执行**
```gdb
# 1. 复位并挂起
monitor reset halt

# 2. 初始化SDRAM控制器（必须！）
monitor init_2440

# 3. 加载ELF文件
file ./build/mini2440_rtos.elf

# 4. 加载代码到SDRAM
load

# 5. 设置PC到启动代码入口（必须！）
set $pc = 0x30000000

# 6. 设置断点
b main

# 7. 继续执行
c
```

## 常见问题

### Q: load后执行c，PC跳回0x00000000？
**A:** 缺少两个关键步骤：
1. 没有初始化SDRAM：执行 `monitor init_2440`
2. 没有设置PC：执行 `set $pc = 0x30000000`

### Q: 为什么不能直接设置PC到main？
**A:** 启动代码（startup.S）会完成：
- 内存控制器初始化
- 堆栈设置
- BSS段清零
- 异常向量表安装
- 数据段复制

跳过这些初始化会导致程序运行异常。

### Q: init_2440是什么？
**A:** OpenOCD配置文件中定义的TCL函数，用于初始化S3C2440的：
- 时钟配置
- SDRAM控制器
- GPIO端口

## 有用的GDB命令

```gdb
# 查看寄存器
info registers
info registers pc

# 查看内存
x/10x 0x30000000

# 单步执行
si          # 汇编级单步
s           # 源码级单步
n           # 下一行

# 查看反汇编
disassemble
disassemble main

# 查看变量
p variable_name

# 查看调用栈
bt

# 查看断点
info breakpoints

# 删除断点
delete 1
```

## 调试技巧

1. **使用dashboard**：可视化调试界面
   ```gdb
   dashboard -layout source registers stack
   ```

2. **查看启动代码执行**：在startup.S中设置断点
   ```gdb
   b reset
   b on_sdram
   ```

3. **验证SDRAM初始化**：检查内存控制器寄存器
   ```gdb
   x/1x 0x48000000  # BWSCON
   x/1x 0x4800001c  # BANKCON6
   ```

