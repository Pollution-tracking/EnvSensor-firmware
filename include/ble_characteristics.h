#ifndef BLE_CHARACTERISTICS_H
#define BLE_CHARACTERISTICS_H

#include <BLE2902.h>
#include <BLEUtils.h>

#define SERVICE_UUID (BLEUUID((uint16_t)0x181A))
#define CLIENT_CHARACTERISTIC_CONFIG_UUID (BLEUUID((uint16_t)0x2902))

#define TEMPERATURE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6E))
BLECharacteristic temperatureCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor temperatureDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define CARBON_DIOXIDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2B8C))
BLECharacteristic carbonDioxideCharacteristic(CARBON_DIOXIDE_CHARACTERISTIC_UUID,
                                               BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor carbonDioxideDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define PM1_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD5))
BLECharacteristic pm1Characteristic(PM1_CHARACTERISTIC_UUID,
                                    BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pm1Descriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define PM2_5_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD6))
BLECharacteristic pm2_5Characteristic(PM2_5_CHARACTERISTIC_UUID,
                                      BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pm2_5Descriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define PM10_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD7))
BLECharacteristic pm10Characteristic(PM10_CHARACTERISTIC_UUID,
                                     BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pm10Descriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define GAS_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BE7)) //???
BLECharacteristic gasCharacteristic(GAS_CHARACTERISTIC_UUID,
                                    BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor gasDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define HUMIDITY_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6F))
BLECharacteristic humidityCharacteristic(HUMIDITY_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor humidityDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define PRESSURE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6D))
BLECharacteristic pressureCharacteristic(PRESSURE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pressureDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#define ALTITUDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2AB3))
BLECharacteristic altitudeCharacteristic(ALTITUDE_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor altitudeDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

#endif // BLE_CHARACTERISTICS_H