#include "Sensors/CO2_sensor.h"

#define logg(message) loggWithBase(message, "CO2")
#define loggWithContext(message, context) loggWithContext(message, context, "CO2")

// Construct CO2 sensor
CO2Sensor::CO2Sensor() {
  Serial2.begin(9600, SERIAL_8N1, CO2_TX_PIN, CO2_RX_PIN);
}

// Destruct CO2 sensor
CO2Sensor::~CO2Sensor() { }

// Routine to initialize CO2 sensor
void CO2Sensor::init() {
  mhz19.begin(Serial2);

  if (mhz19.errorCode != RESULT_OK) {
    logg("Initialization failed!");
    return;
  } else {
    logg("Initialized");
    _sensorFound = true;
  }

  mhz19.autoCalibration();
  _initialised = true;
}

// Routine to update CO2 and temperature values
void CO2Sensor::read() {
  if (!this->_initialised) {
    this->markError();
    return;
  }

  logg("Reading");
  this->data.co2 = mhz19.getCO2();
  loggWithContext(String(this->data.co2), "CO2");
  this->data.temperature = mhz19.getTemperature();
  loggWithContext(String(this->data.temperature), "Temperature");

  this->checkErrors();
}

// Getters
CO2Data CO2Sensor::getData() {
  return data;
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

String CO2Sensor::getName() {
    return "MH-Z19";
}

// Internal functions
void CO2Sensor::checkErrors() {
  if (this->data.co2 < 0) {
    loggWithContext("Error", "CO2");
    this->_errorCO2 = true;
    this->data.co2 = READ_ERROR;
  } else {
    this->_errorCO2 = false;
  }

  if (this->data.temperature == -273) {
    loggWithContext("Error", "Temperature");
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
