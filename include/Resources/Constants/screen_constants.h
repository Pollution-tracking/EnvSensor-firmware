#ifndef SCREEN_CONSTANTS_H
#define SCREEN_CONSTANTS_H

#include <Arduino.h>

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

#endif // SCREEN_CONSTANTS_H