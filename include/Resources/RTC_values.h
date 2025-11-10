#ifndef RTC_VALUES_H
#define RTC_VALUES_H

#include <Arduino.h>
#include <Resources/Constants/board_constants.h>

// BLE server name
extern RTC_DATA_ATTR String bleServerName;

extern volatile RTC_DATA_ATTR BOARD_CFG board_config;

extern RTC_DATA_ATTR RTC_SensorData lastSensorsData;

extern RTC_DATA_ATTR RTC_ScreenData lastScreenData;

#endif // RTC_VALUES_H