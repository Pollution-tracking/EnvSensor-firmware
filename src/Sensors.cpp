#include <Sensors.hpp>

uint8_t read_sensor = SENSORS::NO_SENSOR; // Which sensors to read?

void init_sensors() {
    // Initialize CO2 sensor (serial1)
    co2Sensor.init();
    
    // Initialize PM sensor (serial2)
    pmSensor.init();
    
    // Initialize BME sensor
    bmeSensor.init();
}

void handle_sensor_readings() {
    if (read_sensor == SENSORS::NO_SENSOR) {
        return;
    }
    
    // Read CO2 sensor (BLE updates are sent automatically)
    check_CO2_sensor();

    // Read PM sensor (BLE updates are sent automatically)
    check_PM_sensor();

    // Read BME sensor (BLE updates are sent automatically)
    check_BME_sensor();

    // Update display
    display.updateScreen(SCREENUPDATE::SENSORS);
}

void read_all_sensors() {
    read_sensor = SENSORS::SENSOR_BME | SENSORS::SENSOR_CO2 | SENSORS::SENSOR_PM;
    handle_sensor_readings();
}

void check_CO2_sensor() {
  if (read_sensor & SENSORS::SENSOR_CO2) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_CO2;

    treat_CO2_sensor();
  }
}

void check_PM_sensor() {
  if (read_sensor & SENSORS::SENSOR_PM) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_PM;

    treat_PM_sensor();
  }
}

void check_BME_sensor() {
  if (read_sensor & SENSORS::SENSOR_BME) {
    // Clear flag
    read_sensor &= ~SENSORS::SENSOR_BME;

    treat_BME_sensor();
  }
}

void treat_CO2_sensor() {
    co2Sensor.update();
}

void treat_PM_sensor() {
    pmSensor.update();
}

void treat_BME_sensor() {
    bmeSensor.update();
}