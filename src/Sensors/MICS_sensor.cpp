#include "Sensors/MICS_sensor.h"
#include <math.h>
#include <Resources/Constants/compensation_constants.h>

#define logg(message) loggWithObj(message, "MICS")
#define loggValue(message, value) loggWithCtx(message, "MICS", value)

// MICS Calibration data stored in RTC memory
RTC_DATA_ATTR MICSCalibration micsCalibration = {
    .baseNH3    = 0,
    .baseCO     = 0,
    .baseNO2    = 0,
    .isValid    = false,
    .r0CO_Ohm  = 0.0f,
    .r0NO2_Ohm = 0.0f,
    .r0NH3_Ohm = 0.0f
};

// Cache of the most recent raw ADC readings per channel (updated every read())
static float lastADC[3] = { 0.0f, 0.0f, 0.0f };  // [CO, NO2, NH3]

// MICS-6814 Calibration Constants
#define MICS_WARMUP_TIME_MS             (1000)   // Initial warm-up time (1 second minimum)
#define MICS_CALIBRATION_WINDOW_SIZE    (10)     // Average window size for calibration
#define MICS_CALIBRATION_SAMPLES        (3)      // Number of ADC reads per calibration sample
#define MICS_SAMPLE_INTERVAL_MS         (1000)   // Time between calibration samples
#define MICS_MAX_CALIBRATION_TIME_MS    (16000)  // Maximum calibration time (16 seconds = 5s buffer fill + 10s stability checks + 1s warm-up)

// Stability criteria - reading is stable when consecutive samples are within threshold
#define MICS_STABILITY_DELTA            (3)      // Absolute difference threshold for stability
#define MICS_STABILITY_CONSECUTIVE      (5)      // Number of consecutive stable readings required

// Operational reading parameters
#define MICS_OPERATIONAL_SAMPLES        (100)    // Samples per operational read
#define MICS_OPERATIONAL_DELAY_MS       (2)      // Milliseconds between ADC samples

// Gas concentration calculation constants (from MICS-6814 datasheet)
namespace MICSConstants {
    // CO: Rs/R0 = 1.85 * (CO ppm) ^ (-0.85)
    const float CO_EXPONENT = -1.179f;
    const float CO_MULTIPLIER = 4.385f;
    
    // NO2: Rs/R0 = 0.5 + (NO2 ppm) ^ 0.91 / 17.5
    const float NO2_EXPONENT = 1.007f;
    const float NO2_DIVISOR = 6.855f;
    
    // NH3: Rs/R0 = 0.7 * (NH3 ppm) ^ (-0.6)
    const float NH3_EXPONENT = -1.67f;
    const float NH3_DIVISOR = 1.47f;
}

// Gas channel enumeration
enum MICSGasChannel {
    CHANNEL_CO = 0,
    CHANNEL_NO2 = 1,
    CHANNEL_NH3 = 2,
    CHANNEL_COUNT = 3
};

// Construct MICS sensor
MICSSensor::MICSSensor() {
}

// Destruct MICS sensor
MICSSensor::~MICSSensor() {
}

// Initialize MICS sensor
void MICSSensor::init() {
    logg(INITIALIZING);

    // Configure sensor pins as analog inputs
    pinMode(MICS_CO_PIN, INPUT);
    pinMode(MICS_NO2_PIN, INPUT);
    pinMode(MICS_NH3_PIN, INPUT);

    // MICS-6814 has no I2C communication - assume found if pins are configured
    this->status.found = true;
    
    // Check if we have valid calibration data from RTC memory
    if (micsCalibration.isValid) {
        logg("Using stored calibration from RTC memory");
        loggValue("Base CO", String(micsCalibration.baseCO));
        loggValue("Base NO2", String(micsCalibration.baseNO2));
        loggValue("Base NH3", String(micsCalibration.baseNH3));
        
        this->status.initialised = true;
        this->status.error = false;
        logg(INITIALIZED);
    } else {
        // No valid calibration - perform calibration procedure
        logg("Starting calibration procedure...");
        
        calibrate(); // Always succeeds, uses best available baseline
        
        this->status.initialised = true;
        this->status.error = false;
        logg(INITIALIZED);
    }
}

void MICSSensor::read() {
    logg(TRIGGER_READ);

    if (!this->status.initialised) {
        logg(NOT_INITIALIZED);
        this->markReadError();
        return;
    }
    
    // Measure all three gas channels
    float coValue  = measureGasConcentration(CHANNEL_CO);
    float no2Value = measureGasConcentration(CHANNEL_NO2);
    float nh3Value = measureGasConcentration(CHANNEL_NH3);
    
    // Cache last raw ADC readings (set inside calculateResistanceRatio)
    // The cache is updated as a side-effect of measureGasConcentration via
    // the readADCAverage call inside calculateResistanceRatio.
    // We also snapshot directly here so CompensationService can retrieve them.
    lastADC[CHANNEL_CO]  = readADCAverage(MICS_CO_PIN,  MICS_OPERATIONAL_SAMPLES);
    lastADC[CHANNEL_NO2] = readADCAverage(MICS_NO2_PIN, MICS_OPERATIONAL_SAMPLES);
    lastADC[CHANNEL_NH3] = readADCAverage(MICS_NH3_PIN, MICS_OPERATIONAL_SAMPLES);
    
    // Check for measurement errors (negative values indicate error)
    if (coValue < 0 || no2Value < 0 || nh3Value < 0) {
        this->markReadError();
        logg(FAILED_READ);
        return;
    }
    
    // Update sensor data (convert to integers scaled by 100 for storage)
    this->status.error = false;
    
    data.co = coValue * 100;
    loggValue(String(data.co), "CO");
    
    data.no2 = no2Value * 100;
    loggValue(String(data.no2), "NO2");
    
    data.nh3 = nh3Value * 100;
    loggValue(String(data.nh3), "NH3");
}

MICSData& MICSSensor::getData() {
    return data;
}

SensorStatus& MICSSensor::getStatus() {
    return status;
}

String MICSSensor::getName() {
    return "MICS6814";
}

// Read ADC value from a specific pin with averaging
// Returns averaged ADC reading
float MICSSensor::readADCAverage(uint8_t pin, uint16_t numSamples) {
    uint32_t sum = 0;
    
    for (uint16_t i = 0; i < numSamples; i++) {
        sum += analogRead(pin);
        delay(MICS_OPERATIONAL_DELAY_MS);
    }
    
    return sum * 1.0f / numSamples;
}

// Read all three gas channels
// Returns array with readings [CO, NO2, NH3]
void MICSSensor::readAllChannels(uint16_t* readings, uint16_t numSamples) {
    unsigned long rs = 0;

    delay(50);
    for (uint16_t i = 0; i < numSamples; i++) {
        delay(1);
        rs += analogRead(MICS_NH3_PIN);
    }
    readings[CHANNEL_NH3] = rs / numSamples;
    rs = 0;

    delay(50);
    for (uint16_t i = 0; i < numSamples; i++) {
        delay(1);
        rs += analogRead(MICS_CO_PIN);
    }
    readings[CHANNEL_CO] = rs / numSamples;
    rs = 0;

    delay(50);
    for (uint16_t i = 0; i < numSamples; i++) {
        delay(1);
        rs += analogRead(MICS_NO2_PIN);
    }
    readings[CHANNEL_NO2] = rs / numSamples;
}

// Check if a reading is stable (within absolute threshold of average)
bool MICSSensor::isReadingStable(float current, float average) {
    return fabs(average - current) < MICS_STABILITY_DELTA;
}

// Helper to compute R0 in kΩ from baseline ADC values for log-log PPM formulas.
// Uses the same voltage-divider formula as CompensationService::adcToResistance().
static float adcToR0(uint16_t adc, float rl) {
    if (adc == 0 || adc >= static_cast<uint16_t>(COMP_ADC_FULL)) return 0.0f;
    float v_adc    = (static_cast<float>(adc) / COMP_ADC_FULL) * COMP_ADC_VREF;
    float v_sensor = v_adc * (MICS_R1 + MICS_R2) / MICS_R2;
    if (v_sensor >= MICS_VCC) return 0.0f;
    return (rl * (MICS_VCC / v_sensor - 1.0f));
}

// Calibrate MICS sensor
// Continuously measures resistance with floating average
// If current measurement is close to average, calibration is successful
// After timeout, uses best available baseline
bool MICSSensor::calibrate() {
    delay(MICS_WARMUP_TIME_MS);
    
    // Measurement buffers for circular averaging
    uint16_t bufferNH3[MICS_CALIBRATION_WINDOW_SIZE];
    uint16_t bufferCO[MICS_CALIBRATION_WINDOW_SIZE];
    uint16_t bufferNO2[MICS_CALIBRATION_WINDOW_SIZE];
    memset(bufferNH3, 0, sizeof(bufferNH3));
    memset(bufferCO, 0, sizeof(bufferCO));
    memset(bufferNO2, 0, sizeof(bufferNO2));

    // Pointers for the next item in the buffer
    uint8_t pntrNH3 = 0;
    uint8_t pntrCO = 0;
    uint8_t pntrNO2 = 0;

    // Number of valid samples currently in the buffer
    uint8_t samplesFilled = 0;

    // The current floating sum in the buffer
    uint16_t fltSumNH3 = 0;
    uint16_t fltSumCO = 0;
    uint16_t fltSumNO2 = 0;

    // Current measurement
    uint16_t curNH3;
    uint16_t curCO;
    uint16_t curNO2;

    // Consecutive stable reading counters
    uint8_t stableCountNH3 = 0;
    uint8_t stableCountCO = 0;
    uint8_t stableCountNO2 = 0;
    
    unsigned long calibrationStartTime = millis();
    unsigned long lastSampleTime = millis();

    do {
        unsigned long currentTime = millis();
        
        // Use half interval while filling buffer, full after
        unsigned long interval = (samplesFilled < MICS_CALIBRATION_WINDOW_SIZE) ? MICS_SAMPLE_INTERVAL_MS / 2 : MICS_SAMPLE_INTERVAL_MS;
        
        // Only take measurements at intervals
        if (currentTime - lastSampleTime < interval) {
            delay(10);
            continue;
        }
        lastSampleTime = currentTime;

        // Read all channels
        uint16_t readings[CHANNEL_COUNT];
        readAllChannels(readings, MICS_CALIBRATION_SAMPLES);
        
        curNH3 = readings[CHANNEL_NH3];
        curCO = readings[CHANNEL_CO];
        curNO2 = readings[CHANNEL_NO2];

        // Advance pointers to the next position in the circular buffer
        pntrNH3 = (pntrNH3 + 1) % MICS_CALIBRATION_WINDOW_SIZE;
        pntrCO = (pntrCO + 1) % MICS_CALIBRATION_WINDOW_SIZE;
        pntrNO2 = (pntrNO2 + 1) % MICS_CALIBRATION_WINDOW_SIZE;

        // Update the floating sum
        if (samplesFilled >= MICS_CALIBRATION_WINDOW_SIZE) {
            // Buffer is full, subtract old value
            fltSumNH3 = fltSumNH3 + curNH3 - bufferNH3[pntrNH3];
            fltSumCO = fltSumCO + curCO - bufferCO[pntrCO];
            fltSumNO2 = fltSumNO2 + curNO2 - bufferNO2[pntrNO2];
        } else {
            // Buffer still filling, just add
            fltSumNH3 += curNH3;
            fltSumCO += curCO;
            fltSumNO2 += curNO2;
            samplesFilled++;
        }

        // Store new values into buffer
        bufferNH3[pntrNH3] = curNH3;
        bufferCO[pntrCO] = curCO;
        bufferNO2[pntrNO2] = curNO2;

        // Check stability only after we have enough samples
        if (samplesFilled >= MICS_CALIBRATION_WINDOW_SIZE) {
            float avgNH3 = fltSumNH3 / (float)MICS_CALIBRATION_WINDOW_SIZE;
            float avgCO = fltSumCO / (float)MICS_CALIBRATION_WINDOW_SIZE;
            float avgNO2 = fltSumNO2 / (float)MICS_CALIBRATION_WINDOW_SIZE;

            // Update consecutive stable counters
            if (isReadingStable(curNH3, avgNH3)) {
                if (stableCountNH3 < MICS_STABILITY_CONSECUTIVE) {
                    stableCountNH3++;
                }
            } else {
                stableCountNH3 = 0;
            }
            
            if (isReadingStable(curCO, avgCO)) {
                if (stableCountCO < MICS_STABILITY_CONSECUTIVE) {
                    stableCountCO++;
                }
            } else {
                stableCountCO = 0;
            }
            
            if (isReadingStable(curNO2, avgNO2)) {
                if (stableCountNO2 < MICS_STABILITY_CONSECUTIVE) {
                    stableCountNO2++;
                }
            } else {
                stableCountNO2 = 0;
            }

            // Log each calibration attempt
            DEBUG("Calib: NH3=" + String(avgNH3, 1) + " [" + String(stableCountNH3) + "/" + String(MICS_STABILITY_CONSECUTIVE) + "]" +
                  " CO=" + String(avgCO, 1) + " [" + String(stableCountCO) + "/" + String(MICS_STABILITY_CONSECUTIVE) + "]" +
                  " NO2=" + String(avgNO2, 1) + " [" + String(stableCountNO2) + "/" + String(MICS_STABILITY_CONSECUTIVE) + "]");

            // All channels stable for required consecutive readings?
            if (stableCountNH3 >= MICS_STABILITY_CONSECUTIVE &&
                stableCountCO >= MICS_STABILITY_CONSECUTIVE &&
                stableCountNO2 >= MICS_STABILITY_CONSECUTIVE) {
                
                micsCalibration.baseNH3 = fltSumNH3 / MICS_CALIBRATION_WINDOW_SIZE;
                micsCalibration.baseCO  = fltSumCO  / MICS_CALIBRATION_WINDOW_SIZE;
                micsCalibration.baseNO2 = fltSumNO2 / MICS_CALIBRATION_WINDOW_SIZE;
                micsCalibration.isValid = true;

                // Compute R0 in kΩ from baseline ADC values for log-log PPM formulas.
                // Uses the same voltage-divider formula as CompensationService::adcToResistance().
                micsCalibration.r0CO_Ohm = adcToR0(micsCalibration.baseCO,  MICS_RL_CO);
                micsCalibration.r0NO2_Ohm = adcToR0(micsCalibration.baseNO2, MICS_RL_NO2);
                micsCalibration.r0NH3_Ohm = adcToR0(micsCalibration.baseNH3, MICS_RL_NH3);
                micsCalibration.isValid = true;
                
                loggValue("Baseline NH3",    String(micsCalibration.baseNH3));
                loggValue("Baseline CO",     String(micsCalibration.baseCO));
                loggValue("Baseline NO2",    String(micsCalibration.baseNO2));
                loggValue("R0 CO (Ω)", String(micsCalibration.r0CO_Ohm));
                loggValue("R0 NO2 (Ω)", String(micsCalibration.r0NO2_Ohm));
                loggValue("R0 NH3 (Ω)", String(micsCalibration.r0NH3_Ohm));
                logg("Calibration successful");
                
                return true;
            }
        }
        
        // Check if we've exceeded calibration time
        if (currentTime - calibrationStartTime >= MICS_MAX_CALIBRATION_TIME_MS) {
            break;
        }
    } while (true);

    // Timeout reached - use best values we have
    logg("Calibration timeout - using current baseline");
    
    if (samplesFilled > 0) {
        micsCalibration.baseNH3 = fltSumNH3 / samplesFilled;
        micsCalibration.baseCO  = fltSumCO  / samplesFilled;
        micsCalibration.baseNO2 = fltSumNO2 / samplesFilled;
        micsCalibration.isValid = true;

        // Compute R0 in kΩ from baseline ADC values for log-log PPM formulas.
        // Uses the same voltage-divider formula as CompensationService::adcToResistance().
        micsCalibration.r0CO_Ohm = adcToR0(micsCalibration.baseCO,  MICS_RL_CO);
        micsCalibration.r0NO2_Ohm = adcToR0(micsCalibration.baseNO2, MICS_RL_NO2);
        micsCalibration.r0NH3_Ohm = adcToR0(micsCalibration.baseNH3, MICS_RL_NH3);
        micsCalibration.isValid = true;
        
        loggValue("Baseline NH3",    String(micsCalibration.baseNH3));
        loggValue("Baseline CO",     String(micsCalibration.baseCO));
        loggValue("Baseline NO2",    String(micsCalibration.baseNO2));
        loggValue("R0 CO (Ω)", String(micsCalibration.r0CO_Ohm));
        loggValue("R0 NO2 (Ω)", String(micsCalibration.r0NO2_Ohm));
        loggValue("R0 NH3 (Ω)", String(micsCalibration.r0NH3_Ohm));
    }
    
    return true; // Return true even on timeout - we have baseline values
}

// Calculate resistance ratio for gas concentration
// Rs/R0 where Rs = sensor resistance in gas, R0 = resistance in clean air
float MICSSensor::calculateResistanceRatio(uint8_t pin, uint16_t baselineADC) {
    unsigned long rs = 0;

    for (int i = 0; i < MICS_OPERATIONAL_SAMPLES; i++) {
        rs += analogRead(pin);
        delay(MICS_OPERATIONAL_DELAY_MS);
    }

    float currentADC = rs * 1.0 / MICS_OPERATIONAL_SAMPLES;
    
    // Prevent division by zero and invalid values
    if (baselineADC == 0 || baselineADC >= ADC_MAX_VALUE) {
        DEBUG("Invalid baseline: " + String(baselineADC));
        return -1.0f;
    }
    
    if (currentADC == 0 || currentADC >= ADC_MAX_VALUE) {
        DEBUG("Invalid current ADC: " + String(currentADC, 1));
        return -1.0f;
    }
    
    // Calculate resistance ratio using voltage divider formula
    // Rs/R0 = ADC_current / ADC_baseline * (ADC_MAX - ADC_baseline) / (ADC_MAX - ADC_current)
    float ratio = currentADC / baselineADC * (ADC_MAX_VALUE - baselineADC) / (ADC_MAX_VALUE - currentADC);
    
    DEBUG("Pin " + String(pin) + ": current=" + String(currentADC, 1) + " baseline=" + String(baselineADC) + " ratio=" + String(ratio, 3));
    
    return ratio;
}

// Measure gas concentration in ppm for specified channel
// Uses empirical formulas from MICS-6814 datasheet
float MICSSensor::measureGasConcentration(uint8_t channel) {
    uint8_t pin;
    uint16_t baseline;
    float concentration = -1.0f;
    
    // Select pin and baseline based on channel
    switch (channel) {
        case CHANNEL_CO:
            pin = MICS_CO_PIN;
            baseline = micsCalibration.baseCO;
            break;
        case CHANNEL_NO2:
            pin = MICS_NO2_PIN;
            baseline = micsCalibration.baseNO2;
            break;
        case CHANNEL_NH3:
            pin = MICS_NH3_PIN;
            baseline = micsCalibration.baseNH3;
            break;
        default:
            return -1.0f; // Invalid channel
    }
    
    // Calculate resistance ratio
    float ratio = calculateResistanceRatio(pin, baseline);
    
    if (ratio < 0) {
        return -1.0f; // Error in ratio calculation
    }
    
    // Apply gas-specific conversion formula
    switch (channel) {
        case CHANNEL_CO:
            // CO: ppm = (Rs/R0)^(-1.179) * 4.385
            concentration = powf(ratio, MICSConstants::CO_EXPONENT) * MICSConstants::CO_MULTIPLIER;
            break;
            
        case CHANNEL_NO2:
            // NO2: ppm = (Rs/R0)^1.007 / 6.855
            concentration = powf(ratio, MICSConstants::NO2_EXPONENT) / MICSConstants::NO2_DIVISOR;
            break;
            
        case CHANNEL_NH3:
            // NH3: ppm = (Rs/R0)^(-1.67) / 1.47
            concentration = powf(ratio, MICSConstants::NH3_EXPONENT) / MICSConstants::NH3_DIVISOR;
            break;
    }
    
    // Sanity check - return error if result is NaN or unreasonably high
    if (isnan(concentration) || concentration > 1000.0f) {
        return -1.0f;
    }
    
    // Clamp negative values to zero (sensor can't read negative concentrations)
    return concentration < 0 ? 0.0f : concentration;
}

// Mark all sensor data as read error
void MICSSensor::markReadError() {
    this->status.error = true;
    this->data.co  = READ_ERROR;
    this->data.no2 = READ_ERROR;
    this->data.nh3 = READ_ERROR;
}

// ---------------------------------------------------------------------------
// Returns the last averaged 12-bit ADC reading for a given channel.
// channel: 0=CO, 1=NO2, 2=NH3 (use MICSGasChannel enum values)
// ---------------------------------------------------------------------------
float MICSSensor::getRawADC(uint8_t channel) const {
    if (channel >= CHANNEL_COUNT) return 0.0f;
    return lastADC[channel];
}

// ---------------------------------------------------------------------------
// Returns a const reference to the micsCalibration RTC struct (R0 baselines).
// ---------------------------------------------------------------------------
const MICSCalibration& MICSSensor::getCalibration() const {
    return micsCalibration;
}