#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

#include "BLE2902.h"
#include "BLEUtils.h"
#include "nvs_flash.h"
#include "BLEDevice.h"
#include "BLEServer.h"

#include <Resources/RTC_values.h>
#include <Resources/Constants/bluetooth_constants.h>

#include <Logger/logger.h>

#define _logg(message) loggWithObj(message, "BLE SERVER CALLBACK")

class Bluetooth {
    public:
        Bluetooth();
        ~Bluetooth();
        void init();
        
        bool isConnected();
        bool isEnabled();
        void disable();
        void enable();
        void toggle();
        String getTimestamp();
        void updatePM1Characteristic(int32_t pm1);
        void updatePM2_5Characteristic(int32_t pm2_5);
        void updatePM10Characteristic(int32_t pm10);
        void updateCO2Characteristic(int32_t co2);
        void updateTemperatureCharacteristic(int32_t temperature);
        void updateGasCharacteristic(int32_t gas);
        void updateHumidityCharacteristic(int32_t humidity);
        void updatePressureCharacteristic(int32_t pressure);
        void updateAltitudeCharacteristic(int32_t altitude);
        void updateCarbonMonoxideCharacteristic(int32_t co);
        void updateNitrogenDioxideCharacteristic(int32_t no2);
        void updateAmmoniaCharacteristic(int32_t nh3);
        void updateBatteryCharacteristic(int32_t voltage);
        void updateTimestampCharacteristic(String time);
    private:
        String received_timestamp;
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
        BLECharacteristic *carbonMonoxideCharacteristic;
        BLECharacteristic *nitrogenDioxideCharacteristic;
        BLECharacteristic *ammoniaCharacteristic;
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
        BLEDescriptor *carbonMonoxideDescriptor;
        BLEDescriptor *nitrogenDioxideDescriptor;
        BLEDescriptor *ammoniaDescriptor;
        BLEDescriptor *batteryDescriptor;
        BLEDescriptor *timestampDescriptor;
        void startAdvertising();
        void stopAdvertising();
        void createCharacteristics();
        void createDescriptors();
        void destroyCharacteristics();
        void destroyDescriptors();
        void updateTimestamp(String timestamp);
        class MyServerCallbacks: public BLEServerCallbacks {
        public:
            MyServerCallbacks(Bluetooth *outerClass) : outerClass(outerClass) {};
            void onConnect(BLEServer* envServer) {
                _logg("Client connected");

                // Announce connection
                board_config.ble_stats.connected = true;
                board_config.ble_state = BLE::CLIENT_UPDATE;
                board_config.to_treat = true;
                outerClass->startAdvertising();
            };
            void onDisconnect(BLEServer* envServer) {
                _logg("Client disconnected");

                // Announce disconnection
                board_config.ble_stats.connected = false;
                board_config.ble_state = BLE::CLIENT_UPDATE;
                board_config.to_treat = true;

                // If user didn't disable BLE, start advertising again
                if (board_config.ble_stats.enabled) {
                    outerClass->startAdvertising();
                }
            };
        private:
            Bluetooth *outerClass;
        };
        class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
        public:
            MyCharacteristicCallbacks(Bluetooth *outerClass) : outerClass(outerClass) {};
            void onWrite(BLECharacteristic* characteristic) {
                _logg("Received update");
                
                if (characteristic->getUUID().equals(TIMESTAMP_CHARACTERISTIC_UUID)) {
                    std::string receivedTimestamp = characteristic->getValue();
                    outerClass->updateTimestamp(String(receivedTimestamp.c_str()));

                    // Announce timestamp update
                    board_config.ble_state = BLE::TIMESTAMP_UPDATE;
                    board_config.to_treat = true;
                }
            };
        private:
            Bluetooth *outerClass;
        };
};

#endif // BLUETOOTH_H