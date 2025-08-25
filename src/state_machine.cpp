#include "state_machine.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 定义全局状态变量
volatile SystemState currentState = STATE_IDLE;
volatile uint32_t eventFlags = 0; // 初始化事件标志为0

// 存储采集数据的变量
uint16_t ad7680_data = 0;
uint16_t ad1220_data = 0;
uint16_t ad7680_data2 = 0;

void initState() {
    currentState = STATE_IDLE;
    gpio_pad_select_gpio((gpio_num_t)PIN_SWITCH_CTRL2);
    gpio_set_direction((gpio_num_t)PIN_SWITCH_CTRL2, GPIO_MODE_OUTPUT);
    GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL2); // 默认不选中
}

// 主状态机，现在作为一个独立的任务持续运行
void stateMachineTask(void *parameter) {
    // 存储采集数据的变量
    uint16_t ad7680_data = 0;
    uint16_t ad1220_data = 0;
    
    // 初始化状态机
    initState();

    while(1) {
        // 使用本地变量保存事件标志，防止在处理过程中被ISR修改
        /*
        noInterrupts();
        uint32_t currentEvents = eventFlags;
        interrupts();
        */

        // 使用快速分支预测优化的switch语句
        switch (currentState) {
            case STATE_IDLE:
                if (eventFlags & EVENT_NEW_CYCLE) {
                    noInterrupts();
                    eventFlags &= ~EVENT_NEW_CYCLE; // 清除标志位
                    interrupts();                    
                    //ad7680_sample_count = 0;
                    //digitalWrite(18, HIGH);
                    GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL); // 选中
                    currentState = STATE_READ_AD7680_DATA;
                }
                break; // 使用break而不是return

            case STATE_READ_AD7680_DATA:
                if (eventFlags & EVENT_TRIGGER_ADC) {
                    noInterrupts();
                    eventFlags &= ~EVENT_TRIGGER_ADC; // 清除标志位
                    interrupts();
                    //ad7680_data = AD7680::readDataMean(ad7680_data);
                    //ad7680_data = AD7680::readData();
                }
                if (eventFlags & EVENT_END_PULSE) {
                    noInterrupts();
                    eventFlags &= ~EVENT_END_PULSE; // 清除标志位
                    interrupts();
                    //短时屏蔽中断
                    //digitalWrite(18, LOW);
                    GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL); // 不选中
                    currentState = STATE_SET_IDAC;
                }
                break;

            case STATE_SET_IDAC:
                if (eventFlags & EVENT_IDAC_AFE) {
                    noInterrupts();
                    eventFlags &= ~EVENT_IDAC_AFE; // 清除标志位
                    interrupts();
                    //digitalWrite(PIN_SWITCH_CTRL, HIGH);
                    //digitalWrite(18, HIGH);
                    GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL); // 选中
                    //Serial.println("IDAC");
                    currentState = STATE_READ_AD7680_DATA2;
                }
                break;

            case STATE_READ_AD7680_DATA2:
                if (eventFlags & EVENT_TRIGGER_AFE) {
                    noInterrupts();
                    eventFlags &= ~EVENT_TRIGGER_AFE; // 清除标志位
                    interrupts();
                    //ad1220_data = ADS1220::readData();
                    //digitalWrite(PIN_SWITCH_CTRL, LOW);
                    //digitalWrite(18, LOW);
                    GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL); // 不选中
                    //Serial.println("AFEEND");
                    currentState = STATE_PROCESS_DATA;
                }
                break;

            case STATE_PROCESS_DATA:
                // 封装数据并直接通过DMA发送
                DataFrame frame;
                frame.header1 = FRAME_HEADER1;
                frame.header2 = FRAME_HEADER2;
                frame.adc_datahigh = (ad7680_data >> 8) & 0xFF;
                frame.adc_datalow = ad7680_data & 0xFF;
                frame.afe_datahigh = 0x00;
                frame.afe_datamedium = (ad1220_data >> 8) & 0xFF;
                frame.afe_datalow = ad1220_data & 0xFF;
                frame.checksum = frame.adc_datahigh ^ frame.adc_datalow ^ frame.afe_datahigh ^ frame.afe_datamedium ^ frame.afe_datalow;
                frame.footer1 = FRAME_FOOTER1;
                frame.footer2 = FRAME_FOOTER2;
                
                // 将数据发送给通信函数，该函数会将其推入DMA缓冲区
                //sendDataFrame(&frame);

                currentState = STATE_IDLE; // 回到空闲状态，等待下一个周期
                break;
        }
        // 短暂延迟，让 FreeRTOS 调度器有机会切换到其他任务
        vTaskDelay(1);
    }
}
// 原有的 runStateMachine() 函数不再使用，但为了保留原始代码，此处保留
void runStateMachine() {
    // 核心架构：主循环只负责快速、非阻塞地运行状态机
    /*
    noInterrupts();
    uint32_t currentEvents = eventFlags;
    eventFlags = 0;
    interrupts();
    */

    // ... 原始switch语句 ...
            // 使用快速分支预测优化的switch语句
    switch (currentState) {
        case STATE_IDLE:
            if (eventFlags & EVENT_NEW_CYCLE) {
                noInterrupts();
                eventFlags &= ~EVENT_NEW_CYCLE; // 清除标志位
                interrupts();                    
                //ad7680_sample_count = 0;
                //digitalWrite(18, HIGH);
                //GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL); // 选中
                GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL); // 选中
                currentState = STATE_READ_AD7680_DATA;
            }
            break; // 使用break而不是return

        case STATE_READ_AD7680_DATA:
            if (eventFlags & EVENT_TRIGGER_ADC) {
                noInterrupts();
                eventFlags &= ~EVENT_TRIGGER_ADC; // 清除标志位
                interrupts();
                //ad7680_data = AD7680::readDataMean(ad7680_data);
                //ad7680_data = AD7680::readData();
                currentState = STATE_PULSE_END;
            }
            break;

        case STATE_PULSE_END:
            if (eventFlags & EVENT_END_PULSE) {
                noInterrupts();
                eventFlags &= ~EVENT_END_PULSE; // 清除标志位
                interrupts();
                //短时屏蔽中断
                //digitalWrite(18, LOW);
                GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL); // 不选中
                //currentState = STATE_SET_IDAC;
                currentState = STATE_PROCESS_DATA;
            }
            break;

        case STATE_SET_IDAC:
            if (eventFlags & EVENT_IDAC_AFE) {
                noInterrupts();
                eventFlags &= ~EVENT_IDAC_AFE; // 清除标志位
                interrupts();
                //ADS1220::reset();
                //ADS1220::startConversion();
                //digitalWrite(PIN_SWITCH_CTRL, HIGH);
                //digitalWrite(18, HIGH);
                //GPIO.out_w1ts.val = (1U << PIN_SWITCH_CTRL2); // 选中
                //Serial.println("IDAC");
                currentState = STATE_SET_IDAC2;
            }
            break;

        case STATE_SET_IDAC2:
            if(digitalRead(PIN_DRDY_ADS1220) == LOW){
                uint32_t ad1220_data3 = ADS1220::readData();
                currentState = STATE_READ_AD7680_DATA2;
            }
            break;

        case STATE_READ_AD7680_DATA2:
            if (eventFlags & EVENT_TRIGGER_AFE) {
                noInterrupts();
                eventFlags &= ~EVENT_TRIGGER_AFE; // 清除标志位
                interrupts();
                //ad1220_data = AD7680::readData();
                ADS1220::powerDown();
                //ADS1220::reset();
                //ADS1220::configureoff();
                //digitalWrite(PIN_SWITCH_CTRL, LOW);
                //digitalWrite(18, LOW);
                //GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL2); // 不选中
                //Serial.println("AFEEND");
                currentState = STATE_PROCESS_DATA;
            }

            break;

        case STATE_PROCESS_DATA:
            // 封装数据并直接通过DMA发送
            DataFrame frame;
            frame.header1 = FRAME_HEADER1;
            frame.header2 = FRAME_HEADER2;
            frame.adc_datahigh = (ad7680_data >> 8) & 0xFF;
            frame.adc_datalow = ad7680_data & 0xFF;
            frame.afe_datahigh = 0x00;
            frame.afe_datamedium = (ad1220_data >> 8) & 0xFF;
            frame.afe_datalow = ad1220_data & 0xFF;
            frame.checksum = frame.adc_datahigh ^ frame.adc_datalow ^ frame.afe_datahigh ^ frame.afe_datamedium ^ frame.afe_datalow;
            frame.footer1 = FRAME_FOOTER1;
            frame.footer2 = FRAME_FOOTER2;
            
            // 将数据发送给通信函数，该函数会将其推入DMA缓冲区
            //sendDataFrame(&frame);

            currentState = STATE_IDLE; // 回到空闲状态，等待下一个周期
            break;
    }
}
