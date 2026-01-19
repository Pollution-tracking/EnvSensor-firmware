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

String convertBattery(BatteryData data, bool includeUnit = true);
#ifdef BME_ENABLE
String convertTemperature(BMEData data, bool includeUnit = true, bool integerOnly = false);
String convertHumidity(BMEData data, bool includeUnit = true, bool integerOnly = false);
String convertPressure(BMEData data, bool includeUnit = true, bool integerOnly = false);
String convertAltitude(BMEData data, bool includeUnit = true, bool integerOnly = false);
#endif
#ifdef CO2_ENABLE
String convertCO2(CO2Data data, bool includeUnit = true, bool integerOnly = false);
#endif
#ifdef PM_ENABLE
String convertPM1(PMData data, bool includeUnit = true, bool integerOnly = false);
String convertPM2_5(PMData data, bool includeUnit = true, bool integerOnly = false);
String convertPM10(PMData data, bool includeUnit = true, bool integerOnly = false);
#endif
#ifdef MICS_ENABLE
String convertCO(MICSData data, bool includeUnit = true, bool integerOnly = false);
String convertNO2(MICSData data, bool includeUnit = true, bool integerOnly = false);
String convertNH3(MICSData data, bool includeUnit = true, bool integerOnly = false);
#endif

#endif // SCREENBUFFERUTILS_H