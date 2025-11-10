#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <Arduino.h>
#include <configs.h>
#include <vector>
#include <Logger/logger.h>
#include <Modules/RTC.h>
#include <Modules/Bluetooth.h>
#include <Modules/SDcard.h>

#ifdef BME_ENABLE
#include <Sensors/BME_sensor.h>
#endif
#ifdef PM_ENABLE
#include <Sensors/PM_sensor.h>
#endif
#ifdef CO2_ENABLE
#include <Sensors/CO2_sensor.h>
#endif
#ifdef MICS_ENABLE
#include <Sensors/MICS_sensor.h>
#endif
#include <Sensors/Battery.h>

extern Bluetooth bluetooth;
extern SDcard sdcard;

void handleLiveData();
void handleHistoricalData();

#endif // DATA_HANDLER_H