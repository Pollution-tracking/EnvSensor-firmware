#include "Sensors/PM_sensor.h"

// Construct PM sensor
PMSensor::PMSensor() {
  pms = new SerialPM(PMSA003, PM_TX_PIN, PM_RX_PIN);
}

PMSensor::~PMSensor() {
  delete pms;
}

bool PMSensor::sensorFound() {
  return this->_sensorFound;
}

bool PMSensor::sensorError() {
  return this->_errorPM;
}

bool PMSensor::sensorInitialised() {
  return this->_initialised;
}

// Routine to initialize PM sensor
void PMSensor::init() {
  pms->init();
  this->_sensorFound = true;
  logg("[PM] PM sensor initialized");
  
  this->_initialised = true;
}

// Routine to update PM values
void PMSensor::read() {
  if (!this->_sensorFound) {
    this->markError();
    return;
  }
  
  logg("\tPMSA003 reading...");
  SerialPM::STATUS status = pms->read();
  
  if (status != SerialPM::OK) {
    this->markError();
    this->checkErrors(status);
  } else {
    this->_errorPM = false;

    data.pm1 = pms->pm01;
    logg("\tPMSA003 PM1: " + String(data.pm1));

    data.pm2_5 = pms->pm25;
    logg("\tPMSA003 PM2.5: " + String(data.pm2_5));

    data.pm10 = pms->pm10;
    logg("\tPMSA003 PM10: " + String(data.pm10));
  }
}

// Getters
PMData PMSensor::getData() {
  return data;
}

// Internal functions
void PMSensor::checkErrors(SerialPM::STATUS status) {
  switch (status)
    {
      case SerialPM::OK: // should never come here
        break;           // included to compile without warnings
      case SerialPM::ERROR_TIMEOUT:
        logg(F(PMS_ERROR_TIMEOUT));
        break;
      case SerialPM::ERROR_MSG_UNKNOWN:
        logg(F(PMS_ERROR_MSG_UNKNOWN));
        break;
      case SerialPM::ERROR_MSG_HEADER:
        logg(F(PMS_ERROR_MSG_HEADER));
        break;
      case SerialPM::ERROR_MSG_BODY:
        logg(F(PMS_ERROR_MSG_BODY));
        break;
      case SerialPM::ERROR_MSG_START:
        logg(F(PMS_ERROR_MSG_START));
        break;
      case SerialPM::ERROR_MSG_LENGTH:
        logg(F(PMS_ERROR_MSG_LENGTH));
        break;
      case SerialPM::ERROR_MSG_CKSUM:
        logg(F(PMS_ERROR_MSG_CKSUM));
        break;
      case SerialPM::ERROR_PMS_TYPE:
        logg(F(PMS_ERROR_PMS_TYPE));
        break;
    }
}

void PMSensor::markError() {
  this->_errorPM = true;
  this->data.pm1 = READ_ERROR;
  this->data.pm2_5 = READ_ERROR;
  this->data.pm10 = READ_ERROR;
}