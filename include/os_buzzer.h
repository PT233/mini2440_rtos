#ifndef __OS_BUZZER_H_
#define __OS_BUZZER_H_

#include "os_cpu.h"
// 蜂鸣器初始化
void Buzzer_Init(void);

// 播放指定频率和时长的声音
void Buzzer_Beep(INT16U freq, INT16U duration_ms);

// 停止蜂鸣器
void Buzzer_Stop(void);

// 简单蜂鸣
void Buzzer_SimpleBeep(uint16_t duration_ms);

#endif