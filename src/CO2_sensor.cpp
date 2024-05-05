#include "CO2_sensor.h"

// Construct CO2 sensor
CO2Sensor::CO2Sensor() {
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
    logg("[CO2] Could not initialize MH-Z19 sensor, check wiring!");
  } else {
    logg("[CO2] MH-Z19 initialized");
    _sensorFound = true;
  }

  _initialised = true;
}

// Routine to update CO2 and temperature values
void CO2Sensor::read() {
  if (!this->_sensorFound) {
    this->markError();
    return;
  }

  logg("\tMH-Z19 reading...");
  this->data.co2 = mhz19.getCO2();
  logg("\tMH-Z19 CO2: " + String(this->data.co2));
  this->data.temperature = mhz19.getTemperature();
  logg("\tMH-Z19 temperature: " + String(this->data.temperature));

  this->checkErrors();
}

// Getters
CO2Data CO2Sensor::getData() {
  return data;
}

// Internal functions
void CO2Sensor::checkErrors() {
  if (this->data.co2 == 0) {
    logg("[CO2] MHZ19 CO2 error");
    this->_errorCO2 = true;
    this->data.co2 = READ_ERROR;
  } else {
    this->_errorCO2 = false;
  }

  if (this->data.temperature == -273.15) {
    logg("[CO2] MHZ19 temperature error");
    this->_errorTemperature = true;
    this->data.temperature = READ_ERROR;
  } else {
    this->_errorTemperature = false;
  }
}

void CO2Sensor::markError() {
  this->_errorCO2 = true;
  this->_errorTemperature = true;
  this->data.co2 = READ_ERROR;
  this->data.temperature = READ_ERROR;
}
