#ifndef SCREENBUFFERUTILS_H
#define SCREENBUFFERUTILS_H

#include <Arduino.h>
#include <configs.h>
#include <Resources/RTC_values.h>
#include <Resources/Constants/sensor_constants.h>
#include <Sensors/Battery.h>
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

String convertBattery(BatteryData data);
#ifdef BME_ENABLE
String convertTemperature(BMEData data);
String convertHumidity(BMEData data);
String convertPressure(BMEData data);
String convertAltitude(BMEData data);
#endif
#ifdef CO2_ENABLE
String convertCO2(CO2Data data);
#endif
#ifdef PM_ENABLE
String convertPM1(PMData data);
String convertPM2_5(PMData data);
String convertPM10(PMData data);
#endif
#ifdef MICS_ENABLE
String convertCO(MICSData data);
String convertNO2(MICSData data);
String convertNH3(MICSData data);
#endif

#endif // SCREENBUFFERUTILS_H