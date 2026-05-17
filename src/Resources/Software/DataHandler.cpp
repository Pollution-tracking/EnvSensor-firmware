#include <Resources/Constants/storage_constants.h>
#include <Resources/Software/DataHandler.h>

#define logg(message) loggWithObj(message, "HANDLER")
#define loggValue(message, value) loggWithCtx(message, "HANDLER", value)

static void transferBLE();
static void transferSDcard();
static String convertDataToCSV();
static String getCSVHeader();

static void transferBLE() {
#ifdef SHTC3_ENABLE
  bluetooth.updateTemperatureCharacteristic(lastSensorsData.lastSHTC3Data.temperature);
#elif defined(BME_ENABLE)
  bluetooth.updateTemperatureCharacteristic(lastSensorsData.lastBMEData.temperature);
#endif
#ifdef BME_ENABLE
#ifdef COMPENSATION_ENABLE
  bluetooth.updateHumidityCharacteristic(lastSensorsData.bmeCompensatedRH);
  bluetooth.updateGasCharacteristic(lastSensorsData.aqScore);
#else
  bluetooth.updateHumidityCharacteristic(lastSensorsData.lastBMEData.humidity);
  bluetooth.updateGasCharacteristic(lastSensorsData.lastBMEData.gas);
#endif
  bluetooth.updatePressureCharacteristic(lastSensorsData.lastBMEData.pressure);
  bluetooth.updateAltitudeCharacteristic(lastSensorsData.lastBMEData.altitude);
#endif
#ifdef PM_ENABLE
  bluetooth.updatePM1Characteristic(lastSensorsData.lastPMData.pm1);
  bluetooth.updatePM2_5Characteristic(lastSensorsData.lastPMData.pm2_5);
  bluetooth.updatePM10Characteristic(lastSensorsData.lastPMData.pm10);
#endif
#ifdef CO2_ENABLE
  bluetooth.updateCO2Characteristic(lastSensorsData.lastCO2Data.co2);
#endif
#ifdef MICS_ENABLE
#ifdef COMPENSATION_ENABLE
  bluetooth.updateCarbonMonoxideCharacteristic(lastSensorsData.coPPMComp);
  bluetooth.updateNitrogenDioxideCharacteristic(lastSensorsData.no2PPMComp);
  bluetooth.updateAmmoniaCharacteristic(lastSensorsData.nh3PPMComp);
#else
  bluetooth.updateCarbonMonoxideCharacteristic(lastSensorsData.lastMICSData.co);
  bluetooth.updateNitrogenDioxideCharacteristic(lastSensorsData.lastMICSData.no2);
  bluetooth.updateAmmoniaCharacteristic(lastSensorsData.lastMICSData.nh3);
#endif
#endif
  bluetooth.updateBatteryCharacteristic(lastSensorsData.lastBatteryData.voltage);
  bluetooth.updateTimestampCharacteristic(lastSensorsData.timestamp);
}

static void transferSDcard() {
  // Reinitialize SD card if needed
  if (!sdcard.isInitialised()) {
    sdcard.init();
  }

  // Convert data to CSV format
  String convertedData = convertDataToCSV();

  // Store data on SD card
  if(sdcard.isInitialised() && !sdcard.writeHistoricalData(convertedData, getCSVHeader())) {
    logg("Failed to write data to SD card");
  }
}

static String getCSVHeader() {
  String header = "Timestamp,Battery_V_x100,";
#ifdef BME_ENABLE
    header += "BME_Temp_C_x100,BME_Press_hPa_x100,BME_Hum_%_x100,BME_Gas_Ohm_x10,BME_Alt_m_x100,";
#endif
#ifdef SHTC3_ENABLE
  header += "SHTC3_Temp_C_x100,SHTC3_Hum_%_x100,";
#endif
#ifdef CO2_ENABLE
  header += "CO2_ppm,";
#endif
#ifdef PM_ENABLE
  header += "PM1_ugm3,PM25_ugm3,PM10_ugm3,";
#endif
#ifdef MICS_ENABLE
  header += "MICS_CO_Ohm,MICS_NO2_Ohm,MICS_NH3_Ohm,";
#endif
#ifdef COMPENSATION_ENABLE
#ifdef BME_ENABLE
  header += "Comp_BME_RH_%_x100,Comp_BME_AQ_Score_x100,";
#endif
#ifdef MICS_ENABLE
    header += "Comp_MICS_CO_ppm_x100,Comp_MICS_NO2_ppm_x100,Comp_MICS_NH3_ppm_x100,";
#endif
#endif

  // Remove trailing comma
  if (header.endsWith(",")) {
    header = header.substring(0, header.length() - 1);
  }

  return header;
}

static String convertDataToCSV() {
  // Keep the same order as in storage_constants.h
  String dataLine = lastSensorsData.timestamp + ",";
  dataLine += lastSensorsData.lastBatteryData.getData() + ",";
#ifdef BME_ENABLE
  dataLine += lastSensorsData.lastBMEData.getData() + ",";
#endif
#ifdef SHTC3_ENABLE
  dataLine += lastSensorsData.lastSHTC3Data.getData() + ",";
#endif
#ifdef CO2_ENABLE
  dataLine += lastSensorsData.lastCO2Data.getData() + ",";
#endif
#ifdef PM_ENABLE
  dataLine += lastSensorsData.lastPMData.getData() + ",";
#endif
#ifdef MICS_ENABLE
  dataLine += lastSensorsData.lastMICSData.getData() + ",";
#endif
#ifdef COMPENSATION_ENABLE
#ifdef BME_ENABLE
  dataLine += String(lastSensorsData.bmeCompensatedRH) + "," +
              String(lastSensorsData.aqScore) + ",";
#endif
#ifdef MICS_ENABLE
  dataLine += String(lastSensorsData.coPPMComp) + "," +
              String(lastSensorsData.no2PPMComp) + "," +
              String(lastSensorsData.nh3PPMComp) + ",";
#endif
#endif

  // Remove trailing comma
  if (dataLine.endsWith(",")) {
    dataLine = dataLine.substring(0, dataLine.length() - 1);
  }

  return dataLine;
}

static std::vector<String> decodeCSVData(String lineData) {
  int separatorIndex;
  std::vector<String> result;

  // Extract data from the string
  while ((separatorIndex = lineData.indexOf(',')) != -1) {
    result.push_back(lineData.substring(0, separatorIndex));
    lineData = lineData.substring(separatorIndex + 1);
  }

  // Add the remaining part of the data
  result.push_back(lineData);

  return result;
}

void handleLiveData() {
  // Update Bluetooth characteristics if enabled and connected
  if (bluetooth.isEnabled() && bluetooth.isConnected()) {
    logg("Transferring data via BLE");
    transferBLE();
  } else {
    logg("Storing data on SD card");
    transferSDcard();
  }
}

void handleHistoricalData() {
  // Reinitialize SD card if needed
  if (!sdcard.isInitialised()) {
    sdcard.init();
  }

  // Check if there is historical data to send
  if (sdcard.isInitialised() && !sdcard.haveHistoricalData()) {
    logg("No historical data to send");
    return;
  }

  if (!sdcard.isInitialised()) {
    logg("SD card not initialized, cannot transfer historical data");
    return;
  }

  logg("Transferring historical data via BLE");

  // Retrieve historical data from SD card
  String historicalData = sdcard.getHistoricalData();
  if (historicalData == NO_MORE_DATA) {
    logg("No more historical data available on SD card");
    return;
  }

  // Split data into lines and send via BLE
  int startIdx = 0;
  int endIdx = historicalData.indexOf('\n');
  while (endIdx != -1) {
    String rawDataLine = historicalData.substring(startIdx, endIdx);
    std::vector<String> decodedDataLine = decodeCSVData(rawDataLine);

    startIdx = endIdx + 1;
    endIdx = historicalData.indexOf('\n', startIdx);

    if (bluetooth.isEnabled() && bluetooth.isConnected()) {
      // Parse and send historical data according to storage order
      if (decodedDataLine.size() == NR_VALUES) {
                bluetooth.updateTimestampCharacteristic(decodedDataLine[TIMESTAMP_INDEX]);
                bluetooth.updateBatteryCharacteristic(decodedDataLine[BATTERY_INDEX].toFloat());
#ifdef SHTC3_ENABLE
                bluetooth.updateTemperatureCharacteristic(decodedDataLine[SHTC3_TEMPERATURE_INDEX].toInt());
#elif defined(BME_ENABLE)
                bluetooth.updateTemperatureCharacteristic(decodedDataLine[BME_TEMPERATURE_INDEX].toInt());
#endif
#ifdef BME_ENABLE
#ifdef COMPENSATION_ENABLE
                bluetooth.updateHumidityCharacteristic(decodedDataLine[COMP_BME_RH_INDEX].toInt());
                bluetooth.updateGasCharacteristic(decodedDataLine[COMP_BME_AQ_INDEX].toInt());
#else
                bluetooth.updateHumidityCharacteristic(decodedDataLine[BME_HUMIDITY_INDEX].toInt());
                bluetooth.updateGasCharacteristic(decodedDataLine[BME_GAS_INDEX].toInt());
#endif
                bluetooth.updatePressureCharacteristic(decodedDataLine[BME_PRESSURE_INDEX].toInt());
                bluetooth.updateAltitudeCharacteristic(decodedDataLine[BME_ALTITUDE_INDEX].toInt());
#endif // BME_ENABLE
#ifdef CO2_ENABLE
                bluetooth.updateCO2Characteristic(decodedDataLine[CO2_CO2_INDEX].toInt());
#endif // CO2_ENABLE
#ifdef PM_ENABLE
                bluetooth.updatePM1Characteristic(decodedDataLine[PM_PM1_INDEX].toInt());
                bluetooth.updatePM2_5Characteristic(decodedDataLine[PM_PM2_5_INDEX].toInt());
                bluetooth.updatePM10Characteristic(decodedDataLine[PM_PM10_INDEX].toInt());
#endif // PM_ENABLE
#ifdef MICS_ENABLE
#ifdef COMPENSATION_ENABLE
                bluetooth.updateCarbonMonoxideCharacteristic(decodedDataLine[COMP_MICS_CO_INDEX].toInt());
                bluetooth.updateNitrogenDioxideCharacteristic(decodedDataLine[COMP_MICS_NO2_INDEX].toInt());
                bluetooth.updateAmmoniaCharacteristic(decodedDataLine[COMP_MICS_NH3_INDEX].toInt());
#else
                bluetooth.updateCarbonMonoxideCharacteristic(decodedDataLine[MICS_CO_INDEX].toInt());
                bluetooth.updateNitrogenDioxideCharacteristic(decodedDataLine[MICS_NO2_INDEX].toInt());
                bluetooth.updateAmmoniaCharacteristic(decodedDataLine[MICS_NH3_INDEX].toInt());
#endif
#endif // MICS_ENABLE
      } else {
        logg("Invalid historical data line, skipping");
        DEBUG("Invalid data line size: " + String(decodedDataLine.size()));
      }
    }
  }
}