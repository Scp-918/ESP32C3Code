#include "timer_control.h"
#include "state_machine.h"
#include "config.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "esp_log.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/gpio.h"

// 仅需一个主定时器
hw_timer_t *masterTimer = NULL;

// 用于在ISR中计数的volatile变量
volatile int timer_counter = 0;

// 主定时器的中断服务程序，每25us调用一次，声明为IRAM_ATTR以保证中断响应速度
void IRAM_ATTR onMasterTimer() {
    // 在周期的开始 (t=0)，开始高电平脉冲
    if (timer_counter == 0) {
        // 确保下一个周期可以开始
        newCycleFlag = true;
        // 拉高IO引脚，开始高电平脉冲
        //GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
        GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
    }

    // 在指定时间点触发AD7680转换
    if (timer_counter == ADC_TRIGGER_COUNT) {
        triggerAdcFlag = true;
    }

    // 在高电平脉冲结束时
    if (timer_counter == PULSE_END_COUNT) {
        // 拉低IO引脚，结束高电平脉冲
        GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL);
        endPulseFlag = true;
    }

    //开始AFE电流输出
    if (timer_counter == AFE_START_COUNT) {
        // 电流源AD1220标志位
        idacAFEFlag = true;
        GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
    }

    // 在指定时间点触发AD7680转换
    if (timer_counter == AFE_TRIGGER_COUNT) {
        triggerAFEFlag = true;
    }

    // 最晚转换结束时间
    if (timer_counter == AFE_END_COUNT) {
        endAFEFlag = true;
        GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL);
    }

    // 计数器递增
    timer_counter++;

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
    timerAlarmWrite(masterTimer, TIMER_TICK_US, true); // true表示自动重载
    
    // 使能定时器
    timerAlarmEnable(masterTimer);
}