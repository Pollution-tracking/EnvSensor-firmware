#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include <Arduino.h>
#include "MHZ19.h"
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include "Bluetooth_module.h"


class CO2Sensor {
  public:
    CO2Sensor(Bluetooth_module *bluetoothModule);
    ~CO2Sensor();
    void update();
    void init();
    uint16_t getCO2();
    uint16_t getTemperature();
    bool errorCO2 = false;
    bool errorTemperature = false;
    bool sensorFound = false;
  private:
    Bluetooth_module *bluetoothModule;
    uint16_t co2;
    uint16_t temperature;
    HardwareSerial* mhz19Serial;
    MHZ19 mhz19;
    void updateCharacteristic();
    void checkErrors();
};

#endif // CO2_SENSOR_H