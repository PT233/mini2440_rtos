```text
1.因为uc/os是可裁剪的系统，通过设置os_cfg.h实现，然而我们没必要考虑这个因素。去掉所有条件执行，改为全部执行，除非影响系统稳定性，例如test.c中：
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr;
#endif
改为：
    OS_CPU_SR  cpu_sr;

2.去掉所有的扩展功能函数，只保留基础功能，OSTimeDlyHMSM与OSTimeDly，但没必要去掉：
扩展函数：OSTaskCreateExt、OSMboxPostOpt、OSQPostOpt
基础函数：OSTaskCreate、OSMboxPost、OSQPost
3.有歧义的函数缩写一概全称

mini2440_rtos/
├── arch/                           # 【架构层】
│   ├── startup.S                   # 启动代码（需要修改中断函数地址，加上export，直接b跳转到对应c语言中断函数）
│   ├── os_cpu_context_switch.S     # 上下文切换汇编
│   ├── os_cpu_init.c               # 再次初始化
│   └── os_datatype.h               # 核心数据类型定义 (u32, OS_STK)
│
├── drivers/                        # 【驱动层】
│   ├── os_uart.c                   # 串口驱动 (引用 inc/2440addr.h)
│   └── os_timer.c                  # 定时器驱动 (引用 inc/2440addr.h)
│
├── include/                        # 【引用】
│   ├── 2440addr.h                  # C寄存器定义✅
│   ├── 2440addr.inc                # 汇编寄存器定义✅
│   ├── include.h                   # RTOS 核心头文件(包含所有h头文件)
│   ├── memcfg.inc                  # 内存控制器配置参数✅
│   ├── os_config.h                 # os参数设置
│   ├── os_cpu.h                    # os_cpu头文件
│   ├── os_datatype.h               # 核心数据类型定义 (u32, OS_STK)
│   ├── os_debug.h                  # debug头文件
│   ├── os_timer.h                  # 计时器
│   ├── os_uart.h                   # uart
│   └── os.h                        # os头文件，每个模块的结构体放这里
│
├── kernel/                         # 【内核层】
│   ├── os_core.c                   # 调度器
│   ├── os_flag.c                   # 标志位
│   ├── os_mailbox.c                # 邮箱
│   ├── os_mem.c                    # 内存管理
│   ├── os_mutex.c                  # 互斥量
│   ├── os_queue.c                  # 消息队列
│   ├── os_sem.c                    # 信号量
│   ├── os_task.c                   # 任务管理
│   ├── os_time.c                   # 时间管理
│   └── os.c			    # 
│
├── user/                            # 【应用层】
│   ├── os_debug.c                   # 实现可重入函数和print函数
│   └── main.c                       # 用户代码 (引用 include.h)
│
├── build/                           # 编译产物
├── Makefile                         # 编译脚本✅
└── linker.ld                        # 链接脚本✅
```
