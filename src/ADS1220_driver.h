#ifndef ADS1220_DRIVER_H
#define ADS1220_DRIVER_H

#include "config.h"
#include <Arduino.h>

namespace ADS1220 {
    void init();
    void reset();
    void configure();
    void startConversion();
    uint32_t readData();
    void powerDown();
}

#endif // ADS1220_DRIVER_H