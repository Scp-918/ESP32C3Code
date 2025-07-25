#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#include <Arduino.h>

/**
 * @brief 串行通信模块类
 * 
 * 负责将采集到的ADC数据按照指定协议打包成数据帧，并通过串口发送。
 */
class SerialComms {
public:
    /**
     * @brief 构造函数
     * @param stream 一个指向串口对象的指针 (例如 &Serial)。
     */
    SerialComms(Stream* stream);

    /**
     * @brief 初始化串口通信
     * @param baud_rate 串口波特率。
     */
    void begin(unsigned long baud_rate);

    /**
     * @brief 发送一帧数据
     * 
     * @param ad7680_data 从AD7680读取的16位数据。
     * @param ads1220_data 从ADS1220读取的24位数据。
     */
    void sendDataFrame(uint16_t ad7680_data, int32_t ads1220_data);

private:
    Stream* _stream; // 指向串口对象的指针，增加了灵活性
};

#endif // SERIAL_COMMS_H