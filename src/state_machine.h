#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"

// 定义系统运行的各个状态
typedef enum {
    STATE_IDLE,
    STATE_READ_AD7680_DATA,
    STATE_SET_IDAC,
    STATE_READ_AD7680_DATA2,
    STATE_PROCESS_DATA
} SystemState;

// 声明全局状态变量 (volatile关键字用于确保在ISR和主循环间安全访问)
extern volatile SystemState currentState;
extern volatile bool newCycleFlag;
extern volatile bool triggerAdcFlag;
extern volatile bool endPulseFlag;
extern volatile bool idacAFEFlag;
extern volatile bool triggerAFEFlag;
extern volatile bool endAFEFlag;
extern volatile int ads1220_read_count;

// 声明数据变量
extern uint16_t ad7680_data;
extern uint16_t ad1220_data;
extern uint16_t ad7680_data2;

// 声明状态机初始化和运行函数
void initState();
void runStateMachine();

#endif // STATE_MACHINE_H