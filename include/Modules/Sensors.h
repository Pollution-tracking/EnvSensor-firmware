#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

#include <configs.h>
#include <Logger/logger.h>
#include <Resources/Constants/sensor_constants.h>
#include <Modules/BoardUtilities.h>
#include <Resources/RTC_values.h>

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
#include <Modules/RTC.h>

#ifdef CO2_ENABLE
extern CO2Sensor co2Sensor;
#endif
#ifdef PM_ENABLE
extern PMSensor pmSensor;
#endif
#ifdef BME_ENABLE
extern BMESensor bmeSensor;
#endif
#ifdef MICS_ENABLE
extern MICSSensor micsSensor;
#endif
extern Battery battery;
extern RTC rtc;
extern BoardUtilities boardUtilities;

SENSORS handleSensorsState(SENSORS state);
void readAllSensors();
void initializeSensors();
void wakeUpSensors();
void sleepSensors();

#endif // SENSORS_H
