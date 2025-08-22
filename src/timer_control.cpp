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
    // 使用位或操作原子地设置事件标志
    // 在周期的开始 (t=0)，开始高电平脉冲
    if (timer_counter == 0) {
        // 确保下一个周期可以开始
        GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
        eventFlags |= EVENT_NEW_CYCLE;
        // 拉高IO引脚，开始高电平脉冲
        //GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
    }

    // 在指定时间点触发AD7680转换
    if (timer_counter == ADC_TRIGGER_COUNT) {
        eventFlags |= EVENT_TRIGGER_ADC;
    }

    // 在高电平脉冲结束时
    if (timer_counter == PULSE_END_COUNT) {
        // 拉低IO引脚，结束高电平脉冲
        GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL);
        eventFlags |= EVENT_END_PULSE;
    }

    //开始AFE电流输出
    if (timer_counter == AFE_START_COUNT) {
        // 电流源AD1220标志位
        GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL);
        eventFlags |= EVENT_IDAC_AFE;
    }

    // 在指定时间点触发AD7680转换
    if (timer_counter == AFE_TRIGGER_COUNT) {
        GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL);
        eventFlags |= EVENT_TRIGGER_AFE;
    }

    // 最晚转换结束时间
    if (timer_counter == AFE_END_COUNT) {
        eventFlags |= EVENT_END_AFE;
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