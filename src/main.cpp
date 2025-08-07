#include "config.h"
#include "state_machine.h"
#include "timer_control.h"
#include "AD7680_driver.h"
#include "ADS1220_driver.h"
#include "communication.h"
#include <SPI.h>

uint32_t ads1220_data2=0;

void setup() {
    pinMode(PIN_SWITCH_CTRL, OUTPUT);
    digitalWrite(PIN_SWITCH_CTRL, LOW);

    // 1. 初始化通信
    initCommunication();
    Serial.println("ESP32-C3 Firmware Initializing...");

    // 2. 初始化SPI总线
    // 明确指定SPI引脚，防止使用错误的默认引脚
    SPI.begin(PIN_SPI_SCLK, PIN_SPI_MISO, PIN_SPI_MOSI, -1); // -1表示手动管理CS

    // 3. 初始化硬件驱动
    AD7680::init();
    ADS1220::init();
    // 4. 配置外部芯片
    ADS1220::reset();
    ADS1220::configure();
    //ADS1220::powerUpIdacs();
    Serial.println("ADS1220 Configured.");

    // 5. 初始化状态机
    initState();
    Serial.println("State Machine Initialized.");

    // 6. 初始化并启动定时器
    initTimers();
    Serial.println("Timers Initialized and Started. System is running.");
    delay(2000);
    
    
}
void loop() {
    // 核心架构：主循环只负责快速、非阻塞地运行状态机
    // 1. 确保IDAC是开启的 (通过configure)
    ADS1220::reset();
    ADS1220::configure(); // IDAC设置为250uA
    Serial.println("ADS1220 configured. IDAC should be ON (250uA).");
    delay(10); // 观察IDAC开启状态
    if (digitalRead(PIN_DRDY_ADS1220) == LOW) {
        ads1220_data2 = ADS1220::readData();
    } 
    delay(5000);

    // 2. 发送POWERDOWN指令
    ADS1220::powerDown();
    Serial.println("POWERDOWN command sent. IDAC should be OFF now.");
    delay(5000); // 长时间观察IDAC关闭状态
    //runStateMachine(); 

    //delay(3000);
}