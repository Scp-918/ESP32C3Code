#include "AD7680_driver.h"
#include <SPI.h>

namespace AD7680 {

    // AD7680 SPI设置
    // 2.5MHz时钟，SPI模式0
    SPISettings spiSettings(2500000, MSBFIRST, SPI_MODE0);

    void init() {
        pinMode(PIN_CS_AD7680, OUTPUT);
        digitalWrite(PIN_CS_AD7680, HIGH); // 默认不选中
    }

    // 触发转换，不进行任何SPI传输，仅将CS拉低
    void triggerConversion() {
        digitalWrite(PIN_CS_AD7680, LOW);
    }
    
    // 读取数据，完成SPI传输并释放CS
    uint16_t readData() {
        SPI.beginTransaction(spiSettings);
        
        // 读20个时钟周期的数据，共3个字节
        uint8_t byte1 = SPI.transfer(0x00);
        uint8_t byte2 = SPI.transfer(0x00);
        uint8_t byte3 = SPI.transfer(0x00);
        
        SPI.endTransaction();
        digitalWrite(PIN_CS_AD7680, HIGH); // 结束通信

        // 数据格式为 4个前导零 + 16位数据 + 4个末尾零 (在24个时钟周期下)
        // 我们需要将整个24位数据右移4位来对齐
        uint32_t raw_data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;
        uint16_t result = (raw_data >> 4) & 0xFFFF;
        
        return result;
    }
}

    