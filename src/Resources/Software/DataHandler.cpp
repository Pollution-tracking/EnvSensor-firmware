#include <Resources/Software/DataHandler.h>

#define logg(message) loggWithObj(message, "HANDLER")
#define loggValue(message, value) loggWithCtx(message, "HANDLER", value)

static void transferBLE();
static void transferSDcard();
static String convertDataToCSV();

static void transferBLE() {
#ifdef BME_ENABLE
    bluetooth.updateTemperatureCharacteristic(lastSensorsData.lastBMEData.temperature);
    bluetooth.updateHumidityCharacteristic(lastSensorsData.lastBMEData.humidity);
    bluetooth.updatePressureCharacteristic(lastSensorsData.lastBMEData.pressure);
    bluetooth.updateGasCharacteristic(lastSensorsData.lastBMEData.gas);
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
    bluetooth.updateCarbonMonoxideCharacteristic(lastSensorsData.lastMICSData.co);
    bluetooth.updateNitrogenDioxideCharacteristic(lastSensorsData.lastMICSData.no2);
    bluetooth.updateAmmoniaCharacteristic(lastSensorsData.lastMICSData.nh3);
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
    if(!sdcard.writeHistoricalData(convertedData)) {
        logg("Failed to write data to SD card");
    }
}

static String convertDataToCSV() {
    // Keep the same order as in storage_constants.h
    String dataLine = lastSensorsData.timestamp + ",";
    dataLine += lastSensorsData.lastBatteryData.getData() + ",";
#ifdef BME_ENABLE
    dataLine += lastSensorsData.lastBMEData.getData() + ",";
#endif
#ifdef CO2_ENABLE
    dataLine += lastSensorsData.lastCO2Data.getData() + ",";
#endif
#ifdef PM_ENABLE
    dataLine += lastSensorsData.lastPMData.getData() + ",";
#endif
#ifdef MICS_ENABLE
    dataLine += lastSensorsData.lastMICSData.getData();
#endif
    
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
    if (!sdcard.haveHistoricalData()) {
        logg("No historical data to send");
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
#ifdef BME_ENABLE
            bluetooth.updateTemperatureCharacteristic(lastSensorsData.lastBMEData.temperature);
            bluetooth.updateHumidityCharacteristic(lastSensorsData.lastBMEData.humidity);
            bluetooth.updatePressureCharacteristic(lastSensorsData.lastBMEData.pressure);
            bluetooth.updateGasCharacteristic(lastSensorsData.lastBMEData.gas);
            bluetooth.updateAltitudeCharacteristic(lastSensorsData.lastBMEData.altitude);
#endif // BME_ENABLE
#ifdef PM_ENABLE
            bluetooth.updatePM1Characteristic(lastSensorsData.lastPMData.pm1);
            bluetooth.updatePM2_5Characteristic(lastSensorsData.lastPMData.pm2_5);
            bluetooth.updatePM10Characteristic(lastSensorsData.lastPMData.pm10);
#endif // PM_ENABLE
#ifdef CO2_ENABLE
            bluetooth.updateCO2Characteristic(lastSensorsData.lastCO2Data.co2);
#endif // CO2_ENABLE
#ifdef MICS_ENABLE
            bluetooth.updateCarbonMonoxideCharacteristic(lastSensorsData.lastMICSData.co);
            bluetooth.updateNitrogenDioxideCharacteristic(lastSensorsData.lastMICSData.no2);
            bluetooth.updateAmmoniaCharacteristic(lastSensorsData.lastMICSData.nh3);
#endif // MICS_ENABLE
            bluetooth.updateBatteryCharacteristic(lastSensorsData.lastBatteryData.voltage);
            bluetooth.updateTimestampCharacteristic(lastSensorsData.timestamp);
        }
    }
}