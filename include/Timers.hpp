#ifndef TIMERS_HPP
#define TIMERS_HPP

#include <Arduino.h>
#include "constants.h"
#include "driver/timer.h"
#include <logger.h>

extern SleepUtils sleepUtils;
extern uint8_t read_sensor;

void IRAM_ATTR ISR_sensors_read();
void IRAM_ATTR ISR_reenable_sleep();

void init_timer_read_sensors();
void disable_timer_read_sensors();

void init_timer_reanable_sleep();
void restart_timer_reenable_sleep();
void disable_timer_reenable_sleep();

#endif // TIMERS_HPP