#include <SensorsReadAdapter.h>

SensorsReadAdapter::SensorsReadAdapter(Bluetooth_module *bluetoothModule, SDcard *sdcard) {
    this->bluetoothModule = bluetoothModule;
    this->sdcard = sdcard;
}

SensorsReadAdapter::~SensorsReadAdapter() { }

void SensorsReadAdapter::init() {
    this->initCSVData();
    this->sdcard->init();
    this->bluetoothModule->init();
}

void SensorsReadAdapter::updateCO2Sensor(CO2Data co2data) {
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

void SensorsReadAdapter::storeData() {
    // Need to store data in SD card
    if (!bluetoothModule->isConnected() || !bluetoothModule->isEnabled()) {
        String data = this->convertDataToCSV();
        
        // Reinitialize SD card if needed
        if (!this->sdcard->isInitialised()) {
            this->sdcard->init();
        }

        // Write data to SD card
        if (!this->sdcard->writeData(dataPath, data)) {
            logg("Error writing data to SD card");
        } else {
            logg("Data written to SD card");
        }
    }

    // Reset CSV data
    this->initCSVData();
}

String* SensorsReadAdapter::getData() {
    return this->csvData;
}

// Internal functions
void SensorsReadAdapter::initCSVData() {
    for (int i = 0; i < NR_VALUES; i++) {
        csvData[i] = String(SENSOR_ERROR);
    }
}

String SensorsReadAdapter::convertDataToCSV() {
    String csv = "";

    for (int i = 0; i < NR_VALUES; i++) {
        csv += csvData[i];
        if (i < NR_VALUES - 1) {
            csv += ",";
        }
    }

    return csv;
}