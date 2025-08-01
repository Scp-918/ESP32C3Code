#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "config.h"

void initCommunication();
void addDataToBuffer(uint16_t adc_data, uint32_t afe_data);
void sendBufferIfFull();
void addDataToBufferSingle(uint16_t adc_data);
void sendBufferIfFullSingle();

#endif // COMMUNICATION_H