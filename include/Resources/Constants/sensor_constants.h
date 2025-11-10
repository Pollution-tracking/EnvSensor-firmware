#ifndef SENSOR_CONSTANTS_H
#define SENSOR_CONSTANTS_H

#include <Arduino.h>

// Sensor errors
const int32_t READ_ERROR   = INT32_MIN;
const int32_t SENSOR_ERROR = INT32_MAX;
const int32_t NO_DATA      = INT16_MAX;

const char SENSOR_ERROR_STRING[] = "Sensor err\0";
const char READ_ERROR_STRING[]   = "Read err\0";
const char NO_DATA_STRING[]      = "No data\0";

// Sensor enum guarded by config defines
enum SENSORS {
    NO_ACTION = 0,
#if defined(BME_ENABLE)
    SENSOR_BME = 1,
#endif
#if defined(PM_ENABLE)
    SENSOR_PM = 2,
#endif
#if defined(CO2_ENABLE)
    SENSOR_CO2 = 3,
#endif
#if defined(MICS_ENABLE)
    SENSOR_MICS = 4,
#endif
    BATTERY = 5,
    INIT_SENSORS = 6,
    PREPARE_SENSORS = 7,
    READ_SENSORS = 8,
    HEATED_SENSORS = 9,
};

#endif // SENSOR_CONSTANTS_H