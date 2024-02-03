#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "pins.h"
#include <logger.h>
#include "constants.h"
#include <BME_sensor.h>
#include <PM_sensor.h>
#include <CO2_sensor.h>
#include <Bluetooth_module.h>
// Display
#include <GxEPD.h>
#include <GxGDEH0154D67/GxGDEH0154D67.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
// Fonts
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

class Display {
    public:
        Display(BMESensor *bmeSensor, PMSensor *pmSensor, CO2Sensor *co2Sensor, Bluetooth_module *bluetoothModule);
        void init();
        void test();
        void changeScreenRight();
        void changeScreenLeft();
        uint8_t getScreenMode();
        void updateScreen();
    private:
        GxIO_Class io;
        GxEPD_Class display;
        uint8_t currScreenMode;
        uint8_t prevScreenMode;
        BMESensor *bmeSensor;
        PMSensor *pmSensor;
        CO2Sensor *co2Sensor;
        Bluetooth_module *bluetoothModule;
        void updateMainScreen();
        void updateBluetoothScreen();
        void updateTemperatureScreen();
        void updatePMScreen();
        void updateCO2Screen();
        void updateHumidityScreen();
        void updatePressureScreen();
        void updateAltitudeScreen();
        void showSensorError();
        void showReadError();
        
};

#endif // DISPLAY_H