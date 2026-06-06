#include "Modules/Display.h"

#define logg(message) loggWithObj(message, "DISPLAY")
#define loggValue(message, value) loggWithCtx(message, "DISPLAY", value)

// Construct display
Display::Display() : 
    partialRefreshCounter(0),
    display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
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

// Routine to change screen to the right
void Display::changeScreenRight() {
    switch (lastScreenData.currentScreen) {
        case SCREEN_MODE::BLUETOOTH:
            setScreenMode(SCREEN_MODE::SENSORS);
            break;
        case SCREEN_MODE::SENSORS:
            setScreenMode(SCREEN_MODE::AMBIENT);
            break;
        case SCREEN_MODE::AMBIENT:
            setScreenMode(SCREEN_MODE::POLLUTION);
            break;
        case SCREEN_MODE::POLLUTION:
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
            setScreenMode(SCREEN_MODE::POLLUTION);
            break;
        case SCREEN_MODE::SENSORS:
            setScreenMode(SCREEN_MODE::BLUETOOTH);
            break;
        case SCREEN_MODE::AMBIENT:
            setScreenMode(SCREEN_MODE::SENSORS);
            break;
        case SCREEN_MODE::POLLUTION:
            setScreenMode(SCREEN_MODE::AMBIENT);
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
            } else if (lastScreenData.currentScreen == SCREEN_MODE::AMBIENT) {
                setScreenMode(SCREEN_MODE::AMBIENT);
            } else if (lastScreenData.currentScreen == SCREEN_MODE::POLLUTION) {
                setScreenMode(SCREEN_MODE::POLLUTION);
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
        case SCREEN_MODE::AMBIENT:
            showAmbientScreen();
            break;
        case SCREEN_MODE::POLLUTION:
            showPollutionScreen();
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
    
    performFullRefresh("", [this]() {
        contentLoadingScreen();
    });
    
    
    performFullRefresh("", [this]() {
        contentLoadingScreen();
    });
    
    logg("Rendered loading screen");
}

// Routine to show the heating screen
void Display::showHeatingScreen() {
    logg("Preparing heating screen");
    
    performFullRefresh("", [this]() {
        contentHeatingScreen();
    });
    
    
    performFullRefresh("", [this]() {
        contentHeatingScreen();
    });
    
    logg("Rendered heating screen");
}

// Routine to show the sending historical data screen
void Display::showSendingScreen() {
    logg("Preparing sending screen");
    
    performFullRefresh("", [this]() {
        contentSendingScreen();
    });
    
    
    performFullRefresh("", [this]() {
        contentSendingScreen();
    });
    
    logg("Rendered sending screen");
}

// Routines for updating the Bluetooth information screen (full or partial)
void Display::showBluetoothScreen() {
    logg("Preparing Bluetooth screen");
    
    if (lastScreenData.previousScreen == SCREEN_MODE::BLUETOOTH &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialBluetoothScreen();
    } else {
        fullBluetoothScreen();
    }
    logg("Rendered Bluetooth screen");
}

void Display::fullBluetoothScreen() {
    loggValue("Full refresh", "Bluetooth");
    
    performFullRefresh(BluetoothScreenText[0], [this]() {
        contentBluetoothScreen();
    });
    performFullRefresh(BluetoothScreenText[0], [this]() {
        contentBluetoothScreen();
    });
}

void Display::partialBluetoothScreen() {
    loggValue("Partial refresh", "Bluetooth");
    
    partialRefreshCounter++;
    
    using namespace BluetoothLayout;
    
    // Status box region
    clearPartialRegion(STATUS_BOX_X, STATUS_BOX_Y, STATUS_BOX_W, STATUS_BOX_H);
    redrawPartialRegion(STATUS_BOX_X, STATUS_BOX_Y, STATUS_BOX_W, STATUS_BOX_H, [this]() {
        display.drawRoundRect(STATUS_BOX_X, STATUS_BOX_Y, STATUS_BOX_W, STATUS_BOX_H, STATUS_BOX_RADIUS, GxEPD_BLACK);
        printBLEStatus();
    });
}

// Rendering helpers
void Display::printBLEStatus() {
    using namespace BluetoothLayout;
    
    // Print action status
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(board_config.ble_stats.enabled ? "Yes" : "No");
    display.setCursor(centerText_X(line1), STATUS_LINE1_Y);
    display.print(line1);

    // Print connection status with indicator dots
    String line2 = "Connected: " + String(board_config.ble_stats.connected ? "Yes" : "No");
    display.setCursor(centerText_X(line2), STATUS_LINE2_Y);
    display.print(line2);
    
    // Draw connection indicator
    int dotX2 = centerText_X(line2) - 15;
    if (board_config.ble_stats.connected) {
        display.fillCircle(dotX2, 142, 4, GxEPD_BLACK);
    } else {
        display.drawCircle(dotX2, 142, 4, GxEPD_BLACK);
    }
}

void Display::contentBluetoothScreen() {
    using namespace BluetoothLayout;
    
    // Draw Bluetooth icon (classic Bluetooth symbol with thicker lines)
    int iconCenterX = ICON_CENTER_X;
    int iconCenterY = ICON_CENTER_Y;
    int iconHeight = ICON_HEIGHT;
    int iconWidth = ICON_WIDTH;
    
    // Vertical center line (3 pixels wide)
    display.drawLine(iconCenterX - 1, iconCenterY - iconHeight/2, iconCenterX - 1, iconCenterY + iconHeight/2, GxEPD_BLACK);
    display.drawLine(iconCenterX, iconCenterY - iconHeight/2, iconCenterX, iconCenterY + iconHeight/2, GxEPD_BLACK);
    display.drawLine(iconCenterX + 1, iconCenterY - iconHeight/2, iconCenterX + 1, iconCenterY + iconHeight/2, GxEPD_BLACK);
    
    // Top triangle (right side) - thicker lines
    display.drawLine(iconCenterX, iconCenterY - iconHeight/2, iconCenterX + iconWidth, iconCenterY - 2, GxEPD_BLACK);
    display.drawLine(iconCenterX, iconCenterY - iconHeight/2 + 1, iconCenterX + iconWidth, iconCenterY - 1, GxEPD_BLACK);
    display.drawLine(iconCenterX + iconWidth, iconCenterY - 2, iconCenterX, iconCenterY, GxEPD_BLACK);
    display.drawLine(iconCenterX + iconWidth, iconCenterY - 1, iconCenterX, iconCenterY + 1, GxEPD_BLACK);
    
    // Bottom triangle (right side) - thicker lines
    display.drawLine(iconCenterX, iconCenterY, iconCenterX + iconWidth, iconCenterY + 2, GxEPD_BLACK);
    display.drawLine(iconCenterX, iconCenterY + 1, iconCenterX + iconWidth, iconCenterY + 3, GxEPD_BLACK);
    display.drawLine(iconCenterX + iconWidth, iconCenterY + 2, iconCenterX, iconCenterY + iconHeight/2, GxEPD_BLACK);
    display.drawLine(iconCenterX + iconWidth, iconCenterY + 3, iconCenterX, iconCenterY + iconHeight/2 + 1, GxEPD_BLACK);
    
    // Diagonal cross lines - thicker
    display.drawLine(iconCenterX - iconWidth + 2, iconCenterY - iconHeight/2 + 2, iconCenterX + iconWidth - 2, iconCenterY + iconHeight/2 - 2, GxEPD_BLACK);
    display.drawLine(iconCenterX - iconWidth + 3, iconCenterY - iconHeight/2 + 2, iconCenterX + iconWidth - 1, iconCenterY + iconHeight/2 - 2, GxEPD_BLACK);
    display.drawLine(iconCenterX - iconWidth + 2, iconCenterY + iconHeight/2 - 2, iconCenterX + iconWidth - 2, iconCenterY - iconHeight/2 + 2, GxEPD_BLACK);
    display.drawLine(iconCenterX - iconWidth + 3, iconCenterY + iconHeight/2 - 2, iconCenterX + iconWidth - 1, iconCenterY - iconHeight/2 + 2, GxEPD_BLACK);
    
    
    // Write BLE server name in a box
    display.drawRoundRect(NAME_BOX_X, NAME_BOX_Y, NAME_BOX_W, NAME_BOX_H, NAME_BOX_RADIUS, GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(bleServerName), NAME_TEXT_Y);
    display.print(bleServerName);

    // Status section with decorative box (maximum width)
    display.drawRoundRect(STATUS_BOX_X, STATUS_BOX_Y, STATUS_BOX_W, STATUS_BOX_H, STATUS_BOX_RADIUS, GxEPD_BLACK);
    printBLEStatus();

    // Draw footer divider
    display.drawLine(10, FOOTER_LINE_Y, 190, FOOTER_LINE_Y, GxEPD_BLACK);
    
    // Write instructions
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(BluetoothScreenText[1]), FOOTER_TEXT1_Y);
    display.print(BluetoothScreenText[1]);
    display.setCursor(centerText_X(BluetoothScreenText[2]), FOOTER_TEXT2_Y);
    display.print(BluetoothScreenText[2]);
}

void Display::contentLoadingScreen() {
    using namespace LoadingLayout;
    
    // Draw decorative border - top part
    display.drawLine(BORDER_OUTER, BORDER_OUTER, BORDER_RIGHT_OUTER, BORDER_OUTER, GxEPD_BLACK);
    display.drawLine(BORDER_INNER, BORDER_INNER, BORDER_RIGHT_INNER, BORDER_INNER, GxEPD_BLACK);
    display.drawLine(BORDER_OUTER, BORDER_OUTER, BORDER_OUTER, BORDER_TOP_END, GxEPD_BLACK);
    display.drawLine(BORDER_INNER, BORDER_INNER, BORDER_INNER, BORDER_TOP_END, GxEPD_BLACK);
    display.drawLine(BORDER_RIGHT_OUTER, BORDER_OUTER, BORDER_RIGHT_OUTER, BORDER_TOP_END, GxEPD_BLACK);
    display.drawLine(BORDER_RIGHT_INNER, BORDER_INNER, BORDER_RIGHT_INNER, BORDER_TOP_END, GxEPD_BLACK);
    
    // Draw decorative border - bottom part
    display.drawLine(BORDER_OUTER, BORDER_BOTTOM_START, BORDER_OUTER, BORDER_BOTTOM, GxEPD_BLACK);
    display.drawLine(BORDER_INNER, BORDER_BOTTOM_START, BORDER_INNER, BORDER_BOTTOM_INNER, GxEPD_BLACK);
    display.drawLine(BORDER_RIGHT_OUTER, BORDER_BOTTOM_START, BORDER_RIGHT_OUTER, BORDER_BOTTOM, GxEPD_BLACK);
    display.drawLine(BORDER_RIGHT_INNER, BORDER_BOTTOM_START, BORDER_RIGHT_INNER, BORDER_BOTTOM_INNER, GxEPD_BLACK);
    display.drawLine(BORDER_OUTER, BORDER_BOTTOM, BORDER_RIGHT_OUTER, BORDER_BOTTOM, GxEPD_BLACK);
    display.drawLine(BORDER_INNER, BORDER_BOTTOM_INNER, BORDER_RIGHT_INNER, BORDER_BOTTOM_INNER, GxEPD_BLACK);

    // Write title with custom fonts
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X(LoadingScreenText[0]), TEXT1_Y);
    display.print(LoadingScreenText[0]);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(LoadingScreenText[1]), TEXT2_Y);
    display.print(LoadingScreenText[1]);
}

void Display::contentHeatingScreen() {
    using namespace HeatingLayout;
    
    // Draw heating icon (thermometer with rising bars)
    int iconX = ICON_X;
    int iconY = ICON_Y;
    display.fillRect(iconX, iconY, 8, 30, GxEPD_BLACK);
    display.fillCircle(iconX + 4, iconY + 35, 8, GxEPD_BLACK);
    display.fillCircle(iconX + 4, iconY + 35, 4, GxEPD_WHITE);
    
    // Rising heat bars
    for (int i = 0; i < 3; i++) {
        display.drawLine(iconX + 15 + i * 6, iconY + 20, iconX + 15 + i * 6, iconY + 10, GxEPD_BLACK);
        display.drawLine(iconX + 17 + i * 6, iconY + 25, iconX + 17 + i * 6, iconY + 15, GxEPD_BLACK);
    }

    // Write title with custom fonts
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X(HeatingScreenText[0]), TEXT1_Y);
    display.print(HeatingScreenText[0]);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(HeatingScreenText[1]), TEXT2_Y);
    display.print(HeatingScreenText[1]);
    
    // Progress indicator
    display.drawRoundRect(PROGRESS_X, PROGRESS_Y, PROGRESS_W, PROGRESS_H, PROGRESS_RADIUS, GxEPD_BLACK);
    display.fillRoundRect(PROGRESS_X + 2, PROGRESS_Y + 2, 60, PROGRESS_H - 4, PROGRESS_FILL_RADIUS, GxEPD_BLACK);
}

void Display::contentSendingScreen() {
    using namespace SendingLayout;
    
    // Draw sync/upload icon (cloud with arrow)
    int cloudX = CLOUD_X;
    int cloudY = CLOUD_Y;
    display.drawCircle(cloudX, cloudY, 8, GxEPD_BLACK);
    display.drawCircle(cloudX + 12, cloudY, 8, GxEPD_BLACK);
    display.drawCircle(cloudX + 6, cloudY - 6, 6, GxEPD_BLACK);
    display.fillRect(cloudX - 6, cloudY, 24, 8, GxEPD_BLACK);
    
    // Upload arrow
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 6, cloudY + 30, GxEPD_BLACK);
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 2, cloudY + 20, GxEPD_BLACK);
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 10, cloudY + 20, GxEPD_BLACK);

    // Write title with custom fonts
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(SendingScreenText[0]), TEXT1_Y);
    display.print(SendingScreenText[0]);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(centerText_X(SendingScreenText[1]), TEXT2_Y);
    display.print(SendingScreenText[1]);
    
    // Animated dots
    display.fillCircle(60, DOTS_Y, 3, GxEPD_BLACK);
    display.fillCircle(75, DOTS_Y, 3, GxEPD_BLACK);
    display.fillCircle(90, DOTS_Y, 3, GxEPD_BLACK);
    display.drawCircle(105, DOTS_Y, 3, GxEPD_BLACK);
    display.drawCircle(120, DOTS_Y, 3, GxEPD_BLACK);
}

// Routines for updating the Sensors information screen (full or partial)
void Display::showSensorsScreen() {
    logg("Preparing sensors screen");
    
    if (lastScreenData.previousScreen == SCREEN_MODE::SENSORS &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialSensorsScreen();
    } else {
        fullSensorsScreen();
    }
    logg("Rendered sensors screen");
}

void Display::fullSensorsScreen() {
    loggValue("Full refresh", "Sensors");
    
    performFullRefresh(SensorsScreenText[0], [this]() {
    performFullRefresh(SensorsScreenText[0], [this]() {
        printSensorsStatus();
    });
    });
}

void Display::partialSensorsScreen() {
    loggValue("Partial refresh", "Sensors");
    
    partialRefreshCounter++;
    
    using namespace SensorsLayout;
    
    int dataLines[] = {1, 3, 5, 7};
    
    for (int lineIdx : dataLines) {
        int yPos = START_Y + LINE_SPACING * lineIdx - 3 - (LINE_HEIGHT / 2);
        
        clearPartialRegion(PARTIAL_X, yPos, PARTIAL_W, LINE_HEIGHT);
        redrawPartialRegion(PARTIAL_X, yPos, PARTIAL_W, LINE_HEIGHT, [this]() {
            printSensorsStatus();
        });
    }
}


// Sensors screen data printer
void Display::printSensorsStatus() {
    using namespace SensorsLayout;
    
    // Draw decorative header box (extended downward)
    display.drawRoundRect(BOX_X, BOX_Y, BOX_W, BOX_H, BOX_RADIUS, GxEPD_BLACK);
    
    // Define column positions for 3-column layout (more evenly distributed)
    int boxLeft = BOX_X;
    int boxRight = BOX_X + BOX_W;
    int boxWidth = boxRight - boxLeft;
    int columnWidth = boxWidth / 3;
    
    int col1X = boxLeft + columnWidth / 2;   // Center of first column
    int col2X = boxLeft + columnWidth + columnWidth / 2;  // Center of second column
    int col3X = boxLeft + 2 * columnWidth + columnWidth / 2; // Center of third column
    
    int startY = START_Y;
    int lineSpacing = LINE_SPACING;
    
    display.setFont(&FreeMonoBold9pt7b);
    
    // Helper function to center text in column
    auto centerInColumn = [this](String text, int colCenterX, int y) {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(colCenterX - w / 2, y);
    };
    
    // Line 1: T, H, P labels
    centerInColumn("T", col1X, startY);
    display.print("T");
    centerInColumn("H", col2X, startY);
    display.print("H");
    centerInColumn("P", col3X, startY);
    display.print("P");
    
#if defined(BME_ENABLE) || defined(SHTC3_ENABLE)
    // Line 2: Temperature, Humidity, Pressure values
    String temp =
#ifdef SHTC3_ENABLE
    convertTemperature(lastSensorsData.lastSHTC3Data, false, true);
#else
    convertTemperature(lastSensorsData.lastBMEData, false, true);
#endif
    String hum = convertHumidity(lastSensorsData.lastBMEData, false, true);
    String press = convertPressure(lastSensorsData.lastBMEData, false, true);
    
    centerInColumn(temp, col1X, startY + lineSpacing);
    display.print(temp);
    centerInColumn(hum, col2X, startY + lineSpacing);
    display.print(hum);
    centerInColumn(press, col3X, startY + lineSpacing);
    display.print(press);
#endif
    
    // Line 3: Alt, Bat, CO2 labels
    centerInColumn("Alt", col1X, startY + lineSpacing * 2);
    display.print("Alt");
    centerInColumn("Bat", col2X, startY + lineSpacing * 2);
    display.print("Bat");
    centerInColumn("CO2", col3X, startY + lineSpacing * 2);
    display.print("CO2");
    
    // Line 4: Alt, Battery and CO2 values
#ifdef BME_ENABLE
    String alt = convertAltitude(lastSensorsData.lastBMEData, false, true);
    centerInColumn(alt, col1X, startY + lineSpacing * 3);
    display.print(alt);
#endif
    String bat = convertBattery(lastSensorsData.lastBatteryData, false);
    centerInColumn(bat, col2X, startY + lineSpacing * 3);
    display.print(bat);
#ifdef CO2_ENABLE
    String co2 = convertCO2(lastSensorsData.lastCO2Data, false, true);
    centerInColumn(co2, col3X, startY + lineSpacing * 3);
    display.print(co2);
#endif
    
    // Line 5: PM1, PM2.5, PM10 labels
    centerInColumn("PM1", col1X, startY + lineSpacing * 4);
    display.print("PM1");
    centerInColumn("PM2.5", col2X, startY + lineSpacing * 4);
    display.print("PM2.5");
    centerInColumn("PM10", col3X, startY + lineSpacing * 4);
    display.print("PM10");
    
#ifdef PM_ENABLE
    // Line 6: PM values
    String pm1 = convertPM1(lastSensorsData.lastPMData, false, true);
    String pm25 = convertPM2_5(lastSensorsData.lastPMData, false, true);
    String pm10 = convertPM10(lastSensorsData.lastPMData, false, true);
    
    centerInColumn(pm1, col1X, startY + lineSpacing * 5);
    display.print(pm1);
    centerInColumn(pm25, col2X, startY + lineSpacing * 5);
    display.print(pm25);
    centerInColumn(pm10, col3X, startY + lineSpacing * 5);
    display.print(pm10);
#endif
    
    // Line 7: CO, NO2, NH3 labels
    centerInColumn("CO", col1X, startY + lineSpacing * 6);
    display.print("CO");
    centerInColumn("NO2", col2X, startY + lineSpacing * 6);
    display.print("NO2");
    centerInColumn("NH3", col3X, startY + lineSpacing * 6);
    display.print("NH3");
    
#ifdef MICS_ENABLE
    // Line 8: CO, NO2, NH3 values
    String co = convertCO(lastSensorsData.lastMICSData, false, false);
    String no2 = convertNO2(lastSensorsData.lastMICSData, false, false);
    String nh3 = convertNH3(lastSensorsData.lastMICSData, false, false);
    
    centerInColumn(co, col1X, startY + lineSpacing * 7);
    display.print(co);
    centerInColumn(no2, col2X, startY + lineSpacing * 7);
    display.print(no2);
    centerInColumn(nh3, col3X, startY + lineSpacing * 7);
    display.print(nh3);
#endif
}

// Routines for updating the Ambient information screen (full or partial)
void Display::showAmbientScreen() {
    logg("Preparing ambient screen");

    if (lastScreenData.previousScreen == SCREEN_MODE::AMBIENT &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialAmbientScreen();
    } else {
        fullAmbientScreen();
    }
    logg("Rendered ambient screen");
}

void Display::fullAmbientScreen() {
    loggValue("Full refresh", "Ambient");
    
    performFullRefresh(AmbientScreenText[0], [this]() {
        printAmbientStatus();
    });
}

void Display::partialAmbientScreen() {
    loggValue("Partial refresh", "Ambient");
    
    partialRefreshCounter++;
    
    using namespace AmbientLayout;
    
    // Region 1: Temperature and Humidity data values
    clearPartialRegion(TEMP_HUM_REGION_X, TEMP_HUM_REGION_Y, TEMP_HUM_REGION_W, TEMP_HUM_REGION_H);
    redrawPartialRegion(TEMP_HUM_REGION_X, TEMP_HUM_REGION_Y, TEMP_HUM_REGION_W, TEMP_HUM_REGION_H, [this]() {
        printAmbientStatus();
    });
    
    // Region 2: Pressure and Altitude data values
    clearPartialRegion(PRESS_ALT_REGION_X, PRESS_ALT_REGION_Y, PRESS_ALT_REGION_W, PRESS_ALT_REGION_H);
    redrawPartialRegion(PRESS_ALT_REGION_X, PRESS_ALT_REGION_Y, PRESS_ALT_REGION_W, PRESS_ALT_REGION_H, [this]() {
        printAmbientStatus();
    });
    
    // Region 3: Battery rectangle
    clearPartialRegion(BATTERY_X, BATTERY_Y, BATTERY_W, BATTERY_HEIGHT);
    redrawPartialRegion(BATTERY_X, BATTERY_Y, BATTERY_W, BATTERY_HEIGHT, [this]() {
        display.fillRoundRect(BATTERY_X, BATTERY_Y, BATTERY_W, BATTERY_HEIGHT, BATTERY_RADIUS, GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(BATTERY_TEXT_X, BATTERY_TEXT_Y);
        display.println("Battery: " + convertBattery(lastSensorsData.lastBatteryData));
        display.setTextColor(GxEPD_BLACK);
    });
}


// Ambient screen data printer
void Display::printAmbientStatus() {
    using namespace AmbientLayout;
    
    // Draw decorative header line under title
    display.drawLine(10, HEADER_LINE_Y, 190, HEADER_LINE_Y, GxEPD_BLACK);
    
    // Print line by line ambient data with icons/markers
    display.setFont(&FreeMonoBold9pt7b);
    
    int startY = START_Y;
    int lineSpacing = LINE_SPACING;
    int iconX = ICON_X;
    int textX = TEXT_X;
    
#if defined(BME_ENABLE) || defined(SHTC3_ENABLE)
    // Temperature with thermometer icon
    drawThermometerIcon(iconX, startY);
    // Temperature with thermometer icon
    drawThermometerIcon(iconX, startY);
    display.setCursor(textX, startY);
    display.println("Temp: " +
#ifdef SHTC3_ENABLE
                    convertTemperature(lastSensorsData.lastSHTC3Data)
#else
                    convertTemperature(lastSensorsData.lastBMEData)
#endif
                    );

#ifdef BME_ENABLE
    
    // Humidity with droplet icon
    drawDropletIcon(iconX, startY + lineSpacing);
    // Humidity with droplet icon
    drawDropletIcon(iconX, startY + lineSpacing);
    display.setCursor(textX, startY + lineSpacing);
    display.println("Hum:  " + convertHumidity(lastSensorsData.lastBMEData));
    
    // Altitude with mountain icon
    drawTriangleIcon(iconX, startY + lineSpacing * 3);
    // Altitude with mountain icon
    drawTriangleIcon(iconX, startY + lineSpacing * 3);
    display.setCursor(textX, startY + lineSpacing * 3);
    display.println("Alt: " + convertAltitude(lastSensorsData.lastBMEData));

    // Pressure with gauge icon
    drawGaugeIcon(iconX, startY + lineSpacing * 2);
    // Pressure with gauge icon
    drawGaugeIcon(iconX, startY + lineSpacing * 2);
    display.setCursor(textX, startY + lineSpacing * 2);
    display.println("Pres:" + convertPressure(lastSensorsData.lastBMEData, true, true));
#endif
#endif

    // Draw battery section with filled background
    display.fillRoundRect(BATTERY_X, BATTERY_Y, BATTERY_W, BATTERY_HEIGHT, BATTERY_RADIUS, GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(BATTERY_TEXT_X, BATTERY_TEXT_Y);
    display.println("Battery: " + convertBattery(lastSensorsData.lastBatteryData));
    display.setTextColor(GxEPD_BLACK);
}

// Routines for updating the Pollution information screen (full or partial)
void Display::showPollutionScreen() {
    logg("Preparing pollution screen");

    if (lastScreenData.previousScreen == SCREEN_MODE::POLLUTION &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialPollutionScreen();
    } else {
        fullPollutionScreen();
    }
    logg("Rendered pollution screen");
}

void Display::fullPollutionScreen() {
    loggValue("Full refresh", "Pollution");
    
    performFullRefresh(PollutionScreenText[0], [this]() {
        printPollutionStatus();
    });
}

void Display::partialPollutionScreen() {
    loggValue("Partial refresh", "Pollution");
    
    partialRefreshCounter++;
    
    using namespace PollutionLayout;
    
    // Region 1: Top half (CO2, CO, NO2, NH3)
    clearPartialRegion(TOP_REGION_X, TOP_REGION_Y, TOP_REGION_W, TOP_REGION_H);
    redrawPartialRegion(TOP_REGION_X, TOP_REGION_Y, TOP_REGION_W, TOP_REGION_H, [this]() {
        printPollutionStatus();
    });
    
    // Region 2: Bottom half (PM1, PM2.5, PM10)
    clearPartialRegion(BOTTOM_REGION_X, BOTTOM_REGION_Y, BOTTOM_REGION_W, BOTTOM_REGION_H);
    redrawPartialRegion(BOTTOM_REGION_X, BOTTOM_REGION_Y, BOTTOM_REGION_W, BOTTOM_REGION_H, [this]() {
        printPollutionStatus();
    });
}


// Pollution screen data printer
void Display::printPollutionStatus() {
    using namespace PollutionLayout;
    
    // Draw decorative header line under title
    display.drawLine(10, HEADER_LINE_Y, 190, HEADER_LINE_Y, GxEPD_BLACK);
    
    // Print line by line pollution data with warning boxes
    display.setFont(&FreeMonoBold9pt7b);
    
    int startY = START_Y;
    int lineSpacing = LINE_SPACING;
    int boxX = BOX_X;
    int textX = TEXT_X;
    
    int currentLine = 0;
    
#ifdef CO2_ENABLE
    // CO2 with alert box icon
    drawAlertBoxIcon(boxX, startY + lineSpacing * currentLine);
    drawAlertBoxIcon(boxX, startY + lineSpacing * currentLine);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("CO2: " + convertCO2(lastSensorsData.lastCO2Data));
    currentLine++;
#endif

#ifdef MICS_ENABLE
    // CO with warning circle icon
    drawWarningCircleIcon(boxX, startY + lineSpacing * currentLine);
    // CO with warning circle icon
    drawWarningCircleIcon(boxX, startY + lineSpacing * currentLine);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("CO:  " + convertCO(lastSensorsData.lastMICSData));
    currentLine++;
    
    // NO2 with double box icon
    drawDoubleBoxIcon(boxX, startY + lineSpacing * currentLine);
    // NO2 with double box icon
    drawDoubleBoxIcon(boxX, startY + lineSpacing * currentLine);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("NO2: " + convertNO2(lastSensorsData.lastMICSData));
    currentLine++;
    
    // NH3 with triangle icon
    drawTriangleIcon(boxX + 6, startY + lineSpacing * currentLine);
    // NH3 with triangle icon
    drawTriangleIcon(boxX + 6, startY + lineSpacing * currentLine);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("NH3: " + convertNH3(lastSensorsData.lastMICSData));
    currentLine++;
#endif

#ifdef PM_ENABLE
    // PM1 with small dots
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 1);
    // PM1 with small dots
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 1);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("PM1: " + convertPM1(lastSensorsData.lastPMData));
    currentLine++;
    
    // PM2.5 with medium dots
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 2);
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 2);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("PM25:" + convertPM2_5(lastSensorsData.lastPMData));
    currentLine++;
    
    // PM10 with large dots
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 3);
    // PM10 with large dots
    drawPMDotsIcon(boxX, startY + lineSpacing * currentLine, 3);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("PM10:" + convertPM10(lastSensorsData.lastPMData));
    currentLine++;
#endif
}

// Reusable print method for battery data
void Display::printBatteryData() {
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

// Partial refresh helper: clear a specific region
void Display::clearPartialRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do {
        display.fillRect(x, y, w, h, GxEPD_WHITE);
    } while (display.nextPage());
}

// Partial refresh helper: redraw content in a specific region
void Display::redrawPartialRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h, std::function<void()> drawFunc) {
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do {
        display.fillRect(x, y, w, h, GxEPD_WHITE);
        drawFunc();
    } while (display.nextPage());
}

// Full refresh helper: render entire screen with title and content
void Display::performFullRefresh(const String& title, std::function<void()> contentFunc) {
    partialRefreshCounter = 0;
    
    display.setFullWindow();
    display.firstPage();
    
    do {
        display.fillScreen(GxEPD_WHITE);
        
        if (title.length() > 0) {
            display.setFont(&FreeMonoBold18pt7b);
            display.setCursor(centerText_X(title), 30);
            display.print(title);
        }
        
        contentFunc();
    } while (display.nextPage());
}

// Icon drawing helpers
void Display::drawThermometerIcon(int16_t x, int16_t y) {
    display.fillRect(x, y - 13, 3, 12, GxEPD_BLACK);
    display.fillCircle(x + 1, y + 1, 3, GxEPD_BLACK);
}

void Display::drawDropletIcon(int16_t x, int16_t y) {
    display.fillCircle(x + 1, y - 4, 4, GxEPD_BLACK);
}

void Display::drawGaugeIcon(int16_t x, int16_t y) {
    display.drawCircle(x + 1, y - 4, 5, GxEPD_BLACK);
    display.drawLine(x + 1, y - 4, x + 4, y - 7, GxEPD_BLACK);
}

void Display::drawTriangleIcon(int16_t x, int16_t y) {
    display.drawLine(x + 1, y - 5, x - 4, y, GxEPD_BLACK);
    display.drawLine(x + 1, y - 5, x + 6, y, GxEPD_BLACK);
    display.drawLine(x - 4, y, x + 6, y, GxEPD_BLACK);
}

void Display::drawAlertBoxIcon(int16_t x, int16_t y) {
    display.drawRect(x, y - 10, 12, 12, GxEPD_BLACK);
}

void Display::drawWarningCircleIcon(int16_t x, int16_t y) {
    display.fillCircle(x + 6, y - 4, 5, GxEPD_BLACK);
    display.fillCircle(x + 6, y - 4, 2, GxEPD_WHITE);
}

void Display::drawDoubleBoxIcon(int16_t x, int16_t y) {
    display.drawRect(x, y - 10, 5, 12, GxEPD_BLACK);
    display.drawRect(x + 7, y - 10, 5, 12, GxEPD_BLACK);
}

void Display::drawPMDotsIcon(int16_t x, int16_t y, uint8_t size) {
    // Size: 1=small, 2=medium, 3=large
    if (size == 1) {
        display.fillCircle(x + 3, y - 6, 2, GxEPD_BLACK);
        display.fillCircle(x + 9, y - 2, 2, GxEPD_BLACK);
    } else if (size == 2) {
        display.fillCircle(x + 3, y - 6, 3, GxEPD_BLACK);
        display.fillCircle(x + 9, y - 2, 2, GxEPD_BLACK);
    } else {
        display.fillCircle(x + 3, y - 6, 4, GxEPD_BLACK);
        display.fillCircle(x + 10, y - 2, 3, GxEPD_BLACK);
    }
}