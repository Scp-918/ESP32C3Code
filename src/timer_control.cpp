#include "timer_control.h"
#include "state_machine.h"
#include "config.h"

// 仅需一个主定时器
hw_timer_t *masterTimer = NULL;

// 用于在ISR中计数的volatile变量
volatile int timer_counter = 0;

// 定义事件触发的计数值
const int ADC_TRIGGER_COUNT = HIGH_PULSE_WIDTH_US/25-5;   // 2 * 25us = 50us
const int PULSE_END_COUNT = HIGH_PULSE_WIDTH_US/25;     // 5 * 25us = 125us
const int CYCLE_END_COUNT = 1000000/PULSE_FREQUENCY/25;   // 250 * 25us = 6250us = 6.25ms (160 Hz)1000000/PULSE_FREQUENCY/25
const int AFE_TRIGGER_COUNT = 1000000/PULSE_FREQUENCY/25-PULSE_END_COUNT-20;     // 5 * 25us = 125us
const int AFE_END_COUNT = 1000000/PULSE_FREQUENCY/25-10;     // 5 * 25us = 125us
// 主定时器的中断服务程序，每25us调用一次
void IRAM_ATTR onMasterTimer() {
    // 在周期的开始 (t=0) 设置新周期标志
    if (timer_counter == 0) {
        newCycleFlag = true;
    }

    // 计数器递增
    timer_counter++;

    // 根据计数值设置相应的事件标志
    if (timer_counter == ADC_TRIGGER_COUNT) {
        triggerAdcFlag = true;
    } else if (timer_counter == PULSE_END_COUNT) {
        endPulseFlag = true;
    } else if (timer_counter == AFE_TRIGGER_COUNT) {
        triggerAFEFlag = true;
    } else if (timer_counter == AFE_END_COUNT) {
        endAFEFlag = true;
    }

    // 如果周期结束，重置计数器
    if (timer_counter >= CYCLE_END_COUNT) {
        timer_counter = 0;
    }
}

void initTimers() {
    // --- 配置一个高频主定时器 ---
    // 使用定时器0，预分频80 (ESP32-C3主频80MHz / 80 = 1MHz tick)
    masterTimer = timerBegin(0, 80, true); 
    timerAttachInterrupt(masterTimer, &onMasterTimer, true);
    // 设置警报值为25，产生25us周期的中断
    timerAlarmWrite(masterTimer, 25, true); // true表示自动重载
    
    // 使能定时器
    timerAlarmEnable(masterTimer);
}