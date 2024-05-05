#ifndef BLUETOOTH_MODULE_H
#define BLUETOOTH_MODULE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <Logger/logger.h>
#include "Resources/constants.h"
#include <Resources/RTC_values.hpp>
#include <BLE2902.h>
#include <BLEUtils.h>
#include <nvs_flash.h>

// TODO anounce display to update device connected state

class Bluetooth_module {
    public:
        Bluetooth_module();
        ~Bluetooth_module();
        void init();
        void startAdvertising();
        void stopAdvertising();
        bool isConnected();
        bool isEnabled();
        void disable();
        void enable();
        void updatePM1Characteristic(int32_t pm1);
        void updatePM2_5Characteristic(int32_t pm2_5);
        void updatePM10Characteristic(int32_t pm10);
        void updateCO2Characteristic(int32_t co2);
        void updateTemperatureCharacteristic(int32_t temperature);
        void updateGasCharacteristic(int32_t gas);
        void updateHumidityCharacteristic(int32_t humidity);
        void updatePressureCharacteristic(int32_t pressure);
        void updateAltitudeCharacteristic(int32_t altitude);
    private:
        BLEServer* envServer; // BLE server
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
        class MyServerCallbacks: public BLEServerCallbacks {
        public:
            MyServerCallbacks(Bluetooth_module *outerClass) : outerClass(outerClass) {};
            void onConnect(BLEServer* envServer) {
                logg("Client connected");
                bleConnected = true;
                outerClass->startAdvertising();
            };
            void onDisconnect(BLEServer* envServer) {
                bleConnected = false;
                logg("Client disconnected");
                if (bleEnabled) {
                    // If user didn't disable BLE, start advertising again
                    outerClass->startAdvertising();
                }
            };
        private:
            Bluetooth_module *outerClass;
        };
        void createCharacteristics();
        void createDescriptors();
        void destroyCharacteristics();
        void destroyDescriptors();
};

#endif // BLUETOOTH_MODULE_H