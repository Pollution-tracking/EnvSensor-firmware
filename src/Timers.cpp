#include "Timers.hpp"

hw_timer_t *timer_read_sensors = NULL; // Timer for sensor readings
hw_timer_t *timer_reenable_sleep = NULL; // Timer for reenabling sleep mode

// Mark all sensors to be read
void IRAM_ATTR ISR_sensors_read() {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
  read_sensor |= SENSORS::SENSOR_BME;
}

// Cooldown ended, reenable sleep mode
void IRAM_ATTR ISR_reenable_sleep() {
  sleepUtils.allow_sleep();
  sleepUtils.disable_cooldown();
  disable_timer_reenable_sleep();
}

void init_timer_read_sensors() {
  // Initialize timer
  timer_read_sensors = timerBegin(1, 80, true);
  // Setup timer interrupt for sensor readings
  timerAttachInterrupt(timer_read_sensors, &ISR_sensors_read, true);
  timerAlarmWrite(timer_read_sensors, WAIT_TIME_READ_SENSORS, true);
  timerAlarmEnable(timer_read_sensors);
  
  // Start timer
  timerStart(timer_read_sensors);
  logg("Timer for sensor readings initialized");
}

void disable_timer_read_sensors() {
  // Check if timer is already disabled
  if (timer_read_sensors == nullptr || !timerStarted(timer_read_sensors)) {
    return;
  }

  // Stop timer
  timerStop(timer_read_sensors);
  // Detach interrupt
  timerDetachInterrupt(timer_read_sensors);
  // Delete timer
  timerEnd(timer_read_sensors);
  timer_read_sensors = nullptr;

  logg("Timer for sensor readings disabled");
}

void init_timer_reanable_sleep() {
  // Initialize timer
  timer_reenable_sleep = timerBegin(0, 80, true);
  // Setup timer interrupt for reenabling sleep mode
  timerAttachInterrupt(timer_reenable_sleep, &ISR_reenable_sleep, true);
  timerAlarmWrite(timer_reenable_sleep, WAIT_TIME_REENABLE_SLEEP, true);
  timerAlarmEnable(timer_reenable_sleep);
  
  // Start timer
  timerStart(timer_reenable_sleep);
}

void restart_timer_reenable_sleep() {
  timerRestart(timer_reenable_sleep);
}

void disable_timer_reenable_sleep() {
  // Check if timer is already disabled
  if (timer_reenable_sleep == nullptr || !timerStarted(timer_reenable_sleep)) {
    return;
  }

  // Stop timer
  timerStop(timer_reenable_sleep);
  // Detach interrupt
  timerDetachInterrupt(timer_reenable_sleep);
  // Delete timer
  timerEnd(timer_reenable_sleep);
  timer_reenable_sleep = nullptr;
}