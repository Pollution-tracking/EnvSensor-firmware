#include <Modules/SDcard.h>
#include <Modules/Sensors.h>
#include <Resources/Constants/compensation_constants.h>
#include <Resources/Constants/storage_constants.h>
#include <Resources/Software/CompensationService.h>
#include <cmath>

#define logg(message) loggWithObj(message, "COMPENSATION")
#define loggValue(message, value) loggWithCtx(message, "COMPENSATION", value)

// ---------------------------------------------------------------------------
// External globals – defined in main.cpp, same pattern as DataHandler.cpp
// ---------------------------------------------------------------------------
extern SDcard sdcard;

// ---------------------------------------------------------------------------
// MICS-6814 m5 polynomial correction coefficients
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// MICS-6814 m5 polynomial correction coefficients
// ---------------------------------------------------------------------------
#ifdef MICS_ENABLE
static const MICSCoefficients COMP_CO_PARAMS  = { M5_CO_C0,  M5_CO_C1,  M5_CO_C2,  M5_CO_C3,  M5_CO_C4,  M5_CO_C5  };
static const MICSCoefficients COMP_NO2_PARAMS = { M5_NO2_C0, M5_NO2_C1, M5_NO2_C2, M5_NO2_C3, M5_NO2_C4, M5_NO2_C5 };
static const MICSCoefficients COMP_NH3_PARAMS = { M5_NH3_C0, M5_NH3_C1, M5_NH3_C2, M5_NH3_C3, M5_NH3_C4, M5_NH3_C5 };
#endif

// ---------------------------------------------------------------------------
// File-scope Donchian buffer – loaded from SD once on first use, then kept
// in RAM for the lifetime of the running session.
// ---------------------------------------------------------------------------
#ifdef BME_ENABLE
static DonchianBuffer donchianBuf = { {}, {}, 0, 0, false };
static bool           donchianBufLoaded = false;
#endif

// ---------------------------------------------------------------------------
// Private helpers (file-scope statics)
// ---------------------------------------------------------------------------

float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

#ifdef BME_ENABLE
void loadDonchianFromSD() {
  logg("Loading Donchian buffer from SD");

  memset(donchianBuf.gasBuffer, 0, sizeof(donchianBuf.gasBuffer));
  memset(donchianBuf.rhBuffer,  0, sizeof(donchianBuf.rhBuffer));
  donchianBuf.head        = 0;
  donchianBuf.count       = 0;
  donchianBuf.initialised = false;

  if (!sdcard.isInitialised()) {
    logg("SD not initialised – starting with empty Donchian buffer");
    return;
  }

  File file = SD.open(DONCHIAN_FILE_PATH, FILE_READ);
  if (!file) {
    logg("Donchian file not found – starting with empty buffer");
    return;
  }

  String headerLine = file.readStringUntil('\n');
  int sepIdx = headerLine.indexOf(',');
  if (sepIdx < 0) {
    logg("Donchian file header corrupt – starting fresh");
    file.close();
    return;
  }

  uint16_t savedHead  = static_cast<uint16_t>(headerLine.substring(0, sepIdx).toInt());
  uint16_t savedCount = static_cast<uint16_t>(headerLine.substring(sepIdx + 1).toInt());

  if (savedHead >= DONCHIAN_BUFFER_SIZE || savedCount > DONCHIAN_BUFFER_SIZE) {
    logg("Donchian file values out of range – starting fresh");
    file.close();
    return;
  }

  uint16_t rowsRead = 0;
  while (file.available() && rowsRead < DONCHIAN_BUFFER_SIZE) {
    String row = file.readStringUntil('\n');
    int comma = row.indexOf(',');
    if (comma < 0)
      break;
    donchianBuf.gasBuffer[rowsRead] = row.substring(0, comma).toFloat();
    donchianBuf.rhBuffer[rowsRead] = row.substring(comma + 1).toFloat();
    rowsRead++;
  }
  file.close();

  if (rowsRead < DONCHIAN_BUFFER_SIZE) {
    logg("Donchian file incomplete – starting fresh");
    memset(donchianBuf.gasBuffer, 0, sizeof(donchianBuf.gasBuffer));
    memset(donchianBuf.rhBuffer, 0, sizeof(donchianBuf.rhBuffer));
    return;
  }

  donchianBuf.head = savedHead;
  donchianBuf.count = savedCount;
  donchianBuf.initialised = true;
  loggValue("Donchian buffer loaded, count", String(donchianBuf.count));
}

void saveDonchianToSD() {
  logg("Saving Donchian buffer to SD");

  if (!sdcard.isInitialised()) {
    logg("SD not initialised – skipping Donchian save");
    return;
  }

  File file = SD.open(DONCHIAN_FILE_PATH, FILE_WRITE, true);
  if (!file) {
    logg("Failed to open Donchian file for writing");
    return;
  }

  file.print(String(donchianBuf.head) + "," + String(donchianBuf.count) + "\n");
  for (uint16_t i = 0; i < DONCHIAN_BUFFER_SIZE; i++) {
    file.print(String(donchianBuf.gasBuffer[i], 4) + "," +
               String(donchianBuf.rhBuffer[i],  4) + "\n");
  }
  file.close();

  loggValue("Donchian buffer saved, count", String(donchianBuf.count));
}
#endif

// ---------------------------------------------------------------------------
// ALGORITHM 1 – BME680 thermodynamic self-heating RH compensation
// ---------------------------------------------------------------------------
#ifdef BME_ENABLE
float calculateCompensatedRH(float t_ambient, float rh_ambient, float t_internal) {
  float exponent = (17.625f * t_ambient) / (t_ambient + 243.04f) -
                   (17.625f * t_internal) / (t_internal + 243.04f);

  float rh_comp = rh_ambient * expf(exponent);
  loggValue("Algorithm 1 – BME RH compensated", String(rh_comp, 2));
  return clampf(rh_comp, 0.0f, 100.0f);
}
#endif

// ---------------------------------------------------------------------------
// ALGORITHM 2 – ADC → Rs (kΩ) conversion
// ---------------------------------------------------------------------------
#ifdef MICS_ENABLE
float adcToResistance(float adc_raw, float rl) {
  if (adc_raw <= 0.0f || adc_raw >= COMP_ADC_FULL) {
    loggValue("Algorithm 2: saturated ADC reading", String(adc_raw));
    return -1.0f;
  }

  float v_adc = (adc_raw / COMP_ADC_FULL) * COMP_ADC_VREF;
  float v_sensor = v_adc * (MICS_R1 + MICS_R2) / MICS_R2;

  if (v_sensor >= MICS_VCC) {
    loggValue("Algorithm 2: V_sensor exceeds VCC", String(v_sensor, 3));
    return -1.0f;
  }

  float rs_ohm = (rl * (MICS_VCC / v_sensor - 1.0f));
  loggValue("Algorithm 2 – Rs (Ohm)", String(rs_ohm, 3));
  return rs_ohm;
}

// ---------------------------------------------------------------------------
// ALGORITHM 2 – m5 polynomial correction
// ---------------------------------------------------------------------------
float compensateMICSResistance(float rs_raw, float t, float rh, const MICSCoefficients &coeffs) {
  if (rs_raw <= 0.0f) {
    return -1.0f;
  }
  float rs_kohm = rs_raw / 1000.0f;
  float rs_comp_kohm = coeffs.c0 +
                       coeffs.c1 * rs_kohm +
                       coeffs.c2 * t +
                       coeffs.c3 * rh +
                       coeffs.c4 * rs_kohm * t +
                       coeffs.c5 * rs_kohm * rh;

  float rs_comp_ohm = rs_comp_kohm * 1000.0f;
  loggValue("Algorithm 2 – m5 Rs corrected (Ohm)", String(rs_comp_ohm, 3));
  return rs_comp_ohm;
}

// ---------------------------------------------------------------------------
// ALGORITHM 2 – log-log PPM conversions
// ---------------------------------------------------------------------------
float convertToCOPPM(float rs_corrected, float r0_baseline) {
  if (r0_baseline <= 0.0f || rs_corrected <= 0.0f) {
    logg("Algorithm 2: invalid Rs or R0 for CO – returning 0");
    return 0.0f;
  }
  float ppm = powf(10.0f, PPM_CO_SLOPE * log10f(rs_corrected / r0_baseline) + PPM_CO_INTERCEPT);
  loggValue("Algorithm 2 – CO PPM", String(ppm, 3));
  return ppm;
}

float convertToNO2PPM(float rs_corrected, float r0_baseline) {
  if (r0_baseline <= 0.0f || rs_corrected <= 0.0f) {
    logg("Algorithm 2: invalid Rs or R0 for NO2 – returning 0");
    return 0.0f;
  }
  float ppm = powf(10.0f, PPM_NO2_SLOPE * log10f(rs_corrected / r0_baseline) + PPM_NO2_INTERCEPT);
  loggValue("Algorithm 2 – NO2 PPM", String(ppm, 3));
  return ppm;
}

float convertToNH3PPM(float rs_corrected, float r0_baseline) {
  if (r0_baseline <= 0.0f || rs_corrected <= 0.0f) {
    logg("Algorithm 2: invalid Rs or R0 for NH3 – returning 0");
    return 0.0f;
  }
  float ppm = powf(10.0f, PPM_NH3_SLOPE * log10f(rs_corrected / r0_baseline) + PPM_NH3_INTERCEPT);
  loggValue("Algorithm 2 – NH3 PPM", String(ppm, 3));
  return ppm;
}
#endif

// ---------------------------------------------------------------------------
// ALGORITHM 3 – Donchian VOC decoupling + AQ score
// ---------------------------------------------------------------------------
#ifdef BME_ENABLE
void updateAQScore(float r_gas_raw, float rh_shtc3, float &voc_pct_out, float &aq_score_out) {
  // Load buffer from SD on first call each session
  if (!donchianBufLoaded) {
    loadDonchianFromSD();
    donchianBufLoaded = true;
  }

  logg("Algorithm 3: updating Donchian buffer");

  donchianBuf.gasBuffer[donchianBuf.head] = r_gas_raw;
  donchianBuf.rhBuffer[donchianBuf.head] = rh_shtc3;
  donchianBuf.head = (donchianBuf.head + 1) % DONCHIAN_BUFFER_SIZE;
  if (donchianBuf.count < DONCHIAN_BUFFER_SIZE) {
    donchianBuf.count++;
  }
  donchianBuf.initialised = true;

  loggValue("Algorithm 3 – buffer count", String(donchianBuf.count));

  uint16_t validSamples = donchianBuf.count;

  float gMin = donchianBuf.gasBuffer[0];
  float gMax = donchianBuf.gasBuffer[0];
  float hMin = donchianBuf.rhBuffer[0];
  float hMax = donchianBuf.rhBuffer[0];

  for (uint16_t i = 0; i < validSamples; i++) {
    float g = donchianBuf.gasBuffer[i];
    float h = donchianBuf.rhBuffer[i];
    if (g < gMin)
      gMin = g;
    if (g > gMax)
      gMax = g;
    if (h < hMin)
      hMin = h;
    if (h > hMax)
      hMax = h;
  }

  float denominator = gMax - gMin;
  float r = (denominator == 0.0f) ? 0.0f : (hMax - hMin) / denominator;

  loggValue("Algorithm 3 – gas range", String(denominator, 1));

  float r_gas_comp = r_gas_raw - r * rh_shtc3;

  float voc_pct = 0.0f;
  if (denominator == 0.0f) {
    voc_pct = 100.0f; // 100% pure air
  } else {
    // Higher resistance = cleaner air.
    // voc_pct = 100% at gMax (clean), 0% at gMin (dirty)
    voc_pct = ((r_gas_comp - gMin) / denominator) * 100.0f;
  }

  voc_pct_out = clampf(voc_pct, 0.0f, 100.0f);
  loggValue("Algorithm 3 – VOC% decoupled", String(voc_pct_out, 2));

  float delta_rh = rh_shtc3 - AQ_HUM_OPTIMAL;
  float score_hum = 0.0f;
  if (delta_rh > 0.0f) {
    score_hum =
        ((100.0f - AQ_HUM_OPTIMAL - delta_rh) / (100.0f - AQ_HUM_OPTIMAL)) *
        AQ_HUM_MAX_SCORE;
  } else {
    score_hum =
        ((AQ_HUM_OPTIMAL + delta_rh) / AQ_HUM_OPTIMAL) * AQ_HUM_MAX_SCORE;
  }
  score_hum = clampf(score_hum, 0.0f, AQ_HUM_MAX_SCORE);

  // Gas score (75 points max)
  // Here, voc_pct_out is 100 (best) to 0 (worst).
  // So score_gas is 75 (best) to 0 (worst).
  float score_gas = (voc_pct_out / 100.0f) * AQ_GAS_MAX_SCORE;
  score_gas = clampf(score_gas, 0.0f, AQ_GAS_MAX_SCORE);

  // Both scores are higher-is-better
  aq_score_out = clampf(score_hum + score_gas, 0.0f, 100.0f);
  loggValue("Algorithm 3 – AQ score", String(aq_score_out, 2));

  saveDonchianToSD();
}
#endif

// ---------------------------------------------------------------------------
// ALGORITHM 4 – Termobaric CO2 Compensation (MH-Z19B)
// ---------------------------------------------------------------------------
#ifdef CO2_ENABLE
float compensateCO2(float co2_raw, float t_ambient_c, float p_measured_hpa) {
  if (p_measured_hpa <= 0.0f || co2_raw <= 0.0f) {
    return co2_raw;
  }
  const float P_REF = 1013.25f;
  const float T_REF_K = 298.15f; // 25 °C in Kelvin
  float t_ambient_k = t_ambient_c + 273.15f;
  
  float co2_corrected = co2_raw * ((t_ambient_k * P_REF) / (p_measured_hpa * T_REF_K));
  float result = co2_corrected < 0.0f ? 0.0f : co2_corrected;
  loggValue("Algorithm 4 – CO2 corrected (ppm)", String(result, 2));
  return result;
}
#endif

// ---------------------------------------------------------------------------
// ALGORITHM 5 – Particulate Matter Compensation (PMSA003)
// ---------------------------------------------------------------------------
#ifdef PM_ENABLE
float compensatePM1(float pm1_raw, float rh_ambient) {
  if (pm1_raw <= 0.0f) {
    return 0.0f;
  }
  float rh_dec = clampf(rh_ambient / 100.0f, 0.0f, 0.99f);
  float growth_factor = 1.0f + 0.25f * (rh_dec * rh_dec) / (1.0f - rh_dec);
  float pm1_corrected = pm1_raw / growth_factor;
  float result = pm1_corrected < 0.0f ? 0.0f : pm1_corrected;
  loggValue("Algorithm 5 – PM1 corrected", String(result, 2));
  return result;
}

float compensatePM25(float pm25_raw, float rh_ambient) {
  if (pm25_raw <= 0.0f) {
    return 0.0f;
  }

  float pm25_corrected = 0.0f;
  if (pm25_raw < 30.0f) {
    pm25_corrected = 0.524f * pm25_raw - 0.0862f * rh_ambient + 5.75f;
  } else if (pm25_raw < 50.0f) {
    float u = (pm25_raw / 20.0f) - 1.5f;
    float slope = 0.786f * u + 0.524f * (1.0f - u);
    pm25_corrected = slope * pm25_raw - 0.0862f * rh_ambient + 5.75f;
  } else if (pm25_raw < 210.0f) {
    pm25_corrected = 0.786f * pm25_raw - 0.0862f * rh_ambient + 5.75f;
  } else if (pm25_raw < 260.0f) {
    float u = (pm25_raw / 50.0f) - 4.2f;
    float slope = 0.69f * u + 0.786f * (1.0f - u);
    float rh_term = 0.0862f * rh_ambient * (1.0f - u);
    pm25_corrected = slope * pm25_raw - rh_term + 2.966f * u + 5.75f * (1.0f - u) + 0.000884f * pm25_raw * pm25_raw * u;
  } else {
    pm25_corrected = 2.966f + 0.69f * pm25_raw + 0.000884f * pm25_raw * pm25_raw;
  }

  float result = pm25_corrected < 0.0f ? 0.0f : pm25_corrected;
  loggValue("Algorithm 5 – PM2.5 corrected", String(result, 2));
  return result;
}

float compensatePM10(float pm10_raw, float rh_ambient, float p_measured_hpa) {
  if (pm10_raw <= 0.0f) {
    return 0.0f;
  }
  if (p_measured_hpa <= 0.0f) {
    return pm10_raw;
  }

  float k = 1.0f;
  if (rh_ambient > 50.0f) {
    k = (50.0f / rh_ambient) * (p_measured_hpa / 1013.25f);
  }

  float pm10_corrected = pm10_raw * k;
  float result = pm10_corrected < 0.0f ? 0.0f : pm10_corrected;
  loggValue("Algorithm 5 – PM10 corrected", String(result, 2));
  return result;
}
#endif

void runCompensationPipeline() {
  logg("Running compensation pipeline");

  float t_amb = 20.0f;
  float rh_amb = 50.0f;
  bool has_amb = false;

#if defined(SHTC3_ENABLE)
  // Ambient reference values from the isolated SHTC3 (ground truth)
  if (lastSensorsData.lastSHTC3Data.temperature != READ_ERROR &&
      lastSensorsData.lastSHTC3Data.temperature != NO_DATA) {
    t_amb = lastSensorsData.lastSHTC3Data.temperature / 100.0f;
    rh_amb = lastSensorsData.lastSHTC3Data.humidity / 100.0f;
    has_amb = true;
  }
#endif

#if defined(BME_ENABLE)
  // Fallback: if SHTC3 is not available, use BME's uncompensated data for
  // ambient
  if (!has_amb && lastSensorsData.lastBMEData.temperature != READ_ERROR &&
      lastSensorsData.lastBMEData.temperature != NO_DATA) {
    t_amb = lastSensorsData.lastBMEData.temperature / 100.0f;
    rh_amb = lastSensorsData.lastBMEData.humidity / 100.0f;
    has_amb = true;
  }

  // -------------------------------------------------------------------
  // Algorithm 1 – BME680 self-heating RH correction
  // -------------------------------------------------------------------
  logg("Algorithm 1: BME680 RH compensation");
  bool bme_ok = lastSensorsData.lastBMEData.temperature != READ_ERROR &&
                lastSensorsData.lastBMEData.temperature != NO_DATA;

  if (bme_ok) {
    // If we only have BME, t_amb == t_bme, so rh_bme_comp = rh_amb (no
    // self-heating fix possible).
    float t_bme = lastSensorsData.lastBMEData.temperature / 100.0f;
    float rh_bme_comp = calculateCompensatedRH(t_amb, rh_amb, t_bme);
    lastSensorsData.bmeCompensatedRH =
        static_cast<int32_t>(rh_bme_comp * 100.0f);
  } else {
    logg("Algorithm 1: skipped – BME data unavailable");
  }

  // -------------------------------------------------------------------
  // Algorithm 3 – Donchian VOC decoupling + AQ score (BME680 gas Ω)
  // -------------------------------------------------------------------
  logg("Algorithm 3: Donchian AQ score");
  bool gas_ok = lastSensorsData.lastBMEData.gas != READ_ERROR &&
                lastSensorsData.lastBMEData.gas != NO_DATA;

  if (gas_ok && has_amb) {
    // BME680 gas is stored as resistance / 10 (see BME_sensor.cpp)
    float r_gas_raw = lastSensorsData.lastBMEData.gas * 10.0f;
    float voc_pct = 0.0f;
    float aq_score = 0.0f;

    updateAQScore(r_gas_raw, rh_amb, voc_pct, aq_score);

    lastSensorsData.vocPctDecoupled = static_cast<int32_t>(voc_pct * 100.0f);
    lastSensorsData.aqScore = static_cast<int32_t>(aq_score * 100.0f);
  } else {
    logg("Algorithm 3: skipped – BME gas or ambient data unavailable");
  }
#endif

#if defined(MICS_ENABLE)
  // -------------------------------------------------------------------
  // Algorithm 2 – MICS-6814 m5 compensation + log-log PPM conversion
  // -------------------------------------------------------------------
  logg("Algorithm 2: MICS-6814 compensation");
  bool mics_ok = lastSensorsData.lastMICSData.co != READ_ERROR &&
                 lastSensorsData.lastMICSData.co != NO_DATA;

  if (mics_ok && has_amb) {
    const MICSCalibration &cal = micsSensor.getCalibration();

    // ADC → Rs (kΩ) per channel
    float rs_co  = adcToResistance(micsSensor.getRawADC(0), MICS_RL_CO);
    float rs_no2 = adcToResistance(micsSensor.getRawADC(1), MICS_RL_NO2);
    float rs_nh3 = adcToResistance(micsSensor.getRawADC(2), MICS_RL_NH3);

    // m5 multivariate polynomial correction
    float rs_co_comp  = compensateMICSResistance(rs_co,  t_amb, rh_amb, COMP_CO_PARAMS);
    float rs_no2_comp = compensateMICSResistance(rs_no2, t_amb, rh_amb, COMP_NO2_PARAMS);
    float rs_nh3_comp = compensateMICSResistance(rs_nh3, t_amb, rh_amb, COMP_NH3_PARAMS);

    // Log-log power-law PPM conversion using stored R0 baseline
    float co_ppm  = convertToCOPPM( rs_co_comp,  cal.r0CO_Ohm);
    float no2_ppm = convertToNO2PPM(rs_no2_comp, cal.r0NO2_Ohm);
    float nh3_ppm = convertToNH3PPM(rs_nh3_comp, cal.r0NH3_Ohm);

    // Clamp to 100,000 PPM to prevent int32_t overflow on extreme noise
    lastSensorsData.coPPMComp  = static_cast<int32_t>(clampf(co_ppm,  0.0f, 100000.0f) * 100.0f);
    lastSensorsData.no2PPMComp = static_cast<int32_t>(clampf(no2_ppm, 0.0f, 100000.0f) * 100.0f);
    lastSensorsData.nh3PPMComp = static_cast<int32_t>(clampf(nh3_ppm, 0.0f, 100000.0f) * 100.0f);
  } else {
    logg("Algorithm 2: skipped – MICS or ambient data unavailable");
  }
#endif

#if defined(CO2_ENABLE) && defined(BME_ENABLE) && defined(SHTC3_ENABLE)
  // -------------------------------------------------------------------
  // Algorithm 4 – NDIR Termobaric CO2 Compensation (MH-Z19B)
  // -------------------------------------------------------------------
  logg("Algorithm 4: MH-Z19B CO2 termobaric compensation");
  bool co2_ok = lastSensorsData.lastCO2Data.co2 != READ_ERROR &&
                lastSensorsData.lastCO2Data.co2 != NO_DATA;
  bool press_ok = lastSensorsData.lastBMEData.pressure != READ_ERROR &&
                  lastSensorsData.lastBMEData.pressure != NO_DATA;

  if (co2_ok && press_ok && has_amb) {
    float co2_raw_ppm = lastSensorsData.lastCO2Data.co2 / 100.0f;
    float p_measured_hpa = lastSensorsData.lastBMEData.pressure / 100.0f;
    
    float co2_comp_ppm = compensateCO2(co2_raw_ppm, t_amb, p_measured_hpa);
    lastSensorsData.co2Comp = static_cast<int32_t>(co2_comp_ppm * 100.0f);
  } else {
    logg("Algorithm 4: skipped – CO2, pressure or ambient data unavailable");
  }
#endif

#if defined(PM_ENABLE) && defined(SHTC3_ENABLE)
  // -------------------------------------------------------------------
  // Algorithm 5 – PMSA003 PM Compensation (Barkjohn + Malings + Polish)
  // -------------------------------------------------------------------
  logg("Algorithm 5: PMSA003 PM environmental compensation");
  bool pm_ok = lastSensorsData.lastPMData.pm2_5 != READ_ERROR &&
               lastSensorsData.lastPMData.pm2_5 != NO_DATA;

  if (pm_ok && has_amb) {
    float pm1_raw = lastSensorsData.lastPMData.pm1 / 100.0f;
    float pm25_raw = lastSensorsData.lastPMData.pm2_5 / 100.0f;
    float pm10_raw = lastSensorsData.lastPMData.pm10 / 100.0f;

    float p_measured_hpa = 1013.25f; // Default sea level pressure if BME is disabled
#ifdef BME_ENABLE
    if (lastSensorsData.lastBMEData.pressure != READ_ERROR &&
        lastSensorsData.lastBMEData.pressure != NO_DATA) {
      p_measured_hpa = lastSensorsData.lastBMEData.pressure / 100.0f;
    }
#endif

    float pm1_comp  = compensatePM1(pm1_raw, rh_amb);
    float pm25_comp = compensatePM25(pm25_raw, rh_amb);
    float pm10_comp = compensatePM10(pm10_raw, rh_amb, p_measured_hpa);

    lastSensorsData.pm1Comp  = static_cast<int32_t>(pm1_comp * 100.0f);
    lastSensorsData.pm25Comp = static_cast<int32_t>(pm25_comp * 100.0f);
    lastSensorsData.pm10Comp = static_cast<int32_t>(pm10_comp * 100.0f);
  } else {
    logg("Algorithm 5: skipped – PM or ambient data unavailable");
  }
#endif
}
