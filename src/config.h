#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =================================================================
// == 全局常量定义 (Global Constants)
// =================================================================

// --- 通信设置 (Communication Settings) ---
#define SERIAL_BAUD_RATE 460800
#define FRAME_HEADER1 0xAA
#define FRAME_FOOTER1 0xCC
#define FRAME_HEADER2 0xBB
#define FRAME_FOOTER2 0xDD
#define COMM_BUFFER_FRAME_COUNT 10 // 每10帧打包发送一次

// --- 时序控制 (Timing Control) ---
#define PULSE_FREQUENCY_HZ 100              // 脉冲频率 (Hz)
#define HIGH_PULSE_DUTY_CYCLE_PERCENT 2     // 高电平占空比 (%)

// ADC转换和SPI通信时序
#define AD7680_CONVERSION_TIME_US 100       // AD7680多次转换+取平均所需时间 (µs)
#define ADS1220_START_TIME_US 1000            // ADS1220需要持续的时间 (µs)
#define ADS1220_CONVERSION_TIME_US 500      // ADS1220多次转换+取平均所需时间 (µs)
#define AFE_TRIGGER_DELAY_US 2000            // ADS1220在低电平开始后的触发延迟 (µs)

// 由频率和占空比计算得到
#define TOTAL_CYCLE_TIME_US (1000000 / PULSE_FREQUENCY_HZ)
//#define HIGH_PULSE_WIDTH_US (TOTAL_CYCLE_TIME_US * HIGH_PULSE_DUTY_CYCLE_PERCENT / 100)
#define HIGH_PULSE_WIDTH_US 1000
#define LOW_PULSE_WIDTH_US (TOTAL_CYCLE_TIME_US - HIGH_PULSE_WIDTH_US)

// --- 定时器中断计数 (基于25us中断周期) ---
#define TIMER_TICK_US 25
#define CYCLE_END_COUNT (TOTAL_CYCLE_TIME_US / TIMER_TICK_US)
#define PULSE_END_COUNT (HIGH_PULSE_WIDTH_US / TIMER_TICK_US)
#define ADC_TRIGGER_COUNT ((HIGH_PULSE_WIDTH_US - AD7680_CONVERSION_TIME_US) / TIMER_TICK_US)
//#define AFE_START_COUNT ((TOTAL_CYCLE_TIME_US - ADS1220_START_TIME_US) / TIMER_TICK_US)
//#define AFE_TRIGGER_COUNT (AFE_START_COUNT + 20)
#define AFE_START_COUNT (CYCLE_END_COUNT-160)
#define AFE_TRIGGER_COUNT (CYCLE_END_COUNT-80)
#define AFE_END_COUNT (CYCLE_END_COUNT-40)

// AD7680平均值计数
#define AD7680_AVERAGE_COUNT 6

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

#define PIN_SWITCH_CTRL2 18

#endif // CONFIG_H