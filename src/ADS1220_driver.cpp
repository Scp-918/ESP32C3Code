#include "ADS1220_driver.h"
#include <SPI.h>
#include "driver/gpio.h"

namespace ADS1220 {

    // ADS1220 SPI设置 (模式1)
    SPISettings spiSettings2(2000000, MSBFIRST, SPI_MODE1);

    void init() {
        gpio_pad_select_gpio((gpio_num_t)PIN_CS_ADS1220);
        gpio_set_direction((gpio_num_t)PIN_CS_ADS1220, GPIO_MODE_OUTPUT);
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220); // 默认不选中
        
        gpio_pad_select_gpio((gpio_num_t)PIN_DRDY_ADS1220);
        gpio_set_direction((gpio_num_t)PIN_DRDY_ADS1220, GPIO_MODE_INPUT); // DRDY为输入引脚
    }

    void reset() {
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        SPI.transfer(0x06); // RESET command [1]
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();
        delayMicroseconds(510); // 等待复位完成
    }

    void configure() {
        // 根据数据手册 [1] 和需求 [1] 进行完整配置
        // 硬件限制：需求中的200uA激励电流不可用，选择最接近的250uA [1]
        
        // 寄存器配置值
        // 1. PGA增益更改为 (4x)
        uint8_t config_reg0 = 0x65; // MUX=AIN0/AIN1 0000, Gain=4(010), PGA disabled 165
        
        // 2. 数据速率更改为1000 SPS以满足160Hz时序要求
        uint8_t config_reg1 = 0xA0;
        
        // 3. IDAC电流初始化为250uA
        uint8_t config_reg2 = 0x44; // VREF=External(REFP0/N0)01, 50/60Hz Rej 00, 0,IDAC=250uA (100)/0 000
        
        uint8_t config_reg3 = 0x80; // I1MUX=AIN3 100, I2MUX=Disabled 000, DRDY only 0,0
        
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        
        // WREG command: 从寄存器0开始，写入4个字节
        SPI.transfer(0x40 | (0x00 << 2) | (4 - 1)); // 0x43
        SPI.transfer(config_reg0);
        SPI.transfer(config_reg1);
        SPI.transfer(config_reg2);
        SPI.transfer(config_reg3);
        
        //delayMicroseconds(5); // 等待配置写入完成
        //SPI.transfer(0x08); // START/SYNC command [1]
        
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();
    }
    
    // 异步启动转换，仅发送命令
    void startConversion() {
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        SPI.transfer(0x08); // START/SYNC command [1]
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();
    }
    
    uint32_t readData() {
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        
        // 读取3个字节的24位数据
        uint8_t byte1 = SPI.transfer(0x00);
        uint8_t byte2 = SPI.transfer(0x00);
        uint8_t byte3 = SPI.transfer(0x00);
        
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();

        // 组合成32位数据
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

    void startsync() {
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        SPI.transfer(0x08);
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();
    }

}