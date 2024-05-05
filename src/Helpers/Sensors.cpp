#include <Helpers/Sensors.hpp>

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
}

// Update sensors with fresh readings and send data to Adapter (will either write to SD card or send via Bluetooth)
void treat_CO2_sensor() {
  co2Sensor.read();
  sensorsReadAdapter.updateCO2Sensor(co2Sensor.getData());
}

void treat_PM_sensor() {
  pmSensor.read();
  sensorsReadAdapter.updatePMSensor(pmSensor.getData());
}

void treat_BME_sensor() {
  bmeSensor.read();
  sensorsReadAdapter.updateBMESensor(bmeSensor.getData());
}