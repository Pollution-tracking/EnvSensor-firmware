#ifndef STORAGE_CONSTANTS_H
#define STORAGE_CONSTANTS_H

#include <Arduino.h>

const String DATA_PATH = "/SensorsData.csv";
const String STATS_PATH = "/SensorsStats.txt";
const String NO_MORE_DATA = "NO_MORE_DATA";
const String ERROR_READING = "ERROR_READING";
const int MAX_LINES_TO_RETRIEVE = 10;

// CSV data values
enum SensorDataIndex {
    TIMESTAMP_INDEX = 0,
    BATTERY_INDEX,
#ifdef CONFIG_BME_SENSOR
    BME_TEMPERATURE_INDEX,
    BME_PRESSURE_INDEX,
    BME_HUMIDITY_INDEX,
    BME_GAS_INDEX,
    BME_ALTITUDE_INDEX,
#endif
#ifdef CONFIG_CO2_SENSOR
    CO2_CO2_INDEX,
#endif
#ifdef CONFIG_PM_SENSOR
    PM_PM1_INDEX,
    PM_PM2_5_INDEX,
    PM_PM10_INDEX,
#endif
#ifdef CONFIG_MICS_SENSOR
    MICS_CO_INDEX,
    MICS_NO2_INDEX,
    MICS_NH3_INDEX,
#endif
    NR_VALUES // Always keep this as the last element
};

#endif // STORAGE_CONSTANTS_H