#include "SerialComms.h"

SerialComms::SerialComms(Stream* stream) : _stream(stream) {
}

void SerialComms::begin(unsigned long baud_rate) {
    // 实际的串口初始化（如Serial.begin()）应在main.cpp的setup()中完成。
    // 这个函数可以留空，或用于其他通信相关的初始化。
}

void SerialComms::sendDataFrame(uint16_t ad7680_data, int32_t ads1220_data) {
    uint8_t frame;

    // 1. 填充帧头
    frame = 0xAA;

    // 2. 填充16位AD7680数据（大端字节序）
    frame = (ad7680_data >> 8) & 0xFF; // MSB
    frame = ad7680_data & 0xFF;        // LSB

    // 3. 填充24位ADS1220数据（大端字节序）
    // ADS1220返回的是一个32位有符号数，我们只需要其低24位。
    frame = (ads1220_data >> 16) & 0xFF; // MSB
    frame = (ads1220_data >> 8) & 0xFF;
    frame = ads1220_data & 0xFF;         // LSB

    // 4. 计算校验和：字节1到字节5的按位与
    uint8_t checksum = frame & frame & frame & frame & frame;
    frame = checksum;

    // 5. 填充帧尾
    frame = 0xBB;

    // 6. 通过串口发送完整的数据帧
    _stream->write(frame, sizeof(frame));
}