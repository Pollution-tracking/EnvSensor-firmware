#include "Sensors/PM_sensor.h"

#define logg(message) loggWithBase(message, "PM")
#define loggWithContext(message, context) loggWithContext(message, context, "PM")

// Construct PM sensor
PMSensor::PMSensor() {
  pms = new SerialPM(PMSA003, PM_TX_PIN, PM_RX_PIN);
}

// Destruct PM sensor
PMSensor::~PMSensor() {
  delete pms;
}

// Routine to initialize PM sensor
void PMSensor::init() {
  pms->init();
  pms->wake();
  this->_sensorFound = true;
  logg("Initialized");
  
  this->_initialised = true;
}

// Routine to update PM values
void PMSensor::read() {
  if (!this->_initialised) {
    this->markError();
    return;
  }
  
  logg("Reading");
  SerialPM::STATUS status = pms->read();
  
  if (status != SerialPM::OK) {
    this->markError();
    this->checkErrors(status);
  } else {
    this->_errorPM = false;

    data.pm1 = pms->pm01;
    loggWithContext(String(data.pm1), "PM1");

    data.pm2_5 = pms->pm25;
    loggWithContext(String(data.pm2_5), "PM2.5");

    data.pm10 = pms->pm10;
    loggWithContext(String(data.pm10), "PM10");
  }
}

// Getters
PMData PMSensor::getData() {
  return data;
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

String PMSensor::getName() {
  return "PMSA003";
}

void PMSensor::sleep() {
  // Early exit if already sleeping
  if (this->_sleeping) {
    logg("Already sleeping");
    return;
}

  pms->sleep();
  this->_sleeping = true;
  logg("Put to sleep");
}

void PMSensor::wake() {
  // Early exit if already awake
  if (!this->_sleeping) {
    logg("Already awake");
    return;
  }

  pms->wake();
  this->_sleeping = false;
  delay(100); // Wait for sensor to wake up
  logg("Woken up");
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