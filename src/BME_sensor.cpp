#include "BME_sensor.h"

// Construct BME sensor
BMESensor::BMESensor(Bluetooth_module *bluetoothModule)
    : bluetoothModule(bluetoothModule) {
    theWire = new TwoWire(0);
    theWire->begin(BME680_SDA_PIN, CLK_PIN);
    bme = new Adafruit_BME680(theWire);
}

BMESensor::~BMESensor() {
    delete bme;
    delete theWire;
}

// Routine to initialize BME sensor
void BMESensor::init() {
    if(!bme->begin()) {
        logg("Could not find a valid BME680 sensor, check wiring!");
    } else {
        sensorFound = true;
        bme->setTemperatureOversampling(BME680_OS_8X);
        bme->setHumidityOversampling(BME680_OS_2X);
        bme->setPressureOversampling(BME680_OS_4X);
        bme->setIIRFilterSize(BME680_FILTER_SIZE_3);
        bme->setGasHeater(320, 150); // 320*C for 150 ms
        logg("BME680 initialized");
    }
}

// Routine to update BME values
void BMESensor::update() {
    if (!this->sensorFound) {
        return;
    }

    logg("BME680 reading...");
    
    bool status = bme->performReading();
    this->checkErrors(status);

    if (!this->errorBME) {
        
        data.temperature = bme->temperature;
        logg("BME680 temperature: " + String(data.temperature));

        data.pressure = bme->pressure / 100.0; //hPa
        logg("BME680 pressure: " + String(data.pressure));

        data.humidity = bme->humidity;
        logg("BME680 humidity: " + String(data.humidity));

        data.gas = bme->gas_resistance / 1000.0;
        logg("BME680 gas: " + String(data.gas));

        data.altitude = bme->readAltitude(seaLevel);
        logg("BME680 altitude: " + String(data.altitude));

        this->updateCharacteristics();
    }
}

// Getters
uint32_t BMESensor::getTemperature() {
    return data.temperature;
}

uint32_t BMESensor::getPressure() {
    return data.pressure;
}

uint32_t BMESensor::getHumidity() {
    return data.humidity;
}

uint32_t BMESensor::getGas() {
    return data.gas;
}

float BMESensor::getAltitude() {
    return data.altitude;
}

// Internal functions
void BMESensor::updateCharacteristics() {
    logg("Updating BME characteristics");
    
    bluetoothModule->updateTemperatureCharacteristic(data.temperature);
    bluetoothModule->updatePressureCharacteristic(data.pressure);
    bluetoothModule->updateHumidityCharacteristic(data.humidity);
    bluetoothModule->updateGasCharacteristic(data.gas);
    bluetoothModule->updateAltitudeCharacteristic(data.altitude);
}

void BMESensor::checkErrors(bool status) {
    if (!status) {
        logg("BME680 error");
        this->errorBME = true;
    } else {
        this->errorBME = false;
    }
}