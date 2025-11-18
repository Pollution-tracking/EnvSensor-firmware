#include "Sensors/MICS_sensor.h"
#include <math.h>

#define logg(message) loggWithObj(message, "MICS")
#define loggValue(message, value) loggWithCtx(message, "MICS", value)

// MICS Calibration data stored in RTC memory
RTC_DATA_ATTR MICSCalibration micsCalibration = {
    .baseNH3 = 0,
    .baseCO = 0,
    .baseNO2 = 0,
    .isValid = false
};


// MICS6814 Calibration Constants
#define MICS_CALIBRATION_SECONDS        (10)   // Number of seconds for calibration buffer
#define MICS_CALIBRATION_DELTA          (2)    // Tolerance for stability check
#define MICS_MAX_CALIBRATION_ATTEMPTS   (30)   // Maximum calibration attempts
#define MICS_CALIBRATION_READS          (3)    // Number of reads per calibration measurement
#define MICS_OPERATIONAL_READS          (100)  // Number of reads per operation measurement

// MICS6814 Gas Type Enumeration
enum MICSGasType {
    MICS_GAS_CO = 0,
    MICS_GAS_NO2 = 1,
    MICS_GAS_NH3 = 2
};

// Construct MICS sensor
MICSSensor::MICSSensor() {
}

// Destruct MICS sensor
MICSSensor::~MICSSensor() {
}

// Routine to initialize MICS sensor
void MICSSensor::init() {
    logg(INITIALIZING);

    // Assume sensor is found by default (MICS6814 has no I2C communication)
    this->status.found = true;
    
    // Check if we have valid calibration data from RTC memory
    if (micsCalibration.isValid) {
        logg("Loading calibration from RTC memory");
        // Calibration already stored in RTC, just mark as initialized
        this->status.initialised = true;
        this->status.error = false;
        logg(INITIALIZED);
    } else {
        // No valid calibration data, perform calibration
        bool ret = calibrate();
        
        if (ret) {
            // If calibration completes successfully, mark as initialized
            this->status.initialised = true;
            this->status.error = false;
            logg(INITIALIZED);
        } else {
            // Calibration failed to stabilize
            this->status.initialised = false;
            this->status.error = true;
            logg(FAILED_READ);
        }
    }
}

// Routine to update MICS values
void MICSSensor::read() {
    logg(TRIGGER_READ);

    if (!this->status.initialised) {
        logg(NOT_INITIALIZED);
        
        // Mark errors
        this->markReadError();
        return;
    }
    
    // Measure all three gas values
    float coValue = measure(MICS_GAS_CO);
    float no2Value = measure(MICS_GAS_NO2);
    float nh3Value = measure(MICS_GAS_NH3);
    
    // Check for measurement errors
    if (coValue < 0 || no2Value < 0 || nh3Value < 0) {
        // Mark errors
        this->markReadError();
        logg(FAILED_READ);
    } else {
        // Update the MICS data
        this->status.error = false;
        
        data.co = (int32_t)coValue;
        loggValue(String(data.co), "CO");
        
        data.no2 = (int32_t)no2Value;
        loggValue(String(data.no2), "NO2");
        
        data.nh3 = (int32_t)nh3Value;
        loggValue(String(data.nh3), "NH3");
    }
}

// Routine to calibrate MICS sensor
// Work algorithm: continuously measures resistance with floating average
// If current measurement is close to average, calibration is successful
bool MICSSensor::calibrate() {
    // Measurement buffers
    uint16_t bufferNH3[MICS_CALIBRATION_SECONDS];
    uint16_t bufferCO[MICS_CALIBRATION_SECONDS];
    uint16_t bufferNO2[MICS_CALIBRATION_SECONDS];
    memset(bufferNH3, 0, sizeof(bufferNH3));
    memset(bufferCO, 0, sizeof(bufferCO));
    memset(bufferNO2, 0, sizeof(bufferNO2));

    // Pointers for the next item in the buffer
    uint8_t pntrNH3 = 0;
    uint8_t pntrCO = 0;
    uint8_t pntrNO2 = 0;

    // The current floating amount in the buffer
    uint16_t fltSumNH3 = 0;
    uint16_t fltSumCO = 0;
    uint16_t fltSumNO2 = 0;

    // Current measurement
    uint16_t curNH3;
    uint16_t curCO;
    uint16_t curNO2;

    // Flag for stability of indications
    bool isStableNH3 = false;
    bool isStableCO = false;
    bool isStableNO2 = false;
    

    // Calibrate
    unsigned long lastMeasurementTime = millis();
    uint8_t calibrationAttempts = 0;
    uint8_t maxCalibrationAttempts = MICS_MAX_CALIBRATION_ATTEMPTS;
    
    do {
        unsigned long currentTime = millis();
        unsigned long rs = 0;
        
        // Only take measurements every 1000ms
        if (currentTime - lastMeasurementTime < 1000) {
            continue;
        }
        
        lastMeasurementTime = currentTime;
        calibrationAttempts++;

        delay(50);
        for (int i = 0; i < MICS_CALIBRATION_READS; i++) {
            delay(1);
            rs += analogRead(MICS_NH3_PIN);
        }

        curNH3 = rs / MICS_CALIBRATION_READS;
        rs = 0;

        delay(50);
        for (int i = 0; i < MICS_CALIBRATION_READS; i++) {
            delay(1);
            rs += analogRead(MICS_CO_PIN);
        }

        curCO = rs / MICS_CALIBRATION_READS;
        rs = 0;

        delay(50);
        for (int i = 0; i < MICS_CALIBRATION_READS; i++) {
            delay(1);
            rs += analogRead(MICS_NO2_PIN);
        }

        curNO2 = rs / MICS_CALIBRATION_READS;

        // Update the floating amount by subtracting the value 
        // to be overwritten, and adding a new value.
        fltSumNH3 = fltSumNH3 + curNH3 - bufferNH3[pntrNH3];
        fltSumCO = fltSumCO + curCO - bufferCO[pntrCO];
        fltSumNO2 = fltSumNO2 + curNO2 - bufferNO2[pntrNO2];

        // Store in buffer new values
        bufferNH3[pntrNH3] = curNH3;
        bufferCO[pntrCO] = curCO;
        bufferNO2[pntrNO2] = curNO2;

        // Define flag states
        isStableNH3 =
            abs((int)(fltSumNH3 / MICS_CALIBRATION_SECONDS) - (int)curNH3) <
            MICS_CALIBRATION_DELTA;
        isStableCO =
            abs((int)(fltSumCO / MICS_CALIBRATION_SECONDS) - (int)curCO) <
            MICS_CALIBRATION_DELTA;
        isStableNO2 =
            abs((int)(fltSumNO2 / MICS_CALIBRATION_SECONDS) - (int)curNO2) <
            MICS_CALIBRATION_DELTA;

        // Pointer to a buffer
        pntrNH3 = (pntrNH3 + 1) % MICS_CALIBRATION_SECONDS;
        pntrCO = (pntrCO + 1) % MICS_CALIBRATION_SECONDS;
        pntrNO2 = (pntrNO2 + 1) % MICS_CALIBRATION_SECONDS;
    } while ((!isStableNH3 || !isStableCO || !isStableNO2) &&
             calibrationAttempts < maxCalibrationAttempts);
    
    // Save calibration to RTC memory if all channels stabilized
    if (isStableNH3 && isStableCO && isStableNO2) {
        micsCalibration.baseNH3 = fltSumNH3 / MICS_CALIBRATION_SECONDS;
        micsCalibration.baseCO = fltSumCO / MICS_CALIBRATION_SECONDS;
        micsCalibration.baseNO2 = fltSumNO2 / MICS_CALIBRATION_SECONDS;
        micsCalibration.isValid = true;
    }
    
    // Return true if all channels stabilized, false if max attempts reached
    return isStableNH3 && isStableCO && isStableNO2;
}

// Get current resistance for a given pin (ADC value 0-1024)
uint16_t MICSSensor::getResistance(uint8_t pin) const {
    unsigned long rs = 0;

    for (int i = 0; i < MICS_OPERATIONAL_READS; i++) {
        rs += analogRead(pin);
        delay(2);
    }

    return rs / MICS_OPERATIONAL_READS;
}

// Get current resistance for a given pin
float MICSSensor::getCurrentRatio(uint8_t pin, float baseResistance) const {
    float resistance = (float)getResistance(pin);

    return resistance / baseResistance * (1023.0 - baseResistance) / (1023.0 - resistance);
}

// Measure gas concentration in ppm (MICS_GAS_CO, MICS_GAS_NO2, MICS_GAS_NH3)
float MICSSensor::measure(uint8_t gasType) {
    float ratio;
    float c = 0;

    switch (gasType) {
    case MICS_GAS_CO:
        ratio = getCurrentRatio(MICS_CO_PIN, micsCalibration.baseCO);
        c = pow(ratio, -1.179) * 4.385;
        break;
    case MICS_GAS_NO2:
        ratio = getCurrentRatio(MICS_NO2_PIN, micsCalibration.baseNO2);
        c = pow(ratio, 1.007) / 6.855;
        break;
    case MICS_GAS_NH3:
        ratio = getCurrentRatio(MICS_NH3_PIN, micsCalibration.baseNH3);
        c = pow(ratio, -1.67) / 1.47;
        break;
    }

    return isnan(c) ? -1 : c;
}

// Getters
MICSData& MICSSensor::getData() {
    return data;
}

SensorStatus& MICSSensor::getStatus() {
    return status;
}

String MICSSensor::getName() {
    return "MICS6814";
}

// Internal functions
void MICSSensor::markReadError() {
    this->status.error = true;
    this->data.co = READ_ERROR;
    this->data.no2 = READ_ERROR;
    this->data.nh3 = READ_ERROR;
}