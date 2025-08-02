#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =================================================================
// == 全局常量定义 (Global Constants)
// =================================================================

// --- 通信设置 (Communication Settings) ---
#define SERIAL_BAUD_RATE 115200
#define FRAME_HEADER 0xAA
#define FRAME_FOOTER 0xBB
#define COMM_BUFFER_FRAME_COUNT 16 // 每16帧打包发送一次

// --- 时序控制 (Timing Control) ---
#define PULSE_FREQUENCY 160       // 脉冲频率 (Hz)
#define HIGH_PULSE_WIDTH_US 125   // 高电平脉冲宽度 (µs)
#define ADC_TRIGGER_TIME_US 50    // AD7680 触发时间 (µs)
#define AFE_TRIGGER_DELAY_MS 2.5 // ADS1220 在低电平开始后的触发延迟 (ms)
#define AFE_LONGEST_DELAY_MS 6 // ADS1220 最长延迟 (ms)

// =================================================================
// == 引脚定义 (Pin Definitions)
//!! 警告: 请根据您的PCB硬件连接修改以下引脚编号!!
// =================================================================

// --- SPI 总线引脚 (SPI Bus Pins) ---
// 在ESP32-C3上, 默认的硬件SPI引脚是:
#define PIN_SPI_SCLK 6
#define PIN_SPI_MISO 2
#define PIN_SPI_MOSI 3

// --- ADC/AFE 片选引脚 (Chip Select Pins) ---
#define PIN_CS_AD7680 10
#define PIN_CS_ADS1220 1

// --- ADS1220 数据就绪引脚 (Data Ready Pin) ---
#define PIN_DRDY_ADS1220 4

// --- 模拟开光控制引脚 (Analog Switch Control Pin) ---
// 高电平: 惠斯通电桥 (AD7680)
// 低电平: 比例式电桥 (ADS1220)
#define PIN_SWITCH_CTRL 12

#endif // CONFIG_H