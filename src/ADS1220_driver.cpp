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
        SPI.transfer(0x06); // RESET command [1]
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();       
        delayMicroseconds(510); // 等待复位完成
    }

    void configure() {
        // BUG修复：根据数据手册 [1] 和需求 [1] 进行完整配置
        // 硬件限制：需求中的200uA激励电流不可用，选择最接近的250uA [1]
        
        // 寄存器配置值
        // 1. PGA增益更改为 (4x)
        uint8_t config_reg0 = 0x65; // MUX=AIN0/AIN1 0000, Gain=4(010), PGA disabled 1
        
        // 2. 数据速率更改为600 SPS以满足160Hz时序要求
        //uint8_t config_reg1 = 0xA0; // DR=1000SPS (110), Normal Mode00, Continuous conversion mode 0,00A1
        uint8_t config_reg1 = 0x90; // DR=1000SPS (110), Normal Mode00, Continuous conversion mode 0,00A1

        // 3. IDAC电流初始化为0A
        uint8_t config_reg2 = 0x44; // VREF=External(REFP0/N0)01, 50/60Hz Rej 00, 0,IDAC=250uA (100)/0 000
        
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3 100, I2MUX=Disabled 000, DRDY only 0,0
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
        
        
        // WREG command: 从寄存器0开始，写入4个字节
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // 0x43
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);
        
        
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }

/*     void startConversion() {
        uint8_t config_reg2_on = 0x44;

        digitalWrite(PIN_CS_ADS1220, LOW);
        SPI.beginTransaction(spiSettings2);
        
        // WREG command: 只写入寄存器2
        SPI.transfer(0x40 | (0x02 << 2) | (1 - 1)); // 0x48
        SPI.transfer(config_reg2_on);
        //延迟500us
        delayMicroseconds(1); // 等待复位完成
        SPI.transfer(0x08); // START/SYNC command [1]
        SPI.endTransaction();
        digitalWrite(PIN_CS_ADS1220, HIGH);
    } */
    void startConversion() {
        // 需求：测量结束后停止激励电流输出 [1]
        // 寄存器配置值
        // 1. PGA增益更改为 (4x)
        uint8_t config_reg0 = 0x65; // MUX=AIN0/AIN1 0000, Gain=4(010), PGA disabled 1
        
        // 2. 数据速率更改为1000 SPS以满足160Hz时序要求
        uint8_t config_reg1 = 0x90; // DR=1000SPS (110), Normal Mode00, Continuous conversion mode 0,00A1
        
        // 3. IDAC电流初始化为0A
        uint8_t config_reg2 = 0x44; // VREF=External(REFP0/N0)01, 50/60Hz Rej 00, 0,IDAC=250uA (100)/0 000
        
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3 100, I2MUX=Disabled 000, DRDY only 0,0
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
        
        
        // WREG command: 从寄存器0开始，写入4个字节
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // 0x43
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);
        delayMicroseconds(5); // 等待复位完成
        SPI.transfer(0x08); // START/SYNC command [1]
        
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

        // 组合成32位数据
        uint32_t result = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;
        
        // 24位数据是二进制补码，如果最高位是1，需要进行符号扩展
        if (result & 0x800000) {
            result |= 0xFF000000;
        }  

        return result;
    }

    void powerDownIdacs() {
        // 寄存器配置值
        // 1. PGA增益更改为(4x)
        uint8_t config_reg0 = 0x61; // MUX=AIN0/AIN1 0000, Gain=4(010), PGA disabled 1
        
        // 2. 数据速率更改为600 SPS以满足160Hz时序要求
        uint8_t config_reg1 = 0x90; // DR=1000SPS (110), Normal Mode00, Continuous conversion mode 0,00A1
        
        // 3. IDAC电流初始化为0A
        uint8_t config_reg2 = 0x40; // VREF=External(REFP0/N0)01, 50/60Hz Rej 00, 0,IDAC=250uA (100)/0 000
        
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3 100, I2MUX=Disabled 000, DRDY only 0,0
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);
        
        
        // WREG command: 从寄存器0开始，写入4个字节
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // 0x43
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);

        
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }

    void powerDown() {
        // 寄存器配置值
        SPI.beginTransaction(spiSettings2);
        digitalWrite(PIN_CS_ADS1220, LOW);          
        SPI.transfer(0x02);
        digitalWrite(PIN_CS_ADS1220, HIGH);
        SPI.endTransaction();
    }

}