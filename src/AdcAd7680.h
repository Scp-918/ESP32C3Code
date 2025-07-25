#ifndef ADC_AD7680_H
#define ADC_AD7680_H

#include <Arduino.h>
#include <SPI.h>

/**
 * @brief AD7680 16位SAR ADC驱动类
 * 
 * 提供一个轻量级的自定义驱动，用于控制AD7680并读取其转换结果。
 * 针对本项目的特定时序要求进行了优化。
 */
class AdcAd7680 {
public:
    /**
     * @brief 构造函数
     * @param cs_pin AD7680的片选引脚。
     * @param spi_settings SPI总线的配置对象。
     */
    AdcAd7680(uint8_t cs_pin, SPISettings spi_settings);

    /**
     * @brief 初始化AD7680驱动
     * 
     * 配置片选引脚。
     */
    void begin();

    /**
     * @brief 读取一次ADC转换结果
     * 
     * 该函数执行完整的采样序列：拉低CS启动转换，通过SPI读取数据，然后拉高CS。
     * @return uint16_t 16位的ADC原始数据。
     */
    uint16_t read();

private:
    uint8_t _cs_pin;         // 片选引脚
    SPISettings _spi_settings; // SPI配置
};

#endif // ADC_AD7680_H