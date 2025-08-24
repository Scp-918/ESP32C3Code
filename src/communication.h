#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "config.h"
#include "driver/uart.h" // 引入ESP-IDF的UART驱动

// 数据帧结构 (10字节)
struct DataFrame {
    uint8_t header1;
    uint8_t header2;
    uint8_t adc_datahigh; // 16位ADC数据的高8位
    uint8_t adc_datalow;  // 16位ADC数据的低8位
    uint8_t afe_datahigh; // 24位AFE数据的高8位
    uint8_t afe_datamedium;  // 24位AFE数据的中间8位
    uint8_t afe_datalow; // 24位AFE数据的低8位
    uint8_t checksum;
    uint8_t footer1;
    uint8_t footer2;
};

void initCommunication();
// 新的非阻塞发送函数
void sendDataFrame(const DataFrame* frame);

// 原来的函数声明不再需要，但为了保留注释，此处保留
void addDataToBuffer(uint16_t ad7680_data, uint16_t ads1220_data);
void sendBufferIfFull();
void addDataToBufferSingle();
void sendBufferIfFullSingle();


#endif // COMMUNICATION_H