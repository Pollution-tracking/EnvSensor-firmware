#include "Sensors/BME_sensor.h"

#define logg(message) loggWithObj(message, "BME")
#define loggValue(message, value) loggWithCtx(message, "BME", value)

// Construct BME sensor
BMESensor::BMESensor() {
    Wire.setPins(_SDA_PIN, _SCL_PIN); // Set I2C pins
    bme = new Adafruit_BME680();
}

// Destruct BME sensor
BMESensor::~BMESensor() {
    delete bme;
}

// Routine to initialize BME sensor
void BMESensor::init() {
    logg(INITIALIZING);

    bool ret = bme->begin(BME680_ADDRESS);

    // Wait for sensor to start
	delay(100);

    // Check if the sensor is found
    if(ret) {
        logg(INITIALIZED);

        this->status.found = true;
		this->status.initialised = true;
		this->status.error = false;

        bme->setTemperatureOversampling(BME680_OS_8X);
        bme->setHumidityOversampling(BME680_OS_2X);
        bme->setPressureOversampling(BME680_OS_4X);
        bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme->setGasHeater(320, 150); // 320*C for 150 ms
    } else {
        logg(NOT_INITIALIZED);

		this->status.found = false;
		this->status.initialised = false;
		this->status.error = true;
    }
}

// Routine to update BME values
void BMESensor::read() {
    logg(TRIGGER_READ);

    if (!this->status.initialised) {
        logg(NOT_INITIALIZED);

        // Mark errors
        this->markReadError();
        return;
    }
    
    bool ret = bme->performReading();
    if (!ret) {
        // Mark errors
        this->markReadError();

        // Log the error
        logg(FAILED_READ);
    } else {
        // Update the BME data
        this->status.error = false;

        data.temperature = bme->temperature;
        loggValue(String(data.temperature), "Temperature");

        data.pressure = bme->pressure / 100.0; // hPa
        loggValue(String(data.pressure), "Pressure");

        data.humidity = bme->humidity;
        loggValue(String(data.humidity), "Humidity");

        data.gas = bme->gas_resistance / 1000.0;
        loggValue(String(data.gas), "Gas");

        data.altitude = bme->readAltitude(SEA_LEVEL);
        loggValue(String(data.altitude), "Altitude");
    }
}

// Getters
BMEData& BMESensor::getData() {
    return data;
}

SensorStatus& BMESensor::getStatus() {
    return status;
}

String BMESensor::getName() {
    return "BME680";
}

// Internal functions
void BMESensor::markReadError() {
    this->status.error = true;
    this->data.temperature = READ_ERROR;
    this->data.pressure = READ_ERROR;
    this->data.humidity = READ_ERROR;
    this->data.gas = READ_ERROR;
    this->data.altitude = READ_ERROR;
}