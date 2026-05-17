// Sensors module: grouped helpers for initializing, preparing, reading and
// sleeping sensors. This file provides higher-level operations used by the
// scheduler and sensor worker so callers don't need to interact with each
// driver individually.

#include <Modules/Sensors.h>

#define logg(message) loggWithObj(message, "SENSORS")
#define loggValue(message, value) loggWithCtx(message, "SENSORS", value)

#define RECOVERING_SENSOR_MSG "Recovering sensor"
#define PREPARING_SENSORS_MSG "Preparing sensors"
#define PREPARED_SENSORS_MSG "Prepared sensors"
#define HEATED_SENSORS_MSG "Sensors heated"
#define READING_POLLUTION_SENSORS_MSG "Reading pollution sensors"
#define READING_ENVIRONMENTAL_SENSORS_MSG "Reading environmental sensors"

// Initialize a single sensor instance via its driver interface.
static void initSensor(Sensor& sensor) {
    sensor.init();
}

// Attempt to recover a sensor by reinitializing it and logging the attempt.
static void recoverSensor(Sensor& sensor) {
    loggValue(RECOVERING_SENSOR_MSG, sensor.getName());
    sensor.init();
}

// Treat sensor: validate status and perform a read. Recovery is attempted
// when the sensor reports an error or hasn't been initialized.
static void treatSensor(Sensor& sensor) {
    // Check if sensor has errors
    SensorStatus& status = sensor.getStatus();
    if (status.isError() || !status.isInitialised()) {
        recoverSensor(sensor);
    }

    // Read sensor data
    sensor.read();
}


// Main loop handler for sensor events (returns completed action)
SENSORS handleSensorsState(const SENSORS state) {
    // Early exit if no action is required
    if (state == SENSORS::NO_ACTION) {
        goto default_exit;
    }

    // Initialize sensors
    if (state == SENSORS::INIT_SENSORS) {
        boardUtilities.enableBuck();
        initializeSensors();

        return SENSORS::INIT_SENSORS;
    }

    // Prepare sensors for reading
    if (state == SENSORS::PREPARE_SENSORS) {
        logg(PREPARING_SENSORS_MSG);
        wakeUpSensors();
        logg(PREPARED_SENSORS_MSG);

        return SENSORS::PREPARE_SENSORS;
    }

    // Read sensors
    if (state == SENSORS::READ_SENSORS) {
        readAllSensors();
        sleepSensors();

        return SENSORS::READ_SENSORS;
    }

    // Sensors are heated and ready => perform a reading, main loop
    // will take care of the rest (start looping timer, change screen)
    if (state == SENSORS::HEATED_SENSORS) {
        logg(HEATED_SENSORS_MSG);
        readAllSensors();

        return SENSORS::HEATED_SENSORS;
    }

    // Individual sensor reading support to be added here
default_exit:
    return SENSORS::NO_ACTION;
}

// Initialize all enabled drivers and the battery monitor.
void initializeSensors() {
    logg(INITIALIZING);
#ifdef BME_ENABLE
    initSensor(bmeSensor);
#endif
#ifdef SHTC3_ENABLE
    initSensor(shtc3Sensor);
#endif
#ifdef PM_ENABLE
    initSensor(pmSensor);
#endif
#ifdef CO2_ENABLE
    initSensor(co2Sensor);
#endif
#ifdef MICS_ENABLE
    initSensor(micsSensor);
#endif
    initSensor(battery);
    logg(INITIALIZED);
}

// Read the pollution sensor group and update the shared lastSensorsData.
void readPollutionSensors() {
    logg(READING_POLLUTION_SENSORS_MSG);
#ifdef PM_ENABLE
    treatSensor(pmSensor);
    lastSensorsData.lastPMData = pmSensor.getData();
#endif
#ifdef CO2_ENABLE
    treatSensor(co2Sensor);
    lastSensorsData.lastCO2Data = co2Sensor.getData();
#endif
#ifdef MICS_ENABLE
    treatSensor(micsSensor);
    lastSensorsData.lastMICSData = micsSensor.getData();
#endif
    lastSensorsData.timestamp = rtc.getTimestamp();
    logg(END_READ);
}

// Read the environmental group (temperature/humidity/etc.) and battery.
void readEnvironmentalSensors() {
    logg(READING_ENVIRONMENTAL_SENSORS_MSG);
#ifdef BME_ENABLE
    treatSensor(bmeSensor);
    lastSensorsData.lastBMEData = bmeSensor.getData();
#endif
#ifdef SHTC3_ENABLE
    treatSensor(shtc3Sensor);
    lastSensorsData.lastSHTC3Data = shtc3Sensor.getData();
#endif
    treatSensor(battery);
    lastSensorsData.lastBatteryData = battery.getData();
    lastSensorsData.timestamp = rtc.getTimestamp();
    logg(END_READ);
}

// Trigger a full read cycle: environmental first (temperature source), then pollution.
void readAllSensors() {
    logg(TRIGGER_READ);
    readEnvironmentalSensors();
    readPollutionSensors();
}

// Group-level prepare/sleep helpers used by the scheduler worker.
void preparePollutionSensors() {
#ifdef PM_ENABLE
    pmSensor.wake();
#endif
}

void prepareEnvironmentalSensors() {
#ifdef SHTC3_ENABLE
    shtc3Sensor.wake();
#endif
}

void sleepPollutionSensors() {
#ifdef PM_ENABLE
    pmSensor.sleep();
#endif
}

void sleepEnvironmentalSensors() {
#ifdef SHTC3_ENABLE
    shtc3Sensor.sleep();
#endif
}

void wakeUpSensors() {
#ifdef PM_ENABLE
    pmSensor.wake();
#endif
#ifdef SHTC3_ENABLE
    shtc3Sensor.wake();
#endif
}

void sleepSensors() {
#ifdef PM_ENABLE
    pmSensor.sleep();
#endif
#ifdef SHTC3_ENABLE
    shtc3Sensor.sleep();
#endif
}