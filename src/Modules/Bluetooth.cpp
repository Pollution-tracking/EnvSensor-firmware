#include "Modules/Bluetooth.h"

#define logg(message) loggWithObj(message, "BLE")
#define loggValue(message, value) loggWithCtx(message, "BLE", value)

// Construct Bluetooth module
Bluetooth::Bluetooth() {
    // Create BLE characteristics and descriptors
    this->createCharacteristics();
    this->createDescriptors();
}

// Destruct Bluetooth module
Bluetooth::~Bluetooth() {
    delete envServer;
    delete envService;
    this->destroyCharacteristics();
    this->destroyDescriptors();
}

// Routine to initialize Bluetooth module
void Bluetooth::init() {
    loggValue(bleServerName, "Server name");

    // BLE purposesly disabled => early exit
    if (!this->isEnabled()) {
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
    temperatureDescriptor     ->setValue((uint8_t*)temperatureDescriptorValue.c_str(), temperatureDescriptorValue.length());
    carbonDioxideDescriptor   ->setValue((uint8_t*)carbonDioxideDescriptorValue.c_str(), carbonDioxideDescriptorValue.length());
    pm1Descriptor             ->setValue((uint8_t*)pm1DescriptorValue.c_str(), pm1DescriptorValue.length());
    pm2_5Descriptor           ->setValue((uint8_t*)pm2_5DescriptorValue.c_str(), pm2_5DescriptorValue.length());
    pm10Descriptor            ->setValue((uint8_t*)pm10DescriptorValue.c_str(), pm10DescriptorValue.length());
    gasDescriptor             ->setValue((uint8_t*)gasDescriptorValue.c_str(), gasDescriptorValue.length());
    humidityDescriptor        ->setValue((uint8_t*)humidityDescriptorValue.c_str(), humidityDescriptorValue.length());
    pressureDescriptor        ->setValue((uint8_t*)pressureDescriptorValue.c_str(), pressureDescriptorValue.length());
    altitudeDescriptor        ->setValue((uint8_t*)altitudeDescriptorValue.c_str(), altitudeDescriptorValue.length());
    carbonMonoxideDescriptor  ->setValue((uint8_t*)carbonMonoxideDescriptorValue.c_str(), carbonMonoxideDescriptorValue.length());
    nitrogenDioxideDescriptor ->setValue((uint8_t*)nitrogenDioxideDescriptorValue.c_str(), nitrogenDioxideDescriptorValue.length());
    ammoniaDescriptor         ->setValue((uint8_t*)ammoniaDescriptorValue.c_str(), ammoniaDescriptorValue.length());
    timestampDescriptor       ->setValue((uint8_t*)timestampDescriptorValue.c_str(), timestampDescriptorValue.length());
    batteryDescriptor         ->setValue((uint8_t*)batteryDescriptorValue.c_str(), batteryDescriptorValue.length());
    
    // Configure BLE characteristics
    temperatureCharacteristic     ->addDescriptor(temperatureDescriptor);
    carbonDioxideCharacteristic   ->addDescriptor(carbonDioxideDescriptor);
    pm1Characteristic             ->addDescriptor(pm1Descriptor);
    pm2_5Characteristic           ->addDescriptor(pm2_5Descriptor);
    pm10Characteristic            ->addDescriptor(pm10Descriptor);
    gasCharacteristic             ->addDescriptor(gasDescriptor);
    humidityCharacteristic        ->addDescriptor(humidityDescriptor);
    pressureCharacteristic        ->addDescriptor(pressureDescriptor);
    altitudeCharacteristic        ->addDescriptor(altitudeDescriptor);
    carbonMonoxideCharacteristic  ->addDescriptor(carbonMonoxideDescriptor);
    nitrogenDioxideCharacteristic ->addDescriptor(nitrogenDioxideDescriptor);
    ammoniaCharacteristic         ->addDescriptor(ammoniaDescriptor);
    timestampCharacteristic       ->addDescriptor(timestampDescriptor);
    batteryCharacteristic         ->addDescriptor(batteryDescriptor);

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
    envService->addCharacteristic(carbonMonoxideCharacteristic);
    envService->addCharacteristic(nitrogenDioxideCharacteristic);
    envService->addCharacteristic(ammoniaCharacteristic);
    envService->addCharacteristic(timestampCharacteristic);
    envService->addCharacteristic(batteryCharacteristic);

    // Add timestamp characteristic callback
    timestampCharacteristic->setCallbacks(new MyCharacteristicCallbacks(this));

    // Start BLE service
    envService->start();

    // Start advertising BLE service
    this->startAdvertising();
    logg("Advertising started");

    // Announce BLE initialization
    board_config.ble_stats.enabled = true;
    board_config.to_treat = true;
}

// Routine to start advertising BLE service
void Bluetooth::startAdvertising() {
    BLEAdvertising *pAdvertising = envServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
}

// Routine to stop advertising BLE service
void Bluetooth::stopAdvertising() {
    envServer->getAdvertising()->stop();
}

// Routine to disable BLE
void Bluetooth::disable() {
    logg("Disabling");
    board_config.ble_stats.enabled = false;
    board_config.ble_stats.connected = false;
    board_config.to_treat = true;
    
    this->stopAdvertising();
    envServer->removeService(envService);
    BLEDevice::deinit(true);
}

// Routine to enable BLE
void Bluetooth::enable() {
    logg("Enabling");
    this->init();
}

// Routine to toggle BLE state
void Bluetooth::toggle() {
    logg("Toggle state");
    if (this->isEnabled()) {
        this->disable();
    } else {
        this->enable();
    }
}

// Extract received timestamp from client
String Bluetooth::getTimestamp() {
    return received_timestamp;
}

// Routine to check if BLE is enabled
bool Bluetooth::isEnabled() {
    return board_config.ble_stats.enabled;
}

// Routine to check if client is connected
bool Bluetooth::isConnected() {
    return board_config.ble_stats.connected;
}

// Routine to update timestamp
void Bluetooth::updateTimestamp(String timestamp) {
    received_timestamp = timestamp;
}

// Routines to update characteristics
void Bluetooth::updatePM1Characteristic(int32_t pm1) {
    pm1Characteristic->setValue((uint8_t*)&pm1, sizeof(pm1));
    pm1Characteristic->notify();

    loggValue("Updated characteristic", "PM1");
}

void Bluetooth::updatePM2_5Characteristic(int32_t pm2_5) {
    pm2_5Characteristic->setValue((uint8_t*)&pm2_5, sizeof(pm2_5));
    pm2_5Characteristic->notify();

    loggValue("Updated characteristic", "PM2.5");
}

void Bluetooth::updatePM10Characteristic(int32_t pm10) {
    pm10Characteristic->setValue((uint8_t*)&pm10, sizeof(pm10));
    pm10Characteristic->notify();

    loggValue("Updated characteristic", "PM10");
}

void Bluetooth::updateCO2Characteristic(int32_t co2) {
    carbonDioxideCharacteristic->setValue((uint8_t*)&co2, sizeof(co2));
    carbonDioxideCharacteristic->notify();

    loggValue("Updated characteristic", "CO2");
}

void Bluetooth::updateTemperatureCharacteristic(int32_t temperature) {
    temperatureCharacteristic->setValue((uint8_t*)&temperature, sizeof(temperature));
    temperatureCharacteristic->notify();

    loggValue("Updated characteristic", "Temperature");
}

void Bluetooth::updateGasCharacteristic(int32_t gas) {
    gasCharacteristic->setValue((uint8_t*)&gas, sizeof(gas));
    gasCharacteristic->notify();

    loggValue("Updated characteristic", "Gas");
}

void Bluetooth::updateHumidityCharacteristic(int32_t humidity) {
    humidityCharacteristic->setValue((uint8_t*)&humidity, sizeof(humidity));
    humidityCharacteristic->notify();

    loggValue("Updated characteristic", "Humidity");
}

void Bluetooth::updatePressureCharacteristic(int32_t pressure) {
    pressureCharacteristic->setValue((uint8_t*)&pressure, sizeof(pressure));
    pressureCharacteristic->notify();

    loggValue("Updated characteristic", "Pressure");
}

void Bluetooth::updateAltitudeCharacteristic(int32_t altitude) {
    altitudeCharacteristic->setValue((uint8_t*)&altitude, sizeof(altitude));
    altitudeCharacteristic->notify();

    loggValue("Updated characteristic", "Altitude");
}

void Bluetooth::updateCarbonMonoxideCharacteristic(int32_t co) {
    carbonMonoxideCharacteristic->setValue((uint8_t*)&co, sizeof(co));
    carbonMonoxideCharacteristic->notify();

    loggValue("Updated characteristic", "Carbon Monoxide");
}

void Bluetooth::updateNitrogenDioxideCharacteristic(int32_t no2) {
    nitrogenDioxideCharacteristic->setValue((uint8_t*)&no2, sizeof(no2));
    nitrogenDioxideCharacteristic->notify();

    loggValue("Updated characteristic", "Nitrogen Dioxide");
}

void Bluetooth::updateAmmoniaCharacteristic(int32_t nh3) {
    ammoniaCharacteristic->setValue((uint8_t*)&nh3, sizeof(nh3));
    ammoniaCharacteristic->notify();

    loggValue("Updated characteristic", "Ammonia");
}

void Bluetooth::updateBatteryCharacteristic(int32_t voltage) {
    batteryCharacteristic->setValue((uint8_t*)&voltage, sizeof(voltage));
    batteryCharacteristic->notify();

    loggValue("Updated characteristic", "Battery");
}

void Bluetooth::updateTimestampCharacteristic(String time) {
    timestampCharacteristic->setValue(time.c_str());
    timestampCharacteristic->notify();
    loggValue("Updated characteristic", "Timestamp");
}

// Routine to create BLE characteristics
void Bluetooth::createCharacteristics() {
    temperatureCharacteristic     = new BLECharacteristic(TEMPERATURE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    carbonDioxideCharacteristic   = new BLECharacteristic(CARBON_DIOXIDE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    pm1Characteristic             = new BLECharacteristic(PM1_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    pm2_5Characteristic           = new BLECharacteristic(PM2_5_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    pm10Characteristic            = new BLECharacteristic(PM10_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    gasCharacteristic             = new BLECharacteristic(GAS_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    humidityCharacteristic        = new BLECharacteristic(HUMIDITY_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    pressureCharacteristic        = new BLECharacteristic(PRESSURE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    altitudeCharacteristic        = new BLECharacteristic(ALTITUDE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    carbonMonoxideCharacteristic  = new BLECharacteristic(CARBON_MONOXIDE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    nitrogenDioxideCharacteristic = new BLECharacteristic(NITROGEN_DIOXIDE_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    ammoniaCharacteristic         = new BLECharacteristic(AMMONIA_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    batteryCharacteristic         = new BLECharacteristic(BATTERY_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES);
    timestampCharacteristic       = new BLECharacteristic(TIMESTAMP_CHARACTERISTIC_UUID,
                                                          CHARACTERISTIC_PROPERTIES |
                                                          BLECharacteristic::PROPERTY_WRITE);
}

// Routine to create BLE descriptors
void Bluetooth::createDescriptors() {
    temperatureDescriptor     = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    carbonDioxideDescriptor   = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm1Descriptor             = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm2_5Descriptor           = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pm10Descriptor            = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    gasDescriptor             = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    humidityDescriptor        = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    pressureDescriptor        = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    altitudeDescriptor        = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    carbonMonoxideDescriptor  = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    nitrogenDioxideDescriptor = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    ammoniaDescriptor         = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    batteryDescriptor         = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
    timestampDescriptor       = new BLEDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
}

// Routine to destroy BLE characteristics
void Bluetooth::destroyCharacteristics() {
    delete temperatureCharacteristic;
    delete carbonDioxideCharacteristic;
    delete pm1Characteristic;
    delete pm2_5Characteristic;
    delete pm10Characteristic;
    delete gasCharacteristic;
    delete humidityCharacteristic;
    delete pressureCharacteristic;
    delete altitudeCharacteristic;
    delete carbonMonoxideCharacteristic;
    delete nitrogenDioxideCharacteristic;
    delete ammoniaCharacteristic;
    delete batteryCharacteristic;
    delete timestampCharacteristic;
}

// Routine to destroy BLE descriptors
void Bluetooth::destroyDescriptors() {
    delete temperatureDescriptor;
    delete carbonDioxideDescriptor;
    delete pm1Descriptor;
    delete pm2_5Descriptor;
    delete pm10Descriptor;
    delete gasDescriptor;
    delete humidityDescriptor;
    delete pressureDescriptor;
    delete altitudeDescriptor;
    delete carbonMonoxideDescriptor;
    delete nitrogenDioxideDescriptor;
    delete ammoniaDescriptor;
    delete batteryDescriptor;
    delete timestampDescriptor;
}