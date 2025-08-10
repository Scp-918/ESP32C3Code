#include "communication.h"

// 数据帧结构 (8字节)
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

// 校验数据帧结构 (4字节)
struct DataFrameSingle {
    uint8_t header;
/*     uint8_t adc_datahigh; // 16位ADC数据的高8位
    uint8_t adc_datalow;  // 16位ADC数据的低8位 */
    uint8_t afe_datahigh; // 24位AFE数据的高8位
    uint8_t afe_datamedium;  // 24位AFE数据的中间8位
    uint8_t afe_datalow; // 24位AFE数据的低8位 
    uint8_t footer;
};

// 16帧的发送缓冲区
DataFrame commBuffer[COMM_BUFFER_FRAME_COUNT];
DataFrameSingle commBufferSingle[COMM_BUFFER_FRAME_COUNT];
int frameCount = 0;
int frameCountSingle = 0;

void initCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
}

void addDataToBuffer(uint16_t adc_data, uint16_t afe_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        // 获取当前帧的引用
        DataFrame &frame = commBuffer[frameCount];

        frame.header1 = FRAME_HEADER1;
        frame.header2 = FRAME_HEADER2;
        
        // 填充16位ADC数据 (大端模式, MSB first)
        frame.adc_datahigh = (adc_data >> 8) & 0xFF;
        frame.adc_datalow = adc_data & 0xFF;
        // 注意：adc_data是uint8_t，不能用数组索引，需要修改结构体定义

        // 填充24位AFE数据 (大端模式, MSB first)  
        frame.afe_datahigh = 0x00;
        frame.afe_datamedium = (afe_data >> 8) & 0xFF;
        frame.afe_datalow = afe_data & 0xFF;
        // 注意：afe_data是uint8_t，不能用数组索引，需要修改结构体定义

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

void addDataToBufferSingle(uint32_t adc_data) {
    if (frameCountSingle < COMM_BUFFER_FRAME_COUNT) {
        // 获取当前帧的引用
        DataFrameSingle &frame = commBufferSingle[frameCountSingle];

        frame.header = FRAME_HEADER1;
        
        
/*         // 填充16位ADC数据 (大端模式, MSB first)
        frame.adc_datahigh = (adc_data >> 8) & 0xFF;
        frame.adc_datalow = adc_data & 0xFF; */
        
        // 填充24位AFE数据 (大端模式, MSB first)
        frame.afe_datahigh = (adc_data >> 16) & 0xFF;
        frame.afe_datamedium = (adc_data >> 8) & 0xFF;
        frame.afe_datalow = adc_data & 0xFF; 

        // 计算校验和：前面数据字节的位与

        frame.footer = FRAME_FOOTER1;

        frameCountSingle++;
    }
}

void sendBufferIfFullSingle() {
    if (frameCountSingle >= COMM_BUFFER_FRAME_COUNT) {
        // 将整个缓冲区作为字节数组发送
        Serial.write((uint8_t*)commBufferSingle, sizeof(commBufferSingle));
        // 重置计数器
        frameCountSingle = 0;
    }
}