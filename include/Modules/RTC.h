#ifndef RTC_H
#define RTC_H

#include <Arduino.h>

#include "RV-3028-C7.h"

#include <Resources/pins.h>
#include <Resources/Constants/rtc_constants.h>
#include <Logger/logger.h>
#include <Resources/RTC_values.h>
#include <string>

class RTC {
    public:
        RTC();
        ~RTC();
        void init();
        bool isInitialised();
        bool setHour(uint8_t hour);
        bool setMinute(uint8_t minute);
        bool setSecond(uint8_t second);
        bool setDay(uint8_t day);
        bool setMonth(uint8_t month);
        bool setYear(uint16_t year);
        bool setFromTimestamp(const String timestamp);
        String getTimestamp();
    private:
        RV3028 rtc;
        bool _initialised = false;
};

#endif // RTC_H