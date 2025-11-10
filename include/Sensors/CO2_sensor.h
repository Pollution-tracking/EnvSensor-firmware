#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include <Arduino.h>

#include "MHZ19.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

struct CO2Data : public SensorData {
    int32_t co2         = NO_DATA;
    int32_t temperature = NO_DATA;

    String getData() override {
        return String(co2) + "," +
               String(temperature);
    }
};

struct CO2Status : public SensorStatus {
    bool isFound() override {
        return found;
    }

    bool isError() override {
        return error;
    }

    bool isInitialised() override {
        return initialised;
    }

    bool found = false;
    bool error = false;
    bool initialised = false;
};

class CO2Sensor : public Sensor{
  public:
    CO2Sensor();
    ~CO2Sensor();
    SensorStatus& getStatus() override;
    void read() override;
    void init() override;
    String getName() override;
    CO2Data& getData();
  private:
    CO2Data data;
    MHZ19 mhz19;
    CO2Status status;
    void checkErrors();
    void markReadError();
};

#endif // CO2_SENSOR_H