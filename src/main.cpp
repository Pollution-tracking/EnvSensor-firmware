#include <Arduino.h>
#include "ble_characteristics.h"
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include "PMserial.h"
#include <CO2_sensor.h>
#include <PM_sensor.h>

// Global variables
bool deviceConnected = false;  // Client conneted to server?
CO2Sensor co2Sensor(&carbonDioxideCharacteristic); // CO2 sensor
PMSensor pmSensor(&pm1Characteristic, &pm2_5Characteristic, &pm10Characteristic); // PM sensor
hw_timer_t *timer_read_sensors = NULL; // Timer for sensor readings
uint8_t read_sensor = SENSORS::NO_SENSOR; // Which sensors to read?
uint8_t pressed_button = BUTTONS::NO_BUTTON; // Which button was pressed?

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* envServer) {
    deviceConnected = true;
    logg("Client connected");
  };
  void onDisconnect(BLEServer* envServer) {
    deviceConnected = false;
    logg("Client disconnected");
  }
};

// Function prototypes
void init_BLE();
void init_timer_read_sensors();
void init_buttons();

void ISR_sensors_5V();
void ISR_sensors_3V3();
void IRAM_ATTR ISR_button_B();
void IRAM_ATTR ISR_button_Y();
void IRAM_ATTR ISR_button_R();

void setup() {
  Serial.begin(115200);
  // Init buttons
  init_buttons();
  // Init CO2 sensor
  co2Sensor.init();
  // Initialize PM sensor
  //pmSensor.init();
  // Initialize BLE
  init_BLE();

  // Initialize timer for sensor reads
  init_timer_read_sensors();
}

void loop() {
  // Check if there is a sensor to read
  if (read_sensor != SENSORS::NO_SENSOR) {
    // Read CO2 sensor (BLE updates are sent automatically)
    if (read_sensor & SENSORS::SENSOR_CO2) {
      co2Sensor.update();

      // Check for errors
      if (co2Sensor.errorCO2 || co2Sensor.errorTemperature) {
        // treat error
      }

      // Clear flag
      read_sensor &= ~SENSORS::SENSOR_CO2;
    }
    // Read PM sensor
    if (read_sensor & SENSORS::SENSOR_PM) {
      pmSensor.update();

      // Check for errors
      if (pmSensor.errorPM) {
        // treat error
      }

      // Clear flag
      read_sensor &= ~SENSORS::SENSOR_PM;
    }
    // Read BME sensor
    if (read_sensor & SENSORS::SENSOR_BME) {
      
      read_sensor &= ~SENSORS::SENSOR_BME;
    }
  }

  // Check if there is a button pressed
  if (pressed_button != BUTTONS::NO_BUTTON) {
    // Button blue pressed
    if (pressed_button & BUTTONS::BUTTON_B) {
      // treat button pressed
      logg("Button B pressed");
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_B;
    }
    // Button yellow pressed
    if (pressed_button & BUTTONS::BUTTON_Y) {
      // treat button pressed
      logg("Button Y pressed");
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_Y;
    }
    // Button red pressed
    if (pressed_button & BUTTONS::BUTTON_R) {
      // treat button pressed
      logg("Button R pressed");
      // Clear flag
      pressed_button &= ~BUTTONS::BUTTON_R;
    }
  }
}

// Mark PM and CO2 sensors to be read
void IRAM_ATTR ISR_sensors_5V() {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
}

// Mark BME sensor to be read
void IRAM_ATTR ISR_sensors_3V3() {
  read_sensor |= SENSORS::SENSOR_BME;
}

// Button blue pressed
void IRAM_ATTR ISR_button_B() {
  pressed_button |= BUTTONS::BUTTON_B;
}

// Button yellow pressed
void IRAM_ATTR ISR_button_Y(){
  pressed_button |= BUTTONS::BUTTON_Y;
}

// Button red pressed
void IRAM_ATTR ISR_button_R(){
  pressed_button |= BUTTONS::BUTTON_R;
}

void init_timer_read_sensors() {
  // Initialize timer
  timer_read_sensors = timerBegin(1, 80, true);
  // Setup timer interrupt for 5V sensors
  timerAttachInterrupt(timer_read_sensors, &ISR_sensors_5V, true);
  timerAlarmWrite(timer_read_sensors, WAIT_TIME_5V, true);
  timerAlarmEnable(timer_read_sensors);
  
  // Setup timer interrupt for 3V3 sensors
  // timerAttachInterrupt(timer_read_sensors, &ISR_sensors_3V3, true);
  // timerAlarmWrite(timer_read_sensors, WAIT_TIME_3V3, true);
  // timerAlarmEnable(timer_read_sensors);

  // Start timer
  timerStart(timer_read_sensors);
  logg("Timer initialized");
}

void init_BLE() {
  // Create BLE device
  BLEDevice::init(bleServerName);

  // Create BLE server
  BLEServer* envServer = BLEDevice::createServer();
  envServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service
  BLEService* envService = envServer->createService(SERVICE_UUID);

  // Set BLE descriptors values
  temperatureDescriptor.setValue("Temperature");
  carbonDioxideDescriptor.setValue("Carbon dioxide");
  pm1Descriptor.setValue("PM1");
  pm2_5Descriptor.setValue("PM2.5");
  pm10Descriptor.setValue("PM10");
  
  // Configure BLE characteristics
  temperatureCharacteristic.addDescriptor(&temperatureDescriptor);
  carbonDioxideCharacteristic.addDescriptor(&carbonDioxideDescriptor);
  pm1Characteristic.addDescriptor(&pm1Descriptor);
  pm2_5Characteristic.addDescriptor(&pm2_5Descriptor);
  pm10Characteristic.addDescriptor(&pm10Descriptor);

  // Add BLE characteristics to BLE service
  envService->addCharacteristic(&temperatureCharacteristic);
  envService->addCharacteristic(&carbonDioxideCharacteristic);
  envService->addCharacteristic(&pm1Characteristic);
  envService->addCharacteristic(&pm2_5Characteristic);
  envService->addCharacteristic(&pm10Characteristic);

  // Start BLE service
  envService->start();

  // Start advertising BLE service
  BLEAdvertising* envAdvertising = envServer->getAdvertising();
  envAdvertising->start();

  logg("Waiting a client connection to notify...");
}

void init_buttons() {
  pinMode(BUTTON_BLUE, INPUT);
  pinMode(BUTTON_YELLOW, INPUT);
  pinMode(BUTTON_RED, INPUT);

  attachInterrupt(BUTTON_BLUE, ISR_button_B, FALLING);
  attachInterrupt(BUTTON_YELLOW, ISR_button_Y, FALLING);
  attachInterrupt(BUTTON_RED, ISR_button_R, FALLING);
}