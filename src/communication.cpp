#include "communication.h"

// 数据帧结构
struct DataFrame {
    uint8_t header;
    uint8_t adc_data[2];  // 16位ADC数据需要2个字节
    uint8_t afe_data[3];  // 24位AFE数据需要3个字节
    uint8_t checksum;
    uint8_t footer;
};

// 16帧的发送缓冲区
DataFrame commBuffer[COMM_BUFFER_FRAME_COUNT];
int frameCount = 0;

void initCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
}

void addDataToBuffer(uint16_t adc_data, uint32_t afe_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        DataFrame &frame = commBuffer[frameCount];

        frame.header = FRAME_HEADER;
        
        // 填充16位ADC数据 (大端模式)
        frame.adc_data[0] = (adc_data >> 8) & 0xFF;
        frame.adc_data[1] = adc_data & 0xFF;

        // 填充24位AFE数据 (大端模式)
        frame.afe_data[0] = (afe_data >> 16) & 0xFF;
        frame.afe_data[1] = (afe_data >> 8) & 0xFF;
        frame.afe_data[2] = afe_data & 0xFF;

        // 计算校验和：前5个数据字节的位与
        // 这是一个非常脆弱的校验和，但按需求实现 [1]
        frame.checksum = frame.adc_data[0] & frame.adc_data[1] &
                         frame.afe_data[0] & frame.afe_data[1] & frame.afe_data[2];

        frame.footer = FRAME_FOOTER;

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