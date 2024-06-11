#include "Modules/RTC.h"

#define logg(message) loggWithBase(message, "RTC")
#define loggWithContext(message, context) loggWithContext(message, context, "RTC")

// Construct RTC
RTC::RTC() {
    wire = new TwoWire(0);
    wire->begin(_SDA_PIN, _SCL_PIN);
    logg("Wire initialized");
}

// Destruct RTC
RTC::~RTC() {
    delete wire;
}

// Routine to initialize RTC
void RTC::init() {
    if(!rtc.begin(*wire)) {
        logg("Initialization failed!");
    } else {
        logg("Initialized");
        _initialised = true;
    }

    errorMsg = String(TIME_ERROR_8) + ":" +
               String(TIME_ERROR_8) + ":" +
               String(TIME_ERROR_8) + ":" +
               String(TIME_ERROR_8) + ":" +
               String(TIME_ERROR_8) + ":" +
               String(TIME_ERROR_16);
}

// Routine to check if RTC is initialized
bool RTC::isInitialised() {
    return _initialised;
}

// Routine to set hour
bool RTC::setHour(uint8_t hour) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Hour");
        return false;
    }

    if (hour > 23) {
        loggWithContext("Invalid value", "Hour");
        return false;
    }

    return rtc.setHours(hour);
}

// Routine to set minute
bool RTC::setMinute(uint8_t minute) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Minute");
        return false;
    }

    if (minute > 59) {
        loggWithContext("Invalid value", "Minute");
        return false;
    }

    return rtc.setMinutes(minute);
}

// Routine to set second
bool RTC::setSecond(uint8_t second) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Second");
        return false;
    }

    if (second > 59) {
        loggWithContext("Invalid value", "Second");
        return false;
    }

    return rtc.setSeconds(second);
}

// Routine to set day
bool RTC::setDay(uint8_t day) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Day");
        return false;
    }

    // TODO: correlate with month and year
    if (day > 31) {
        loggWithContext("Invalid value", "Day");
        return false;
    }

    return rtc.setWeekday(day);
}

// Routine to set month
bool RTC::setMonth(uint8_t month) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Month");
        return false;
    }

    if (month > 12) {
        loggWithContext("Invalid value", "Month");
        return false;
    }

    return rtc.setMonth(month);
}

// Routine to set year
bool RTC::setYear(uint16_t year) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Year");
        return false;
    }

    return rtc.setYear(year);
}

// Routine to set from timestamp
bool RTC::setFromTimestamp(const std::string timestamp) {
    if (!_initialised) {
        loggWithContext("Not initialized", "Set timestamp");
        return false;
    }

    // Strip timestamp by delimiter ":"
    std::string hour, minute, second, day, month, year;
    hour.assign(timestamp, 0, 2);
    minute.assign(timestamp, 3, 2);
    second.assign(timestamp, 6, 2);
    day.assign(timestamp, 9, 2);
    month.assign(timestamp, 12, 2);
    year.assign(timestamp, 15, 4);
    
    // Try to update time
    if (setHour  (uint8_t(std::stoi(hour)))   &&
        setMinute(uint8_t(std::stoi(minute))) &&
        setSecond(uint8_t(std::stoi(second))) &&
        setDay   (uint8_t(std::stoi(day)))    &&
        setMonth (uint8_t(std::stoi(month)))  &&
        setYear  (uint16_t(std::stoi(year)))) {
            RTCUpdated = true;
            return true;
    }
    
    return false;
}

// Routine to check if RTC is updated
bool RTC::isRTCUpdated() {
    return RTCUpdated;
}

// Routine to get timestamp
String RTC::getTimestamp() {
    if (!_initialised) {
        loggWithContext("Not initialized", "Get timestamp");
        return errorMsg;
    }

    if (!rtc.updateTime()) {
        logg("Update error");
        return errorMsg;
    }

    return String(rtc.getHours()) + ":" +
           String(rtc.getMinutes()) + ":" +
           String(rtc.getSeconds()) + ":" +
           String(rtc.getWeekday()) + ":" +
           String(rtc.getMonth()) + ":" +
           String(rtc.getYear());
}