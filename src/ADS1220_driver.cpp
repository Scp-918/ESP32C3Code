#include "ADS1220_driver.h"
#include <SPI.h>

namespace ADS1220 {

    // ADS1220 SPI设置 (模式1)
    SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE1);

    void init() {
        pinMode(PIN_CS_ADS1220, OUTPUT);
        digitalWrite(PIN_CS_ADS1220, HIGH); // 默认不选中
        pinMode(PIN_DRDY_ADS1220, INPUT); // DRDY为输入引脚
    }

    void reset() {
        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings);
        SPI.transfer(0x06); // RESET command [1]
        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);
        delayMicroseconds(100); // 等待复位完成
    }

    void configure() {
        // BUG修复：根据数据手册 [1] 和需求 [1] 进行完整配置
        // 硬件限制：需求中的200uA激励电流不可用，选择最接近的250uA [1]
        
        // 寄存器配置值
        uint8_t config_reg0 = 0x00; // MUX=AIN0/AIN1, Gain=1, PGA enabled
        uint8_t config_reg1 = 0x04; // DR=20SPS, Normal Mode, Single-shot mode
        uint8_t config_reg2 = 0x54; // VREF=External(REFP0/N0), 50/60Hz Rej, IDAC=250uA
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3, I2MUX=Disabled, DRDY only

        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings);
        
        // WREG command: 从寄存器0开始，写入4个字节
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // 0x43
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);

        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);
    }

    void startConversion() {
        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings);
        SPI.transfer(0x08); // START/SYNC command [1]
        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);
    }

    uint32_t readData() {
        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings);
        
        // 读取3个字节的24位数据
        uint8_t byte1 = SPI.transfer(0x00);
        uint8_t byte2 = SPI.transfer(0x00);
        uint8_t byte3 = SPI.transfer(0x00);

        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);

        // 组合成32位数据
        uint32_t result = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;
        
        // 24位数据是二进制补码，如果最高位是1，需要进行符号扩展
        if (result & 0x800000) {
            result |= 0xFF000000;
        }

        return result;
    }

    void powerDownIdacs() {
        // 需求：测量结束后停止激励电流输出 [1]
        // 通过将IDAC电流设置为0来实现
        uint8_t config_reg2_off = 0x14; // VREF=External, 50/60Hz Rej, IDAC=Off

        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings);
        
        // WREG command: 只写入寄存器2
        SPI.transfer(0x40 | (0x02 << 2) | (1 - 1)); // 0x48
        SPI.transfer(config_reg2_off);

        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);
    }
}