#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor {
    public:
        virtual bool sensorFound() = 0;
        virtual bool sensorError() = 0;
        virtual bool sensorInitialised() = 0;
        virtual void read() = 0;
        virtual void init() = 0;
        virtual String getName() = 0;
};

class SensorData {
    public:
        virtual String getData() = 0;
};

#endif // SENSOR_H