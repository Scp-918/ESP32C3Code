#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "config.h"
#include "communication.h" // 引用DataFrame
#include "freertos/task.h" // 引入任务头文件

// 定义系统运行的各个状态
typedef enum {
    STATE_IDLE,
    STATE_READ_AD7680_DATA,
    STATE_PULSE_END,
    STATE_SET_IDAC,
    STATE_SET_IDAC2,
    STATE_READ_AD7680_DATA2,
    STATE_PROCESS_DATA
} SystemState;

// 使用位掩码定义事件标志
#define EVENT_NEW_CYCLE          (1 << 0) // 新周期开始
#define EVENT_TRIGGER_ADC        (1 << 1) // 触发AD7680转换
#define EVENT_END_PULSE          (1 << 2) // 高电平脉冲结束
#define EVENT_IDAC_AFE           (1 << 3) // 开始AFE电流输出
#define EVENT_TRIGGER_AFE        (1 << 4) // 触发AFE转换
#define EVENT_END_AFE            (1 << 5) // AFE转换结束

// 声明全局状态变量和事件标志
extern volatile SystemState currentState;
extern volatile uint32_t eventFlags; // 新的事件标志变量

// 声明数据变量
extern uint16_t ad7680_data;
extern uint16_t ad1220_data;
extern uint16_t ad7680_data2;

// 声明状态机初始化和运行函数
void initState();
// 状态机函数被修改为一个任务函数
void stateMachineTask(void *parameter);
// 原有的 runStateMachine() 函数不再使用，但为了保留注释，此处保留
void runStateMachine();

#endif // STATE_MACHINE_H