#include <Arduino.h>
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <CO2_sensor.h>
#include <PM_sensor.h>
#include <BME_sensor.h>
#include <Bluetooth_module.h>
#include <Display.h>
#include <SDcard.h>
#include <Buttons.hpp>
#include <Sensors.hpp>
#include <Timers.hpp>
#include "esp_sleep.h"
#include "esp_wifi.h"

// Global variables
Bluetooth_module bluetoothModule; // Bluetooth module
CO2Sensor co2Sensor(&bluetoothModule); // CO2 sensor
PMSensor pmSensor(&bluetoothModule); // PM sensor
BMESensor bmeSensor(&bluetoothModule); // BME sensor
SDcard sdCard; // SD card
Display display(&bmeSensor, &pmSensor, &co2Sensor, &bluetoothModule); // Display

SleepUtils sleepUtils;

// Function prototypes
void enter_sleep_mode();
void configure_wakeup_sources();
void treat_wakeup_reason();

void setup() {
    // Disable WiFi
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();

    // Configure serial
    Serial.begin(115200);
    
    // Configure SPI
    SPI.begin(_CLK_PIN, _MISO_PIN, _MOSI_PIN, SD_CS_PIN);

    // Initialize BLE
    bluetoothModule.init();
    
    // Init SD card
    sdCard.init();

    // Initialize sensors
    init_sensors();

    // Initialize buttons
    init_buttons();

    // Do an initial read of the sensors
    read_all_sensors();

    // Initialize display
    display.init();

    // Initialize timer for sensor reads
    init_timer_read_sensors();
}

void loop() {
  // Check if there is a sensor to read
  handle_sensor_readings();

  // Check if there is a button pressed
  handle_button_readings();

  // Check if we can sleep
  if (sleepUtils.is_sleep_allowed()) {
    enter_sleep_mode();
  }
}

void enter_sleep_mode() {
  logg("Entering light sleep mode");
  
  // Enable light sleep wakes
  configure_wakeup_sources();

  // Disable timer
  disable_timer_read_sensors();

  // Flush debug messages
  forcePrint();

  // Enter sleep mode
  esp_light_sleep_start();

  // Treat wakeup reason
  treat_wakeup_reason();
}

void configure_wakeup_sources() {
  // Timer wakeup for sensor readings
  esp_sleep_enable_timer_wakeup(WAIT_TIME_READ_SENSORS);
  // GPIO wakeup for button presses
  gpio_wakeup_enable(GPIO_BUTTON_LEFT, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable(GPIO_BUTTON_RIGHT, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
}

void treat_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    // Need to read sensors
    case ESP_SLEEP_WAKEUP_TIMER:
      logg("Wakeup caused by timer");
      ISR_sensors_read();
      break;
    // Button pressed - interrupt will handle it
    case ESP_SLEEP_WAKEUP_GPIO:
      logg("Wakeup caused by GPIO");
      // Momentarily disable sleep mode
      sleepUtils.disable_sleep();
      sleepUtils.enable_cooldown();
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
      init_timer_reanable_sleep();
      break;
    default:
      logg("Wakeup was not caused by timer or GPIO");
      break;
  }
}