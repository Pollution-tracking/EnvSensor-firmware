#include <Helpers/Sensors.hpp>

#define logg(message) loggWithBase(message, "SENSORS")
#define loggWithContext(message, context) loggWithContext(message, context, "SENSORS")

uint8_t read_sensor = SENSORS::NO_SENSOR; // Which sensors to read?

void init_sensors() {
  // Initialize PM sensor (serial2)
  pmSensor.init();
  
  // Initialize BME sensor
  bmeSensor.init();

  // Initialize CO2 sensor (serial1)
  co2Sensor.init();
}

void handle_sensor_readings() {
  if (read_sensor == SENSORS::NO_SENSOR) {
    return;
  }
  
  if (read_sensor == SENSORS::ALL_SENSORS) {
    // Reset flag
    read_sensor = SENSORS::NO_SENSOR;
    
    // Update sensor values (BLE updates sent if connected)
    read_all_sensors();

    // Update display data and view
    display.updateSensorsStats(sensorsReadAdapter.getData());
    display.updateScreen(SCREENUPDATE::SENSORS);

    // Store data to SD card if bluetooth is not connected
    sensorsReadAdapter.storeData();
  }
}

// Force reading all sensors
void read_all_sensors() {
  treat_CO2_sensor();
  treat_PM_sensor();
  treat_BME_sensor();
  treat_Battery();
}

// Update sensors with fresh readings and send data to Adapter (will either write to SD card or send via Bluetooth)
void treat_CO2_sensor() {
  if (co2Sensor.sensorError() || !co2Sensor.sensorInitialised()) {
    recover_from_failure(&co2Sensor);
  }

  co2Sensor.read();
  sensorsReadAdapter.updateCO2Sensor(co2Sensor.getData());
}

void treat_PM_sensor() {
  if (pmSensor.sensorError() || !pmSensor.sensorInitialised()) {
    recover_from_failure(&pmSensor);
  }

  pmSensor.read();
  sensorsReadAdapter.updatePMSensor(pmSensor.getData());
}

void treat_BME_sensor() {
  if (bmeSensor.sensorError() || !bmeSensor.sensorInitialised()) {
    recover_from_failure(&bmeSensor);
  }

  bmeSensor.read();
  sensorsReadAdapter.updateBMESensor(bmeSensor.getData());
}

void treat_Battery() {
  uint16_t raw_battery = analogRead(BATTERY_PIN);
  batteryVoltage = raw_battery * BATTERY_MAX_VOLTAGE * BATTERY_VOLTAGE_DIVIDER / ADC_MAX_VALUE;
  batteryVoltage *= BATTERY_CORRECTION_FACTOR;

  loggWithContext("Voltage: " + String(batteryVoltage) + "V", "Battery");
  
  sensorsReadAdapter.updateBatteryStatus(batteryVoltage);
}

void recover_from_failure(Sensor *sensor) {
  loggWithContext("Trying to recover sensor from failure", sensor->getName());
  sensor->init();
}