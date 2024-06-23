#ifndef BLUETOOTH_MODULE_H
#define BLUETOOTH_MODULE_H

#include <Arduino.h>

#include "BLE2902.h"
#include "BLEUtils.h"
#include "nvs_flash.h"
#include "BLEDevice.h"
#include "BLEServer.h"

#include <Resources/Constants.h>
#include <Resources/RTC_values.hpp>

#include <Logger/Logger.h>

class Bluetooth_module {
    public:
        Bluetooth_module();
        ~Bluetooth_module();
        void init();
        void startAdvertising();
        void stopAdvertising();
        bool isConnected();
        bool isEnabled();
        uint8_t getStatusUpdates();
        void acknowledgeStatusUpdates(uint8_t status);
        void disable();
        void enable();
        std::string getTimestamp();
        void updatePM1Characteristic(int32_t pm1);
        void updatePM2_5Characteristic(int32_t pm2_5);
        void updatePM10Characteristic(int32_t pm10);
        void updateCO2Characteristic(int32_t co2);
        void updateTemperatureCharacteristic(int32_t temperature);
        void updateGasCharacteristic(int32_t gas);
        void updateHumidityCharacteristic(int32_t humidity);
        void updatePressureCharacteristic(int32_t pressure);
        void updateAltitudeCharacteristic(int32_t altitude);
        void updateBatteryCharacteristic(int32_t voltage);
        void updateTimestampCharacteristic(String time);
    private:
        bool _isOn;
        uint8_t status;
        std::string received_timestamp;
        BLEServer* envServer;   // BLE server
        BLEService* envService; // BLE service
        // BLE characteristics
        BLECharacteristic *temperatureCharacteristic;
        BLECharacteristic *carbonDioxideCharacteristic;
        BLECharacteristic *pm1Characteristic;
        BLECharacteristic *pm2_5Characteristic;
        BLECharacteristic *pm10Characteristic;
        BLECharacteristic *gasCharacteristic;
        BLECharacteristic *humidityCharacteristic;
        BLECharacteristic *pressureCharacteristic;
        BLECharacteristic *altitudeCharacteristic;
        BLECharacteristic *batteryCharacteristic;
        BLECharacteristic *timestampCharacteristic;
        // BLE descriptors
        BLEDescriptor *temperatureDescriptor;
        BLEDescriptor *carbonDioxideDescriptor;
        BLEDescriptor *pm1Descriptor;
        BLEDescriptor *pm2_5Descriptor;
        BLEDescriptor *pm10Descriptor;
        BLEDescriptor *gasDescriptor;
        BLEDescriptor *humidityDescriptor;
        BLEDescriptor *pressureDescriptor;
        BLEDescriptor *altitudeDescriptor;
        BLEDescriptor *batteryDescriptor;
        BLEDescriptor *timestampDescriptor;
        void createCharacteristics();
        void createDescriptors();
        void destroyCharacteristics();
        void destroyDescriptors();
        void updateTimestamp(std::string timestamp);
        class MyServerCallbacks: public BLEServerCallbacks {
        public:
            MyServerCallbacks(Bluetooth_module *outerClass) : outerClass(outerClass) {};
            void onConnect(BLEServer* envServer) {
                logg("[SERVER_CALLBACK] Client connected");

                bleConnected = true;
                outerClass->status |= BLE_STATUS::CLIENT_UPDATE;
                outerClass->startAdvertising();
            };
            void onDisconnect(BLEServer* envServer) {
                logg("[SERVER_CALLBACK] Client disconnected");

                bleConnected = false;
                outerClass->status |= BLE_STATUS::CLIENT_UPDATE;
                if (bleEnabled) {
                    // If user didn't disable BLE, start advertising again
                    outerClass->startAdvertising();
                }
            };
        private:
            Bluetooth_module *outerClass;
        };
        class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
        public:
            MyCharacteristicCallbacks(Bluetooth_module *outerClass) : outerClass(outerClass) {};
            void onWrite(BLECharacteristic* characteristic) {
                logg("[CHARACTERISTIC_CALLBACK] Received update");
                
                if (characteristic->getUUID().equals(TIMESTAMP_CHARACTERISTIC_UUID)) {
                    std::string timestamp = characteristic->getValue();
                    outerClass->updateTimestamp(timestamp);
                    outerClass->status |= BLE_STATUS::TIMESTAMP_UPDATE;
                }
            };
        private:
            Bluetooth_module *outerClass;
        };
};

#endif // BLUETOOTH_MODULE_H