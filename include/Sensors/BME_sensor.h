#ifndef BME_SENSOR_H
#define BME_SENSOR_H

#include <Arduino.h>

#include "Wire.h"
#include "SPI.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"

#include <Resources/Pins.h>
#include <Resources/Constants.h>

#include <Logger/Logger.h>

#include <Sensors/Sensor.h>

struct BMEData : public SensorData {
    int32_t temperature = READ_ERROR;
    int32_t pressure    = READ_ERROR;
    int32_t humidity    = READ_ERROR;
    int32_t gas         = READ_ERROR;
    int32_t altitude    = READ_ERROR;

    String getData() override {
        return String(temperature) + "," +
               String(pressure) + "," +
               String(humidity) + "," +
               String(gas) + "," +
               String(altitude);
    }
};

class BMESensor : public Sensor {
  public:
    BMESensor();
    ~BMESensor();
    bool sensorFound() override;
    bool sensorError() override;
    bool sensorInitialised() override;
    void read() override;
    void init() override;
    String getName() override;
    BMEData getData();
  private:
    Adafruit_BME680 *bme;
    BMEData data;
    bool _errorBME = false;
    bool _sensorFound = false;
    bool _initialised = false;
    void checkErrors(bool status);
    void markError();
};

#endif // BME_SENSOR_H