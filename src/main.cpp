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
#include <BME_sensor.h>

// Global variables
bool deviceConnected = false;  // Client conneted to server?
CO2Sensor co2Sensor(&carbonDioxideCharacteristic); // CO2 sensor
PMSensor pmSensor(&pm1Characteristic, &pm2_5Characteristic, &pm10Characteristic); // PM sensor
BMESensor bmeSensor(&temperatureCharacteristic, &pressureCharacteristic, &humidityCharacteristic, &gasCharacteristic, &altitudeCharacteristic); // BME sensor
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
  Serial.begin(115200);
  // Init buttons
  init_buttons();
  // Init CO2 sensor (serial0)
  // co2Sensor.init();
  // Initialize PM sensor (serial1)
  pmSensor.init();
  // Initialize BME sensor
  bmeSensor.init();
  // Initialize BLE
  init_BLE();

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

// Mark all sensors to be read
void IRAM_ATTR ISR_sensors_read() {
  read_sensor |= SENSORS::SENSOR_PM;
  read_sensor |= SENSORS::SENSOR_CO2;
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
  // Setup timer interrupt for sensor readings
  timerAttachInterrupt(timer_read_sensors, &ISR_sensors_read, true);
  timerAlarmWrite(timer_read_sensors, WAIT_TIME_READ_SENSORS, true);
  timerAlarmEnable(timer_read_sensors);
  
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
  gasDescriptor.setValue("Gas");
  humidityDescriptor.setValue("Humidity");
  pressureDescriptor.setValue("Pressure");
  altitudeDescriptor.setValue("Altitude");
  
  // Configure BLE characteristics
  temperatureCharacteristic.addDescriptor(&temperatureDescriptor);
  carbonDioxideCharacteristic.addDescriptor(&carbonDioxideDescriptor);
  pm1Characteristic.addDescriptor(&pm1Descriptor);
  pm2_5Characteristic.addDescriptor(&pm2_5Descriptor);
  pm10Characteristic.addDescriptor(&pm10Descriptor);
  gasCharacteristic.addDescriptor(&gasDescriptor);
  humidityCharacteristic.addDescriptor(&humidityDescriptor);
  pressureCharacteristic.addDescriptor(&pressureDescriptor);
  altitudeCharacteristic.addDescriptor(&altitudeDescriptor);

  // Add BLE characteristics to BLE service
  envService->addCharacteristic(&temperatureCharacteristic);
  envService->addCharacteristic(&carbonDioxideCharacteristic);
  envService->addCharacteristic(&pm1Characteristic);
  envService->addCharacteristic(&pm2_5Characteristic);
  envService->addCharacteristic(&pm10Characteristic);
  envService->addCharacteristic(&gasCharacteristic);
  envService->addCharacteristic(&humidityCharacteristic);
  envService->addCharacteristic(&pressureCharacteristic);
  envService->addCharacteristic(&altitudeCharacteristic);

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