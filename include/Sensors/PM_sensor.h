#ifndef PM_SENSOR_H
#define PM_SENSOR_H

#include <Arduino.h>

#include "PMserial.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

const char name[] = "PMSA003\0";

struct PMData : public SensorData {
    int32_t pm1   = NO_DATA;
    int32_t pm2_5 = NO_DATA;
    int32_t pm10  = NO_DATA;

    String getData() override {
        return String(pm1) + "," +
               String(pm2_5) + "," +
               String(pm10);
    }
};

struct PMStatus : public SensorStatus {
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

class PMSensor : public Sensor{
  public:
    PMSensor();
    ~PMSensor();
    void read() override;
    void init() override;
    String getName() override;
    SensorStatus& getStatus() override;
    PMData& getData() override;
    void sleep();
    void wake();
  private:
    SerialPM *pms;
    PMData data;
    PMStatus status;
    void checkErrors(SerialPM::STATUS status);
    void markReadError();
};

#endif // PM_SENSOR_H