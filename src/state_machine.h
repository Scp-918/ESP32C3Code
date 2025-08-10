#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"

// 定义系统运行的各个状态
enum SystemState {
    STATE_IDLE,                 // 空闲状态，等待新周期
    STATE_PULSE_HIGH_STARTED,   // 高电平脉冲已开始
    STATE_TRIGGER_AD7680,       // 准备触发AD7680
    STATE_READ_AD7680,          // 准备读取AD7680数据
    STATE_PULSE_LOW_STARTED,    // 低电平脉冲已开始，等待AFE触发延时
    STATE_TRIGGER_ADS1220,      // 准备触发ADS1220
    STATE_WAIT_ADS1220_READY,   // 等待ADS1220数据就绪 (DRDY)
    STATE_READ_ADS1220,         // 准备读取ADS1220数据
    STATE_PROCESS_DATA          // 处理和封装数据
};

// 声明全局状态变量 (volatile关键字用于确保在ISR和主循环间安全访问)
extern volatile SystemState currentState;
extern volatile bool newCycleFlag;
extern volatile bool triggerAdcFlag;
extern volatile bool endPulseFlag;
extern volatile bool startADCFlag;
extern volatile bool triggerAFEFlag;
extern volatile bool endAFEFlag;

// 声明状态机初始化和运行函数
void initState();
void runStateMachine();

#endif // STATE_MACHINE_H