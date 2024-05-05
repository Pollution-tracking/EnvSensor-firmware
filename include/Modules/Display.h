#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "Resources/pins.h"
#include <Logger/logger.h>
#include "Resources/constants.h"
#include <Resources/RTC_values.hpp>

// Display
#include <GxEPD2_BW.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
// #define GxEPD2_DRIVER_CLASS GxEPD2_154_D67
#define GxEPD2_DRIVER_CLASS GxEPD2_150_BN
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
// Fonts
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

class Display {
    public:
        Display();
        ~Display();
        void init();
        void changeScreenRight();
        void changeScreenLeft();
        uint8_t getScreenMode();
        void updateScreen(SCREENUPDATE update = SCREENUPDATE::GENERAL);
        void updateSensorsStats(String *data);
        void updateBluetoothStats(bool enabled, bool connected);
    private:
        GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;
        void checkSensorsScreen();
        void checkBluetoothScreen();
        void updateBluetoothScreen();
        void fullBluetoothScreen();
        void partialBluetoothScreen();
        void updateSensorsScreen();
        void fullSensorsScreen();
        void partialSensorsScreen();
        uint16_t centerText_X(String text);
        void printBLEStatus();
        void printSensorsStatus();
        void convertData(int idx, String data);
        void convertTemperature(String data);
        void convertHumidity(String data);
        void convertPressure(String data);
        void convertAltitude(String data);
        void convertCO2(String data);
        void convertPM1(String data);
        void convertPM2_5(String data);
        void convertPM10(String data);
};

#endif // DISPLAY_H