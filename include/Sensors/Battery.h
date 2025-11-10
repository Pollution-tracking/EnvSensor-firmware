#ifndef BATTERY_H
#define BATTERY_H

#include <Resources/Constants/battery_constants.h>

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

struct BatteryData : public SensorData {
    float voltage = NO_DATA;

    String getData() override {
        return String(voltage);
    }
};

struct BatteryStatus : public SensorStatus {
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

class Battery : public Sensor {
public:
    Battery() = default;
    ~Battery() = default;
    void read() override;
    void init() override;
    SensorStatus& getStatus() override;
    String getName() override;
    BatteryData& getData() override;
private:
    BatteryData data;
    BatteryStatus status;
};

#endif // BATTERY_H
