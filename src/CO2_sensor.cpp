#include "CO2_sensor.h"

// Construct CO2 sensor
CO2Sensor::CO2Sensor(BLECharacteristic *co2Characteristic): co2Characteristic(co2Characteristic) {
  mhz19Serial = new HardwareSerial(1);
}

CO2Sensor::~CO2Sensor() {
  delete mhz19Serial;
}

// Routine to initialize CO2 sensor
void CO2Sensor::init() {
  mhz19Serial->begin(9600, SERIAL_8N1, CO2_RX_PIN, CO2_TX_PIN);
  mhz19.begin(*mhz19Serial);
  mhz19.autoCalibration();

  logg("CO2 sensor initialized");
}

// Routine to update CO2 and temperature values
void CO2Sensor::update() {
  logg("MH-Z19 reading...");
  this->co2 = mhz19.getCO2();
  logg("MH-Z19 CO2: " + String(this->co2));
  this->temperature = mhz19.getTemperature();
  logg("MH-Z19 temperature: " + String(this->temperature));

  this->checkErrors();
  
  if (!this->errorCO2 && !this->errorTemperature)
    this->updateCharacteristic();
}

// Getters
uint16_t CO2Sensor::getCO2() {
  return this->co2;
}

uint16_t CO2Sensor::getTemperature() {
  return this->temperature;
}

// Internal functions
void CO2Sensor::updateCharacteristic() {
  logg("Updating CO2 characteristic");
  this->co2Characteristic->setValue(this->co2);
  this->co2Characteristic->notify();
}

void CO2Sensor::checkErrors() {
  if (this->co2 == 0) {
    logg("MHZ19 CO2 error");
    this->errorCO2 = true;
  } else {
    this->errorCO2 = false;
  }

  if (this->temperature == -273.15) {
    logg("MHZ19 temperature error");
    this->errorTemperature = true;
  } else {
    this->errorTemperature = false;
  }
}
