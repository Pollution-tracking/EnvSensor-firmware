#include "CO2_sensor.h"

// Construct CO2 sensor
CO2Sensor::CO2Sensor(Bluetooth_module *bluetoothModule): bluetoothModule(bluetoothModule) {
  mhz19Serial = &Serial1;
}

CO2Sensor::~CO2Sensor() {
  delete mhz19Serial;
}

bool CO2Sensor::sensorFound() {
    return this->_sensorFound;
}

bool CO2Sensor::sensorError() {
    return this->_errorCO2 || this->_errorTemperature;
}

bool CO2Sensor::sensorInitialised() {
    return this->_initialised;
}

// Routine to initialize CO2 sensor
void CO2Sensor::init() {
  mhz19Serial->begin(9600, SERIAL_8N1, CO2_TX_PIN, CO2_RX_PIN);
  mhz19.begin(*mhz19Serial);
  mhz19.autoCalibration();

  if (mhz19.errorCode != RESULT_OK) {
    logg("Could not initialize MH-Z19 sensor, check wiring!");
  } else {
    logg("MH-Z19 initialized");
    _sensorFound = true;
  }

  _initialised = true;
}

// Routine to update CO2 and temperature values
void CO2Sensor::update() {
  if (!this->_sensorFound) {
    return;
  }

  logg("MH-Z19 reading...");
  this->co2 = mhz19.getCO2();
  logg("MH-Z19 CO2: " + String(this->co2));
  this->temperature = mhz19.getTemperature();
  logg("MH-Z19 temperature: " + String(this->temperature));

  this->checkErrors();
  
  if (!this->_errorCO2 && !this->_errorTemperature)
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
  if (!bluetoothModule->isConnected() || !bluetoothModule->isEnabled()) {
    return;
  }

  logg("Updating CO2 characteristic");
  
  bluetoothModule->updateCO2Characteristic(this->co2);
}

void CO2Sensor::checkErrors() {
  if (this->co2 == 0) {
    logg("MHZ19 CO2 error");
    this->_errorCO2 = true;
  } else {
    this->_errorCO2 = false;
  }

  if (this->temperature == -273.15) {
    logg("MHZ19 temperature error");
    this->_errorTemperature = true;
  } else {
    this->_errorTemperature = false;
  }
}
