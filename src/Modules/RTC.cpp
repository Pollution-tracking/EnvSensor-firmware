#include "Modules/RTC.h"

#define logg(message) loggWithObj(message, "RTC")
#define loggValue(message, value) loggWithCtx(message, "RTC", value)

// Construct RTC
RTC::RTC() { }

// Destruct RTC
RTC::~RTC() { }

// Routine to initialize RTC
void RTC::init() {
    logg(INITIALIZING);
    if(!rtc.begin()) {
        logg("Initialization failed!");
        _initialised = false;
    } else {
        logg("Initialized");
        _initialised = true;
    }
}

// Routine to check if RTC is initialized
bool RTC::isInitialised() {
    return _initialised;
}

// Routine to set hour
bool RTC::setHour(uint8_t hour) {
    if (!_initialised) {
        loggValue("Not initialized", "Hour");
        return false;
    }

    if (hour > MAX_HOUR) {
        loggValue("Invalid value", "Hour");
        return false;
    }

    return rtc.setHours(hour);
}

// Routine to set minute
bool RTC::setMinute(uint8_t minute) {
    if (!_initialised) {
        loggValue("Not initialized", "Minute");
        return false;
    }

    if (minute > MAX_MINUTE) {
        loggValue("Invalid value", "Minute");
        return false;
    }

    return rtc.setMinutes(minute);
}

// Routine to set second
bool RTC::setSecond(uint8_t second) {
    if (!_initialised) {
        loggValue("Not initialized", "Second");
        return false;
    }

    if (second > MAX_SECOND) {
        loggValue("Invalid value", "Second");
        return false;
    }

    return rtc.setSeconds(second);
}

// Routine to set day
bool RTC::setDay(uint8_t day) {
    if (!_initialised) {
        loggValue("Not initialized", "Day");
        return false;
    }

    // TODO: correlate with month and year
    if (day > MAX_DAY) {
        loggValue("Invalid value", "Day");
        return false;
    }

    return rtc.setDate(day);
}

// Routine to set month
bool RTC::setMonth(uint8_t month) {
    if (!_initialised) {
        loggValue("Not initialized", "Month");
        return false;
    }

    if (month > MAX_MONTH) {
        loggValue("Invalid value", "Month");
        return false;
    }

    return rtc.setMonth(month);
}

// Routine to set year
bool RTC::setYear(uint16_t year) {
    if (!_initialised) {
        loggValue("Not initialized", "Year");
        return false;
    }

    return rtc.setYear(year);
}

// Routine to set from timestamp
bool RTC::setFromTimestamp(const String timestamp) {
    if (!_initialised) {
        loggValue("Not initialized", "Set timestamp");
        return false;
    }

    // Strip timestamp by delimiter ":"
    String hour, minute, second, day, month, year;
    hour = timestamp.substring(0, 2);
    minute = timestamp.substring(3, 5);
    second = timestamp.substring(6, 8);
    day = timestamp.substring(9, 11);
    month = timestamp.substring(12, 14);
    year = timestamp.substring(15, 19);
    
    // Try to update time
    if (setHour  ((uint8_t)hour.toInt())   &&
        setMinute((uint8_t)minute.toInt()) &&
        setSecond((uint8_t)second.toInt()) &&
        setDay   ((uint8_t)day.toInt())    &&
        setMonth ((uint8_t)month.toInt())  &&
        setYear  ((uint16_t)year.toInt())) {
            return true;
    }
    
    return false;
}

// Routine to get timestamp
String RTC::getTimestamp() {
    if (!_initialised) {
        loggValue("Not initialized", "Get timestamp");
        return TIME_ERROR;
    }

    if (!rtc.updateTime()) {
        logg("Update error");
        return TIME_ERROR;
    }

    return String(rtc.getHours()) + ":" +
           String(rtc.getMinutes()) + ":" +
           String(rtc.getSeconds()) + ":" +
           String(rtc.getDate()) + ":" +
           String(rtc.getMonth()) + ":" +
           String(rtc.getYear());
}