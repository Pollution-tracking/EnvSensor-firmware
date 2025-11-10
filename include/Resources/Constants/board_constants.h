#ifndef BOARD_CONSTANTS_H
#define BOARD_CONSTANTS_H

#include <Arduino.h>
#include <configs.h>
#include <Resources/Constants/timers_constants.h>
#include <Resources/Constants/sensor_constants.h>
#include <Resources/Constants/button_constants.h>
#include <Resources/Constants/bluetooth_constants.h>
#include <Resources/Constants/screen_constants.h>

#ifdef BME_ENABLE
#include <Sensors/BME_sensor.h>
#endif
#ifdef PM_ENABLE
#include <Sensors/PM_sensor.h>
#endif
#ifdef CO2_ENABLE
#include <Sensors/CO2_sensor.h>
#endif
#ifdef MICS_ENABLE
#include <Sensors/MICS_sensor.h>
#endif
#include <Sensors/Battery.h>

enum class SLEEP_STATE {
    AWAKE, // normal operation
    AWAKENING, // waking up from sleep
    TO_SLEEP, // going to deep sleep mode
    FROM_SLEEP // woke up from sleep, but will go back
};

typedef struct {
    /* Made changes that need to be treated */
    bool to_treat;
    /* Sensor states */
    volatile SENSORS sensors_state;
    /* Button states */
    volatile BUTTONS buttons_state;
    /* Timer states */
    TIMER_TYPES timer_type;
    TIMER_MODES timer_mode;
    /* Bluetooth states */
    BLE_STATS ble_stats;
    BLE ble_state;
    /* Screen states */
    SCREEN_MODE screen;
    /* Board states */
    volatile SLEEP_STATE board_state;
} BOARD_CFG;

struct RTC_SensorData {
#ifdef BME_ENABLE
    BMEData lastBMEData;
#endif
#ifdef PM_ENABLE
    PMData lastPMData;
#endif
#ifdef CO2_ENABLE
    CO2Data lastCO2Data;
#endif
#ifdef MICS_ENABLE
    MICSData lastMICSData;
#endif
    BatteryData lastBatteryData;
    String timestamp;
};

struct RTC_ScreenData {
    SCREEN_MODE previousScreen;
    SCREEN_MODE currentScreen;
};

#endif // BOARD_CONSTANTS_H