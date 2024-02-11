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
#include <Sensor.h>

// Display
#include <GxEPD2_BW.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_154_D67
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
// Fonts
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

class Display {
    public:
        Display(BMESensor *bmeSensor,
                PMSensor *pmSensor,
                CO2Sensor *co2Sensor,
                Bluetooth_module *bluetoothModule);
        void init();
        void changeScreenRight();
        void changeScreenLeft();
        uint8_t getScreenMode();
        void updateScreen();
    private:
        GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;
        uint8_t currScreenMode;
        uint8_t prevScreenMode;
        BMESensor *bmeSensor;
        PMSensor *pmSensor;
        CO2Sensor *co2Sensor;
        Bluetooth_module *bluetoothModule;
        void updateBluetoothScreen();
        void fullBluetoothScreen();
        void partialBluetoothScreen();
        void updateSensorsScreen();
        void fullSensorsScreen();
        void partialSensorsScreen();
        bool checkSensorConnection(Sensor *sensor); // true if connected, false otherwise
        bool checkSensorError(Sensor *sensor); // true if error, false otherwise
        String readTemperature();
        String readHumidity();
        String readPressure();
        String readAltitude();
        String readCO2();
        String readPM1();
        String readPM2_5();
        String readPM10();
};

#endif // DISPLAY_H