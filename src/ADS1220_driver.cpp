#include "ADS1220_driver.h"
#include <SPI.h>

namespace ADS1220 {

    // ADS1220 SPI设置 (模式1)
    SPISettings spiSettings2(2000000, MSBFIRST, SPI_MODE1);

    void init() {
        pinMode(PIN_CS_ADS1220, OUTPUT);
        digitalWrite(PIN_CS_ADS1220, HIGH); // 默认不选中
        pinMode(PIN_DRDY_ADS1220, INPUT); // DRDY为输入引脚
    }

    void reset() {
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);        
        SPI.transfer(0x06); // RESET command
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();       
        delayMicroseconds(510); // 等待复位完成
    }

    void configure() {
        // 配置寄存器以满足需求
        uint8_t config_reg0 = 0x65; // MUX=AIN0/AIN1, Gain=4, PGA enabled
        uint8_t config_reg1 = 0xA0; // DR=1000SPS, Normal Mode, Continuous conversion mode
        uint8_t config_reg2 = 0x44; // VREF=External(REFP0/N0), 50/60Hz Rej, IDAC=250uA
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3, I2MUX=Disabled, DRDY only
        
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
        
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // WREG command: start at reg0, write 4 bytes
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);
        
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }
    
    // 启动/同步转换
    void startConversion() {
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.transfer(0x08); // START/SYNC command
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }
    
    uint32_t readData() {
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
                
        // 读取3个字节的24位数据
        uint8_t byte1 = SPI.transfer(0x00);
        uint8_t byte2 = SPI.transfer(0x00);
        uint8_t byte3 = SPI.transfer(0x00);
        
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();

        uint32_t result = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;
        
        return result;
    }

    void powerDown() {
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);          
        SPI.transfer(0x02);
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }

}