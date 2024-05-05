#include "BME_sensor.h"

// Construct BME sensor
BMESensor::BMESensor() {
    bme = new Adafruit_BME680(BME680_CS_PIN);
}

BMESensor::~BMESensor() {
    delete bme;
}

bool BMESensor::sensorFound() {
    return this->_sensorFound;
}

bool BMESensor::sensorError() {
    return this->_errorBME;
}

bool BMESensor::sensorInitialised() {
    return this->_initialised;
}

// Routine to initialize BME sensor
void BMESensor::init() {
    if(!bme->begin()) {
        logg("[BME] Could not find a valid BME680 sensor, check wiring!");
    } else {
        _sensorFound = true;
        bme->setTemperatureOversampling(BME680_OS_8X);
        bme->setHumidityOversampling(BME680_OS_2X);
        bme->setPressureOversampling(BME680_OS_4X);
        bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme->setGasHeater(320, 150); // 320*C for 150 ms
        logg("[BME] BME680 initialized");
    }
    
    _initialised = true;
}

// Routine to update BME values
void BMESensor::read() {
    if (!this->_sensorFound) {
        this->markError();
        return;
    }

    logg("\tBME680 reading...");
    
    bool status = bme->performReading();
    this->checkErrors(status);

    if (!this->_errorBME) {
        data.temperature = bme->temperature;
        logg("\tBME680 temperature: " + String(data.temperature));

        data.pressure = bme->pressure / 100.0; //hPa
        logg("\tBME680 pressure: " + String(data.pressure));

        data.humidity = bme->humidity;
        logg("\tBME680 humidity: " + String(data.humidity));

        data.gas = bme->gas_resistance / 1000.0;
        logg("\tBME680 gas: " + String(data.gas));

        data.altitude = bme->readAltitude(seaLevel);
        logg("\tBME680 altitude: " + String(data.altitude));
    }
}

// Getters
BMEData BMESensor::getData() {
  return data;
} 

// Internal functions
void BMESensor::checkErrors(bool status) {
    if (!status) {
        logg("[BME] BME680 error");
        this->markError();
    } else {
        this->_errorBME = false;
    }
}

void BMESensor::markError() {
    this->_errorBME = true;
    this->data.temperature = READ_ERROR;
    this->data.pressure = READ_ERROR;
    this->data.humidity = READ_ERROR;
    this->data.gas = READ_ERROR;
    this->data.altitude = READ_ERROR;
}