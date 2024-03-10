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
#include <RTC_values.hpp>
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
void determine_wakeup_button();

void setup() {
    // Disable WiFi
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();

    // Configure serial
    Serial.begin(115200);
    
    // Configure SPI
    SPI.begin(_CLK_PIN, _MISO_PIN, _MOSI_PIN, SD_CS_PIN);

    // Initialize display
    display.init();

    // Initialize BLE
    bluetoothModule.init();
    
    // Init SD card
    sdCard.init();

    // Initialize sensors
    init_sensors();

    // Treat wakeup reason
    treat_wakeup_reason();

    // Initialize buttons
    init_buttons();

    // Do an initial read of the sensors
    read_all_sensors();

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
  logg("Entering deep sleep mode");
  
  // Enable deep sleep wakes
  configure_wakeup_sources();

  // Disable timer
  disable_timer_read_sensors();

  // Flush debug messages
  forcePrint();

  // Enter sleep mode
  esp_deep_sleep_start();
}

void configure_wakeup_sources() {
  // Timer wakeup for sensor readings
  esp_sleep_enable_timer_wakeup(WAIT_TIME_READ_SENSORS);
  // ext1 wakeup for button presses
  esp_sleep_enable_ext1_wakeup(BUTTONS_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void treat_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER:
        logg("Wakeup caused by timer");
        // Read sensors, go back to sleep
        init_sensors();
        sdCard.init();
        read_all_sensors();
        configure_wakeup_sources();
        esp_deep_sleep_start();
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        logg("Wakeup caused by button press");
        
        // Momentarily disable sleep mode (until cooldown is over)
        sleepUtils.disable_sleep();
        sleepUtils.enable_cooldown();
        init_timer_reanable_sleep();

        // Determine which button was pressed
        determine_wakeup_button();
        break;
    default:
        logg("Wakeup was not caused by timer or GPIO");
        break;
  }
}

void determine_wakeup_button() {
    uint64_t wakeup_pin = esp_sleep_get_ext1_wakeup_status();
    wakeup_pin = log(wakeup_pin)/log(2);

    if (wakeup_pin == BUTTON_LEFT_PIN) {
        logg("Left button");
        press_button(BUTTONS::BUTTON_LEFT);
    } else if (wakeup_pin == BUTTON_CENTER_PIN) {
        logg("Center button");
        press_button(BUTTONS::BUTTON_CENTER);
    } else if (wakeup_pin == BUTTON_RIGHT_PIN) {
        logg("Right button");
        press_button(BUTTONS::BUTTON_RIGHT);
    }

    // Force screen update
    handle_button_readings();
}