#include "PM_sensor.h"

// Construct PM sensor
PMSensor::PMSensor(Bluetooth_module *bluetoothModule)
    : bluetoothModule(bluetoothModule) {
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

// Routine to initialize PM sensor
void PMSensor::init() {
  pms->init();
  this->_sensorFound = true;
  logg("PM sensor initialized");
}

// Routine to update PM values
void PMSensor::update() {
  if (!this->_sensorFound) {
      return;
  }
  
  logg("PMSA003 reading...");
  SerialPM::STATUS status = pms->read();
  
  if (status != SerialPM::OK) {
      this->_errorPM = true;
      this->checkErrors(status);
  } else {
      this->_errorPM = false;

      this->pm1 = pms->pm01;
      logg("PMSA003 PM1: " + String(this->pm1));

      this->pm2_5 = pms->pm25;
      logg("PMSA003 PM2.5: " + String(this->pm2_5));

      this->pm10 = pms->pm10;
      logg("PMSA003 PM10: " + String(this->pm10));

      this->updateCharacteristics();
  }
}

// Getters
uint16_t PMSensor::getPM1() {
  return this->pm1;
}

uint16_t PMSensor::getPM2_5() {
  return this->pm2_5;
}

uint16_t PMSensor::getPM10() {
  return this->pm10;
}

// Internal functions
void PMSensor::updateCharacteristics() {
  if (!bluetoothModule->isConnected() || !bluetoothModule->isEnabled()) {
    return;
  }

  logg("Updating PM characteristics");

  bluetoothModule->updatePM1Characteristic(this->pm1);
  bluetoothModule->updatePM2_5Characteristic(this->pm2_5);
  bluetoothModule->updatePM10Characteristic(this->pm10);
}

void PMSensor::checkErrors(SerialPM::STATUS status) {
  switch (status)
    {
    case SerialPM::OK: // should never come here
      break;                  // included to compile without warnings
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