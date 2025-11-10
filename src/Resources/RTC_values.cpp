#include <Arduino.h>
#include <configs.h>
#include <Resources/Constants/screen_constants.h>
#include <Resources/Constants/board_constants.h>

// Update board name with chipid
RTC_DATA_ATTR String bleServerName =
    "EnvSensor_" + String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);

volatile RTC_DATA_ATTR BOARD_CFG board_config = {
    .to_treat = false,
    .sensors_state = SENSORS::NO_ACTION,
    .buttons_state = BUTTONS::NO_BUTTON,
    .timer_type = TIMER_TYPES::NO_TIMER,
    .timer_mode = TIMER_MODES::T_DISABLE,
    .ble_stats = { .enabled = false, .connected = false },
    .ble_state = BLE::NO_UPDATE,
    .screen = SCREEN_MODE::NO_SCREEN,
    .board_state = SLEEP_STATE::AWAKE
};

RTC_DATA_ATTR RTC_SensorData lastSensorsData = {
#ifdef BME_ENABLE
    .lastBMEData = BMEData(),
#endif
#ifdef PM_ENABLE
    .lastPMData = PMData(),
#endif
#ifdef CO2_ENABLE
    .lastCO2Data = CO2Data(),
#endif
#ifdef MICS_ENABLE
    .lastMICSData = MICSData(),
#endif
    .lastBatteryData = {},
    .timestamp = ""
};

RTC_DATA_ATTR RTC_ScreenData lastScreenData = {
    .previousScreen = SCREEN_MODE::NO_SCREEN,
    .currentScreen = SCREEN_MODE::NO_SCREEN
};
