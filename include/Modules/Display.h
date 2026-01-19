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
#include <functional>

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
        uint8_t partialRefreshCounter;
        void showScreen(SCREEN_MODE mode);
        void showLoadingScreen();
        void showHeatingScreen();
        void showSendingScreen();
        void contentLoadingScreen();
        void contentHeatingScreen();
        void contentSendingScreen();
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
        
        // Partial refresh helper functions
        void clearPartialRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void redrawPartialRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h, std::function<void()> drawFunc);
        void performFullRefresh(const String& title, std::function<void()> contentFunc);
        
        // Icon drawing helper functions
        void drawThermometerIcon(int16_t x, int16_t y);
        void drawDropletIcon(int16_t x, int16_t y);
        void drawGaugeIcon(int16_t x, int16_t y);
        void drawTriangleIcon(int16_t x, int16_t y);
        void drawAlertBoxIcon(int16_t x, int16_t y);
        void drawWarningCircleIcon(int16_t x, int16_t y);
        void drawDoubleBoxIcon(int16_t x, int16_t y);
        void drawPMDotsIcon(int16_t x, int16_t y, uint8_t size);
};

#endif // DISPLAY_H