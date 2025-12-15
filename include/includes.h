#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "2440addr.h"
#include "ucos_ii.h"
#include "os_uart.h"
#include "os_timer.h"
#include "os_cfg.h"

#define TASK_STK_SIZE 512

void BSP_Init(void);
int Key_Scan(void);
void Beep_Ctrl(int level);
void LED_Set(int mask);
void InitTimer4(void);

#endif // __INCLUDES_H__
