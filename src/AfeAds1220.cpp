#include "AfeAds1220.h"

AfeAds1220::AfeAds1220(uint8_t cs_pin, uint8_t drdy_pin)
    : ads(cs_pin, drdy_pin), _cs_pin(cs_pin), _drdy_pin(drdy_pin) {
}

bool AfeAds1220::begin() {
    if (!ads.init()) {
        Serial.println("ADS1220 连接失败或初始化错误!");
        return false;
    }

    // --- 根据项目需求进行详细配置 ---
    
    // 1. 配置输入多路复用器：测量AIN0和AIN1之间的差分电压。
    // 待测电阻与8kΩ电阻组成的比例式电桥连接到AIN0和AIN1。
    ads.setCompareChannels(ADS1220_MUX_0_1);

    // 2. 设置增益。由于是比例式测量，且信号幅度未知，先设置为1倍增益。
    // 后续可根据实际信号大小进行调整以优化动态范围。
    ads.setGain(ADS1220_GAIN_1);

    // 3. 设置数据率。选择20SPS，可以利用其50/60Hz工频抑制功能，提高抗干扰能力。
    ads.setDataRate(ADS1220_DR_20SPS);
    ads.setFIRFilter(ADS1220_50HZ_60HZ); // 明确启用50/60Hz抑制

    // 4. 设置参考电压源为外部参考。
    // 在比例式测量中，参考电压由外部基准电阻产生，连接到REFP0和REFN0。
    ads.setVRefSource(ADS1220_VREF_REFP0_REFN0);
    // 注意：需要外部提供一个精确的基准电阻，此处假设为8kΩ，与待测电阻进行比例测量。
    // 库需要知道参考电压值才能计算实际电压，但我们只关心原始数据，所以可以不设置。

    // 5. 设置IDAC电流。需求为200uA，ADS1220无此档位，选择最接近的250uA。
    // 这是一个重要的设计权衡，需要在上位机软件中进行校正。
    ads.setIdacCurrent(ADS1220_IDAC_250_MU_A);

    // 6. 路由IDAC1到AIN3，用于激励待测电阻。
    ads.setIdac1Routing(ADS1220_IDAC_AIN3);
    ads.setIdac2Routing(ADS1220_IDAC_NONE); // 禁用IDAC2

    // 7. 设置为单次转换模式，便于精确控制转换时机。
    ads.setConversionMode(ADS1220_SINGLE_SHOT);

    // 8. 默认禁用IDAC，在需要时再通过 enableIdac(true) 开启
    enableIdac(false);

    Serial.println("ADS1220 初始化配置完成。");
    return true;
}

void AfeAds1220::enableIdac(bool enable) {
    if (enable) {
        // 路由IDAC1到AIN3以输出电流
        ads.setIdac1Routing(ADS1220_IDAC_AIN3);
    } else {
        // 将IDAC1路由到“无连接”以关闭电流输出
        ads.setIdac1Routing(ADS1220_IDAC_NONE);
    }
}

int32_t AfeAds1220::read() {
    // ADS1220_WE库的getRawData()函数内部会自动处理启动转换和等待DRDY的过程。
    // 它会发送START/SYNC命令，然后轮询DRDY引脚，直到数据就绪。
    return ads.getRawData();
}