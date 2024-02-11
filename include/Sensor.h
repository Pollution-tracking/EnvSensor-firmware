#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor {
    public:
        virtual bool sensorFound() = 0;
        virtual bool sensorError() = 0;
};

#endif // SENSOR_H