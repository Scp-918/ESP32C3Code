#include "communication.h"

// 数据帧结构 (8字节)
struct DataFrame {
    uint8_t header;
    uint8_t adc_data;
    uint8_t afe_data;
    uint8_t checksum;
    uint8_t footer;
};

// 16帧的发送缓冲区
DataFrame commBuffer;
int frameCount = 0;

void initCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
}

void addDataToBuffer(uint16_t adc_data, uint32_t afe_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        // 获取当前帧的引用
        DataFrame &frame = commBuffer[frameCount];

        frame.header = FRAME_HEADER;
        
        // 填充16位ADC数据 (大端模式, MSB first)
        frame.adc_data = (adc_data >> 8) & 0xFF;
        frame.adc_data[1] = adc_data & 0xFF;

        // 填充24位AFE数据 (大端模式, MSB first)
        frame.afe_data = (afe_data >> 16) & 0xFF;
        frame.afe_data[1] = (afe_data >> 8) & 0xFF;
        frame.afe_data = afe_data & 0xFF;

        // 计算校验和：前5个数据字节的位与
        frame.checksum = frame.adc_data & frame.adc_data[1] &
                         frame.afe_data & frame.afe_data[1] & frame.afe_data;

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