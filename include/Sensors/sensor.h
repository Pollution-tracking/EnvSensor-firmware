#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

#include <Resources/Constants/sensor_constants.h>
#include <Resources/Constants/logging_constants.h>

class SensorData {
    public:
        virtual String getData() = 0;
};

class SensorStatus {
    public:
        virtual bool isFound() = 0;
        virtual bool isError() = 0;
        virtual bool isInitialised() = 0;
        virtual bool isSleeping() { return false; } // Default implementation, can be overridden
};

class Sensor {
    public:
        virtual void read() = 0;
        virtual void init() = 0;
        virtual String getName() = 0;
        virtual SensorStatus& getStatus() = 0;
        virtual SensorData& getData() = 0;
};

#endif // SENSOR_H