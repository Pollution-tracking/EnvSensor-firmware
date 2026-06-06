#ifndef COMPENSATION_CONSTANTS_H
#define COMPENSATION_CONSTANTS_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Donchian buffer configuration
// ---------------------------------------------------------------------------

// Capacity: 1200 slots = 20 minutes @ 1 Hz
#define DONCHIAN_BUFFER_SIZE 1200

// ---------------------------------------------------------------------------
// MICS-6814 hardware circuit constants
// ---------------------------------------------------------------------------

// Supply voltage fed to the MICS-6814 module (V)
const float MICS_VCC = 5.0f;

// On-board voltage-divider resistors between MICS output and ESP32 ADC (Ω)
const float MICS_R1 = 47000.0f;
const float MICS_R2 = 100000.0f;

// Load resistors per channel (Ω) - check module schematic
const float MICS_RL_CO = 56000.0f;  // RED channel (CO)
const float MICS_RL_NO2 = 56000.0f; // OX  channel (NO2)
const float MICS_RL_NH3 = 56000.0f; // NH3 channel

// ---------------------------------------------------------------------------
// ESP32-S3 ADC constants (12-bit, 3.3 V reference)
// ---------------------------------------------------------------------------
const float COMP_ADC_VREF = 3.3f;
const float COMP_ADC_FULL = 4095.0f; // 2^12 - 1

// ---------------------------------------------------------------------------
// Magnus-Tetens vapour-pressure equation coefficients
// es(T) = MAGNUS_A * exp(MAGNUS_B * T / (T + MAGNUS_C))   [hPa]
// ---------------------------------------------------------------------------
const float MAGNUS_A = 6.112f;
const float MAGNUS_B = 17.625f;
const float MAGNUS_C = 243.04f;

// ---------------------------------------------------------------------------
// m5 multivariate polynomial correction coefficients (Leidinger et al.)
// Rs_corrected = c0 + c1*Rs + c2*T + c3*RH + c4*Rs*T + c5*Rs*RH
//
// These are literature-derived PLACEHOLDERS.
// Replace c0..c5 with values from climate-chamber curve fitting (Protocol A).
// ---------------------------------------------------------------------------

// RED channel (CO)
const float M5_CO_C0 = -7.4941f;
const float M5_CO_C1 = -0.6914f;
const float M5_CO_C2 = -0.0531f;
const float M5_CO_C3 = 0.2860f;
const float M5_CO_C4 = 0.0484f;
const float M5_CO_C5 = 0.0071f;

// OX channel (NO2) - Proxy based on GM-502B from Sensors 2022 (Eq. 12)
const float M5_NO2_C0 = -48.4869f;
const float M5_NO2_C1 = 0.0135f;
const float M5_NO2_C2 = 2.5624f;
const float M5_NO2_C3 = 0.9459f;
const float M5_NO2_C4 = 0.0251f;
const float M5_NO2_C5 = 0.0027f;

// NH3 channel (MICS-6814) - Real experimental coefficients from Sensors 2022, Eq. 13
const float M5_NH3_C0 = 10.2243f;
const float M5_NH3_C1 = 0.7495f;
const float M5_NH3_C2 = -0.1953f;
const float M5_NH3_C3 = 0.0672f;
const float M5_NH3_C4 = 0.0016f;
const float M5_NH3_C5 = 0.0081f;

// ---------------------------------------------------------------------------
// Log-log PPM curve constants (datasheet sensitivity curves)
// PPM = 10 ^ (SLOPE * log10(Rs/R0) + INTERCEPT)
// ---------------------------------------------------------------------------

// CO (RED channel): PPM = 10^(-1.25 * log10(Rs/R0) + 0.64)
const float PPM_CO_SLOPE = -1.25f;
const float PPM_CO_INTERCEPT = 0.64f;

// NO2 (OX channel): PPM = 10^(log10(Rs/R0) - 0.8129)
const float PPM_NO2_SLOPE = 1.0f;
const float PPM_NO2_INTERCEPT = -0.8129f;

// NH3 channel: PPM = 10^(-1.8 * log10(Rs/R0) - 0.163)
const float PPM_NH3_SLOPE = -1.8f;
const float PPM_NH3_INTERCEPT = -0.163f;

// ---------------------------------------------------------------------------
// AQ score weighting constants
// ---------------------------------------------------------------------------

// Optimal ambient relative humidity for indoor air quality (%)
const float AQ_HUM_OPTIMAL = 40.0f;

// Contribution of humidity deviation to the AQ score (out of 25 points)
const float AQ_HUM_MAX_SCORE = 25.0f;

// Contribution of VOC gas reading to the AQ score (out of 75 points)
const float AQ_GAS_MAX_SCORE = 75.0f;

#endif // COMPENSATION_CONSTANTS_H
