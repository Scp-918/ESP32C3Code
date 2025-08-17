#ifndef AD7680_DRIVER_H
#define AD7680_DRIVER_H

#include "config.h"
#include <Arduino.h>

namespace AD7680 {
    void init();
    // 触发转换，仅拉低CS，不进行数据传输
    void triggerConversion();
    // 读取数据，不进行CS拉低操作，直接进行SPI传输
    uint16_t readData();
}

#endif // AD7680_DRIVER_H