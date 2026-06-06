#include <Arduino.h>
#include <Resources/Constants/board_constants.h>
#include <Resources/Constants/screen_constants.h>
#include <configs.h>

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
#ifdef SHTC3_ENABLE
    .lastSHTC3Data = SHTC3Data(),
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
    .timestamp = "",
#ifdef COMPENSATION_ENABLE
#if defined(BME_ENABLE) && defined(SHTC3_ENABLE)
    .bmeCompensatedRH = NO_DATA,
    .vocPctDecoupled  = NO_DATA,
    .aqScore          = NO_DATA,
#endif
#if defined(MICS_ENABLE) && defined(SHTC3_ENABLE)
    .coPPMComp  = NO_DATA,
    .no2PPMComp = NO_DATA,
    .nh3PPMComp = NO_DATA,
#endif
#if defined(CO2_ENABLE) && defined(BME_ENABLE) && defined(SHTC3_ENABLE)
    .co2Comp = NO_DATA,
#endif
#if defined(PM_ENABLE) && defined(SHTC3_ENABLE)
    .pm1Comp  = NO_DATA,
    .pm25Comp = NO_DATA,
    .pm10Comp = NO_DATA,
#endif
#endif
};

RTC_DATA_ATTR RTC_ScreenData lastScreenData = {
    .previousScreen = SCREEN_MODE::NO_SCREEN,
    .currentScreen = SCREEN_MODE::NO_SCREEN
};
