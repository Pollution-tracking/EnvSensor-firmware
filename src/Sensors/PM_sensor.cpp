#include "Sensors/PM_sensor.h"

#define logg(message) loggWithObj(message, "PM")
#define loggValue(message, value) loggWithCtx(message, "PM", value)

// Construct PM sensor
PMSensor::PMSensor() {
	pms = new SerialPM(PMSA003, PM_TX_PIN, PM_RX_PIN);
}

// Destruct PM sensor
PMSensor::~PMSensor() {
	// End Serial1 communication
	Serial1.flush();
	Serial1.end();
	// Delete the PM sensor object
	if (pms != nullptr)
		delete pms;
}

// Routine to initialize PM sensor
void PMSensor::init() {
	logg(INITIALIZING);

  	pms->init();
  	pms->wake();
	
	// Wait for sensor to wake up
	delay(100);

	// Check if the sensor is found
	if (pms) {
		logg(INITIALIZED);

		this->status.found = true;
		this->status.initialised = true;
		this->status.error = false;
		this->status.sleeping = false;
	} else {
		logg(NOT_INITIALIZED);

		this->status.found = false;
		this->status.initialised = false;
		this->status.error = true;
		this->status.sleeping = false;
	}
}

// Routine to update PM values
void PMSensor::read() {
	logg(TRIGGER_READ);

	if (!this->status.initialised) {
		logg(NOT_INITIALIZED);

		// Mark errors
		markReadError();
		return;
	}

	pms->read();

	// Wait for the sensor to process the data
	delay(100);

	if (!pms) {
		// Mark errors
		markReadError();

		// Log the error
		this->checkErrors(pms->status);
	} else {
		// Update the PM data
		this->status.error = false;

		data.pm1 = pms->pm01 * 100;
		loggValue(String(data.pm1), "PM1");

		data.pm2_5 = pms->pm25 * 100;
		loggValue(String(data.pm2_5), "PM2.5");

		data.pm10 = pms->pm10 * 100;
		loggValue(String(data.pm10), "PM10");
	}
}

// Getters
SensorStatus& PMSensor::getStatus() {
  	return status;
}

PMData& PMSensor::getData() {
  	return data;
}

String PMSensor::getName() {
  	return name;
}

// Working cycle functions
void PMSensor::sleep() {
  	// Early exit if already sleeping
  	if (this->status.isSleeping()) {
		logg("Already sleeping");
		return;
	}

	pms->sleep();

	// Wait for the sensor to sleep
	delay(100);

	// Update status
	this->status.sleeping = true;
	logg("Sleeping");
}

void PMSensor::wake() {
	// Early exit if already awake
	if (!this->status.isSleeping()) {
		logg("Already awake");
		return;
	}

	pms->wake();

	// Wait for the sensor to wake up
	delay(100);

	// Update status
	this->status.sleeping = false;
	logg("Awake");
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

void PMSensor::markReadError() {
	this->status.error = true;
	this->data.pm1 = READ_ERROR;
	this->data.pm2_5 = READ_ERROR;
	this->data.pm10 = READ_ERROR;
}
