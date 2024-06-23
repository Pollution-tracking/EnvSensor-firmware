#ifndef BUTTONS_HPP
#define BUTTONS_HPP

#include <Arduino.h>

#include <Resources/Pins.h>
#include <Resources/Constants.h>
#include <Resources/RTC_values.hpp>

#include <Helpers/Timers.hpp>
#include <Helpers/SleepUtils.hpp>
#include <Logger/Logger.h>

#include <Modules/Display.h>
#include <Modules/Bluetooth_module.h>

extern Display display;
extern Bluetooth_module bluetoothModule;
extern SleepUtils sleepUtils;

void IRAM_ATTR ISR_button_left();
void IRAM_ATTR ISR_button_center();
void IRAM_ATTR ISR_button_right();

void init_buttons();
void handle_button_readings();

void virtual_press_button(uint8_t button);

void check_left_button();
void check_center_button();
void check_right_button();

void treat_left_button();
void treat_center_button();
void treat_right_button();

void interact_BLE();
void interact_sensors();

#endif // BUTTONS_HPP