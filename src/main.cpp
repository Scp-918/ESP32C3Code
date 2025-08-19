#include "config.h"
#include "state_machine.h"
#include "timer_control.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include <SPI.h>
#include "driver/gpio.h"

void setup() {

    // 1. 初始化GPIO
    pinMode(PIN_SWITCH_CTRL, OUTPUT);
    digitalWrite(PIN_SWITCH_CTRL, LOW);

    pinMode(18, OUTPUT);
    digitalWrite(18, LOW);
    /*
    gpio_pad_select_gpio((gpio_num_t)PIN_SWITCH_CTRL);
    gpio_set_direction((gpio_num_t)PIN_SWITCH_CTRL, GPIO_MODE_OUTPUT);
    GPIO.out_w1tc.val = (1U << PIN_SWITCH_CTRL); // 确保引脚默认拉低
    */
    // 2. 初始化通信
    initCommunication();
    Serial.println("ESP32-C3 Firmware Initializing...");

    // 3. 初始化SPI总线
    // 明确指定SPI引脚，防止使用错误的默认引脚
    SPI.begin(PIN_SPI_SCLK, PIN_SPI_MISO, PIN_SPI_MOSI, -1); // -1表示手动管理CS

    // 4. 初始化硬件驱动
    AD7680::init();
    ADS1220::init();
    ADS1220::reset();
    ADS1220::configure();
    delay(100); // 等待初始化读出数据
    if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
        uint32_t ads1220_data2 = ADS1220::readData();
    }  
    ADS1220::powerDown();
    Serial.println("ADS1220 Configured.");    

    // 5. 初始化状态机
    initState();
    Serial.println("State Machine Initialized.");

    // 6. 初始化并启动定时器
    initTimers();
    //Serial.println("Timers Initialized and Started. System is running.");
    //delay(2000);
}

void loop() {
    // 核心架构：主循环只负责快速、非阻塞地运行状态机
    runStateMachine(); 
    /* 
    float start = micros();
    //unsigned long duration = millis() - start; // 单位：毫秒
    digitalWrite(18, !digitalRead(18)); // 电平翻转
    float duration = micros() - start; // 单位：毫秒
    Serial.println(duration, 6);//显示小数点后六位
    */
    /*
    Serial.println(CYCLE_END_COUNT);
    Serial.println(PULSE_END_COUNT);
    Serial.println(ADC_TRIGGER_COUNT);
    Serial.println(AFE_START_COUNT);
    Serial.println(AFE_TRIGGER_COUNT);
    Serial.println(AFE_END_COUNT);
    delay(1000);
    */
}