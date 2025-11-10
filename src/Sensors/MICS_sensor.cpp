#include "Sensors/MICS_sensor.h"

#define logg(message) loggWithObj(message, "MICS")
#define loggValue(message, value) loggWithCtx(message, "MICS", value)

// Construct MICS sensor
MICSSensor::MICSSensor() {
}

// Destruct MICS sensor
MICSSensor::~MICSSensor() {
}

// Routine to initialize MICS sensor
void MICSSensor::init() {
    logg(INITIALIZING);
}

// Routine to update MICS values
void MICSSensor::read() {
    logg(TRIGGER_READ);
}

// Getters
MICSData& MICSSensor::getData() {
    return data;
}

SensorStatus& MICSSensor::getStatus() {
    return status;
}

String MICSSensor::getName() {
    return "MICS6814";
}