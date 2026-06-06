#include "Sensors/BME_sensor.h"

#define logg(message) loggWithObj(message, "BME")
#define loggValue(message, value) loggWithCtx(message, "BME", value)

// Construct BME sensor
BMESensor::BMESensor() {
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
    if (ret) {
        logg(INITIALIZED);

        this->status.found = true;
		this->status.initialised = true;
		this->status.error = false;

        // Default configuration
        if (!bme->setTemperatureOversampling(BME680_OS_8X)) {
            logg("Failed to set temperature oversampling");
        }
        if (!bme->setHumidityOversampling(BME680_OS_2X)) {
            logg("Failed to set humidity oversampling");
        }
        if (!bme->setPressureOversampling(BME680_OS_4X)) {
            logg("Failed to set pressure oversampling");
        }
        if (!bme->setIIRFilterSize(BME680_FILTER_SIZE_3)) {
            logg("Failed to set IIR filter size");
        }
        if (!bme->setGasHeater(320, 150)) {
            logg("Failed to start gas heater");
        }
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

        data.temperature = bme->temperature * 100;
        loggValue(String(data.temperature), "Temperature");

        data.pressure = bme->pressure;
        loggValue(String(data.pressure), "Pressure");

        data.humidity = bme->humidity * 100;
        loggValue(String(data.humidity), "Humidity");

        data.gas = bme->gas_resistance / 10;
        loggValue(String(data.gas), "Gas");

        data.altitude = data.pressure / 100.0 > SEA_LEVEL_STANDARD ?
                        compute_altitude(data.pressure, data.temperature / 100.0, SEA_LEVEL_SPECIFIC) * 100 :
                        compute_altitude(data.pressure, data.temperature / 100.0) * 100;
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
float BMESensor::compute_altitude(float pressure, float temp_c, float seaLevel) {
    pressure /= 100.0; // Convert Pa to hPa
    return ((powf((seaLevel / pressure), (1.0f / 5.257f)) - 1.0f) * (temp_c + 273.15f)) / 0.0065f;    
}

void BMESensor::markReadError() {
    this->status.error = true;
    this->data.temperature = READ_ERROR;
    this->data.pressure = READ_ERROR;
    this->data.humidity = READ_ERROR;
    this->data.gas = READ_ERROR;
    this->data.altitude = READ_ERROR;
}