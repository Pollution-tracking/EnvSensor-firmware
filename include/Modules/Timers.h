#ifndef M_TIMERS_H
#define M_TIMERS_H

#include <Arduino.h>

#include "driver/timer.h"

#include <Logger/logger.h>
#include <Resources/Constants/timers_constants.h>
#include <Resources/RTC_values.h>

void configureTimer(const TIMER_MODES mode, const TIMER_TYPES type);
TIMER_MODES getTimerMode(const TIMER_TYPES type);

void pause_all_timers();
void disable_all_timers();
void resume_all_timers();

#endif // M_TIMERS_H