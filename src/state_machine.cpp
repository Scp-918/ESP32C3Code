#include "state_machine.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include "config.h"

// 定义全局状态变量
volatile SystemState currentState = STATE_IDLE;
volatile bool newCycleFlag = false;
volatile bool triggerAdcFlag = false;
volatile bool endPulseFlag = false;
volatile bool idacAFEFlag = false;
volatile bool triggerAFEFlag = false;
volatile bool endAFEFlag = false;
volatile int ads1220_read_count = 0;

// 存储采集数据的变量
uint16_t ad7680_data = 0;
uint16_t ad1220_data = 0;
uint16_t ad7680_data2 = 0;

void initState() {
    currentState = STATE_IDLE;
}

// 处理缓冲区发送，在主循环中调用，不会阻塞状态机
void handleCommunication() {
    sendBufferIfFull();
}

// 主状态机，由loop()函数持续调用
void runStateMachine() {
    switch (currentState) {
        case STATE_IDLE:
            if (newCycleFlag) {
                noInterrupts();
                newCycleFlag = false;
                interrupts();
                //ad7680_sample_count = 0;
                digitalWrite(18, HIGH);
                currentState = STATE_READ_AD7680_DATA;
                
            }
            break;


        case STATE_READ_AD7680_DATA:
            if (triggerAdcFlag) {
                noInterrupts();
                triggerAdcFlag = false;
                interrupts();
                //ad7680_data = AD7680::readDataMean(ad7680_data);
                //ad7680_data = AD7680::readData();                
            }
            if (endPulseFlag) {
                //短时屏蔽中断
                noInterrupts();
                endPulseFlag = false;
                interrupts();
                digitalWrite(18, LOW);
                currentState = STATE_SET_IDAC;
                
            }
            
            break;

        case STATE_SET_IDAC:
            // 设置AFE电流
            
            /*
            if (idacAFEFlag) {
                idacAFEFlag = false;
                ADS1220::reset();
                ADS1220::configure(); // IDAC设置为250uA，控制电流输出250uA 
                digitalWrite(18, HIGH);
                //digitalWrite(PIN_SWITCH_CTRL, HIGH); 
                currentState = STATE_READ_AD7680_DATA2;  
            }
                */
            
            /*
            if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
                uint32_t ads1220_data2 = ADS1220::readData();
                currentState = STATE_READ_AD7680_DATA2;
            }
                */
            
            
            
            if (idacAFEFlag) {
                noInterrupts();
                idacAFEFlag = false;
                interrupts();
                //digitalWrite(PIN_SWITCH_CTRL, HIGH);
                digitalWrite(18, HIGH);
                Serial.println("IDAC");
                currentState = STATE_READ_AD7680_DATA2;  
            }
                
            
                

            break;        

        case STATE_READ_AD7680_DATA2:
            /*
            // 第二次读AD7680
            if (triggerAFEFlag) {
                triggerAFEFlag = false;
                //ad1220_data = AD7680::readDataMean(ad1220_data);
                ADS1220::powerDown(); // 测量后关闭IDAC
                digitalWrite(18, LOW);
                currentState = STATE_PROCESS_DATA;
            }else if(endAFEFlag){
                endAFEFlag = false;
                ADS1220::powerDown(); // 测量后关闭IDAC
                currentState = STATE_PROCESS_DATA;
            }
                */
            
           
            if (triggerAFEFlag) {
                noInterrupts();
                triggerAFEFlag = false;
                interrupts();
                //ad1220_data = AD7680::readDataMean(ad1220_data);
                //digitalWrite(PIN_SWITCH_CTRL, LOW);
                digitalWrite(18, LOW);
                Serial.println("AFEEND");
                currentState = STATE_PROCESS_DATA;
            }
                
            break;

        case STATE_PROCESS_DATA:
            // 封装数据并放入缓冲区
            addDataToBuffer(ad7680_data, ad1220_data); // 使用第一个样本作为示例
            
            currentState = STATE_IDLE; // 回到空闲状态，等待下一个周期
            break;
    }
}