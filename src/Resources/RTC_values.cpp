#include <Resources/RTC_values.hpp>

// Update board name with chipid
RTC_DATA_ATTR String bleServerName = "EnvSensor_" + String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);

RTC_DATA_ATTR bool bleEnabled = true;
RTC_DATA_ATTR bool bleConnected = false;

RTC_DATA_ATTR bool RTCUpdated = false;
RTC_DATA_ATTR bool hasHistoricalData = false;

RTC_DATA_ATTR float batteryVoltage = 0.0;

RTC_DATA_ATTR uint8_t currScreenMode = SCREENMODE::NO_SCREEN;
RTC_DATA_ATTR uint8_t prevScreenMode = SCREENMODE::NO_SCREEN;

RTC_DATA_ATTR char _battery[12]     = "Null\0";
RTC_DATA_ATTR char _temperature[12] = "Null\0";
RTC_DATA_ATTR char _humidity[12]    = "Null\0";
RTC_DATA_ATTR char _pressure[12]    = "Null\0";
RTC_DATA_ATTR char _altitude[12]    = "Null\0";
RTC_DATA_ATTR char _co2[12]         = "Null\0";
RTC_DATA_ATTR char _pm1[12]         = "Null\0";
RTC_DATA_ATTR char _pm2_5[12]       = "Null\0";
RTC_DATA_ATTR char _pm10[12]        = "Null\0";