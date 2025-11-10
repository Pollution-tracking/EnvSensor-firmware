#include <Modules/Sensors.h>

#define logg(message) loggWithObj(message, "SENSORS")
#define loggValue(message, value) loggWithCtx(message, "SENSORS", value)

static void initSensor(Sensor& sensor) {
    sensor.init();
}

static void recoverSensor(Sensor& sensor) {
    loggValue("Recovering sensor", sensor.getName());
    sensor.init();
}

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
        logg("Preparing");
        wakeUpSensors();
        logg("Prepared");

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
        logg("Sensors heated");
        readAllSensors();

        return SENSORS::HEATED_SENSORS;
    }

    // Individual sensor reading support to be added here
default_exit:
    return SENSORS::NO_ACTION;
}

void initializeSensors() {
    logg(INITIALIZING);
#ifdef BME_ENABLE
    initSensor(bmeSensor);
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

void readAllSensors() {
    logg(TRIGGER_READ);
#ifdef BME_ENABLE
    treatSensor(bmeSensor);
    lastSensorsData.lastBMEData = bmeSensor.getData();
#endif
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
    treatSensor(battery);
    lastSensorsData.lastBatteryData = battery.getData();
    // Update timestamp
    lastSensorsData.timestamp = rtc.getTimestamp();
    
    logg(END_READ);
}

void wakeUpSensors() {
#ifdef PM_ENABLE
    pmSensor.wake();
#endif
}

void sleepSensors() {
#ifdef PM_ENABLE
    pmSensor.sleep();
#endif
}