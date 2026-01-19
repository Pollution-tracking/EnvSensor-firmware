#ifndef BATTERY_CONSTANTS_H
#define BATTERY_CONSTANTS_H

#include <Arduino.h>

// Battery voltage monitor
const uint8_t BATTERY_VOLTAGE_DIVIDER = 2;
const float BATTERY_MAX_VOLTAGE       = 3.3;
const float BATTERY_CORRECTION_FACTOR = 1.115;

#endif // BATTERY_CONSTANTS_H