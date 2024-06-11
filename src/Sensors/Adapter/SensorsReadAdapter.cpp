#include <Sensors/Adapter/SensorsReadAdapter.h>

#define logg(message) loggWithBase(message, "ADAPTER")
#define loggWithContext(message, context) loggWithContext(message, context, "ADAPTER")

SensorsReadAdapter::SensorsReadAdapter(Bluetooth_module *bluetoothModule, SDcard *sdcard, RTC *rtc) {
    this->bluetoothModule = bluetoothModule;
    this->sdcard = sdcard;
    this->rtc = rtc;
}

SensorsReadAdapter::~SensorsReadAdapter() { }

void SensorsReadAdapter::init() {
    this->prepareCSVData();
    this->sdcard->init();
    this->bluetoothModule->init();
    if (!this->rtc->isInitialised()) {
        this->rtc->init();
    }

    logg("Initialized");
}

void SensorsReadAdapter::updateCO2Sensor(CO2Data co2data) {
    loggWithContext("Handling data", "CO2");

    // Update the CO2 sensor data
    this->co2data = co2data;

    // Update the CSV data
    csvData[CO2_CO2_INDEX] = String(this->co2data.co2);

    // Update characteristics
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        bluetoothModule->updateCO2Characteristic(this->co2data.co2);
    }
}

void SensorsReadAdapter::updateBMESensor(BMEData bmedata) {
    loggWithContext("Handling data", "BME");

    // Update the BME sensor data
    this->bmedata = bmedata;

    // Update the CSV data
    csvData[BME_TEMPERATURE_INDEX] = String(this->bmedata.temperature);
    csvData[BME_HUMIDITY_INDEX] = String(this->bmedata.humidity);
    csvData[BME_PRESSURE_INDEX] = String(this->bmedata.pressure);
    csvData[BME_GAS_INDEX] = String(this->bmedata.gas);
    csvData[BME_ALTITUDE_INDEX] = String(this->bmedata.altitude);

    // Update characteristics
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        bluetoothModule->updateTemperatureCharacteristic(this->bmedata.temperature);
        bluetoothModule->updateHumidityCharacteristic(this->bmedata.humidity);
        bluetoothModule->updatePressureCharacteristic(this->bmedata.pressure);
        bluetoothModule->updateGasCharacteristic(this->bmedata.gas);
        bluetoothModule->updateAltitudeCharacteristic(this->bmedata.altitude);
    }
}

void SensorsReadAdapter::updatePMSensor(PMData pmdata) {
    loggWithContext("Handling data", "PM");

    // Update the PM sensor data
    this->pmdata = pmdata;

    // Update the CSV data
    csvData[PM_PM1_INDEX] = String(this->pmdata.pm1);
    csvData[PM_PM2_5_INDEX] = String(this->pmdata.pm2_5);
    csvData[PM_PM10_INDEX] = String(this->pmdata.pm10);

    // Update characteristics
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        bluetoothModule->updatePM1Characteristic(this->pmdata.pm1);
        bluetoothModule->updatePM2_5Characteristic(this->pmdata.pm2_5);
        bluetoothModule->updatePM10Characteristic(this->pmdata.pm10);
    }
}

void SensorsReadAdapter::updateBatteryStatus(float voltage) {
    loggWithContext("Handling data", "Battery");

    // Update the CSV data
    csvData[BATTERY_INDEX] = String(voltage);

    // Update characteristics
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        bluetoothModule->updateBatteryCharacteristic((int)(voltage * 100));
    }
}

void SensorsReadAdapter::storeData() {
    // Need to store data on SD card
    if (!bluetoothModule->isConnected() || !bluetoothModule->isEnabled()) {
        String data = this->convertDataToCSV();
        
        // Reinitialize SD card if needed
        if (!this->sdcard->isInitialised()) {
            this->sdcard->init();
        }

        // Write data to SD card
        if (!this->sdcard->writeHistoricalData(data)) {
            logg("Error writing in HistoricalData file");
        } 
    }

    // Reset CSV data
    this->prepareCSVData();
}

bool SensorsReadAdapter::ableToSendHistoricalData() {
    return rtc->isRTCUpdated() && bluetoothModule->isConnected() && sdcard->haveHistoricalData();
}

void SensorsReadAdapter::sendHistoricalData() {
    // Open the historical data file
    File file = SD.open(dataPath, FILE_READ);
    if (!file) {
        logg("Error opening HistoricalData file");
        return;
    }

    logg("Sending historical data");

    // Read the file line by line and send data via BLE
    while (file.available()) {
        String line = file.readStringUntil('\n');
        handleCSVData(line);
    }

    // Delete the historical data file
    file.close();
    if (!this->sdcard->deleteHistoricalData()) {
        logg("Error deleting HistoricalData file");
    }
    
}

void SensorsReadAdapter::handleCSVData(String csvData) {
    std::vector<String> dataDecoded = this->decodeCSVData(csvData);

    if (dataDecoded.size() != NR_VALUES) {
        logg("Error decoding CSV data");
        return;
    }

    // Send BLE updates
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        loggWithContext(csvData, "Historical data");

        bluetoothModule->updateTimestampCharacteristic(dataDecoded[TIMESTAMP_INDEX]);

        bluetoothModule->updateTemperatureCharacteristic(dataDecoded[BME_TEMPERATURE_INDEX].toInt());
        bluetoothModule->updateHumidityCharacteristic(dataDecoded[BME_HUMIDITY_INDEX].toInt());
        bluetoothModule->updatePressureCharacteristic(dataDecoded[BME_PRESSURE_INDEX].toInt());
        bluetoothModule->updateGasCharacteristic(dataDecoded[BME_GAS_INDEX].toInt());
        bluetoothModule->updateAltitudeCharacteristic(dataDecoded[BME_ALTITUDE_INDEX].toInt());

        bluetoothModule->updateCO2Characteristic(dataDecoded[CO2_CO2_INDEX].toInt());

        bluetoothModule->updatePM1Characteristic(dataDecoded[PM_PM1_INDEX].toInt());
        bluetoothModule->updatePM2_5Characteristic(dataDecoded[PM_PM2_5_INDEX].toInt());
        bluetoothModule->updatePM10Characteristic(dataDecoded[PM_PM10_INDEX].toInt());

        bluetoothModule->updateBatteryCharacteristic((int)(dataDecoded[BATTERY_INDEX].toFloat() * 100));
    }
}

std::vector<String> SensorsReadAdapter::decodeCSVData(String str) {
    // std::vector<String> result;
    // int found = str.indexOf(",");
    // int startIndex = 0;

    // while (found != -1) {
    //     result.push_back(str.substring(startIndex, found));
    //     startIndex = found + 1;
    //     found = str.indexOf(",", startIndex);
    // }

    // result.push_back(str.substring(startIndex, str.length()));

    // return result;

    int separatorIndex;
    std::vector<String> result;

    // Extract data from the string
    while ((separatorIndex = str.indexOf(',')) != -1) {
        result.push_back(str.substring(0, separatorIndex));
        str = str.substring(separatorIndex + 1);
    }

    // Add the remaining part of the data
    result.push_back(str);

    return result;
}

String* SensorsReadAdapter::getData() {
    return this->csvData;
}

// Internal functions
void SensorsReadAdapter::prepareCSVData() {
    for (int i = 0; i < NR_VALUES; i++) {
        csvData[i] = String(SENSOR_ERROR);
    }
}

String SensorsReadAdapter::convertDataToCSV() {
    // Add timestamp to the CSV data
    csvData[TIMESTAMP_INDEX] = rtc->getTimestamp();
    
    String csv = "";

    for (int i = 0; i < NR_VALUES; i++) {
        csv += csvData[i];
        if (i < NR_VALUES - 1) {
            csv += ",";
        }
    }
    
    loggWithContext(csv, "CSV data");

    return csv;
}