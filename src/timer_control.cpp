#include "timer_control.h"
#include "state_machine.h"

hw_timer_t *mainCycleTimer = NULL;
hw_timer_t *adcTriggerTimer = NULL;
hw_timer_t *pulseEndTimer = NULL;

// 160Hz 周期性定时器中断服务程序
void IRAM_ATTR onMainCycleTimer() {
    // 仅设置标志位，通知主循环开始新周期
    // 实际的IO操作和定时器启动移至状态机中，以保证ISR极简
    newCycleFlag = true;
}

// 50us 单次定时器中断服务程序
void IRAM_ATTR onAdcTriggerTimer() {
    // 仅设置标志位，通知主循环触发ADC
    triggerAdcFlag = true;
}

// 125us 单次定时器中断服务程序
void IRAM_ATTR onPulseEndTimer() {
    // 仅设置标志位，通知主循环结束脉冲
    endPulseFlag = true;
}

void initTimers() {
    // --- 配置160Hz主周期定时器 ---
    // 使用定时器0，预分频80 (80MHz / 80 = 1MHz)
    mainCycleTimer = timerBegin(0, 80, true); 
    timerAttachInterrupt(mainCycleTimer, &onMainCycleTimer, true);
    // 1MHz / 160Hz = 6250
    uint64_t alarmValue = 1000000 / PULSE_FREQUENCY;
    timerAlarmWrite(mainCycleTimer, alarmValue, true); // 自动重载
    
    // --- 配置50us单次触发定时器 ---
    // 使用定时器1，预分频80 (1MHz tick)
    adcTriggerTimer = timerBegin(1, 80, true);
    timerAttachInterrupt(adcTriggerTimer, &onAdcTriggerTimer, true);
    timerAlarmWrite(adcTriggerTimer, ADC_TRIGGER_TIME_US, false); // false表示不自动重载

    // --- 配置125us单次触发定时器 ---
    // 使用定时器2，预分频80 (1MHz tick)
    pulseEndTimer = timerBegin(2, 80, true);
    timerAttachInterrupt(pulseEndTimer, &onPulseEndTimer, true);
    timerAlarmWrite(pulseEndTimer, HIGH_PULSE_WIDTH_US, false); // false表示不自动重载

    // 初始时使能主周期定时器
    timerAlarmEnable(mainCycleTimer);
}

void startOneShotTimers() {
    // 重置并启动两个单次定时器
    timerRestart(adcTriggerTimer);
    timerAlarmEnable(adcTriggerTimer);
    
    timerRestart(pulseEndTimer);
    timerAlarmEnable(pulseEndTimer);
}