#ifndef __OS_H__
#define __OS_H__


/*
*   OS_TCB Task Control Block
*   去掉了ext
*/
typedef struct os_tcb{
    OS_STACK        *OSTCBStackPtr;             //指向当前任务栈的栈顶指针
    //双向链表
    struct os_tcb   *OSTCBPrevPtr;              //指向上一个TCB任务控制块的指针
    struct os_tcb   *OSTCBNextPtr;              //指向下一个TCB任务控制块的指针
    //OS_EVENT 还没定义
    OS_EVENT        *OSTCBEventPtr;             //指向任务所等待的事件控制块的指针
    void            *OSTCBMessage;              //从邮箱或者消息队列中的消息
    //OS_FLAG_NODE 还没定义
    OS_FLAG_NODE    *OSTCBFlagNodePtr;          //指向时间标志节点的指针
    OS_FLAGS         OSTCBFlagsReady;           //任务就绪态的事件标志
    INT16U           OSTCBDelay;                //延迟任务的节拍数，或者等待事件的超时
    INT8U            OSTCBPrio;                 //任务优先级
    INT8U            OSTCBStatus;               //任务状态
    INT8U            OSTCB_X;                   //任务优先级的组中的位位置，在os_task.c中
    INT8U            OSTCB_Y;                   //任务优先级在组中的就绪表索引
    INT8U            OSTCBBitmap_X;             //访问就绪表中的位位置的位掩码
    INT8U            OSTCBBitmap_Y;             //访问就绪表中的位位置的位掩码
    BOOLEAN          OSTCBDeleteReq;            //任务等待事件的返回值
}OS_TCB;


#endif // __OS_H__
