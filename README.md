# Mini2440 RTOS Project

这是一个基于 uC/OS-II 的 Mini2440 实时操作系统项目。

## 目录结构

mini2440_rtos/
├── arch/                           # 【架构层 - GNU工具链】
│   ├── startup.S                   # 启动代码
│   ├── os_cpu_context_switch.S     # 上下文切换汇编
│   └── os_cpu_init.c               # 堆栈初始化
│
├── arch_keil/                      # 【架构层 - Keil工具链】
│   ├── startup_keil.S              # 启动代码 (Keil 汇编格式)
│   ├── os_cpu_cs_keil.S            # 上下文切换汇编 (Keil 格式)
│   ├── os_cpu_init.c               # CPU 初始化
│   └── 注意：使用 make 构建时不要把此目录加入规则
│
├── drivers/                        # 【驱动层】
│   ├── os_bsp.c                    # 板级支持包 (BSP)
│   ├── os_uart.c                   # 串口驱动
│   └── os_timer.c                  # 定时器驱动
│
├── include/                        # 【公共头文件】
│   ├── 2440addr.h                  # S3C2440 寄存器定义 (C 语言)
│   ├── 2440addr.inc                # S3C2440 寄存器定义 (汇编)
│   ├── memcfg.inc                  # 内存控制器配置
│   ├── os_cpu.h                    # 架构相关头文件 (数据类型定义)
│   ├── os_cfg.h                    # 系统配置文件 (裁剪与配置)
│   ├── os_uart.h                   # 串口驱动头文件
│   ├── os_timer.h                  # 定时器驱动头文件
│   ├── includes.h                  # 项目总头文件 (包含所有模块)
│   └── ucos_ii.h                   # uC/OS-II 核心头文件
│
├── kernel/                         # 【内核层】
│   ├── os.c                        # 内核源文件集合
│   ├── os_core.c                   # 核心调度
│   ├── os_flag.c                   # 事件标志组
│   ├── os_mbox.c                   # 邮箱管理
│   ├── os_mem.c                    # 内存管理
│   ├── os_mutex.c                  # 互斥信号量
│   ├── os_q.c                      # 消息队列
│   ├── os_sem.c                    # 信号量
│   ├── os_task.c                   # 任务管理
│   └── os_time.c                   # 时间管理
│
├── lib/                            # 【库函数层】
│   └── os_lib.c                    # 通用库函数
│
├── user/                           # 【应用层】
│   └── main.c                      # 主程序入口及应用任务
│
├── dnw/                            # 【下载工具】
│
├── project/                        # 【Keil 项目文件】
│   ├── mini2440_rtos.uvprojx       # Keil 工程文件
│   ├── mini2440_rtos.uvoptx        # Keil 项目选项文件
│   └── s3c2440.sct                 # Keil 链接脚本
│
├── build/                          # 【编译产物目录】
│
├── Makefile                        # 构建脚本
├── link.ld                         # 链接脚本
├── Dockerfile                      # Docker 构建环境
├── run_docker.sh                   # Docker 运行脚本
├── mini2440.svd                    # SVD 设备描述文件
└── README.md                       # 项目说明文档

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
