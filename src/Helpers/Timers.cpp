#include "Helpers/Timers.hpp"

#define logg(message) loggWithBase(message, "TIMERS")
#define loggWithContext(message, context) loggWithContext(message, context, "TIMERS")

// Mark all sensors to be read
bool IRAM_ATTR ISR_sensors_read(void *args) {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
  read_sensor |= SENSORS::SENSOR_BME;
  read_sensor |= SENSORS::BATTERY;

  return true;
}

// Cooldown ended, reenable sleep mode
bool IRAM_ATTR ISR_reenable_sleep(void *args) {
  sleepUtils.allow_sleep();
  sleepUtils.disable_cooldown();

  return true;
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
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, WAIT_TIME_READ_SENSORS);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, ISR_sensors_read, NULL, 0);
  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_0);

  loggWithContext("Timer initialized", "Sensor readings");
}

void disable_timer_read_sensors() {
  // Disable timer
  timer_pause(TIMER_GROUP_0, TIMER_0);
  // Detach interrupt
  timer_disable_intr(TIMER_GROUP_0, TIMER_0);
  // Delete timer
  timer_deinit(TIMER_GROUP_0, TIMER_0);

  loggWithContext("Timer disabled", "Sensor readings");
}

void pause_timer_read_sensors() {
  timer_pause(TIMER_GROUP_0, TIMER_0);

  loggWithContext("Timer paused", "Sensor readings");
}

void restart_timer_read_sensors() {
  timer_start(TIMER_GROUP_0, TIMER_0);

  loggWithContext("Timer resumed", "Sensor readings");
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

  loggWithContext("Timer initialized", "Reenable sleep");
}

void restart_timer_reenable_sleep() {
  timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);

  loggWithContext("Timer restarted", "Reenable sleep");
}

void disable_timer_reenable_sleep() {
  // Disable timer
  timer_pause(TIMER_GROUP_0, TIMER_1);
  // Detach interrupt
  timer_disable_intr(TIMER_GROUP_0, TIMER_1);
  // Delete timer
  timer_deinit(TIMER_GROUP_0, TIMER_1);

  loggWithContext("Timer disabled", "Reenable sleep");
}