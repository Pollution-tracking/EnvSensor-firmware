#ifndef TIMERS_HPP
#define TIMERS_HPP

#include <Arduino.h>

#include "driver/timer.h"

#include <Resources/Constants.h>

#include <Helpers/SleepUtils.hpp>

#include <Logger/Logger.h>

extern SleepUtils sleepUtils;
extern volatile uint8_t read_sensor;
static TIMER_MODES timers_state[NR_TIMERS] = {TIMER_MODES::T_DISABLED,
                                              TIMER_MODES::T_DISABLED,
                                              TIMER_MODES::T_DISABLED};
static TIMER_READ_STATES read_timer_state  = TIMER_READ_STATES::T_NULL;

bool IRAM_ATTR ISR_sensors_read(void *args);
bool IRAM_ATTR ISR_sensors_prepare(void *args);
bool IRAM_ATTR ISR_sensors_init(void *args);
bool IRAM_ATTR ISR_reenable_sleep(void *args);

void pause_active_timers();
void disable_active_timers();
void enable_paused_timers();

void init_timer_read_sensors();
void disable_timer_read_sensors();
void pause_timer_read_sensors();
void restart_timer_read_sensors();
void register_read_interrupt();
void register_prepare_interrupt();
void force_read_sensors_interrupt();

void init_timer_init_sensors();
void disable_timer_init_sensors();
void pause_timer_init_sensors();
void restart_timer_init_sensors();

void init_timer_reanable_sleep();
void restart_timer_reenable_sleep();
void disable_timer_reenable_sleep();

bool request_sensor_readings();

#endif // TIMERS_HPP