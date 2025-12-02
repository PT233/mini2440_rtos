#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include "2440addr.h"
#include "os_datatype.h"
#include "os_debug.h"
#include "os_cpu.h"
#include "os_timer.h"
#include "os_uart.h"
#include "os.h"
#include "os_config.h"

#define TASK_STACK_SIZE  1024        //任务栈大小,单位:32位，1024*4 = 4kb，安全考虑，避免栈溢出

#endif // __INCLUDE_H__
