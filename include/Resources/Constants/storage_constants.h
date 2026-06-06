#ifndef STORAGE_CONSTANTS_H
#define STORAGE_CONSTANTS_H

#include <Arduino.h>
#include <configs.h>

const String DATA_PATH = "/SensorsData.csv";
const String STATS_PATH = "/SensorsStats.txt";
const String DONCHIAN_FILE_PATH = "/Donchian.csv";
const String NO_MORE_DATA = "NO_MORE_DATA";
const String ERROR_READING = "ERROR_READING";
const int MAX_LINES_TO_RETRIEVE = 10;

// CSV data values
enum SensorDataIndex {
    TIMESTAMP_INDEX = 0,
    BATTERY_INDEX,
#ifdef BME_ENABLE
    BME_TEMPERATURE_INDEX,
    BME_PRESSURE_INDEX,
    BME_HUMIDITY_INDEX,
    BME_GAS_INDEX,
    BME_ALTITUDE_INDEX,
#endif
#ifdef SHTC3_ENABLE
    SHTC3_TEMPERATURE_INDEX,
    SHTC3_HUMIDITY_INDEX,
#endif
#ifdef CO2_ENABLE
    CO2_CO2_INDEX,
#endif
#ifdef PM_ENABLE
    PM_PM1_INDEX,
    PM_PM2_5_INDEX,
    PM_PM10_INDEX,
#endif
#ifdef MICS_ENABLE
    MICS_CO_INDEX,
    MICS_NO2_INDEX,
    MICS_NH3_INDEX,
#endif
#ifdef COMPENSATION_ENABLE
#ifdef BME_ENABLE
    COMP_BME_RH_INDEX,
    COMP_BME_AQ_INDEX,
#endif
#ifdef MICS_ENABLE
    COMP_MICS_CO_INDEX,
    COMP_MICS_NO2_INDEX,
    COMP_MICS_NH3_INDEX,
#endif
#ifdef CO2_ENABLE
    COMP_CO2_INDEX,
#endif
#ifdef PM_ENABLE
    COMP_PM_PM1_INDEX,
    COMP_PM_PM2_5_INDEX,
    COMP_PM_PM10_INDEX,
#endif
#endif
    NR_VALUES // Always keep this as the last element
};

#endif // STORAGE_CONSTANTS_H