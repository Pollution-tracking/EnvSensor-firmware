#ifndef PM_SENSOR_H
#define PM_SENSOR_H

#include <Arduino.h>

#include "PMserial.h"

#include <Resources/Pins.h>
#include <Resources/Constants.h>

#include <Logger/Logger.h>

#include <Sensors/Sensor.h>

struct PMData : public SensorData {
    int32_t pm1   = READ_ERROR;
    int32_t pm2_5 = READ_ERROR;
    int32_t pm10  = READ_ERROR;

    String getData() override {
        return String(pm1) + "," +
               String(pm2_5) + "," +
               String(pm10);
    }
};

class PMSensor : public Sensor{
  public:
    PMSensor();
    ~PMSensor();
    bool sensorFound() override;
    bool sensorError() override;
    bool sensorInitialised() override;
    void read() override;
    void init() override;
    String getName() override;
    void sleep();
    void wake();
    PMData getData();
  private:
    SerialPM *pms;
    PMData data;
    bool _errorPM = false;
    bool _sensorFound = false;
    bool _initialised = false;
    bool _sleeping = false;
    void checkErrors(SerialPM::STATUS status);
    void markError();
};

#endif // PM_SENSOR_H