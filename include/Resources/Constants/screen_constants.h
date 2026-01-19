#ifndef SCREEN_CONSTANTS_H
#define SCREEN_CONSTANTS_H

#include <Arduino.h>

// Partial refresh configuration
// After this many partial refreshes, a full refresh will be performed to prevent ghosting
const uint8_t FULL_REFRESH_INTERVAL = 5;

enum class SCREEN_REFRESH {
    GENERAL,
    BLUETOOTH,
    SENSORS
};

enum class SCREEN_MODE {
    NO_SCREEN = -1,
    LOADING,
    BLUETOOTH,
    SENSORS,
    ENVIRONMENTAL,
    POLLUTANTS,
    SENDING,
    HEATING
};

const String LoadingScreenText[2] = {
    "Welcome",
    "CityAirQ!"
};

const String HeatingScreenText[2] = {
    "Sensors",
    "heating"
};

const String SendingScreenText[2] = {
    "Syncing",
    "historical data"
};

const String BluetoothScreenText[3] = {
    "Bluetooth",
    "Press button to",
    "toggle BLE"
};

const String SensorsScreenText[1] = {
    "Sensors"
};

const String EnvironmentalScreenText[1] = {
    "Ambient"
};

const String PollutantsScreenText[1] = {
    "Pollution"
};

#endif // SCREEN_CONSTANTS_H