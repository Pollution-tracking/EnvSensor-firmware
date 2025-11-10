#include <Sensors/Battery.h>

#define logg(message) loggWithObj(message, "BATTERY")
#define loggValue(message, value) loggWithCtx(message, "BATTERY", value)

void Battery::init() {
    logg(INITIALIZING);

    status.found = true;
    status.error = false;
    status.initialised = true;
}

void Battery::read() {
    logg(TRIGGER_READ);

    uint16_t raw_battery = analogRead(BATTERY_PIN);
    data.voltage = raw_battery * BATTERY_MAX_VOLTAGE * BATTERY_VOLTAGE_DIVIDER / ADC_MAX_VALUE;
    data.voltage *= BATTERY_CORRECTION_FACTOR;
}

SensorStatus& Battery::getStatus() {
    return status;
}

String Battery::getName() {
    return "Battery";
}

BatteryData& Battery::getData() {
    return data;
}
