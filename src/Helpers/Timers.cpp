#include "Helpers/Timers.hpp"

#define logg(message) loggWithBase(message, "TIMERS")
#define loggWithContext(message, context) loggWithContext(message, context, "TIMERS")

// Mark all sensors to be read
bool IRAM_ATTR ISR_sensors_read(void *args) {
  read_sensor = SENSORS::ALL_SENSORS;

  return true;
}

// Mark all sensors to be prepared for reading
bool IRAM_ATTR ISR_sensors_prepare(void *args) {
  read_sensor = SENSORS::SENSOR_PREPARE;

  return true;
}

// Mark all sensors to be initialized
bool IRAM_ATTR ISR_sensors_init(void *args) {
  read_sensor |= SENSORS::INIT;

  return true;
}

// Cooldown ended, reenable sleep mode
bool IRAM_ATTR ISR_reenable_sleep(void *args) {
  sleepUtils.allow_sleep();
  sleepUtils.disable_cooldown();

  return true;
}

// Pause all active timers
void pause_active_timers() {
  if (timers_state[TIMER_READ_INDEX] == TIMER_MODES::T_ACTIVE) {
    pause_timer_read_sensors();
  }

  if (timers_state[TIMER_INIT_INDEX] == TIMER_MODES::T_ACTIVE) {
    pause_timer_init_sensors();
  }

  if (timers_state[TIMER_SLEEP_INDEX] == TIMER_MODES::T_ACTIVE) {
    disable_timer_reenable_sleep();
  }
}

// Disable all active timers
void disable_active_timers() {
  if (timers_state[TIMER_READ_INDEX] == TIMER_MODES::T_ACTIVE) {
    disable_timer_read_sensors();
  }

  if (timers_state[TIMER_INIT_INDEX] == TIMER_MODES::T_ACTIVE) {
    disable_timer_init_sensors();
  }

  if (timers_state[TIMER_SLEEP_INDEX] == TIMER_MODES::T_ACTIVE) {
    disable_timer_reenable_sleep();
  }
}

// Enable all paused timers
void enable_paused_timers() {
  if (timers_state[TIMER_READ_INDEX] == TIMER_MODES::T_PAUSED) {
    restart_timer_read_sensors();
  }

  if (timers_state[TIMER_INIT_INDEX] == TIMER_MODES::T_PAUSED) {
    restart_timer_init_sensors();
  }

  if (timers_state[TIMER_SLEEP_INDEX] == TIMER_MODES::T_PAUSED) {
    restart_timer_reenable_sleep();
  }
}

// Timer that triggers sensor readings
void init_timer_read_sensors() {
  timer_config_t config = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = 80 // 1 us per tick
  };

  // Initialize timer
  timer_init(TIMER_GROUP_0, TIMER_0, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  // Setup timer interrupt for sensor readings
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_PREPARE_SENSORS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_prepare, NULL, 0);
  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Mark timer as active
  timers_state[TIMER_READ_INDEX] = TIMER_MODES::T_ACTIVE;
  read_timer_state = TIMER_READ_STATES::T_PREPARE;

  loggWithContext("Timer initialized", "Sensor readings");
}

void disable_timer_read_sensors() {
  // Disable timer
  timer_pause(TIMER_GROUP_0, TIMER_0);
  // Detach interrupt
  timer_disable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
  // Delete timer
  timer_deinit(TIMER_GROUP_0, TIMER_0);

  // Mark timer as disabled
  timers_state[TIMER_READ_INDEX] = TIMER_MODES::T_DISABLED;

  loggWithContext("Timer disabled", "Sensor readings");
}

void pause_timer_read_sensors() {
  timer_pause(TIMER_GROUP_0, TIMER_0);

  // Mark timer as paused
  timers_state[TIMER_READ_INDEX] = TIMER_MODES::T_PAUSED;

  loggWithContext("Timer paused", "Sensor readings");
}

void restart_timer_read_sensors() {
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Mark timer as active
  timers_state[TIMER_READ_INDEX] = TIMER_MODES::T_ACTIVE;

  loggWithContext("Timer resumed", "Sensor readings");
}

void register_read_interrupt() {
  timer_disable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_read, NULL, 0);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_READ_SENSORS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);

  // Change timer state
  read_timer_state = TIMER_READ_STATES::T_POLL;

  loggWithContext("Changed interrupt to POLL", "Sensor readings");
}

void register_prepare_interrupt() {
  timer_disable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_prepare, NULL, 0);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_PREPARE_SENSORS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);

  // Change timer state
  read_timer_state = TIMER_READ_STATES::T_PREPARE;

  loggWithContext("Changed interrupt to PREPARE", "Sensor readings");
}

void force_read_sensors_interrupt() {
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_PREPARE_SENSORS - 500003);
}

// Timer that triggers sensor initializations
void init_timer_init_sensors() {
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
  // Setup timer interrupt for sensor initializations
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_INIT_SENSORS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_init, NULL, 0);
  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Mark timer as active
  timers_state[TIMER_INIT_INDEX] = TIMER_MODES::T_ACTIVE;

  loggWithContext("Timer initialized", "Sensor initializations");
}

void disable_timer_init_sensors() {
  // Disable timer
  timer_pause(TIMER_GROUP_0, TIMER_0);
  // Detach interrupt
  timer_disable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_remove(TIMER_GROUP_0, TIMER_0);
  // Delete timer
  timer_deinit(TIMER_GROUP_0, TIMER_0);

  // Mark timer as disabled
  timers_state[TIMER_INIT_INDEX] = TIMER_MODES::T_DISABLED;

  loggWithContext("Timer disabled", "Sensor initializations");
}

void pause_timer_init_sensors() {
  timer_pause(TIMER_GROUP_0, TIMER_0);

  // Mark timer as paused
  timers_state[TIMER_INIT_INDEX] = TIMER_MODES::T_PAUSED;

  loggWithContext("Timer paused", "Sensor initializations");
}

void restart_timer_init_sensors() {
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Mark timer as active
  timers_state[TIMER_INIT_INDEX] = TIMER_MODES::T_ACTIVE;

  loggWithContext("Timer resumed", "Sensor initializations");
}

// Timer that triggers reenabling sleep mode after cooldown when buttons are pressed (occurs in sleep mode)
void init_timer_reanable_sleep() {
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
  // Setup timer interrupt for reenabling sleep mode after cooldown
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, WAIT_TIME_REENABLE_SLEEP);
  timer_enable_intr(TIMER_GROUP_0, TIMER_1);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_1, ISR_reenable_sleep, NULL, 0);
  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_1);

  // Mark timer as active
  timers_state[TIMER_SLEEP_INDEX] = TIMER_MODES::T_ACTIVE;

  loggWithContext("Timer initialized", "Reenable sleep");
}

void restart_timer_reenable_sleep() {
  timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);

  // Mark timer as active
  timers_state[TIMER_SLEEP_INDEX] = TIMER_MODES::T_ACTIVE;

  loggWithContext("Timer restarted", "Reenable sleep");
}

void disable_timer_reenable_sleep() {
  // Disable timer
  timer_pause(TIMER_GROUP_0, TIMER_1);
  // Detach interrupt
  timer_disable_intr(TIMER_GROUP_0, TIMER_1);
  timer_isr_callback_remove(TIMER_GROUP_0, TIMER_1);
  // Delete timer
  timer_deinit(TIMER_GROUP_0, TIMER_1);

  // Mark timer as disabled
  timers_state[TIMER_SLEEP_INDEX] = TIMER_MODES::T_DISABLED;

  loggWithContext("Timer disabled", "Reenable sleep");
}

// Request sensor readings
bool request_sensor_readings() {
  // Timer not in use
  if (timers_state[TIMER_READ_INDEX] != TIMER_MODES::T_ACTIVE) {
    return false;
  }

  // Already in the polling state
  if (read_timer_state == TIMER_READ_STATES::T_POLL) {
    return false;
  }

  // Trigger sensor readings
  force_read_sensors_interrupt();

  return true;
}