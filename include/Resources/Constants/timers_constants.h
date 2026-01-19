#ifndef TIMERS_CONSTANTS_H
#define TIMERS_CONSTANTS_H

#include <Arduino.h>
#include <string>

#define READ_POLL    true
#define READ_PREPARE false
typedef bool T_READ_STATE;

enum class TIMER_MODES {
    T_ACTIVE,
    T_RESTART,
    T_PAUSE,
    T_RESUME,
    T_DISABLE
};

enum class TIMER_TYPES {
    T_HEAT = 0,
    T_READ = 1,
    T_SLEEP = 2,
    NR_TIMERS,
    NO_TIMER
};

// Timer values (in us -> chosen to be prime numbers)
const uint64_t WAIT_TIME_READ_SENSORS    = 7000027;
const uint64_t WAIT_TIME_PREPARE_SENSORS = 12999989;
const uint64_t WAIT_TIME_HEAT_SENSORS    = 60000011;
const uint64_t WAIT_TIME_REENABLE_SLEEP  = 10000007;

#endif // TIMERS_CONSTANTS_H