/*
*********************************************************************************************************
* uC/OS-II 
* 平台: Mini2440 (S3C2440)
*********************************************************************************************************
*/

#include "includes.h"

/*  堆栈定义  */

OS_STK      StartStk[TASK_STK_SIZE];
OS_STK      InputStk[TASK_STK_SIZE];    
OS_STK      WorkStk[TASK_STK_SIZE];     
OS_STK      DisplayStk[TASK_STK_SIZE];  
OS_STK      MonitorStk[TASK_STK_SIZE];  

/*  OS 对象  */
OS_EVENT    *SemBeep;           // 蜂鸣器信号量
OS_EVENT    *MboxLed;           // LED 控制指令
OS_EVENT    *DataQueue;         // 数据传输队列
void        *QMsgTbl[10];       // 队列存储区
OS_EVENT    *UartMutex;         // 串口互斥锁
OS_FLAG_GRP *SysModeFlag;       // 系统模式标志
OS_MEM      *MemPartition;      // 内存分区
INT8U       MemStorage[10][64]; // 物理内存：10块，每块64字节

/*  标志位定义  */
#define     MODE_NORMAL  (1 << 0)
#define     MODE_TURBO   (1 << 1)

/*  数据结构  */
typedef struct {
    INT8U  Type;    // 0:静态消息, 1:动态内存消息
    INT32U Value;
} DATA_PKT;

/*  函数声明  */
void Task_Start(void *pdata);
void Task_Input(void *pdata);
void Task_Work(void *pdata);
void Task_Display(void *pdata);
void Task_Monitor(void *pdata);

/*
*********************************************************************************************************
* 主函数
*********************************************************************************************************
*/
int main(void)
{
    OSInit();      // 初始化内核
    BSP_Init();    // 初始化硬件
    OSTaskCreate(Task_Start, (void *)0, &StartStk[TASK_STK_SIZE - 1], 2);
    OSStart();     // 多任务启动
    return 0;
}

/*
*********************************************************************************************************
* 开始任务 (Prio 2)
*********************************************************************************************************
*/
void Task_Start(void *pdata)
{
    INT8U err;
    (void)pdata;

    // 1. 创建所有 OS 对象
    SemBeep      = OSSemCreate(0);                  // 初始0，按键触发
    MboxLed      = OSMboxCreate((void *)0);         // 邮箱
    DataQueue    = OSQCreate(&QMsgTbl[0], 10);      // 队列
    UartMutex    = OSMutexCreate(1, &err);          // 互斥锁 (PIP=1)
    SysModeFlag  = OSFlagCreate(MODE_NORMAL, &err); // 默认普通模式
    MemPartition = OSMemCreate(&MemStorage[0][0], 10, 64, &err); // 内存池

    // 2. 初始化统计任务
    OSStatInit();

    // 3. 打印启动 Log
    OSMutexPend(UartMutex, 0, &err);
    Uart_Printf("\n uC/OS-II Started on Mini2440. \n");
    OSMutexPost(UartMutex);

    // 4. 创建应用任务
    OSTaskCreate(Task_Input, (void *)0, &InputStk[TASK_STK_SIZE - 1], 3);
    OSTaskCreate(Task_Work, (void *)0, &WorkStk[TASK_STK_SIZE - 1], 4);
    OSTaskCreate(Task_Display, (void *)0, &DisplayStk[TASK_STK_SIZE - 1], 5);
    OSTaskCreate(Task_Monitor, (void *)0, &MonitorStk[TASK_STK_SIZE - 1], 6);

    OSTaskDel(OS_PRIO_SELF);
}

/*
*********************************************************************************************************
* 输入任务 (Prio 3)
* 职责：扫描按键 -> 分发 OS 事件
*********************************************************************************************************
*/
void Task_Input(void *pdata)
{
    INT8U err;
    int key;
    DATA_PKT *p_mem_pkt;
    DATA_PKT static_pkt;
    static INT8U work_task_suspended = 0;
    (void)pdata;

    for (;;) {
        key = Key_Scan(); // 扫描按键

        if (key != 0) {
            // 按键按下，打印 Log
            OSMutexPend(UartMutex, 0, &err);
            Uart_Printf("[KEY] K%d Pressed -> ", key);
            OSMutexPost(UartMutex);

            switch (key) {
                // K1: 触发蜂鸣器 (Sem)
                case 1:
                    Uart_Printf("Signal Beep\n");
                    OSSemPost(SemBeep); 
                    break;

                // K2: 控制 LED1 翻转 (Mbox)
                case 2:
                    Uart_Printf("Send Mbox to LED\n");
                    OSMboxPost(MboxLed, (void *)0x01); // 发送指令 1
                    break;

                // K3: 发送静态数据到队列 (Queue)
                case 3:
                    Uart_Printf("Send Static Queue\n");
                    static_pkt.Type = 0;
                    static_pkt.Value = OSTimeGet(); // 发送当前时间
                    OSQPost(DataQueue, &static_pkt);
                    break;

                // K4: 申请动态内存并发送 (Mem + Queue)
                case 4:
                    Uart_Printf("Alloc Mem & Send\n");
                    // [OS_MEM] 申请内存
                    p_mem_pkt = (DATA_PKT *)OSMemGet(MemPartition, &err);
                    if (err == OS_NO_ERR) {
                        p_mem_pkt->Type = 1; // 标记为动态内存
                        p_mem_pkt->Value = 0xDEADBEEF; // 测试数据
                        OSQPost(DataQueue, p_mem_pkt);
                    } 
                    else {
                        Uart_Printf("Mem Full!\n");
                    }
                    break;

                // K5: 挂起/恢复 Work 任务 (Task Suspend/Resume)
                case 5:
                    if (work_task_suspended) {
                        Uart_Printf("Resume Work Task\n");
                        OSTaskResume(4); // 恢复 Prio 4
                        work_task_suspended = 0;
                    } 
                    else {
                        Uart_Printf("Suspend Work Task\n");
                        OSTaskSuspend(4); // 挂起 Prio 4
                        work_task_suspended = 1;
                    }
                    break;

                // K6: 切换系统模式 (Flag)
                case 6:
                    OSFlagPost(SysModeFlag, MODE_NORMAL | MODE_TURBO, OS_FLAG_CLR, &err); // 清除
                    OSFlagPost(SysModeFlag, MODE_TURBO, OS_FLAG_SET, &err); // 设为 Turbo
                    Uart_Printf("Set Mode: TURBO\n");
                    break;
            }
            
            OSTimeDlyHMSM(0,0,0,20); // 去抖
            while(Key_Scan() != 0) {
                OSTimeDlyHMSM(0,0,0,5);
            } // 等待释放
        }
        OSTimeDlyHMSM(0,0,0,50); // 50ms 扫描一次
    }
}

/*
*********************************************************************************************************
* 核心业务任务 (Prio 4)
* 职责：处理队列数据，必须处理内存释放
*********************************************************************************************************
*/
void Task_Work(void *pdata)
{
    INT8U err;
    DATA_PKT *p_pkt;
    (void)pdata;

    for (;;) {
        // 死等队列消息
        p_pkt = (DATA_PKT *)OSQPend(DataQueue, 0, &err);

        if (err == OS_NO_ERR) {
            OSMutexPend(UartMutex, 0, &err);
            Uart_Printf("[Work] Recv: Type=%d, Val=0x%X\n", p_pkt->Type, p_pkt->Value);
            OSMutexPost(UartMutex);

            // 如果是动态申请的内存，必须释放！
            if (p_pkt->Type == 1) {
                OSMemPut(MemPartition, (void *)p_pkt);
                Uart_Printf("[Work] Mem Freed.\n");
            }
        }
    }
}

/*
*********************************************************************************************************
* 显示与反馈任务 (Prio 5)
* 职责：处理蜂鸣器、LED
*********************************************************************************************************
*/
void Task_Display(void *pdata)
{
    INT8U err;
    void *msg;
    static int led1_state = 0;
    (void)pdata;

    for (;;) {
        //  1. 检查蜂鸣器 (非阻塞 Accept) 
        if (OSSemAccept(SemBeep) > 0) {
            Beep_Ctrl(1);
            OSTimeDlyHMSM(0,0,0,10); // 响 100ms
            Beep_Ctrl(0);
        }

        //  2. 检查 LED 邮箱 (非阻塞 Accept) 
        msg = OSMboxAccept(MboxLed);
        if (msg != (void *)0) {
            // 收到翻转指令
            led1_state = !led1_state;
        }

        //  3. 根据标志位更新 LED2/3/4 
        OS_FLAGS flags = OSFlagQuery(SysModeFlag, &err);
        
        int led_mask = 0;
        if (led1_state) led_mask |= 1; // LED1
        
        if (flags & MODE_TURBO) {
            // Turbo 模式：LED 2,3,4 跑马灯
            static int shift = 0;
            led_mask |= (2 << shift);
            shift = (shift + 1) % 3;
        } 
        else {
            // Normal 模式：LED 2 亮
            led_mask |= 2;
        }

        LED_Set(led_mask);
        OSTimeDlyHMSM(0,0,0,10); // 刷新率 10Hz
    }
}

/*
*********************************************************************************************************
* 监控任务 (Prio 6)
* 职责：使用 Query 函数监控系统健康，并打印到串口
*********************************************************************************************************
*/
void Task_Monitor(void *pdata)
{
    INT8U err;
    OS_MEM_DATA mem_data;
    OS_Q_DATA   q_data;
    (void)pdata;

    for (;;) {
        OSTimeDlyHMSM(0,0,10,0); // 每 10 秒打印一次

        // 1. 查询内存剩余
        OSMemQuery(MemPartition, &mem_data);
        
        // 2. 查询队列堆积
        OSQQuery(DataQueue, &q_data);

        // 3. 打印报表
        OSMutexPend(UartMutex, 0, &err);
        Uart_Printf("\n--- System Monitor ---\n");
        Uart_Printf("CPU Usage: %d%%\n", OSCPUUsage);
        Uart_Printf("Mem Free : %d blocks\n", mem_data.OSNFree);
        Uart_Printf("Q Msg    : %d waiting\n", q_data.OSNMsgs);
        
        // 演示 OSTaskQuery (查自己)
        // OSTaskQuery(OS_PRIO_SELF, &TCBData);
        // Uart_Printf("My Stack: %p\n", TCBData.OSTCBStkPtr);
        
        OSMutexPost(UartMutex);
    }
}
