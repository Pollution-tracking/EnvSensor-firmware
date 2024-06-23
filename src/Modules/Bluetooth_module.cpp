#include "Modules/Bluetooth_module.h"

#define logg(message) loggWithBase(message, "BLE")
#define loggWithContext(message, context) loggWithContext(message, context, "BLE")

// Construct Bluetooth module
Bluetooth_module::Bluetooth_module() {
    // Create BLE characteristics and descriptors
    this->createCharacteristics();
    this->createDescriptors();
    this->_isOn = false;
}

// Destruct Bluetooth module
Bluetooth_module::~Bluetooth_module() {
    delete envServer;
    delete envService;
    this->destroyCharacteristics();
    this->destroyDescriptors();
}

// Routine to initialize Bluetooth module
void Bluetooth_module::init() {
    loggWithContext(bleServerName, "Server name");

    this->status = BLE_STATUS::NO_UPDATE;
    // BLE purposesly disabled => early exit
    if (!this->isEnabled() || this->_isOn) {
        return;
    }

    // Create BLE device
    BLEDevice::init(bleServerName.c_str());

    // Create BLE server
    envServer = BLEDevice::createServer();
    envServer->setCallbacks(new MyServerCallbacks(this));

    // Create BLE service
    envService = envServer->createService(SERVICE_UUID, 70);

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
    timestampDescriptor     ->setValue(timestampDescriptorValue);
    batteryDescriptor       ->setValue(batteryDescriptorValue);
    
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
    timestampCharacteristic     ->addDescriptor(timestampDescriptor);
    batteryCharacteristic       ->addDescriptor(batteryDescriptor);

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
    envService->addCharacteristic(timestampCharacteristic);
    envService->addCharacteristic(batteryCharacteristic);

    // Add timestamp characteristic callback
    timestampCharacteristic->setCallbacks(new MyCharacteristicCallbacks(this));

    // Start BLE service
    envService->start();

    // Start advertising BLE service
    this->startAdvertising();
    logg("Advertising started");

    this->_isOn = true;
}

// Routine to start advertising BLE service
void Bluetooth_module::startAdvertising() {
    bleEnabled = true;
    BLEAdvertising *pAdvertising = envServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
}

// Routine to stop advertising BLE service
void Bluetooth_module::stopAdvertising() {
    envServer->getAdvertising()->stop();
}

// Routine to disable BLE
void Bluetooth_module::disable() {
    logg("Disabling");
    bleEnabled = false;
    bleConnected = false;
    this->_isOn = false;
    
    this->stopAdvertising();
    envServer->removeService(envService);
    BLEDevice::deinit(true);
}

// Routine to enable BLE
void Bluetooth_module::enable() {
    logg("Enabling");
    bleEnabled = true;
    this->init();
}

// Extract received timestamp from client
std::string Bluetooth_module::getTimestamp() {
    return received_timestamp;
}

// Routine to check if BLE is enabled
bool Bluetooth_module::isEnabled() {
    return bleEnabled;
}

// Routine to check if client is connected
bool Bluetooth_module::isConnected() {
    return bleConnected;
}

// Routine to check if there are status updates
uint8_t Bluetooth_module::getStatusUpdates() {
    return status;
}

// Routine to acknowledge status updates
void Bluetooth_module::acknowledgeStatusUpdates(uint8_t status) {
    this->status &= ~status;
}

// Routine to update timestamp
void Bluetooth_module::updateTimestamp(std::string timestamp) {
    received_timestamp.assign(timestamp);
}

// Routines to update characteristics
void Bluetooth_module::updatePM1Characteristic(int32_t pm1) {
    pm1Characteristic->setValue(pm1);
    pm1Characteristic->notify();

    loggWithContext("Updated characteristic", "PM1");
}

void Bluetooth_module::updatePM2_5Characteristic(int32_t pm2_5) {
    pm2_5Characteristic->setValue(pm2_5);
    pm2_5Characteristic->notify();

    loggWithContext("Updated characteristic", "PM2.5");
}

void Bluetooth_module::updatePM10Characteristic(int32_t pm10) {
    pm10Characteristic->setValue(pm10);
    pm10Characteristic->notify();

    loggWithContext("Updated characteristic", "PM10");
}

void Bluetooth_module::updateCO2Characteristic(int32_t co2) {
    carbonDioxideCharacteristic->setValue(co2);
    carbonDioxideCharacteristic->notify();

    loggWithContext("Updated characteristic", "CO2");
}

void Bluetooth_module::updateTemperatureCharacteristic(int32_t temperature) {
    temperatureCharacteristic->setValue(temperature);
    temperatureCharacteristic->notify();

    loggWithContext("Updated characteristic", "Temperature");
}

void Bluetooth_module::updateGasCharacteristic(int32_t gas) {
    gasCharacteristic->setValue(gas);
    gasCharacteristic->notify();

    loggWithContext("Updated characteristic", "Gas");
}

void Bluetooth_module::updateHumidityCharacteristic(int32_t humidity) {
    humidityCharacteristic->setValue(humidity);
    humidityCharacteristic->notify();

    loggWithContext("Updated characteristic", "Humidity");
}

void Bluetooth_module::updatePressureCharacteristic(int32_t pressure) {
    pressureCharacteristic->setValue(pressure);
    pressureCharacteristic->notify();

    loggWithContext("Updated characteristic", "Pressure");
}

void Bluetooth_module::updateAltitudeCharacteristic(int32_t altitude) {
    altitudeCharacteristic->setValue(altitude);
    altitudeCharacteristic->notify();

    loggWithContext("Updated characteristic", "Altitude");
}

void Bluetooth_module::updateBatteryCharacteristic(int32_t voltage) {
    batteryCharacteristic->setValue(voltage);
    batteryCharacteristic->notify();

    loggWithContext("Updated characteristic", "Battery");
}

void Bluetooth_module::updateTimestampCharacteristic(String time) {
    timestampCharacteristic->setValue(time.c_str());
    timestampCharacteristic->notify();

    loggWithContext("Updated characteristic", "Timestamp");
}

// Routine to create BLE characteristics
void Bluetooth_module::createCharacteristics() {
    temperatureCharacteristic   = new BLECharacteristic(TEMPERATURE_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    carbonDioxideCharacteristic = new BLECharacteristic(CARBON_DIOXIDE_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    pm1Characteristic           = new BLECharacteristic(PM1_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    pm2_5Characteristic         = new BLECharacteristic(PM2_5_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    pm10Characteristic          = new BLECharacteristic(PM10_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    gasCharacteristic           = new BLECharacteristic(GAS_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    humidityCharacteristic      = new BLECharacteristic(HUMIDITY_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    pressureCharacteristic      = new BLECharacteristic(PRESSURE_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    altitudeCharacteristic      = new BLECharacteristic(ALTITUDE_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    batteryCharacteristic       = new BLECharacteristic(BATTERY_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES);
    timestampCharacteristic     = new BLECharacteristic(TIMESTAMP_CHARACTERISTIC_UUID,
                                                        CHARACTERISTIC_PROPERTIES |
                                                        BLECharacteristic::PROPERTY_WRITE);
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
    batteryDescriptor       = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    timestampDescriptor     = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
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
    delete batteryCharacteristic;
    delete timestampCharacteristic;
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
    delete batteryDescriptor;
    delete timestampDescriptor;
}