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
    uint16_t baseNH3;      // ADC baseline (clean air) – used for ratio calculation
    uint16_t baseCO;
    uint16_t baseNO2;
    bool isValid;
    // R0 in Ω – reconstructed from baseline ADC, used for log-log PPM formulas
    float r0CO_Ohm;       // R0 for CO (RED) channel
    float r0NO2_Ohm;      // R0 for NO2 (OX) channel
    float r0NH3_Ohm;      // R0 for NH3 channel
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

    // Returns raw 12-bit ADC reading for a specific channel (used by CompensationService)
    // channel: 0=CO, 1=NO2, 2=NH3
    float getRawADC(uint8_t channel) const;

    // Returns the current calibration data (R0 baselines) for this sensor
    const MICSCalibration& getCalibration() const;
    
  private:
    MICSData data;
    MICSStatus status;
    
    // Calibration
    bool calibrate();
    
    // ADC reading helpers
    float readADCAverage(uint8_t pin, uint16_t numSamples);
    void readAllChannels(uint16_t* readings, uint16_t numSamples);
    bool isReadingStable(float current, float average);
    
    // Gas concentration measurement
    float calculateResistanceRatio(uint8_t pin, uint16_t baselineADC);
    float measureGasConcentration(uint8_t channel);
    
    // Error handling
    void markReadError();
};

#endif // MICS_SENSOR_H