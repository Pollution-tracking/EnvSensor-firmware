#ifndef BLUETOOTH_CONSTANTS_H
#define BLUETOOTH_CONSTANTS_H

#include <Arduino.h>
#include <string>
#include <BLECharacteristic.h>

enum BLE {
    CLIENT_UPDATE,
    TIMESTAMP_UPDATE,
    INIT_BLE,
    NO_UPDATE
};

struct BLE_STATS {
    bool enabled;
    bool connected;
};

const uint32_t CHARACTERISTIC_PROPERTIES = BLECharacteristic::PROPERTY_READ |
                                           BLECharacteristic::PROPERTY_NOTIFY;
// UUIDs 0000xxxx-0000-1000-8000-00805F9B34FB
#define SERVICE_UUID                        (BLEUUID((uint16_t)0x181A))
#define CLIENT_CHARACTERISTIC_CONFIG_UUID   (BLEUUID((uint16_t)0x2902))
#define TEMPERATURE_CHARACTERISTIC_UUID     (BLEUUID((uint16_t)0x2A6E))
#define CARBON_DIOXIDE_CHARACTERISTIC_UUID  (BLEUUID((uint16_t)0x2B8C))
#define PM1_CHARACTERISTIC_UUID             (BLEUUID((uint16_t)0x2BD5))
#define PM2_5_CHARACTERISTIC_UUID           (BLEUUID((uint16_t)0x2BD6))
#define PM10_CHARACTERISTIC_UUID            (BLEUUID((uint16_t)0x2BD7))
#define GAS_CHARACTERISTIC_UUID             (BLEUUID((uint16_t)0x2BE7))
#define HUMIDITY_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2A6F))
#define PRESSURE_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2A6D))
#define ALTITUDE_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2AB3))
#define CARBON_MONOXIDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2B8D))
#define NITROGEN_DIOXIDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2B8E))
#define AMMONIA_CHARACTERISTIC_UUID         (BLEUUID((uint16_t)0x2B8F))
#define BATTERY_CHARACTERISTIC_UUID         (BLEUUID((uint16_t)0x2A76))
#define TIMESTAMP_CHARACTERISTIC_UUID       (BLEUUID((uint16_t)0x2A79))

// BLE descriptor values
const std::string temperatureDescriptorValue     = "Temperature";
const std::string carbonDioxideDescriptorValue   = "Carbon dioxide";
const std::string pm1DescriptorValue             = "PM1";
const std::string pm2_5DescriptorValue           = "PM2.5";
const std::string pm10DescriptorValue            = "PM10";
const std::string gasDescriptorValue             = "Gas resistance";
const std::string humidityDescriptorValue        = "Humidity";
const std::string pressureDescriptorValue        = "Pressure";
const std::string altitudeDescriptorValue        = "Altitude";
const std::string batteryDescriptorValue         = "Battery";
const std::string timestampDescriptorValue       = "Timestamp";
const std::string carbonMonoxideDescriptorValue  = "Carbon monoxide";
const std::string nitrogenDioxideDescriptorValue = "Nitrogen dioxide";
const std::string ammoniaDescriptorValue         = "Ammonia";

#endif // BLUETOOTH_CONSTANTS_H