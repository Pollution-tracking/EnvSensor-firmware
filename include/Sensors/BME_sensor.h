#ifndef BME_SENSOR_H
#define BME_SENSOR_H

#include <Arduino.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_BME680.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>

#define SEA_LEVEL_STANDARD 1013.25 // Standard sea level pressure in hPa
#define SEA_LEVEL_SPECIFIC 1017.25 // Romania average sea level pressure in hPa
#define STANDARD_TEMPERATURE_C 20.0 // Standard temperature in Celsius
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
    float compute_altitude(float pressure, float temp_c = STANDARD_TEMPERATURE_C, float seaLevel = SEA_LEVEL_STANDARD);
    void markReadError();
};

#endif // BME_SENSOR_H