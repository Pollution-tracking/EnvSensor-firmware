#ifndef SHTC3_SENSOR_H
#define SHTC3_SENSOR_H

#include <Arduino.h>

#include "Adafruit_SHTC3.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

#define SHTC3_ADDRESS 0x70 // I2C address for SHTC3

struct SHTC3Data : public SensorData {
    int32_t temperature = NO_DATA;
    int32_t humidity    = NO_DATA;

    String getData() override {
        return String(temperature) + "," +
               String(humidity);
    }
};

struct SHTC3Status : public SensorStatus {
    bool isFound() override {
        return found;
    }

    bool isError() override {
        return error;
    }

    bool isInitialised() override {
        return initialised;
    }

    bool isSleeping() override {
        return sleeping;
    }

    bool found = false;
    bool error = false;
    bool initialised = false;
    bool sleeping = false;
};

class SHTC3Sensor : public Sensor {
  public:
    SHTC3Sensor();
    ~SHTC3Sensor();
    void read() override;
    void init() override;
    SensorStatus& getStatus() override;
    String getName() override;
    SHTC3Data& getData() override;
    void sleep();
    void wake();
  private:
    Adafruit_SHTC3 *shtc3;
    SHTC3Data data;
    SHTC3Status status;
    void markReadError();
};

#endif // SHTC3_SENSOR_H
