#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include <Arduino.h>

#include "MHZ19.h"

#include <Resources/Pins.h>
#include <Resources/Constants.h>

#include <Logger/Logger.h>

#include <Sensors/Sensor.h>

struct CO2Data : public SensorData {
    int32_t co2         = READ_ERROR;
    int32_t temperature = READ_ERROR;

    String getData() override {
        return String(co2) + "," +
               String(temperature);
    }
};

class CO2Sensor : public Sensor{
  public:
    CO2Sensor();
    ~CO2Sensor();
    bool sensorFound() override;
    bool sensorError() override;
    bool sensorInitialised() override;
    void read() override;
    void init() override;
    String getName() override;
    CO2Data getData();
  private:
    CO2Data data;
    MHZ19 mhz19;
    bool _errorCO2 = false;
    bool _errorTemperature = false;
    bool _sensorFound = false;
    bool _initialised = false;
    void checkErrors();
    void markError();
};

#endif // CO2_SENSOR_H