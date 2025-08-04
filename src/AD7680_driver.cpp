#include "AD7680_driver.h"
#include <SPI.h>
#include <cmath>
namespace AD7680 {

    // AD7680 SPI设置
    SPISettings spiSettings(1500000, MSBFIRST, SPI_MODE0);

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

    uint16_t readData3() {
        // 第一次读取3个字节(24个时钟)
        digitalWrite(PIN_CS_AD7680, LOW);
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

        //第二次读取
        digitalWrite(PIN_CS_AD7680, LOW);
        SPI.beginTransaction(spiSettings);    
        // 读取3个字节
        byte1 = SPI.transfer(0x00);
        byte2 = SPI.transfer(0x00);
        byte3 = SPI.transfer(0x00);
        SPI.endTransaction();
        digitalWrite(PIN_CS_AD7680, HIGH); // 结束通信
        // 组合成一个32位整数以便于位移操作
        raw_data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;            
        // 数据格式为 4个前导零 + 16位数据 + 4个末尾零 (在24个时钟周期下)
        // 我们需要将整个24位数据右移4位来对齐
        uint16_t result2 = (raw_data >> 4) & 0xFFFF;

        //第三次读取
        digitalWrite(PIN_CS_AD7680, LOW);
        SPI.beginTransaction(spiSettings);    
        // 读取3个字节
        byte1 = SPI.transfer(0x00);
        byte2 = SPI.transfer(0x00);
        byte3 = SPI.transfer(0x00);
        SPI.endTransaction();
        digitalWrite(PIN_CS_AD7680, HIGH); // 结束通信
        // 组合成一个32位整数以便于位移操作
        raw_data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;            
        // 数据格式为 4个前导零 + 16位数据 + 4个末尾零 (在24个时钟周期下)
        // 我们需要将整个24位数据右移4位来对齐
        uint16_t result3 = (raw_data >> 4) & 0xFFFF;

        
        //Serial.println("adc");
        return result;
        
    }

    uint16_t readDataMean() {
        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        uint8_t byte3 = 0;
        uint8_t j = 6;
        uint16_t resultall[j] = {0};
        uint32_t raw_data = 0;

        for (int i=0;i<j;i++)
        {
            digitalWrite(PIN_CS_AD7680, LOW);
            SPI.beginTransaction(spiSettings);    
            // 读取3个字节
            byte1 = SPI.transfer(0x00);
            byte2 = SPI.transfer(0x00);
            byte3 = SPI.transfer(0x00);
            SPI.endTransaction();
            digitalWrite(PIN_CS_AD7680, HIGH); // 结束通信
            // 组合成一个32位整数以便于位移操作
            raw_data = ((uint32_t)byte1 << 16) | ((uint32_t)byte2 << 8) | byte3;            
            // 数据格式为 4个前导零 + 16位数据 + 4个末尾零 (在24个时钟周期下)
            // 我们需要将整个24位数据右移4位来对齐
            resultall[i] = (raw_data >> 4) & 0xFFFF;
        }

        uint16_t result = 0;

        // 当样本数过少时(小于等于2)，统计方法无意义，直接求平均
        if (j <= 2) {
            uint32_t temp_sum = 0;
            if (j > 0) {
                for (int i = 0; i < j; i++) {
                    temp_sum += resultall[i];
                }
                result = temp_sum / j;
            } else {
                result = 0;
            }
        } else {
            // --- 步骤 1: 计算所有原始数据的平均值和标准差 ---

            // 计算总和
            uint32_t sum_all = 0;
            for (int i = 0; i < j; i++) {
                sum_all += resultall[i];
            }

            // 计算平均值 (μ)
            float mean = static_cast<float>(sum_all) / j;

            // 计算方差的累加和 (Sum of Squared Differences)
            float sum_sq_diff = 0;
            for (int i = 0; i < j; i++) {
                sum_sq_diff += pow(static_cast<float>(resultall[i]) - mean, 2);
            }

            // 计算标准差 (σ, Population Standard Deviation)
            float std_dev = sqrt(sum_sq_diff / j);

            // --- 步骤 2: 剔除离群值并计算最终平均值 ---

            // 定义有效数据范围
            float lower_bound = mean - 3 * std_dev;
            float upper_bound = mean + 3 * std_dev;

            uint32_t final_sum = 0;
            uint8_t final_count = 0;

            // 第二次遍历，只累加在范围内的"正常值"
            for (int i = 0; i < j; i++) {
                if (resultall[i] >= lower_bound && resultall[i] <= upper_bound) {
                    final_sum += resultall[i];
                    final_count++;
                }
            }

            // 计算最终平均值
            if (final_count > 0) {
                result = static_cast<uint16_t>(round(static_cast<float>(final_sum) / final_count));
            } else {
                // 备用逻辑: 如果所有值都被视为离群值，则返回原始平均值
                result = static_cast<uint16_t>(round(mean));
            }
        }

        //Serial.println("adc");
        return result;
        
    }

}

    