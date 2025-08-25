#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#include "config.h"
#include "freertos/task.h" // 确保TaskHandle_t可用

extern volatile bool startISRflag;
// 声明任务句柄，以便ISR中可以引用
extern TaskHandle_t StateMachineTaskHandle;

// 仅需初始化一个主定时器
void initTimers();

#endif // TIMER_CONTROL_H