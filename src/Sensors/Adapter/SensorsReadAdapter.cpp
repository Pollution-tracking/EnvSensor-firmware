#include <Sensors/Adapter/SensorsReadAdapter.h>

#define logg(message) loggWithBase(message, "ADAPTER")
#define loggWithContext(message, context) loggWithContext(message, context, "ADAPTER")

// Construct adapter
SensorsReadAdapter::SensorsReadAdapter(Bluetooth_module *bluetoothModule, SDcard *sdcard, RTC *rtc) {
    this->bluetoothModule = bluetoothModule;
    this->sdcard = sdcard;
    this->rtc = rtc;
}

// Destruct adapter
SensorsReadAdapter::~SensorsReadAdapter() { }

// Routine to initialize adapter
void SensorsReadAdapter::init() {
    this->prepareCSVData();
    this->sdcard->init();
    this->bluetoothModule->init();
    if (!this->rtc->isInitialised()) {
        this->rtc->init();
    }

    logg("Initialized");
}

// Update local buffers with sensors data and send updates to BLE module
void SensorsReadAdapter::updateCO2Sensor(CO2Data co2data) {
    loggWithContext("Handling data", "CO2");

    // Update the CO2 sensor data
    this->co2data = co2data;

    // Update mean temperature
    this->meanTemperature.addTemperature(this->co2data.temperature * 1.25);

    // Update the CSV data
    csvData[CO2_CO2_INDEX] = String(this->co2data.co2);
    csvData[TEMPERATURE_INDEX] = String(this->meanTemperature.getMeanTemperature());
}

void SensorsReadAdapter::updateBMESensor(BMEData bmedata) {
    loggWithContext("Handling data", "BME");

    // Update the BME sensor data
    this->bmedata = bmedata;

    // Update mean temperature
    this->meanTemperature.addTemperature(this->bmedata.temperature * 0.75);

    // Update the CSV data
    csvData[BME_HUMIDITY_INDEX] = String(this->bmedata.humidity);
    csvData[BME_PRESSURE_INDEX] = String(this->bmedata.pressure);
    csvData[BME_GAS_INDEX] = String(this->bmedata.gas);
    csvData[BME_ALTITUDE_INDEX] = String(this->bmedata.altitude);
    csvData[TEMPERATURE_INDEX] = String(this->meanTemperature.getMeanTemperature());
}

void SensorsReadAdapter::updatePMSensor(PMData pmdata) {
    loggWithContext("Handling data", "PM");

    // Update the PM sensor data
    this->pmdata = pmdata;

    // Update the CSV data
    csvData[PM_PM1_INDEX] = String(this->pmdata.pm1);
    csvData[PM_PM2_5_INDEX] = String(this->pmdata.pm2_5);
    csvData[PM_PM10_INDEX] = String(this->pmdata.pm10);
}

void SensorsReadAdapter::updateBatteryStatus(float voltage) {
    loggWithContext("Handling data", "Battery");

    // Update the battery data
    this->batteryData = voltage;

    // Update the CSV data
    csvData[BATTERY_INDEX] = String(voltage);
}

// Routine to save historical data on SD card
void SensorsReadAdapter::storeData() {
    if (bluetoothModule->isEnabled() && bluetoothModule->isConnected()) {
        // Update characteristics
        bluetoothModule->updateCO2Characteristic(this->co2data.co2);

        bluetoothModule->updateTemperatureCharacteristic(this->meanTemperature.getMeanTemperature());
        bluetoothModule->updateHumidityCharacteristic(this->bmedata.humidity);
        bluetoothModule->updatePressureCharacteristic(this->bmedata.pressure);
        bluetoothModule->updateGasCharacteristic(this->bmedata.gas);
        bluetoothModule->updateAltitudeCharacteristic(this->bmedata.altitude);

        bluetoothModule->updatePM1Characteristic(this->pmdata.pm1);
        bluetoothModule->updatePM2_5Characteristic(this->pmdata.pm2_5);
        bluetoothModule->updatePM10Characteristic(this->pmdata.pm10);

        bluetoothModule->updateBatteryCharacteristic((int)(this->batteryData * 100));
    } else {
        // Need to store data on SD card
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

// Routine to check if historical data can be sent via BLE
bool SensorsReadAdapter::ableToSendHistoricalData() {
    return rtc->isRTCUpdated() && bluetoothModule->isConnected() && sdcard->haveHistoricalData();
}

// Routine to send historical data via BLE batch by batch
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
        delay(100); // Delay to avoid BLE buffer overflow
    }

    // Delete the historical data file
    file.close();
    if (!this->sdcard->deleteHistoricalData()) {
        logg("Error deleting HistoricalData file");
    }
    
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

        bluetoothModule->updateTemperatureCharacteristic(dataDecoded[TEMPERATURE_INDEX].toInt());
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

// Decoder for CSV stored data lines
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