#ifndef AD7680_DRIVER_H
#define AD7680_DRIVER_H

#include "config.h"

namespace AD7680 {
    void init();
    void triggerConversion();
    uint16_t readData();
}

#endif // AD7680_DRIVER_H