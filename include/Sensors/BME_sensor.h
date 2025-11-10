#ifndef BME_SENSOR_H
#define BME_SENSOR_H

#include <Arduino.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

#define SEA_LEVEL 1013.25 // Standard sea level pressure in hPa
#define BME680_ADDRESS 0x76 // I2C address for BME680
    
struct BMEData : public SensorData {
    int32_t temperature = NO_DATA;
    int32_t pressure    = NO_DATA;
    int32_t humidity    = NO_DATA;
    int32_t gas         = NO_DATA;
    int32_t altitude    = NO_DATA;

    String getData() override {
        return String(temperature) + "," +
               String(pressure) + "," +
               String(humidity) + "," +
               String(gas) + "," +
               String(altitude);
    }
};

struct BMEStatus : public SensorStatus {
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

class BMESensor : public Sensor {
  public:
    BMESensor();
    ~BMESensor();
    void read() override;
    void init() override;
    SensorStatus& getStatus() override;
    String getName() override;
    BMEData& getData() override;
  private:
    Adafruit_BME680 *bme;
    BMEData data;
    BMEStatus status;
    void markReadError();
};

#endif // BME_SENSOR_H