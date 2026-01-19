#ifndef MICS_SENSOR_H
#define MICS_SENSOR_H

#include <Arduino.h>

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>
#include <configs.h>
#include <Resources/Constants/adc_constants.h>

// MICS Calibration data stored in RTC memory
typedef struct {
    uint16_t baseNH3;
    uint16_t baseCO;
    uint16_t baseNO2;
    bool isValid;
} MICSCalibration;

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
    
    // Public interface (inherited from Sensor)
    SensorStatus& getStatus() override;
    void read() override;
    void init() override;
    String getName() override;
    MICSData& getData();
    
  private:
    MICSData data;
    MICSStatus status;
    
    // Calibration
    bool calibrate();
    
    // ADC reading helpers
    uint16_t readADCAverage(uint8_t pin, uint16_t numSamples);
    void readAllChannels(uint16_t* readings, uint16_t numSamples);
    bool isReadingStable(float current, float average);
    
    // Gas concentration measurement
    float calculateResistanceRatio(uint8_t pin, uint16_t baselineADC);
    float measureGasConcentration(uint8_t channel);
    
    // Error handling
    void markReadError();
};

#endif // MICS_SENSOR_H