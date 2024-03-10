#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <Arduino.h>
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <Display.h>
#include <CO2_sensor.h>
#include <PM_sensor.h>
#include <BME_sensor.h>

extern Display display;
extern CO2Sensor co2Sensor;
extern PMSensor pmSensor;
extern BMESensor bmeSensor;

void init_sensors();
void handle_sensor_readings();
void read_all_sensors();

void check_CO2_sensor();
void check_PM_sensor();
void check_BME_sensor();

void treat_CO2_sensor();
void treat_PM_sensor();
void treat_BME_sensor();
#endif // SENSORS_HPP