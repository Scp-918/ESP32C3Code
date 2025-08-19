#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "config.h"

void initCommunication();
void addDataToBuffer(uint16_t ad7680_data, uint16_t ads1220_data);
void sendBufferIfFull();
void addDataToBufferSingle();
void sendBufferIfFullSingle();

#endif // COMMUNICATION_H