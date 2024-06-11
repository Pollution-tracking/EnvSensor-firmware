#include "Sensors/BME_sensor.h"

#define logg(message) loggWithBase(message, "BME")
#define loggWithContext(message, context) loggWithContext(message, context, "BME")

// Construct BME sensor
BMESensor::BMESensor() {
    bme = new Adafruit_BME680(BME680_CS_PIN);
}

// Destruct BME sensor
BMESensor::~BMESensor() {
    delete bme;
}

// Routine to initialize BME sensor
void BMESensor::init() {
    if(!bme->begin()) {
        logg("Initialization failed!");
        return;
    } else {
        _sensorFound = true;
        bme->setTemperatureOversampling(BME680_OS_8X);
        bme->setHumidityOversampling(BME680_OS_2X);
        bme->setPressureOversampling(BME680_OS_4X);
        bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme->setGasHeater(320, 150); // 320*C for 150 ms
        logg("Initialized");
    }
    
    _initialised = true;
}

// Routine to update BME values
void BMESensor::read() {
    if (!this->_initialised) {
        this->markError();
        return;
    }

    logg("Reading");
    
    bool status = bme->performReading();
    this->checkErrors(status);

    if (!this->_errorBME) {
        data.temperature = bme->temperature;
        loggWithContext(String(data.temperature), "Temperature");

        data.pressure = bme->pressure / 100.0; //hPa
        loggWithContext(String(data.pressure), "Pressure");

        data.humidity = bme->humidity;
        loggWithContext(String(data.humidity), "Humidity");

        data.gas = bme->gas_resistance / 1000.0;
        loggWithContext(String(data.gas), "Gas");

        data.altitude = bme->readAltitude(seaLevel);
        loggWithContext(String(data.altitude), "Altitude");
    }
}

// Getters
BMEData BMESensor::getData() {
  return data;
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

String BMESensor::getName() {
    return "BME680";
}

// Internal functions
void BMESensor::checkErrors(bool status) {
    if (!status) {
        logg("Read error");
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