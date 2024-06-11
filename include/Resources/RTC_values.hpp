#ifndef RTC_VALUES_HPP
#define RTC_VALUES_HPP

#include <Arduino.h>
#include "Resources/constants.h"

// BLE server name
extern RTC_DATA_ATTR String bleServerName;

extern RTC_DATA_ATTR bool bleEnabled;
extern RTC_DATA_ATTR bool bleConnected;

extern RTC_DATA_ATTR bool RTCUpdated;

extern RTC_DATA_ATTR float batteryVoltage;

extern RTC_DATA_ATTR uint8_t currScreenMode;
extern RTC_DATA_ATTR uint8_t prevScreenMode;

extern RTC_DATA_ATTR char _battery[12];
extern RTC_DATA_ATTR char _temperature[12];
extern RTC_DATA_ATTR char _humidity[12];
extern RTC_DATA_ATTR char _pressure[12];
extern RTC_DATA_ATTR char _altitude[12];
extern RTC_DATA_ATTR char _co2[12];
extern RTC_DATA_ATTR char _pm1[12];
extern RTC_DATA_ATTR char _pm2_5[12];
extern RTC_DATA_ATTR char _pm10[12];

#endif // RTC_VALUES_HPP