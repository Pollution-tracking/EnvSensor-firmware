#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#include <configs.h>
#include <Resources/pins.h>
#include <Resources/Constants/screen_constants.h>
#include <Resources/Constants/logging_constants.h>
#include <Resources/RTC_values.h>
#include <Resources/Software/ScreenBufferUtils.h>
#include <Logger/logger.h>

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
        void setScreenMode(SCREEN_MODE mode);
        void changeScreenRight();
        void changeScreenLeft();
        void refreshScreen(SCREEN_REFRESH screen);
    private:
        GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;
        void showScreen(SCREEN_MODE mode);
        void showLoadingScreen();
        void showHeatingScreen();
        void showSendingScreen();
        void showBluetoothScreen();
        void contentBluetoothScreen();
        void fullBluetoothScreen();
        void partialBluetoothScreen();
        void showSensorsScreen();
        void fullSensorsScreen();
        void partialSensorsScreen();
        void showEnvironmentalScreen();
        void fullEnvironmentalScreen();
        void partialEnvironmentalScreen();
        void showPollutantsScreen();
        void fullPollutantsScreen();
        void partialPollutantsScreen();
        uint16_t centerText_X(String text);
        void printBLEStatus();
        void printSensorsStatus();
        void printEnvironmentalStatus();
        void printPollutantsStatus();
        void printBatteryData();
};

#endif // DISPLAY_H