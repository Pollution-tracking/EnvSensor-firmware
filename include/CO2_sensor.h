#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include <Arduino.h>
#include "MHZ19.h"
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include "Bluetooth_module.h"
#include "Sensor.h"

class CO2Sensor : public Sensor{
  public:
    CO2Sensor(Bluetooth_module *bluetoothModule);
    ~CO2Sensor();
    bool sensorFound() override;
    bool sensorError() override;
    bool sensorInitialised() override;
    void update();
    void init();
    uint16_t getCO2();
    uint16_t getTemperature();
  private:
    Bluetooth_module *bluetoothModule;
    uint16_t co2;
    uint16_t temperature;
    HardwareSerial* mhz19Serial;
    MHZ19 mhz19;
    bool _errorCO2 = false;
    bool _errorTemperature = false;
    bool _sensorFound = false;
    bool _initialised = false;
    void updateCharacteristic();
    void checkErrors();
};

#endif // CO2_SENSOR_H