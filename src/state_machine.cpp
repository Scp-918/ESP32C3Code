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
volatile bool triggerAFEFlag = false;
volatile bool dataReadyAFE = false;
volatile bool processDataFlag = false;

// 存储采集数据的变量
uint16_t ad7680_data = 0;
uint32_t ads1220_data = 0;

void initState() {
    currentState = STATE_IDLE;
}

// 主状态机，由loop()函数持续调用
void runStateMachine() {
    switch (currentState) {
        case STATE_IDLE:
            if (newCycleFlag) {
                newCycleFlag = false;
                
                // 启动高电平脉冲和AD7680转换
                // GPIO操作已移至ISR，此处只需设置状态
                AD7680::triggerConversion();
                currentState = STATE_WAIT_AD7680;
            }
            break;

        case STATE_WAIT_AD7680:
            // 等待高电平脉冲结束，此时AD7680转换也应完成
            if (endPulseFlag) {
                endPulseFlag = false;
                currentState = STATE_READ_AD7680;
            }
            break;

        case STATE_READ_AD7680:
            ad7680_data = AD7680::readData();
            
            // 启动ADS1220转换
            ADS1220::startConversion();
            currentState = STATE_WAIT_ADS1220;
            break;

        case STATE_WAIT_ADS1220:
            // 轮询DRDY引脚，等待数据就绪
            if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
                dataReadyAFE = true;
                currentState = STATE_READ_ADS1220;
            }
            break;
            
        case STATE_READ_ADS1220:
            if (dataReadyAFE) {
                ads1220_data = ADS1220::readData();
                dataReadyAFE = false;
                ADS1220::powerDown();
                currentState = STATE_PROCESS_DATA;
            }
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