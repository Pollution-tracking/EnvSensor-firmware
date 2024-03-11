#include <RTC_values.hpp>

RTC_DATA_ATTR bool bleEnabled = true;

RTC_DATA_ATTR uint8_t currScreenMode = SCREENMODE::NO_SCREEN;
RTC_DATA_ATTR uint8_t prevScreenMode = SCREENMODE::NO_SCREEN;

RTC_DATA_ATTR char _temperature[12] = "No data\0";
RTC_DATA_ATTR char _humidity[12]    = "No data\0";
RTC_DATA_ATTR char _pressure[12]    = "No data\0";
RTC_DATA_ATTR char _altitude[12]    = "No data\0";
RTC_DATA_ATTR char _co2[12]         = "No data\0";
RTC_DATA_ATTR char _pm1[12]         = "No data\0";
RTC_DATA_ATTR char _pm2_5[12]       = "No data\0";
RTC_DATA_ATTR char _pm10[12]        = "No data\0";