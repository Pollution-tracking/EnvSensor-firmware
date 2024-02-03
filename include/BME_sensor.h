#ifndef BME_SENSOR_H
#define BME_SENSOR_H

#include <Arduino.h>
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Bluetooth_module.h"

class BMESensor {
  public:
    BMESensor(Bluetooth_module *bluetoothModule);
    ~BMESensor();
    void update();
    void init();
    uint32_t getTemperature();
    uint32_t getPressure();
    uint32_t getHumidity();
    uint32_t getGas();
    float getAltitude();
    bool errorBME = false;
    bool sensorFound = false;
  private:
    Bluetooth_module *bluetoothModule;
    Adafruit_BME680 *bme;
    BMEData data;
    void updateCharacteristics();
    void checkErrors(bool status);
};

#endif // BME_SENSOR_H