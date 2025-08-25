#include "config.h"
#include "state_machine.h"
#include "timer_control.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include <SPI.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h" // 引入FreeRTOS库
#include "freertos/task.h"     // 引入任务相关头文件

// 声明状态机任务句柄
//TaskHandle_t StateMachineTaskHandle = NULL;

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
    initCommunication2();
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

    //vTaskPrioritySet(NULL, 20);
    //Serial.println("Loop task priority set to 20.");
    
    /*
    // 5. 创建高优先级任务来运行状态机
    xTaskCreatePinnedToCore(
        stateMachineTask,   // 任务函数
        "StateMachineTask", // 任务名称
        4096,               // 堆栈大小 (根据需要调整)
        NULL,               // 任务参数
        20,                 // 任务优先级 (高于默认的1，确保高实时性)
        &StateMachineTaskHandle, // 任务句柄
        0 // 运行在核心0
    );
    */
    
    Serial.println("State Machine Task created.");

    // 6. 初始化并启动定时器，它会触发状态机任务
    initTimers();
    Serial.println("Timers Initialized and Started. System is running.");
    //delay(2000);
}

void loop() {
    // 核心架构：主循环现在是空的，所有逻辑都在独立的FreeRTOS任务中执行
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
    //vTaskDelay(1000); // 无限延迟，不执行任何操作
    runStateMachine();
    
    //ADS1220::reset();
    /*
    ADS1220::startConversion();
    delay(100); // 等待初始化读出数据
    if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
        uint32_t ads1220_data3 = ADS1220::readData();
    }
    delay(3000);
    ADS1220::powerDown();
    delay(3000);
    */
    /*
    digitalWrite(PIN_SWITCH_CTRL, LOW);
    delay(1000);
    digitalWrite(PIN_SWITCH_CTRL, HIGH);
    delay(1000);
    */
    //digitalWrite(PIN_SWITCH_CTRL, HIGH);
    //delay(1000);
    //Serial.println("Loop");
}