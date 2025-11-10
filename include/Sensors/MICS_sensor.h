#ifndef MICS_SENSOR_H
#define MICS_SENSOR_H

#include <Arduino.h>

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

struct MICSData : public SensorData {
    int32_t co  = NO_DATA;
    int32_t no2 = NO_DATA;
    int32_t nh3 = NO_DATA;

    String getData() override {
        return String(co) + "," +
               String(no2) + "," +
               String(nh3);
    }
};

struct MICSStatus : public SensorStatus {
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

class MICSSensor : public Sensor {
  public:
    MICSSensor();
    ~MICSSensor();
    SensorStatus& getStatus() override;
    void read() override;
    void init() override;
    String getName() override;
    MICSData& getData();
  private:
    MICSData data;
    MICSStatus status;
};

#endif // MICS_SENSOR_H