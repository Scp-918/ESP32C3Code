#include <Arduino.h>
#include <SPI.h>
#include "TimingController.h"
#include "AdcAd7680.h"
#include "AfeAds1220.h"
#include "SerialComms.h"

// --- 引脚定义 (根据表1) ---
#define PIN_SPI_MISO 10
#define PIN_SPI_MOSI 3
#define PIN_SPI_SCK  2
#define PIN_CS_AD7680 7
#define PIN_CS_ADS1220 4
#define PIN_DRDY_ADS1220 5
#define PIN_SW_CTRL 6 // 已在TimingController.cpp中定义，此处为注释提醒

// --- SPI配置 ---
// AD7680: SPI Mode 3, 2.5MHz
SPISettings spi_settings_ad7680(2500000, MSBFIRST, SPI_MODE3);
// ADS1220: SPI Mode 1, 4MHz (库的默认值，通常是安全的)
// 我们将使用一个通用的SPI总线，在每次通信前通过beginTransaction切换设置

// --- 模块实例化 ---
TimingController timingController;
AdcAd7680 ad7680(PIN_CS_AD7680, spi_settings_ad7680);
AfeAds1220 afe1220(PIN_CS_ADS1220, PIN_DRDY_ADS1220);
SerialComms serialComms(&Serial);

// --- 全局变量 ---
uint16_t ad7680_result = 0;
int32_t ads1220_result = 0;

void setup() {
    // 1. 初始化串口，用于调试信息输出和数据帧发送
    Serial.begin(115200);
    while (!Serial); // 等待串口连接
    Serial.println("\n--- ESP32-C3 高精度采集系统 ---");

    // 2. 初始化SPI总线
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, -1); // CS由软件管理，设为-1

    // 3. 初始化各个模块
    ad7680.begin();
    if (!afe1220.begin()) {
        Serial.println("AFE初始化失败，程序挂起。");
        while(1);
    }
    if (!timingController.begin()) {
        Serial.println("定时器初始化失败，程序挂起。");
        while(1);
    }
    serialComms.begin(115200);

    Serial.println("系统初始化完成，等待第一个测量周期...");
}

void loop() {
    // 主循环的核心是非阻塞的。它只检查ISR设置的标志位。
    if (g_measurementCycleStart) {
        // 清除标志位，准备处理本周期事件
        g_measurementCycleStart = false;

        // 记录周期开始的时间戳
        unsigned long cycleStartTime = micros();
        unsigned long currentTime;

        // --- 高电平激励阶段 (0us - 125us) ---

        // 1. 等待直到高电平经过50us
        while (micros() - cycleStartTime < 50);

        // 2. 启动AD7680转换并读取结果
        ad7680_result = ad7680.read();

        // 3. 等待直到高电平经过125us
        while (micros() - cycleStartTime < 125);
        
        // 4. 切换到低电平激励（比例式电桥）
        digitalWrite(PIN_SW_CTRL, HIGH);
        
        // --- 低电平激励阶段 (125us - 6250us) ---
        
        // 5. 切换到低电平后，立即开启IDAC电流激励
        afe1220.enableIdac(true);

        // 6. 等待直到低电平经过3.75ms (即周期开始后 125us + 3750us = 3875us)
        while (micros() - cycleStartTime < 3875);

        // 7. 启动ADS1220转换并读取结果
        ads1220_result = afe1220.read();
        
        // 8. 测量结束后，立即关闭IDAC电流激励以省电和减少发热
        afe1220.enableIdac(false);

        // --- 数据发送 ---
        // 9. 将本周期采集到的两个数据打包并通过串口发送
        serialComms.sendDataFrame(ad7680_result, ads1220_result);

        // 周期剩余时间为等待期，直到下一次ISR触发
    }
}