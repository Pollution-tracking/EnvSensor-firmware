#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#include <Resources/Pins.h>
#include <Resources/Constants.h>
#include <Resources/RTC_values.hpp>

#include <Helpers/ScreenBufferUtils.hpp>

#include <Logger/Logger.h>

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
// #define GxEPD2_DRIVER_CLASS GxEPD2_154_D67
#define GxEPD2_DRIVER_CLASS GxEPD2_150_BN
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

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
        void showLoadingScreen();
        void showSendingScreen();
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
};

#endif // DISPLAY_H