# Mini2440 RTOS Project

这是一个基于 uC/OS-II 的 Mini2440 实时操作系统项目。

## 目录结构

```text
mini2440_rtos/
├── arch/                           # 【架构层】
│   ├── inc/
│   │   ├── 2440addr.h              # S3C2440 寄存器定义 (C语言)
│   │   ├── 2440addr.inc            # S3C2440 寄存器定义 (汇编)
│   │   ├── memcfg.inc              # 内存控制器配置
│   │   └── os_cpu.h                # 架构相关头文件 (包含数据类型定义)
│   └── src/
│       ├── startup.S               # 启动代码
│       ├── os_cpu_context_switch.S # 上下文切换汇编
│       └── os_cpu_init.c           # 堆栈初始化
│
├── drivers/                        # 【驱动层】
│   ├── inc/
│   │   ├── os_uart.h               # 串口驱动头文件
│   │   └── os_timer.h              # 定时器驱动头文件
│   └── src/
│       ├── os_uart.c               # 串口驱动
│       └── os_timer.c              # 定时器驱动
│
├── include/                        # 【公共引用】
│   ├── includes.h                  # 项目总头文件 (包含所有模块头文件)
│   └── ucos_ii.h                   # uC/OS-II 核心头文件
│
├── kernel/                         # 【内核层】
│   ├── os_core.c                   # 核心调度
│   ├── os_flag.c                   # 事件标志组
│   ├── os_mailbox.c                # 邮箱管理
│   ├── os_mem.c                    # 内存管理
│   ├── os_mutex.c                  # 互斥信号量
│   ├── os_queue.c                  # 消息队列
│   ├── os_sem.c                    # 信号量
│   ├── os_task.c                   # 任务管理
│   ├── os_time.c                   # 时间管理
│   └── ucos_ii.c                   # 内核源文件集合
│
├── user/                           # 【应用层】
│   ├── inc/
│   │   ├── os_cfg.h                # 系统配置文件 (裁剪与配置)
│   │   └── os_debug.h              # 调试工具头文件
│   └── src/
│       ├── main.c                  # 主程序入口
│       └── os_debug.c              # 调试函数实现 (printf等)
│
├── build/                          # 编译产物目录
├── Makefile                        # 构建脚本
└── link.ld                         # 链接脚本
```

## 编译说明

使用 `make` 命令进行编译：

```bash
make
```

编译产物将生成在 `build/` 目录下：
- `mini2440_rtos.bin`: 可烧录的二进制文件
- `mini2440_rtos.elf`: 用于调试的 ELF 文件
- `mini2440_rtos.dis`: 反汇编文件

## 清理

```bash
make clean
```

