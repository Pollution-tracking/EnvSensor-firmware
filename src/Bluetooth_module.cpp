#include "Bluetooth_module.h"

// Construct Bluetooth module
Bluetooth_module::Bluetooth_module() {
    // Create BLE characteristics and descriptors
    this->createCharacteristics();
    this->createDescriptors();
}

Bluetooth_module::~Bluetooth_module() {
    delete envServer;
    delete envService;
    this->destroyCharacteristics();
    this->destroyDescriptors();
}

// Routine to initialize Bluetooth module
void Bluetooth_module::init() {
    // Create BLE device
    BLEDevice::init(bleServerName);

    // Create BLE server
    envServer = BLEDevice::createServer();
    envServer->setCallbacks(new MyServerCallbacks(this));

    // Create BLE service
    envService = envServer->createService(SERVICE_UUID);

    // Set BLE descriptors values
    temperatureDescriptor   ->setValue(temperatureDescriptorValue);
    carbonDioxideDescriptor ->setValue(carbonDioxideDescriptorValue);
    pm1Descriptor           ->setValue(pm1DescriptorValue);
    pm2_5Descriptor         ->setValue(pm2_5DescriptorValue);
    pm10Descriptor          ->setValue(pm10DescriptorValue);
    gasDescriptor           ->setValue(gasDescriptorValue);
    humidityDescriptor      ->setValue(humidityDescriptorValue);
    pressureDescriptor      ->setValue(pressureDescriptorValue);
    altitudeDescriptor      ->setValue(altitudeDescriptorValue);
    
    // Configure BLE characteristics
    temperatureCharacteristic   ->addDescriptor(temperatureDescriptor);
    carbonDioxideCharacteristic ->addDescriptor(carbonDioxideDescriptor);
    pm1Characteristic           ->addDescriptor(pm1Descriptor);
    pm2_5Characteristic         ->addDescriptor(pm2_5Descriptor);
    pm10Characteristic          ->addDescriptor(pm10Descriptor);
    gasCharacteristic           ->addDescriptor(gasDescriptor);
    humidityCharacteristic      ->addDescriptor(humidityDescriptor);
    pressureCharacteristic      ->addDescriptor(pressureDescriptor);
    altitudeCharacteristic      ->addDescriptor(altitudeDescriptor);

    // Add BLE characteristics to BLE service
    envService->addCharacteristic(temperatureCharacteristic);
    envService->addCharacteristic(carbonDioxideCharacteristic);
    envService->addCharacteristic(pm1Characteristic);
    envService->addCharacteristic(pm2_5Characteristic);
    envService->addCharacteristic(pm10Characteristic);
    envService->addCharacteristic(gasCharacteristic);
    envService->addCharacteristic(humidityCharacteristic);
    envService->addCharacteristic(pressureCharacteristic);
    envService->addCharacteristic(altitudeCharacteristic);

    // Start BLE service
    envService->start();

    // Start advertising BLE service
    if (this->isEnabled()) {
        this->startAdvertising();
        logg("Waiting a client connection to notify...");
    }
}

// Routine to start advertising BLE service
void Bluetooth_module::startAdvertising() {
    bleEnabled = true;
    envServer->getAdvertising()->start();
}

// Routine to stop advertising BLE service
void Bluetooth_module::stopAdvertising() {
    bleEnabled = false;
    envServer->getAdvertising()->stop();
}

// Routine to enable BLE
void Bluetooth_module::enable() {
    logg("Enabling BLE");
    bleEnabled = true;
    startAdvertising();
}

// Routine to disable BLE
void Bluetooth_module::disable() {
    logg("Disabling BLE");
    bleEnabled = false;
    stopAdvertising();
}

// Routine to check if BLE is enabled
bool Bluetooth_module::isEnabled() {
    return bleEnabled;
}

// Routine to check if client is connected
bool Bluetooth_module::isConnected() {
    return this->deviceConnected;
}

// Routines to update characteristics
void Bluetooth_module::updatePM1Characteristic(uint16_t pm1) {
    pm1Characteristic->setValue(pm1);
    pm1Characteristic->notify();

    logg("Updated PM1 characteristic");
}

void Bluetooth_module::updatePM2_5Characteristic(uint16_t pm2_5) {
    pm2_5Characteristic->setValue(pm2_5);
    pm2_5Characteristic->notify();

    logg("Updated PM2.5 characteristic");
}

void Bluetooth_module::updatePM10Characteristic(uint16_t pm10) {
    pm10Characteristic->setValue(pm10);
    pm10Characteristic->notify();

    logg("Updated PM10 characteristic");
}

void Bluetooth_module::updateCO2Characteristic(uint16_t co2) {
    carbonDioxideCharacteristic->setValue(co2);
    carbonDioxideCharacteristic->notify();

    logg("Updated CO2 characteristic");
}

void Bluetooth_module::updateTemperatureCharacteristic(int32_t temperature) {
    int cast_temp = static_cast<int>(temperature);
    temperatureCharacteristic->setValue(cast_temp);
    temperatureCharacteristic->notify();

    logg("Updated temperature characteristic");
}

void Bluetooth_module::updateGasCharacteristic(int32_t gas) {
    int cast_gas = static_cast<int>(gas);
    gasCharacteristic->setValue(cast_gas);
    gasCharacteristic->notify();

    logg("Updated gas characteristic");
}

void Bluetooth_module::updateHumidityCharacteristic(int32_t humidity) {
    int cast_hum = static_cast<int>(humidity);
    humidityCharacteristic->setValue(cast_hum);
    humidityCharacteristic->notify();

    logg("Updated humidity characteristic");
}

void Bluetooth_module::updatePressureCharacteristic(int32_t pressure) {
    int cast_press = static_cast<int>(pressure);
    pressureCharacteristic->setValue(cast_press);
    pressureCharacteristic->notify();

    logg("Updated pressure characteristic");
}

void Bluetooth_module::updateAltitudeCharacteristic(float altitude) {
    altitudeCharacteristic->setValue(altitude);
    altitudeCharacteristic->notify();

    logg("Updated altitude characteristic");
}

// Routine to create BLE characteristics
void Bluetooth_module::createCharacteristics() {
    temperatureCharacteristic   = new BLECharacteristic(TEMPERATURE_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    carbonDioxideCharacteristic = new BLECharacteristic(CARBON_DIOXIDE_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    pm1Characteristic           = new BLECharacteristic(PM1_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    pm2_5Characteristic         = new BLECharacteristic(PM2_5_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    pm10Characteristic          = new BLECharacteristic(PM10_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    gasCharacteristic           = new BLECharacteristic(GAS_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    humidityCharacteristic      = new BLECharacteristic(HUMIDITY_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    pressureCharacteristic      = new BLECharacteristic(PRESSURE_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
    altitudeCharacteristic      = new BLECharacteristic(ALTITUDE_CHARACTERISTIC_UUID,
                                                        BLECharacteristic::PROPERTY_NOTIFY);
}

// Routine to create BLE descriptors
void Bluetooth_module::createDescriptors() {
    temperatureDescriptor   = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    carbonDioxideDescriptor = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm1Descriptor           = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm2_5Descriptor         = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm10Descriptor          = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    gasDescriptor           = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    humidityDescriptor      = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pressureDescriptor      = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    altitudeDescriptor      = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
}

// Routine to destroy BLE characteristics
void Bluetooth_module::destroyCharacteristics() {
    delete temperatureCharacteristic;
    delete carbonDioxideCharacteristic;
    delete pm1Characteristic;
    delete pm2_5Characteristic;
    delete pm10Characteristic;
    delete gasCharacteristic;
    delete humidityCharacteristic;
    delete pressureCharacteristic;
    delete altitudeCharacteristic;
}

// Routine to destroy BLE descriptors
void Bluetooth_module::destroyDescriptors() {
    delete temperatureDescriptor;
    delete carbonDioxideDescriptor;
    delete pm1Descriptor;
    delete pm2_5Descriptor;
    delete pm10Descriptor;
    delete gasDescriptor;
    delete humidityDescriptor;
    delete pressureDescriptor;
    delete altitudeDescriptor;
}