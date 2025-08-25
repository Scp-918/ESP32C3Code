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

    void configureoff() {
        // 根据数据手册 [1] 和需求 [1] 进行完整配置
        // 硬件限制：需求中的200uA激励电流不可用，选择最接近的250uA [1]
        
        // 寄存器配置值
        // 1. PGA增益更改为 (4x)
        uint8_t config_reg0 = 0x65; // MUX=AIN0/AIN1 0000, Gain=4(010), PGA disabled 165
        
        // 2. 数据速率更改为1000 SPS以满足160Hz时序要求
        uint8_t config_reg1 = 0xA0;
        
        // 3. IDAC电流初始化为250uA
        uint8_t config_reg2 = 0x40; // VREF=External(REFP0/N0)01, 50/60Hz Rej 00, 0,IDAC=250uA (100)/0 000
        
        uint8_t config_reg3 = 0x00; // I1MUX=AIN3 100, I2MUX=Disabled 000, DRDY only 0,0
        
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

    void configureIDAC() {
        // 根据 ADS1220 数据手册 [cite: 3] 和代码需求，配置IDAC以持续输出250µA电流。
        // 这要求将器件从省电模式唤醒并设置寄存器。

        // 1. 启动 SPI 通信
        SPI.beginTransaction(spiSettings2);

        // 2. 将 CS 引脚拉低以选择 ADS1220
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);

        // 3. 发送 WREG 命令，从寄存器1开始写入3个字节
        // 因为配置寄存器1、2、3需要设置，而寄存器0的默认值可以保留，
        // 所以从寄存器1开始写入3个字节（寄存器1、2和3）
        // WREG 命令格式: 0100 rrnn (rr = 起始地址, nn = 字节数-1)
        // 起始地址 rr = 01h (寄存器1)
        // 字节数 nn = 3-1 = 2 (10b)
        // 命令字节: 0100 0110b = 0x46
        SPI.transfer(0x46);

        // 4. 写入配置寄存器1的值: 0xA0
        // DR = 100b (330 SPS), MODE = 00b (Normal Mode), CM = 1b (连续转换), TS = 0b (禁用), BCS = 0b (禁用)
        // DR = 100 (330 SPS) 是为了在连续转换模式下提供一个初始数据速率 [cite: 2032]
        // 连续转换模式 (CM=1) 可确保器件在一次转换完成后立即开始下一次转换 [cite: 1766]
        // 注意: 即使原代码中 `config_reg1` 为 `0xA0`，它对应的是 DR=110 (1000 SPS)，但这会使模式变为保留 [cite: 2032]。
        // 我们在此选择一个有效的模式，例如正常模式下的1000 SPS，其 DR[2:0] 为 110 [cite: 2032]。
        // 所以 config_reg1 = (110 << 5) | (00 << 3) | (1 << 2) | (0 << 1) | (0 << 0) = 0xE4
        // 为了使代码逻辑清晰，我们直接写入正确的值。
        uint8_t config_reg1 = 0xE4;
        SPI.transfer(config_reg1);

        // 5. 写入配置寄存器2的值: 0x44
        // VREF = 10b (AIN0/REFP1 and AIN3/REFN1), PSW = 0b (开关断开), 50/60 = 00b (无抑制), IDAC = 100b (250uA) 
        // 在这里，原始代码的配置值 0x44 是一个合理的选择，因为它设置了 IDAC=250µA。
        uint8_t config_reg2 = 0x44;
        SPI.transfer(config_reg2);

        // 6. 写入配置寄存器3的值: 0x80
        // I1MUX = 100b (IDAC1 to AIN3/REFN1), I2MUX = 000b (IDAC2禁用), DRDYM = 0b (仅专用DRDY), Bit0 = 0b 
        // 这样配置可将IDAC1路由至AIN3引脚，以便为三线制或四线制RTD测量提供激励电流 [cite: 2559]。
        uint8_t config_reg3 = 0x80;
        SPI.transfer(config_reg3);

        // 7. 将 CS 拉高以结束 SPI 事务并应用配置
        GPIO.out_w1ts.val = (1U << PIN_CS_ADS1220);
        SPI.endTransaction();

        // 8. 延迟等待配置更新完成。
        // 在WREG命令的最后一个SCLK下降沿后，寄存器将被更新 [cite: 1864]。
        // 为了确保在发送下一个命令之前设备已就绪，添加短暂延迟是很好的做法。
        delayMicroseconds(2);

        // 9. 唤醒设备并开始连续转换
        // 发送 START/SYNC 命令（0x08）启动连续转换模式 [cite: 1856]。
        // 在这个模式下，器件在每次转换完成后立即自动开始下一次转换 [cite: 1766]。
        SPI.beginTransaction(spiSettings2);
        GPIO.out_w1tc.val = (1U << PIN_CS_ADS1220);
        SPI.transfer(0x08);
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