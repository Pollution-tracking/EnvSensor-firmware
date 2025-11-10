#include "Sensors/CO2_sensor.h"

#define logg(message) loggWithObj(message, "CO2")
#define loggValue(message, value) loggWithCtx(message, "CO2", value)

// Construct CO2 sensor
CO2Sensor::CO2Sensor() {
    Serial2.flush();
    Serial2.end();
    Serial2.begin(9600, SERIAL_8N1, CO2_RX_PIN, CO2_TX_PIN);
}

// Destruct CO2 sensor
CO2Sensor::~CO2Sensor() { }

// Routine to initialize CO2 sensor
void CO2Sensor::init() {
    logg(INITIALIZING);
    Serial2.flush();
    delay(100);
    mhz19.begin(Serial2);

    // Wait for sensor to start
    delay(100);

    // Check if the sensor is found
    if (mhz19.errorCode == RESULT_OK) {
        logg(INITIALIZED);

        mhz19.autoCalibration();
        this->status.found = true;
        this->status.initialised = true;
        this->status.error = false;
    } else {
        logg(NOT_INITIALIZED);

        this->status.initialised = false;
        this->status.error = false;
        this->status.found = false;
    }
}

// Routine to update CO2 and temperature values
void CO2Sensor::read() {
    logg(TRIGGER_READ);

    if (!this->status.initialised) {
        logg(NOT_INITIALIZED);

        // Mark errors
        this->markReadError();
        return;
    }

    this->data.co2 = mhz19.getCO2();
    this->data.temperature = mhz19.getTemperature();

    // Check for errors
    this->checkErrors();

    if (!this->status.error) {
        loggValue(String(this->data.co2), "CO2");
        loggValue(String(this->data.temperature), "Temperature");
    }
}

// Getters
CO2Data& CO2Sensor::getData() {
    return data;
}

SensorStatus& CO2Sensor::getStatus() {
    return status;
}

String CO2Sensor::getName() {
    return "MH-Z19";
}

// Internal functions
void CO2Sensor::checkErrors() {
    if (mhz19.errorCode != RESULT_OK || this->data.co2 < 0 || this->data.temperature == -273) {
        // Log the error
        logg(FAILED_READ);

        this->status.error = true;
        this->data.co2 = READ_ERROR;
        this->data.temperature = READ_ERROR;
    } else {
        // No errors
        this->status.error = false;
    }
}

void CO2Sensor::markReadError() {
  this->status.error = true;
  this->data.co2 = READ_ERROR;
  this->data.temperature = READ_ERROR;
}
