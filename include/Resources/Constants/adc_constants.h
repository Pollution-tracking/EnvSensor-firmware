#ifndef ADC_CONSTANTS_H
#define ADC_CONSTANTS_H

#include <Arduino.h>
#include <configs.h>

const uint8_t ADC_RESOLUTION = 12;
const uint16_t ADC_MAX_VALUE = (1 << ADC_RESOLUTION) - 1;

#endif // ADC_CONSTANTS_H