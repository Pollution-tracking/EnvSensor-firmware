#include "Modules/Display.h"

#define logg(message) loggWithObj(message, "DISPLAY")
#define loggValue(message, value) loggWithCtx(message, "DISPLAY", value)

// Construct display
Display::Display() : display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
}

// Destruct display
Display::~Display() { }

// Routine to initialize display
void Display::init() {
    logg(INITIALIZING);

    pinMode(DISPLAY_CS_PIN, OUTPUT);
    pinMode(DISPLAY_DC_PIN, OUTPUT);
    pinMode(DISPLAY_RST_PIN, OUTPUT);
    this->display.init(0, true, 2, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    this->display.setRotation(2);
    this->display.setTextColor(GxEPD_BLACK);
    lastScreenData.previousScreen = SCREEN_MODE::NO_SCREEN;

    logg(INITIALIZED);
}

// Routine to set the screen mode
void Display::setScreenMode(SCREEN_MODE mode) {
    // Update screen states
    lastScreenData.previousScreen = lastScreenData.currentScreen;
    lastScreenData.currentScreen = mode;

    // Render the selected screen
    showScreen(mode);
}

// Routine to get screen mode
SCREEN_MODE Display::getScreenMode() {
    return lastScreenData.currentScreen;
}

// Routine to change screen to the right
void Display::changeScreenRight() {
    switch (lastScreenData.currentScreen) {
        case SCREEN_MODE::BLUETOOTH:
            setScreenMode(SCREEN_MODE::SENSORS);
            break;
        case SCREEN_MODE::SENSORS:
            setScreenMode(SCREEN_MODE::BLUETOOTH);
            break;
        default:
            logg("Not allowed to change screen!");
            break;
    }
}

// Routine to change screen to the left
void Display::changeScreenLeft() {
    switch (lastScreenData.currentScreen) {
        case SCREEN_MODE::BLUETOOTH:
            setScreenMode(SCREEN_MODE::SENSORS);
            break;
        case SCREEN_MODE::SENSORS:
            setScreenMode(SCREEN_MODE::BLUETOOTH);
            break;
        default:
            logg("Not allowed to change screen!");
            break;
    }
}

// Routine to refresh current display screen
void Display::refreshScreen(SCREEN_REFRESH screen) {
    logg("Refreshing screen");

    switch (screen) {
        case SCREEN_REFRESH::BLUETOOTH:
            if (lastScreenData.currentScreen == SCREEN_MODE::BLUETOOTH) {
                setScreenMode(SCREEN_MODE::BLUETOOTH);
            }
            break;
        case SCREEN_REFRESH::SENSORS:
            if (lastScreenData.currentScreen == SCREEN_MODE::SENSORS) {
                setScreenMode(SCREEN_MODE::SENSORS);
            }
            break;
        default:
            setScreenMode(lastScreenData.currentScreen);
            break;
    }
}

// Routine to render the selected screen
void Display::showScreen(SCREEN_MODE mode) {
    switch (mode) {
        case SCREEN_MODE::HEATING:
            showHeatingScreen();
            break;
        case SCREEN_MODE::SENDING:
            showSendingScreen();
            break;
        case SCREEN_MODE::BLUETOOTH:
            showBluetoothScreen();
            break;
        case SCREEN_MODE::SENSORS:
            showSensorsScreen();
            break;
        case SCREEN_MODE::LOADING:
            showLoadingScreen();
            break;
        default:
            logg("Screen mode not recognized!");
            break;
    }

    // Power off display after rendering
    display.powerOff();
}

// Routine to show the loading screen (special)
void Display::showLoadingScreen() {
    logg("Preparing loading screen");

    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X(LoadingScreenText[0]), 90);
    display.print(LoadingScreenText[0]);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(LoadingScreenText[1]), 130);
    display.print(LoadingScreenText[1]);

    display.display(false);
    logg("Rendered loading screen");
}

// Routine to show the heating screen
void Display::showHeatingScreen() {
    logg("Preparing heating screen");
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X(HeatingScreenText[0]), 90);
    display.print(HeatingScreenText[0]);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(HeatingScreenText[1]), 130);
    display.print(HeatingScreenText[1]);

    display.display(false);
    logg("Rendered heating screen");
}

// Routine to show the sending historical data screen
void Display::showSendingScreen() {
    logg("Preparing sending screen");
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(SendingScreenText[0]), 90);
    display.print(SendingScreenText[0]);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(centerText_X(SendingScreenText[1]), 130);
    display.print(SendingScreenText[1]);

    display.display(false);
    logg("Rendered sending screen");
}

// Routines for updating the Bluetooth information screen (full or partial)
void Display::showBluetoothScreen() {
    logg("Preparing Bluetooth screen");
    if (lastScreenData.previousScreen == SCREEN_MODE::BLUETOOTH) {
        partialBluetoothScreen();
    } else {
        fullBluetoothScreen();
    }
    logg("Rendered Bluetooth screen");
}

void Display::fullBluetoothScreen() {
    loggValue("Full refresh", "Bluetooth");
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    contentBluetoothScreen();

    display.display(false);
}

void Display::partialBluetoothScreen() {
    loggValue("Partial refresh", "Bluetooth");
    
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    
    // Cover previous text
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    do {
        contentBluetoothScreen();
    } while (display.nextPage());
}

// Rendering helpers
void Display::printBLEStatus() {
    // Print action status
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(board_config.ble_stats.enabled ? "Yes" : "No");
    display.setCursor(centerText_X(line1), 90);
    display.print(line1);

    // Print connection status
    String line2 = "Connected: " + String(board_config.ble_stats.connected ? "Yes" : "No");
    display.setCursor(centerText_X(line2), 120);
    display.print(line2);
}

void Display::contentBluetoothScreen() {
    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(BluetoothScreenText[0]), 30);
    display.print(BluetoothScreenText[0]);

    // Write BLE server name
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(bleServerName), 55);
    display.print(bleServerName);

    printBLEStatus();

    // Write instructions
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(BluetoothScreenText[1]), 170);
    display.print(BluetoothScreenText[1]);
    display.setCursor(centerText_X(BluetoothScreenText[2]), 190);
    display.print(BluetoothScreenText[2]);
}

// Routines for updating the Sensors information screen (full or partial)
void Display::showSensorsScreen() {
    logg("Preparing sensors screen");
    if (lastScreenData.previousScreen == SCREEN_MODE::SENSORS) {
        partialSensorsScreen();
    } else {
        fullSensorsScreen();
    }
    logg("Rendered sensors screen");
}

void Display::fullSensorsScreen() {
    loggValue("Full refresh", "Sensors");

    display.setFullWindow();
    display.firstPage();

    do {
        // Write title
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X(SensorsScreenText[0]), 30);
        display.print(SensorsScreenText[0]);

        printSensorsStatus();
    } while (display.nextPage());
}

void Display::partialSensorsScreen() {
    loggValue("Partial refresh", "Sensors");
    
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    
    // Cover previous text
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    do {
        // Re-print title
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X(SensorsScreenText[0]), 30);
        display.print(SensorsScreenText[0]);

        // Print new text
        printSensorsStatus();
    } while (display.nextPage());
}

// Sensors screen data printer
void Display::printSensorsStatus() {
    // Print line by line sensor data
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 50);
#ifdef BME_ENABLE
    String line1 = "Temperature: " + convertTemperature(lastSensorsData.lastBMEData);
    display.println(line1);
    String line2 = "Humidity: " + convertHumidity(lastSensorsData.lastBMEData);
    display.println(line2);
    String line3 = "Pressure: " + convertPressure(lastSensorsData.lastBMEData);
    display.println(line3);
    String line4 = "Altitude: " + convertAltitude(lastSensorsData.lastBMEData);
    display.println(line4);
#endif
#ifdef CO2_ENABLE
    String line5 = "CO2: " + convertCO2(lastSensorsData.lastCO2Data);
    display.println(line5);
#endif
#ifdef PM_ENABLE
    String line6 = "PM1: " + convertPM1(lastSensorsData.lastPMData);
    display.println(line6);
    String line7 = "PM2.5: " + convertPM2_5(lastSensorsData.lastPMData);
    display.println(line7);
    String line8 = "PM10: " + convertPM10(lastSensorsData.lastPMData);
    display.println(line8);
#endif

    // Print battery voltage on the bottom right corner
    display.setFont(&FreeMono9pt7b);
    display.setCursor(40, 195);
    display.println("Battery: " + convertBattery(lastSensorsData.lastBatteryData));
}

// Routine to center text on the X axis
uint16_t Display::centerText_X(String text) {
    int16_t x_text, y_text;
    uint16_t w_text, h_text, x_centered, y_centered;

    display.getTextBounds(text, 0, 0, &x_text, &y_text, &w_text, &h_text);
    x_centered = ((display.width() - w_text) / 2) - x_text;

    return x_centered;
}