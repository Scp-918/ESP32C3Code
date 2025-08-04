#include "AD7680_driver.h"
#include <SPI.h>

namespace AD7680 {

    // AD7680 SPI设置
    SPISettings spiSettings(500000, MSBFIRST, SPI_MODE0);

    void init() {
        pinMode(PIN_CS_AD7680, OUTPUT);
        digitalWrite(PIN_CS_AD7680, HIGH); // 默认不选中
    }

    void triggerConversion() {
        // 触发转换仅需将CS拉低
        // 实际的SPI通信在readData()中进行
        digitalWrite(PIN_CS_AD7680, LOW);
        //Serial.println("CS down");
    }

    uint16_t readData() {
        // BUG修复：此函数在主循环中调用，而非ISR
        // BUG修复：正确解析带有4个前导零的数据
        // 数据手册 [1] 指出，转换和读取需要至少20个SCLK周期
        // 读取3个字节(24个时钟)是最简单的方式
        
        SPI.beginTransaction(spiSettings);
        
        // 读取3个字节
        uint8_t byte1 = SPI.transfer(0x00);
        uint8_t byte2 = SPI.transfer(0x00);
        uint8_t byte3 = SPI.transfer(0x00);
        
        SPI.endTransaction();
        digitalWrite(PIN_CS_AD7680, HIGH); // 结束通信

        // 组合成一个32位整数以便于位移操作
        uint32_t raw_data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;

    
        
        // 数据格式为 4个前导零 + 16位数据 + 4个末尾零 (在24个时钟周期下)
        // 我们需要将整个24位数据右移4位来对齐
        uint16_t result = (raw_data >> 4) & 0xFFFF;
        //Serial.println("adc");
        return result;
        
    }
}

    