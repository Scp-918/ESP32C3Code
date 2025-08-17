#include "communication.h"
#include "config.h"

// 数据帧结构 (10字节)
struct DataFrame {
    uint8_t header1;
    uint8_t header2;
    uint8_t adc_datahigh; 
    uint8_t adc_datalow;  
    uint8_t afe_datahigh; 
    uint8_t afe_datamedium;  
    uint8_t afe_datalow; 
    uint8_t checksum;
    uint8_t footer1;
    uint8_t footer2;
};

// 缓冲区
DataFrame commBuffer[COMM_BUFFER_FRAME_COUNT];
int frameCount = 0;

void initCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
}

void addDataToBuffer(uint16_t adc_data, uint32_t afe_data) {
    if (frameCount < COMM_BUFFER_FRAME_COUNT) {
        DataFrame &frame = commBuffer[frameCount];

        frame.header1 = FRAME_HEADER1;
        frame.header2 = FRAME_HEADER2;
        
        frame.adc_datahigh = (adc_data >> 8) & 0xFF;
        frame.adc_datalow = adc_data & 0xFF;

        frame.afe_datahigh = (afe_data >> 16) & 0xFF;
        frame.afe_datamedium = (afe_data >> 8) & 0xFF;
        frame.afe_datalow = afe_data & 0xFF;

        frame.checksum = frame.adc_datahigh ^ frame.adc_datalow ^ frame.afe_datahigh ^ frame.afe_datamedium ^ frame.afe_datalow;

        frame.footer1 = FRAME_FOOTER1;
        frame.footer2 = FRAME_FOOTER2;

        frameCount++;
    }
}

void sendBufferIfFull() {
    if (frameCount >= COMM_BUFFER_FRAME_COUNT) {
        Serial.write((uint8_t*)commBuffer, sizeof(commBuffer));
        frameCount = 0;
    }
}