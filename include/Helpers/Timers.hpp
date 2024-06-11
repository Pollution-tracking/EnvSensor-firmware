#ifndef TIMERS_HPP
#define TIMERS_HPP

#include <Arduino.h>

#include "driver/timer.h"

#include <Resources/Constants.h>

#include <Helpers/SleepUtils.hpp>

#include <Logger/Logger.h>

extern SleepUtils sleepUtils;
extern uint8_t read_sensor;

bool IRAM_ATTR ISR_sensors_read(void *args);
bool IRAM_ATTR ISR_reenable_sleep(void *args);

void init_timer_read_sensors();
void disable_timer_read_sensors();
void pause_timer_read_sensors();
void restart_timer_read_sensors();

void init_timer_reanable_sleep();
void restart_timer_reenable_sleep();
void disable_timer_reenable_sleep();

#endif // TIMERS_HPP