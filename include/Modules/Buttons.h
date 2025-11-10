#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Resources/Constants/button_constants.h>
#include <Resources/Constants/logging_constants.h>
#include <Resources/Constants/board_constants.h>
#include <Resources/RTC_values.h>

BUTTONS handleButtonsState(const BUTTONS state);
void act_on_wakeup_button();

#endif // BUTTONS_H