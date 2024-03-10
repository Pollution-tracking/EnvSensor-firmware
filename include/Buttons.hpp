#ifndef BUTTONS_HPP
#define BUTTONS_HPP

#include <Arduino.h>
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <Display.h>
#include <Timers.hpp>
#include <RTC_values.hpp>
#include <Bluetooth_module.h>

extern Display display;
extern Bluetooth_module bluetoothModule;
extern SleepUtils sleepUtils;

void IRAM_ATTR ISR_button_left();
void IRAM_ATTR ISR_button_center();
void IRAM_ATTR ISR_button_right();

void init_buttons();
void handle_button_readings();

void press_button(uint8_t button);

void check_left_button();
void check_center_button();
void check_right_button();

void treat_left_button();
void treat_center_button();
void treat_right_button();

#endif // BUTTONS_HPP