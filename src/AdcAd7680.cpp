#include "AdcAd7680.h"

AdcAd7680::AdcAd7680(uint8_t cs_pin, SPISettings spi_settings)
    : _cs_pin(cs_pin), _spi_settings(spi_settings) {
}

void AdcAd7680::begin() {
    // 初始化片选引脚为输出模式，并默认设置为高电平（非选中状态）
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
}

uint16_t AdcAd7680::read() {
    uint8_t data_buffer = {0}; // AD7680需要至少20个SCLK周期，通常读取3个字节（24位）来确保完整性
    uint16_t result = 0;

    // 开始SPI事务，确保总线设置正确
    SPI.beginTransaction(_spi_settings);

    // 1. 拉低CS引脚。根据数据手册，CS的下降沿会启动采样和转换。
    digitalWrite(_cs_pin, LOW);

    // 2. 稍微延迟以满足t2 (CS to SCLK setup time) 要求，虽然在MHz级别通常不是问题，但这是良好实践。
    // 在此应用中，50us的延迟远大于此需求，故此处无需额外delay。

    // 3. 执行SPI传输。发送虚拟数据（0x00）以生成时钟信号，并接收ADC的转换结果。
    // AD7680的数据流包含前导零，然后是16位数据。
    // 读取3个字节可以确保捕获完整的16位数据。
    SPI.transfer(data_buffer, 3);

    // 4. 拉高CS引脚，结束本次通信。
    digitalWrite(_cs_pin, HIGH);

    // 结束SPI事务，释放总线
    SPI.endTransaction();

    // 5. 从接收到的缓冲区中解析出16位数据。
    // 根据数据手册（图20/21），数据流是4个前导零+16位数据。
    // 这意味着16位数据实际上分布在接收到的24位流中。
    // 最简单的解析方式是假设数据是MSB-first，并直接组合字节。
    // 由于我们读取了3个字节，16位数据通常在最后两个字节。
    // data_buffer可能包含前导零和部分数据，data_buffer和data_buffer包含剩余数据。
    // 实际数据对齐可能需要根据示波器观察微调，但通常是 (data_buffer << 8) | data_buffer
    // 或者 (data_buffer << 8) | data_buffer。
    // 根据AD7680数据手册图20，数据是4个前导0，然后是DB15...DB0。
    // 这意味着16位数据从第5个时钟位开始。
    // 读取24位（3字节）后，数据为 0000 DB15 DB14... DB0 0000
    // 这使得解析变得复杂。一个更简单且可靠的方法是读取2个字节。
    // CS下降沿后，第一个SCLK下降沿送出第二个前导零，同时DB15出现在SDATA上。
    // 因此，直接读取两个字节即可获得16位数据。
    
    // 重新实现为读取2字节
    uint8_t data_buffer_2byte = {0};
    SPI.beginTransaction(_spi_settings);
    digitalWrite(_cs_pin, LOW);
    SPI.transfer(data_buffer_2byte, 2);
    digitalWrite(_cs_pin, HIGH);
    SPI.endTransaction();
    
    result = (uint16_t)(data_buffer_2byte << 8) | data_buffer_2byte;

    return result;
}