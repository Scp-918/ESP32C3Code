#include "state_machine.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include "timer_control.h"

// 定义全局状态变量
volatile SystemState currentState = STATE_IDLE;
volatile bool newCycleFlag = false;
volatile bool triggerAdcFlag = false;
volatile bool endPulseFlag = false;

// 存储采集数据的变量
uint16_t ad7680_data = 0;
uint32_t ads1220_data = 0;

// 用于AFE触发延时的计时器
unsigned long lowPulseStartTime = 0;

void initState() {
    currentState = STATE_IDLE;
}

// 主状态机，由loop()函数持续调用
void runStateMachine() {
    switch (currentState) {
        case STATE_IDLE:
            // 等待主定时器中断设置 newCycleFlag
            if (newCycleFlag) {
                noInterrupts(); // 进入临界区
                newCycleFlag = false;
                interrupts(); // 退出临界区
                
                // 启动高电平脉冲
                digitalWrite(PIN_SWITCH_CTRL, HIGH);
                
                // 不再需要启动单次定时器，因为主定时器已在运行
                // startOneShotTimers(); // <--- 此行已删除
                
                currentState = STATE_PULSE_HIGH_STARTED;
            }
            break;

        case STATE_PULSE_HIGH_STARTED:
            // 等待主定时器中断在50us时设置 triggerAdcFlag
            if (triggerAdcFlag) {
                noInterrupts();
                triggerAdcFlag = false;
                interrupts();
                currentState = STATE_TRIGGER_AD7680;
            }
            break;

        case STATE_TRIGGER_AD7680:
            // 触发AD7680转换 (仅拉低CS线)
            AD7680::triggerConversion();
            currentState = STATE_READ_AD7680;
            break;

        case STATE_READ_AD7680:
            // 在主循环中执行阻塞式SPI读取，避免在ISR中操作
            ad7680_data = AD7680::readData();
            // 等待主定时器中断在125us时设置 endPulseFlag
            if (endPulseFlag) {
                noInterrupts();
                endPulseFlag = false;
                interrupts();
                
                // 结束高电平脉冲
                digitalWrite(PIN_SWITCH_CTRL, LOW);
                lowPulseStartTime = millis(); // 记录低电平开始时间
                currentState = STATE_PULSE_LOW_STARTED;
            }
            break;

        case STATE_PULSE_LOW_STARTED:
            // 等待3.75ms的延时
            if (millis() - lowPulseStartTime >= (unsigned long)AFE_TRIGGER_DELAY_MS) {
                currentState = STATE_TRIGGER_ADS1220;
            }
            break;

        case STATE_TRIGGER_ADS1220:
            ADS1220::startConversion();
            currentState = STATE_WAIT_ADS1220_READY;
            break;

        case STATE_WAIT_ADS1220_READY:
            // 轮询DRDY引脚，等待数据就绪
            if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
                currentState = STATE_READ_ADS1220;
            }
            // 可在此处添加超时逻辑
            break;

        case STATE_READ_ADS1220:
            ads1220_data = ADS1220::readData();
            ADS1220::powerDownIdacs(); // 测量后关闭IDAC
            currentState = STATE_PROCESS_DATA;
            break;

        case STATE_PROCESS_DATA:
            // 封装数据并放入缓冲区
            addDataToBuffer(ad7680_data, ads1220_data);
            // 检查缓冲区是否已满并发送
            sendBufferIfFull();
            currentState = STATE_IDLE; // 回到空闲状态，等待下一个周期
            break;
    }
}