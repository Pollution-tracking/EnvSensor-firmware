#ifndef RTC_H
#define RTC_H

#include <Arduino.h>

#include "RV-3028-C7.h"
#include "Wire.h"

#include <Resources/Pins.h>
#include <Resources/Constants.h>
#include <Resources/RTC_values.hpp>

#include <Logger/Logger.h>

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
        bool setFromTimestamp(const std::string timestamp);
        bool isRTCUpdated();
        String getTimestamp();
    private:
        TwoWire *wire;
        RV3028 rtc;
        bool _initialised = false;
        String errorMsg;
};

#endif // RTC_H