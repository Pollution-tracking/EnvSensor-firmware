#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <Arduino.h>

#include <Resources/Pins.h>
#include <Resources/Constants.h>

#include <Helpers/Timers.hpp>

#include <Logger/Logger.h>

#include <Sensors/CO2_sensor.h>
#include <Sensors/PM_sensor.h>
#include <Sensors/BME_sensor.h>
#include <Sensors/Adapter/SensorsReadAdapter.h>

#include <Modules/Display.h>

extern Display display;
extern CO2Sensor co2Sensor;
extern PMSensor pmSensor;
extern BMESensor bmeSensor;
extern SensorsReadAdapter sensorsReadAdapter;

void init_sensors();
void handle_sensor_readings();
void read_all_sensors();

void treat_CO2_sensor();
void treat_PM_sensor();
void treat_BME_sensor();
void treat_Battery();

void recover_from_failure(Sensor* sensor);

#endif // SENSORS_HPP