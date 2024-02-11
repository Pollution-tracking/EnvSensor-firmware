#include <Arduino.h>
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include "PMserial.h"
#include <CO2_sensor.h>
#include <PM_sensor.h>
#include <BME_sensor.h>
#include <Bluetooth_module.h>
#include <Display.h>
#include <Button.h>
#include "esp_sleep.h"
#include "esp_wifi.h"

// Global variables
Bluetooth_module bluetoothModule; // Bluetooth module
CO2Sensor co2Sensor(&bluetoothModule); // CO2 sensor
PMSensor pmSensor(&bluetoothModule); // PM sensor
BMESensor bmeSensor(&bluetoothModule); // BME sensor
Display display(&bmeSensor, &pmSensor, &co2Sensor, &bluetoothModule); // Display
Button buttonLeft(BUTTON_LEFT_PIN); // Left button
Button buttonRight(BUTTON_RIGHT_PIN); // Right button

hw_timer_t *timer_read_sensors = NULL; // Timer for sensor readings
uint8_t read_sensor = SENSORS::NO_SENSOR; // Which sensors to read?
bool allow_sleep = false; // Enable sleep mode

// Function prototypes
void IRAM_ATTR ISR_sensors_read();

void init_timer_read_sensors();
void disable_timer_read_sensors();
void handle_sensor_readings();
void handle_button_readings();
void check_CO2_sensor();
void check_PM_sensor();
void check_BME_sensor();
void button_left_press();
void button_right_press();
void button_long_press();

void setup() {
  // Disable WiFi
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_stop();
  // Configure serial
  Serial.begin(115200);
  // Configure SPI
  SPI.begin(CLK_PIN, MISO_PIN, MOSI_PIN);
  // Init CO2 sensor (serial0)
  // co2Sensor.init();
  // Initialize PM sensor (serial1)
  pmSensor.init();
  // Initialize BME sensor
  bmeSensor.init();
  // Initialize BLE
  bluetoothModule.init();
  // Initialize display
  display.init();
  // display.updateScreen();

  // Initialize timer for sensor reads
  init_timer_read_sensors();

  // Init buttons
  buttonLeft.init(BUTTONS::LEFT);
  buttonRight.init(BUTTONS::RIGHT);

  display.updateScreen();
}

void loop() {
  // Check if there is a sensor to read
  handle_sensor_readings();

  // Check if there is a button pressed
  handle_button_readings();
}

void handle_sensor_readings() {
  if (read_sensor != SENSORS::NO_SENSOR) {
    // Read CO2 sensor (BLE updates are sent automatically)
    check_CO2_sensor();

    // Read PM sensor (BLE updates are sent automatically)
    check_PM_sensor();

    // Read BME sensor (BLE updates are sent automatically)
    check_BME_sensor();

    // Update display
    display.updateScreen();
  }
}

void check_CO2_sensor() {
  if (read_sensor & SENSORS::SENSOR_CO2) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_CO2;

    co2Sensor.update();
  }
}

void check_PM_sensor() {
  if (read_sensor & SENSORS::SENSOR_PM) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_PM;

    pmSensor.update();
  }
}

void check_BME_sensor() {
  if (read_sensor & SENSORS::SENSOR_BME) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_BME;

    bmeSensor.update();
  }
}

void handle_button_readings() {
  bool buttonPressed = false;

  // Left button short press
  if (buttonLeft.getButtonState() == BUTTON_STATES::SHORT_PRESS) {
    buttonPressed = true;
    button_left_press();
  }

  // Right button short press
  if (buttonRight.getButtonState() == BUTTON_STATES::SHORT_PRESS) {
    buttonPressed = true;
    button_right_press();
  }

  // Any button long press
  if (buttonLeft.getButtonState() == BUTTON_STATES::LONG_PRESS ||
      buttonRight.getButtonState() == BUTTON_STATES::LONG_PRESS) {
    buttonPressed = true;
    button_long_press();
  }

  // Update display
  if (buttonPressed) {
    display.updateScreen();
  }
}

void button_left_press() {
  // Clear flag
  buttonLeft.resetButtonState();

  // Treat button pressed
  logg("Button Left short press");
  display.changeScreenLeft();
}

void button_right_press() {
  // Clear flag
  buttonRight.resetButtonState();

  // Treat button pressed
  logg("Button Right short press");
  display.changeScreenRight();
}

void button_long_press() {
  // Clear flag
  buttonLeft.resetButtonState();
  buttonRight.resetButtonState();

  logg("Button long press");

  // Check if screen is interactive (blueooth screen)
  if (display.getScreenMode() == SCREENMODE::BLUETOOTH) {
    // Change BLE state
    if (bluetoothModule.isEnabled()) {
      bluetoothModule.disable();
      allow_sleep = true;
    } else {
      bluetoothModule.enable();
      allow_sleep = false;
    }
  }
}

// Mark all sensors to be read
void IRAM_ATTR ISR_sensors_read() {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
  read_sensor |= SENSORS::SENSOR_BME;
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
