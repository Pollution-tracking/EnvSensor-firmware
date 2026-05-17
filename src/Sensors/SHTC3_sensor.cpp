#include "Sensors/SHTC3_sensor.h"

#define logg(message) loggWithObj(message, "SHTC3")
#define loggValue(message, value) loggWithCtx(message, "SHTC3", value)

// Construct SHTC3 sensor
SHTC3Sensor::SHTC3Sensor() {
    shtc3 = new Adafruit_SHTC3();
}

// Destruct SHTC3 sensor
SHTC3Sensor::~SHTC3Sensor() {
    delete shtc3;
}

// Routine to initialize SHTC3 sensor
void SHTC3Sensor::init() {
    logg(INITIALIZING);

    bool ret = shtc3->begin(&Wire);

    // Wait for sensor to start
    delay(100);

    // Check if the sensor is found
    if (ret) {
        logg(INITIALIZED);

        this->status.found = true;
        this->status.initialised = true;
        this->status.error = false;
        this->status.sleeping = false;

        // Perform a reset to calibrate
        shtc3->reset();
        delay(100);

        // Set to low power mode by default
        shtc3->lowPowerMode(true);
    } else {
        logg(NOT_INITIALIZED);

        this->status.found = false;
        this->status.initialised = false;
        this->status.error = true;
        this->status.sleeping = false;
    }
}

// Routine to update SHTC3 values
void SHTC3Sensor::read() {
    logg(TRIGGER_READ);

    if (!this->status.initialised) {
        logg(NOT_INITIALIZED);

        // Mark errors
        this->markReadError();
        return;
    }

    sensors_event_t humidity_event, temp_event;
    bool ret = shtc3->getEvent(&humidity_event, &temp_event);

    if (!ret) {
        // Mark errors
        this->markReadError();

        // Log the error
        logg(FAILED_READ);
    } else {
        // Update the SHTC3 data
        this->status.error = false;

        data.temperature = temp_event.temperature * 100;
        loggValue(String(data.temperature), "Temperature");

        data.humidity = humidity_event.relative_humidity * 100;
        loggValue(String(data.humidity), "Humidity");
    }
}

// Getters
SHTC3Data& SHTC3Sensor::getData() {
    return data;
}

SensorStatus& SHTC3Sensor::getStatus() {
    return status;
}

String SHTC3Sensor::getName() {
    return "SHTC3";
}

// Working cycle functions
void SHTC3Sensor::sleep() {
    // Early exit if already sleeping
    if (this->status.isSleeping()) {
        logg("Already sleeping");
        return;
    }

    shtc3->sleep(true);

    // Wait for the sensor to sleep
    delay(1);

    // Update status
    this->status.sleeping = true;
    logg("Sleeping");
}

void SHTC3Sensor::wake() {
    // Early exit if already awake
    if (!this->status.isSleeping()) {
        logg("Already awake");
        return;
    }

    shtc3->sleep(false);

    // Wait for the sensor to wake up
    delay(1);

    // Update status
    this->status.sleeping = false;
    logg("Awake");
}

// Internal functions
void SHTC3Sensor::markReadError() {
    this->status.error = true;
    this->data.temperature = READ_ERROR;
    this->data.humidity = READ_ERROR;
}
