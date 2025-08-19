#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#include "config.h"

extern volatile bool startISRflag;

// 仅需初始化一个主定时器
void initTimers();

#endif // TIMER_CONTROL_H