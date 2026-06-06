#ifndef COMPENSATION_SERVICE_H
#define COMPENSATION_SERVICE_H

#include <Arduino.h>
#include <Resources/Constants/compensation_constants.h>

// ---------------------------------------------------------------------------
// Structures
// ---------------------------------------------------------------------------
struct MICSCoefficients {
  float c0, c1, c2, c3, c4, c5;
};

struct DonchianBuffer {
  float    gasBuffer[DONCHIAN_BUFFER_SIZE];
  float    rhBuffer[DONCHIAN_BUFFER_SIZE];
  uint16_t head;
  uint16_t count;
  bool     initialised;
};

// ---------------------------------------------------------------------------
// Method Pre-declarations
// ---------------------------------------------------------------------------
float clampf(float v, float lo, float hi);
#ifdef BME_ENABLE
void loadDonchianFromSD();
void saveDonchianToSD();
float calculateCompensatedRH(float t_ambient, float rh_ambient, float t_internal);
void updateAQScore(float r_gas_raw, float rh_shtc3, float& voc_pct_out, float& aq_score_out);
#endif

#ifdef MICS_ENABLE
float adcToResistance(float adc_raw, float rl);
float compensateMICSResistance(float rs_raw, float t, float rh, const MICSCoefficients& coeffs);
float convertToCOPPM(float rs_corrected, float r0_baseline);
float convertToNO2PPM(float rs_corrected, float r0_baseline);
float convertToNH3PPM(float rs_corrected, float r0_baseline);
#endif

#ifdef CO2_ENABLE
float compensateCO2(float co2_raw, float t_ambient_c, float p_measured_hpa);
#endif

#ifdef PM_ENABLE
float compensatePM1(float pm1_raw, float rh_ambient);
float compensatePM25(float pm25_raw, float rh_ambient);
float compensatePM10(float pm10_raw, float rh_ambient, float p_measured_hpa);
#endif

// ---------------------------------------------------------------------------
// Compensation Service
// ---------------------------------------------------------------------------
// Encapsulates all environmental compensation algorithms:
// 1. BME680 thermodynamic self-heating RH compensation
// 2. MICS-6814 m5 multivariate polynomial correction & log-log PPM conversion
// 3. Donchian-channel VOC decoupling & Air Quality score
//
// All processing uses `lastSensorsData` directly and handles its own state
// and SD card persistence internally.
// ---------------------------------------------------------------------------
void runCompensationPipeline();

#endif // COMPENSATION_SERVICE_H
