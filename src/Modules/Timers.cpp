#include <Modules/Timers.h>

#include <Resources/Constants/sensor_constants.h>
#include <Resources/Constants/board_constants.h>

#define logg(message) loggWithObj(message, "TIMERS")
#define loggValue(message, value) loggWithCtx(message, "TIMERS", value)

// Forwards declarations
static void init_timer_sensors_read();
static void disable_timer_sensors_read();
static void pause_timer_sensors_read();
static void resume_timer_sensors_read();
static void restart_timer_sensors_read();

static void init_timer_sensors_heat();
static void disable_timer_sensors_heat();
static void pause_timer_sensors_heat();
static void resume_timer_sensors_heat();
static void restart_timer_sensors_heat();

static void init_timer_board_sleep();
static void pause_timer_board_sleep();
static void resume_timer_board_sleep();
static void disable_timer_board_sleep();
static void restart_timer_board_sleep();

static volatile T_READ_STATE current_read_state;

static TIMER_MODES timers_state[(int)TIMER_TYPES::NR_TIMERS] = 
{
    TIMER_MODES::T_DISABLE,
    TIMER_MODES::T_DISABLE,
    TIMER_MODES::T_DISABLE
};

// Change read state (poll or prepare)
bool IRAM_ATTR ISR_sensors_read(void *args) {
    if (current_read_state == READ_POLL) {
        board_config.sensors_state = SENSORS::READ_SENSORS;

        // Swap timer event
        current_read_state = READ_PREPARE;
        timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_PREPARE_SENSORS);
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    } else {
        board_config.sensors_state = SENSORS::PREPARE_SENSORS;

        // Swap timer event
        current_read_state = READ_POLL;
        timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_READ_SENSORS);
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    }
    board_config.to_treat = true;

    return true;
}

// Announce that sensors are heated and ready
bool IRAM_ATTR ISR_sensors_heat(void *args) {
    board_config.sensors_state = SENSORS::HEATED_SENSORS;
    board_config.to_treat = true;

    return true;
}

// Mark the board to go to sleep
bool IRAM_ATTR ISR_board_sleep(void *args) {
    board_config.board_state = SLEEP_STATE::TO_SLEEP;
    board_config.to_treat = true;

    return true;
}

void configureTimer(const TIMER_MODES mode, const TIMER_TYPES type) {
    switch (type) {
        case TIMER_TYPES::T_HEAT:
            if (mode == TIMER_MODES::T_ACTIVE) {
                init_timer_sensors_heat();
            } else if (mode == TIMER_MODES::T_PAUSE) {
                pause_timer_sensors_heat();
            } else if (mode == TIMER_MODES::T_RESUME) {
                resume_timer_sensors_heat();
            } else if (mode == TIMER_MODES::T_RESTART) {
                restart_timer_sensors_heat();
            } else {
                disable_timer_sensors_heat();
            }
            break;

        case TIMER_TYPES::T_READ:
            if (mode == TIMER_MODES::T_ACTIVE) {
                init_timer_sensors_read();
            } else if (mode == TIMER_MODES::T_PAUSE) {
                pause_timer_sensors_read();
            } else if (mode == TIMER_MODES::T_RESUME) {
                resume_timer_sensors_read();
            } else if (mode == TIMER_MODES::T_RESTART) {
                restart_timer_sensors_read();
            } else {
                disable_timer_sensors_read();
            }
            break;

        case TIMER_TYPES::T_SLEEP:
            if (mode == TIMER_MODES::T_ACTIVE) {
                init_timer_board_sleep();
            } else if (mode == TIMER_MODES::T_PAUSE) {
                pause_timer_board_sleep();
            } else if (mode == TIMER_MODES::T_RESUME) {
                resume_timer_board_sleep();
            } else if (mode == TIMER_MODES::T_RESTART) {
                restart_timer_board_sleep();
            } else {
                disable_timer_board_sleep();
            }
            break;

        default:
            break;
    }
}

TIMER_MODES getTimerMode(const TIMER_TYPES type) {
    return timers_state[(int)type];
}

// Timer that triggers sensor readings
static void init_timer_sensors_read() {
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_DIS,
        .divider = 80 // 1 us per tick
    };

    // Initialize timer
    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    // Setup timer interrupt
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_READ_SENSORS);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_read, NULL, 0);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_READ] = TIMER_MODES::T_ACTIVE;
    // Mark initial state (preparing the sensors)
    current_read_state = READ_POLL;

    loggValue("Timer initialized", "Sensor readings");
}

static void pause_timer_sensors_read() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_READ] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot pause", "Sensor readings");
        return;
    }

    // Pause timer
    timer_pause(TIMER_GROUP_0, TIMER_0);

    // Mark timer as paused
    timers_state[(int)TIMER_TYPES::T_READ] = TIMER_MODES::T_PAUSE;

    loggValue("Timer paused", "Sensor readings");
}

static void resume_timer_sensors_read() {
    // Early exit if timer is not paused
    if (timers_state[(int)TIMER_TYPES::T_READ] != TIMER_MODES::T_PAUSE) {
        loggValue("Timer not paused, cannot resume", "Sensor readings");
        return;
    }

    // Restart timer
    timer_start(TIMER_GROUP_0, TIMER_0);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_READ] = TIMER_MODES::T_ACTIVE;

    loggValue("Timer resumed", "Sensor readings");
}

static void disable_timer_sensors_read() {
    // Early exit if timer is disabled
    if (timers_state[(int)TIMER_TYPES::T_READ] == TIMER_MODES::T_DISABLE) {
        loggValue("Timer already disabled", "Sensor readings");
        return;
    }
    
    // Disable timer
    timer_pause(TIMER_GROUP_0, TIMER_0);

    // Detach interrupt
    timer_disable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);

    // Delete timer
    timer_deinit(TIMER_GROUP_0, TIMER_0);

    // Mark timer as disabled
    timers_state[(int)TIMER_TYPES::T_READ] = TIMER_MODES::T_DISABLE;

    loggValue("Timer disabled", "Sensor readings");
}

static void restart_timer_sensors_read() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_READ] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot restart", "Sensor readings");
        return;
    }

    // Restart timer
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    loggValue("Timer restarted", "Sensor readings");
}

// Timer that triggers reenabling sleep mode after cooldown when buttons are pressed (occurs in sleep mode)
static void init_timer_board_sleep() {
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_DIS,
        .divider = 80 // 1 us per tick
    };

    // Initialize timer
    timer_init(TIMER_GROUP_0, TIMER_1, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);

    // Setup timer interrupt
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, WAIT_TIME_REENABLE_SLEEP);
    timer_enable_intr(TIMER_GROUP_0, TIMER_1);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_1, ISR_board_sleep, NULL, 0);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_1);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_SLEEP] = TIMER_MODES::T_ACTIVE;

    loggValue("Timer initialized", "Reenable sleep");
}

static void pause_timer_board_sleep() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_SLEEP] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot pause", "Reenable sleep");
        return;
    }

    // Pause timer
    timer_pause(TIMER_GROUP_0, TIMER_1);

    // Mark timer as paused
    timers_state[(int)TIMER_TYPES::T_SLEEP] = TIMER_MODES::T_PAUSE;

    loggValue("Timer paused", "Reenable sleep");
}

static void resume_timer_board_sleep() {
    // Early exit if timer is not paused
    if (timers_state[(int)TIMER_TYPES::T_SLEEP] != TIMER_MODES::T_PAUSE) {
        loggValue("Timer not paused, cannot resume", "Reenable sleep");
        return;
    }

    timer_start(TIMER_GROUP_0, TIMER_1);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_SLEEP] = TIMER_MODES::T_ACTIVE;

    loggValue("Timer restarted", "Reenable sleep");
}

static void disable_timer_board_sleep() {
    // Early exit if timer is disabled
    if (timers_state[(int)TIMER_TYPES::T_SLEEP] == TIMER_MODES::T_DISABLE) {
        loggValue("Timer already disabled", "Reenable sleep");
        return;
    }

    // Disable timer
    timer_pause(TIMER_GROUP_0, TIMER_1);

    // Detach interrupt
    timer_disable_intr(TIMER_GROUP_0, TIMER_1);
    timer_isr_callback_remove(TIMER_GROUP_0, TIMER_1);

    // Delete timer
    timer_deinit(TIMER_GROUP_0, TIMER_1);

    // Mark timer as disabled
    timers_state[(int)TIMER_TYPES::T_SLEEP] = TIMER_MODES::T_DISABLE;

    loggValue("Timer disabled", "Reenable sleep");
}

static void restart_timer_board_sleep() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_SLEEP] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot restart", "Reenable sleep");
        return;
    }

    // Restart timer
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);

    loggValue("Timer restarted", "Reenable sleep");
}

static void init_timer_sensors_heat() {
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_DIS,
        .divider = 80 // 1 us per tick
    };

    // Initialize timer
    timer_init(TIMER_GROUP_1, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_1, TIMER_0, 0);

    // Setup timer interrupt
    timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, WAIT_TIME_HEAT_SENSORS);
    timer_enable_intr(TIMER_GROUP_1, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_1, TIMER_0, ISR_sensors_heat, NULL, 0);

    // Start timer
    timer_start(TIMER_GROUP_1, TIMER_0);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_HEAT] = TIMER_MODES::T_ACTIVE;

    // Signal screen to show heating state
    board_config.screen = SCREEN_MODE::HEATING;
    board_config.to_treat = true;

    loggValue("Timer initialized", "Sensor heat");
}

static void pause_timer_sensors_heat() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_HEAT] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot pause", "Sensor heat");
        return;
    }

    // Pause timer
    timer_pause(TIMER_GROUP_1, TIMER_0);

    // Mark timer as paused
    timers_state[(int)TIMER_TYPES::T_HEAT] = TIMER_MODES::T_PAUSE;

    loggValue("Timer paused", "Sensor heat");
}

static void resume_timer_sensors_heat() {
    // Early exit if timer is not paused
    if (timers_state[(int)TIMER_TYPES::T_HEAT] != TIMER_MODES::T_PAUSE) {
        loggValue("Timer not paused, cannot resume", "Sensor heat");
        return;
    }

    timer_start(TIMER_GROUP_1, TIMER_0);

    // Mark timer as active
    timers_state[(int)TIMER_TYPES::T_HEAT] = TIMER_MODES::T_ACTIVE;

    loggValue("Timer restarted", "Sensor heat");
}

static void disable_timer_sensors_heat() {
    // Early exit if timer is disabled
    if (timers_state[(int)TIMER_TYPES::T_HEAT] == TIMER_MODES::T_DISABLE) {
        loggValue("Timer already disabled", "Sensor heat");
        return;
    }
    
    // Disable timer
    timer_pause(TIMER_GROUP_1, TIMER_0);

    // Detach interrupt
    timer_disable_intr(TIMER_GROUP_1, TIMER_0);
    timer_isr_callback_remove(TIMER_GROUP_1, TIMER_0);

    // Delete timer
    timer_deinit(TIMER_GROUP_1, TIMER_0);

    // Mark timer as disabled
    timers_state[(int)TIMER_TYPES::T_HEAT] = TIMER_MODES::T_DISABLE;

    loggValue("Timer disabled", "Sensor heat");
}

static void restart_timer_sensors_heat() {
    // Early exit if timer is not active
    if (timers_state[(int)TIMER_TYPES::T_HEAT] != TIMER_MODES::T_ACTIVE) {
        loggValue("Timer not active, cannot restart", "Sensor heat");
        return;
    }

    // Restart timer
    timer_set_counter_value(TIMER_GROUP_1, TIMER_0, 0);

    loggValue("Timer restarted", "Sensor heat");
}

// Pause all active timers
void pause_all_timers() {
  configureTimer(TIMER_MODES::T_PAUSE, TIMER_TYPES::T_READ);
  configureTimer(TIMER_MODES::T_PAUSE, TIMER_TYPES::T_HEAT);
  configureTimer(TIMER_MODES::T_PAUSE, TIMER_TYPES::T_SLEEP);
}

// Disable all active timers
void disable_all_timers() {
  configureTimer(TIMER_MODES::T_DISABLE, TIMER_TYPES::T_READ);
  configureTimer(TIMER_MODES::T_DISABLE, TIMER_TYPES::T_HEAT);
  configureTimer(TIMER_MODES::T_DISABLE, TIMER_TYPES::T_SLEEP);
}

// Enable all paused timers
void resume_all_timers() {
  configureTimer(TIMER_MODES::T_RESUME, TIMER_TYPES::T_READ);
  configureTimer(TIMER_MODES::T_RESUME, TIMER_TYPES::T_HEAT);
  configureTimer(TIMER_MODES::T_RESUME, TIMER_TYPES::T_SLEEP);
}