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

// Global variables
Bluetooth_module bluetoothModule; // Bluetooth module
CO2Sensor co2Sensor(&bluetoothModule); // CO2 sensor
PMSensor pmSensor(&bluetoothModule); // PM sensor
BMESensor bmeSensor(&bluetoothModule); // BME sensor
Display display(&bmeSensor, &pmSensor, &co2Sensor, &bluetoothModule); // Display

hw_timer_t *timer_read_sensors = NULL; // Timer for sensor readings
uint8_t read_sensor = SENSORS::NO_SENSOR; // Which sensors to read?
uint8_t pressed_button = BUTTONS::NO_BUTTON; // Which button was pressed?
unsigned long lastInterruptTime = 0; // Last time a button was pressed


// Function prototypes
void init_timer_read_sensors();
void init_buttons();
void handle_sensor_readings();
void handle_button_readings();
void check_CO2_sensor();
void check_PM_sensor();
void check_BME_sensor();

void IRAM_ATTR ISR_sensors_read();
void IRAM_ATTR ISR_button_B();
void IRAM_ATTR ISR_button_Y();
void IRAM_ATTR ISR_button_R();

void setup() {
  // Configure serial
  Serial.begin(115200);
  // Configure SPI
  SPI.begin(CLK_PIN, MISO_PIN, MOSI_PIN);
  // Init buttons
  init_buttons();
  // Init CO2 sensor (serial0)
  // co2Sensor.init();
  // Initialize PM sensor (serial1)
  // pmSensor.init();
  // Initialize BME sensor
  bmeSensor.init();
  // Initialize BLE
  bluetoothModule.init();
  // Initialize display
  display.init();
  display.updateScreen();

  // Initialize timer for sensor reads
  init_timer_read_sensors();
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
    co2Sensor.update();

    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_CO2;
  }
}

void check_PM_sensor() {
  if (read_sensor & SENSORS::SENSOR_PM) {
    pmSensor.update();

    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_PM;
  }
}

void check_BME_sensor() {
  if (read_sensor & SENSORS::SENSOR_BME) {
    bmeSensor.update();
    
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_BME;
  }
}

void handle_button_readings() {
  if (pressed_button != BUTTONS::NO_BUTTON) {
    // Button blue pressed
    if (pressed_button & BUTTONS::BUTTON_B) {
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_B;

      // treat button pressed
      logg("Button B pressed");
      display.changeScreenLeft();
    }

    // Button yellow pressed
    if (pressed_button & BUTTONS::BUTTON_Y) {
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_Y;

      // treat button pressed
      logg("Button Y pressed");
      // Check if screen is interactive
      if (display.getScreenMode() == SCREENMODE::BLUETOOTH) {
        // Change BLE state
        if (bluetoothModule.isEnabled()) {
          bluetoothModule.disable();
        } else {
          bluetoothModule.enable();
        }
      }
    }

    // Button red pressed
    if (pressed_button & BUTTONS::BUTTON_R) {
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_R;
      
      // treat button pressed
      logg("Button R pressed");
      display.changeScreenRight();
    }

    // Update display
    display.updateScreen();
  }
}

// Mark all sensors to be read
void IRAM_ATTR ISR_sensors_read() {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
  read_sensor |= SENSORS::SENSOR_BME;
}

// Button blue pressed
void IRAM_ATTR ISR_button_B() {
  unsigned long interruptTime = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
    pressed_button |= BUTTONS::BUTTON_B;
  }
  lastInterruptTime = interruptTime;
}

// Button yellow pressed
void IRAM_ATTR ISR_button_Y(){
  unsigned long interruptTime = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
    pressed_button |= BUTTONS::BUTTON_Y;
  }
  lastInterruptTime = interruptTime;
}

// Button red pressed
void IRAM_ATTR ISR_button_R(){
  unsigned long interruptTime = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
    pressed_button |= BUTTONS::BUTTON_R;
  }
  lastInterruptTime = interruptTime;
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
  logg("Timer initialized");
}

void init_buttons() {
  pinMode(BUTTON_BLUE, INPUT);
  pinMode(BUTTON_YELLOW, INPUT);
  pinMode(BUTTON_RED, INPUT);

  attachInterrupt(BUTTON_BLUE, ISR_button_B, FALLING);
  attachInterrupt(BUTTON_YELLOW, ISR_button_Y, FALLING);
  attachInterrupt(BUTTON_RED, ISR_button_R, FALLING);
}