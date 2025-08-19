#include "communication.h"

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

// 16帧的发送缓冲区
DataFrame commBuffer[COMM_BUFFER_FRAME_COUNT];
int frameCount = 0;

void initCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
}

void addDataToBuffer(uint16_t ad7680_data, uint16_t ads1220_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        // 获取当前帧的引用
        DataFrame &frame = commBuffer[frameCount];

        frame.header1 = FRAME_HEADER1;
        frame.header2 = FRAME_HEADER2;
        
        // 填充16位ADC数据 (大端模式, MSB first)
        frame.adc_datahigh = (ad7680_data >> 8) & 0xFF;
        frame.adc_datalow = ad7680_data & 0xFF;

        // 填充24位AFE数据 (大端模式, MSB first)  
        frame.afe_datahigh = 0x00;
        frame.afe_datamedium = (ads1220_data >> 8) & 0xFF;
        frame.afe_datalow = ads1220_data & 0xFF;

        // 计算校验和：前面数据字节的异或
        frame.checksum = frame.adc_datahigh ^ frame.adc_datalow ^ frame.afe_datahigh ^ frame.afe_datamedium ^ frame.afe_datalow;

        frame.footer1 = FRAME_FOOTER1;
        frame.footer2 = FRAME_FOOTER2;

        frameCount++;
    }
}

void sendBufferIfFull() {
    if (frameCount >= COMM_BUFFER_FRAME_COUNT) {
        // 将整个缓冲区作为字节数组发送
        Serial.write((uint8_t*)commBuffer, sizeof(commBuffer));
        // 重置计数器
        frameCount = 0;
    }
}

void addDataToBufferSingle() {
    // 此函数未使用，保留
}

void sendBufferIfFullSingle() {
    // 此函数未使用，保留
}