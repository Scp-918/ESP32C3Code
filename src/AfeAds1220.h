#ifndef AFE_ADS1220_H
#define AFE_ADS1220_H

#include <Arduino.h>
#include <ADS1220_WE.h>
#include <SPI.h>

/**
 * @brief ADS1220 24位AFE接口类
 * 
 * 封装了ADS1220_WE库，提供符合本项目需求的高层接口。
 * 负责初始化AFE的复杂配置，并提供简单的读取方法。
 */
class AfeAds1220 {
public:
    /**
     * @brief 构造函数
     * @param cs_pin ADS1220的片选引脚。
     * @param drdy_pin ADS1220的数据就绪引脚。
     */
    AfeAds1220(uint8_t cs_pin, uint8_t drdy_pin);

    /**
     * @brief 初始化AFE
     * 
     * 根据项目需求配置ADS1220的输入通道、增益、数据率、参考源和IDAC。
     * @return 如果初始化成功，则返回true；否则返回false。
     */
    bool begin();

    /**
     * @brief 启动一次AFE转换并读取结果
     * 
     * 这是一个阻塞函数，它会启动转换并等待DRDY信号，然后返回24位结果。
     * @return int32_t 24位的ADC原始数据（以32位有符号整数形式返回）。
     */
    int32_t read();

    /**
     * @brief 启用或禁用IDAC电流输出
     * 
     * @param enable true为启用，false为禁用。
     */
    void enableIdac(bool enable);

private:
    ADS1220_WE ads; // ADS1220_WE库的实例
    uint8_t _cs_pin;
    uint8_t _drdy_pin;
};

#endif // AFE_ADS1220_H