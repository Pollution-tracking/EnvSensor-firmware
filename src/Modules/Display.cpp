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
            setScreenMode(SCREEN_MODE::ENVIRONMENTAL);
            break;
        case SCREEN_MODE::ENVIRONMENTAL:
            setScreenMode(SCREEN_MODE::POLLUTANTS);
            break;
        case SCREEN_MODE::POLLUTANTS:
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
            setScreenMode(SCREEN_MODE::POLLUTANTS);
            break;
        case SCREEN_MODE::SENSORS:
            setScreenMode(SCREEN_MODE::BLUETOOTH);
            break;
        case SCREEN_MODE::ENVIRONMENTAL:
            setScreenMode(SCREEN_MODE::SENSORS);
            break;
        case SCREEN_MODE::POLLUTANTS:
            setScreenMode(SCREEN_MODE::ENVIRONMENTAL);
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
            } else if (lastScreenData.currentScreen == SCREEN_MODE::ENVIRONMENTAL) {
                setScreenMode(SCREEN_MODE::ENVIRONMENTAL);
            } else if (lastScreenData.currentScreen == SCREEN_MODE::POLLUTANTS) {
                setScreenMode(SCREEN_MODE::POLLUTANTS);
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
        case SCREEN_MODE::ENVIRONMENTAL:
            showEnvironmentalScreen();
            break;
        case SCREEN_MODE::POLLUTANTS:
            showPollutantsScreen();
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

    // Draw decorative border - top part
    display.drawLine(10, 10, 190, 10, GxEPD_BLACK); // top
    display.drawLine(13, 13, 187, 13, GxEPD_BLACK); // top inner
    display.drawLine(10, 10, 10, 50, GxEPD_BLACK); // left top
    display.drawLine(13, 13, 13, 50, GxEPD_BLACK); // left top inner
    display.drawLine(190, 10, 190, 50, GxEPD_BLACK); // right top
    display.drawLine(187, 13, 187, 50, GxEPD_BLACK); // right top inner
    
    // Draw decorative border - bottom part
    display.drawLine(10, 145, 10, 190, GxEPD_BLACK); // left bottom
    display.drawLine(13, 145, 13, 187, GxEPD_BLACK); // left bottom inner
    display.drawLine(190, 145, 190, 190, GxEPD_BLACK); // right bottom
    display.drawLine(187, 145, 187, 187, GxEPD_BLACK); // right bottom inner
    display.drawLine(10, 190, 190, 190, GxEPD_BLACK); // bottom
    display.drawLine(13, 187, 187, 187, GxEPD_BLACK); // bottom inner

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

    // Draw heating icon (thermometer with rising bars)
    int iconX = 90;
    int iconY = 30;
    display.fillRect(iconX, iconY, 8, 30, GxEPD_BLACK);
    display.fillCircle(iconX + 4, iconY + 35, 8, GxEPD_BLACK);
    display.fillCircle(iconX + 4, iconY + 35, 4, GxEPD_WHITE);
    
    // Rising heat bars
    for (int i = 0; i < 3; i++) {
        display.drawLine(iconX + 15 + i * 6, iconY + 20, iconX + 15 + i * 6, iconY + 10, GxEPD_BLACK);
        display.drawLine(iconX + 17 + i * 6, iconY + 25, iconX + 17 + i * 6, iconY + 15, GxEPD_BLACK);
    }

    // Write title
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X(HeatingScreenText[0]), 110);
    display.print(HeatingScreenText[0]);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(HeatingScreenText[1]), 145);
    display.print(HeatingScreenText[1]);
    
    // Progress indicator
    display.drawRoundRect(40, 165, 120, 12, 6, GxEPD_BLACK);
    display.fillRoundRect(42, 167, 60, 8, 4, GxEPD_BLACK);

    display.display(false);
    logg("Rendered heating screen");
}

// Routine to show the sending historical data screen
void Display::showSendingScreen() {
    logg("Preparing sending screen");
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Draw sync/upload icon (cloud with arrow)
    int cloudX = 85;
    int cloudY = 50;
    display.drawCircle(cloudX, cloudY, 8, GxEPD_BLACK);
    display.drawCircle(cloudX + 12, cloudY, 8, GxEPD_BLACK);
    display.drawCircle(cloudX + 6, cloudY - 6, 6, GxEPD_BLACK);
    display.fillRect(cloudX - 6, cloudY, 24, 8, GxEPD_BLACK);
    
    // Upload arrow
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 6, cloudY + 30, GxEPD_BLACK);
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 2, cloudY + 20, GxEPD_BLACK);
    display.drawLine(cloudX + 6, cloudY + 15, cloudX + 10, cloudY + 20, GxEPD_BLACK);

    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(SendingScreenText[0]), 100);
    display.print(SendingScreenText[0]);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(centerText_X(SendingScreenText[1]), 130);
    display.print(SendingScreenText[1]);
    
    // Animated dots
    display.fillCircle(60, 155, 3, GxEPD_BLACK);
    display.fillCircle(75, 155, 3, GxEPD_BLACK);
    display.fillCircle(90, 155, 3, GxEPD_BLACK);
    display.drawCircle(105, 155, 3, GxEPD_BLACK);
    display.drawCircle(120, 155, 3, GxEPD_BLACK);

    display.display(false);
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
    
    // Reset counter on full refresh
    partialRefreshCounter = 0;
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    contentBluetoothScreen();

    display.display(false);
}

void Display::partialBluetoothScreen() {
    loggValue("Partial refresh", "Bluetooth");
    
    partialRefreshCounter++;
    
    // Define the region that changes: status box area (Enabled/Connected text)
    // x: 3, y: 105, width: 194, height: 50
    display.setPartialWindow(3, 105, 194, 50);
    
    // First pass: clear the region
    display.firstPage();
    do {
        display.fillRect(3, 105, 194, 50, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw content
    display.firstPage();
    do {
        display.fillRect(3, 105, 194, 50, GxEPD_WHITE);
        display.drawRoundRect(3, 105, 194, 50, 5, GxEPD_BLACK);
        printBLEStatus();
    } while (display.nextPage());
}

// Rendering helpers
void Display::printBLEStatus() {
    // Print action status
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(board_config.ble_stats.enabled ? "Yes" : "No");
    display.setCursor(centerText_X(line1), 125);
    display.print(line1);

    // Print connection status with indicator dots
    String line2 = "Connected: " + String(board_config.ble_stats.connected ? "Yes" : "No");
    display.setCursor(centerText_X(line2), 147);
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
    // Draw Bluetooth icon (classic Bluetooth symbol with thicker lines)
    int iconCenterX = 100;
    int iconCenterY = 53;
    int iconHeight = 16;
    int iconWidth = 8;
    
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
    
    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X(BluetoothScreenText[0]), 30);
    display.print(BluetoothScreenText[0]);

    // Write BLE server name in a box
    display.drawRoundRect(15, 70, 170, 25, 5, GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(bleServerName), 88);
    display.print(bleServerName);

    // Status section with decorative box (maximum width)
    display.drawRoundRect(3, 105, 194, 50, 5, GxEPD_BLACK);
    printBLEStatus();

    // Draw footer divider
    display.drawLine(10, 160, 190, 160, GxEPD_BLACK);
    
    // Write instructions
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(BluetoothScreenText[1]), 177);
    display.print(BluetoothScreenText[1]);
    display.setCursor(centerText_X(BluetoothScreenText[2]), 193);
    display.print(BluetoothScreenText[2]);
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
    
    // Reset counter on full refresh
    partialRefreshCounter = 0;

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
    
    partialRefreshCounter++;
    
    // Refresh only the 4 data value lines (not labels)
    // Line 2: Temperature, Humidity, Pressure values (y ≈ 75)
    // Line 4: Altitude, Battery, CO2 values (y ≈ 115) 
    // Line 6: PM1, PM2.5, PM10 values (y ≈ 155)
    // Line 8: CO, NO2, NH3 values (y ≈ 195)
    
    int startY = 55;
    int lineSpacing = 20;
    int lineHeight = 18;
    
    // Define 4 narrow horizontal strips for data lines only
    int dataLines[] = {1, 3, 5, 7};
    
    for (int lineIdx : dataLines) {
        // Lift region up by half its height to cover correctly
        int yPos = startY + lineSpacing * lineIdx - 3 - (lineHeight / 2);
        
        // Set partial window for this data line
        display.setPartialWindow(10, yPos, 180, lineHeight);
        
        // First pass: clear
        display.firstPage();
        do {
            display.fillRect(10, yPos, 180, lineHeight, GxEPD_WHITE);
        } while (display.nextPage());
        
        // Second pass: redraw content
        display.firstPage();
        do {
            display.fillRect(10, yPos, 180, lineHeight, GxEPD_WHITE);
            printSensorsStatus();
        } while (display.nextPage());
    }
}

// Sensors screen data printer
void Display::printSensorsStatus() {
    // Draw decorative header box (extended downward)
    display.drawRoundRect(5, 35, 190, 165, 8, GxEPD_BLACK);
    
    // Define column positions for 3-column layout (more evenly distributed)
    int boxLeft = 5;
    int boxRight = 195;
    int boxWidth = boxRight - boxLeft;
    int columnWidth = boxWidth / 3;
    
    int col1X = boxLeft + columnWidth / 2;   // Center of first column
    int col2X = boxLeft + columnWidth + columnWidth / 2;  // Center of second column
    int col3X = boxLeft + 2 * columnWidth + columnWidth / 2; // Center of third column
    
    int startY = 55;
    int lineSpacing = 20;
    
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
    
#ifdef BME_ENABLE
    // Line 2: Temperature, Humidity, Pressure values
    String temp = convertTemperature(lastSensorsData.lastBMEData, false, true);
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
    String co = convertCO(lastSensorsData.lastMICSData, false, true);
    String no2 = convertNO2(lastSensorsData.lastMICSData, false, true);
    String nh3 = convertNH3(lastSensorsData.lastMICSData, false, true);
    
    centerInColumn(co, col1X, startY + lineSpacing * 7);
    display.print(co);
    centerInColumn(no2, col2X, startY + lineSpacing * 7);
    display.print(no2);
    centerInColumn(nh3, col3X, startY + lineSpacing * 7);
    display.print(nh3);
#endif
}

// Routines for updating the Environmental information screen (full or partial)
void Display::showEnvironmentalScreen() {
    logg("Preparing environmental screen");

    if (lastScreenData.previousScreen == SCREEN_MODE::ENVIRONMENTAL &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialEnvironmentalScreen();
    } else {
        fullEnvironmentalScreen();
    }
    logg("Rendered environmental screen");
}

void Display::fullEnvironmentalScreen() {
    loggValue("Full refresh", "Ambient");
    
    // Reset counter on full refresh
    partialRefreshCounter = 0;

    display.setFullWindow();
    display.firstPage();

    do {
        // Write title
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X(EnvironmentalScreenText[0]), 30);
        display.print(EnvironmentalScreenText[0]);

        printEnvironmentalStatus();
    } while (display.nextPage());
}

void Display::partialEnvironmentalScreen() {
    loggValue("Partial refresh", "Ambient");
    
    partialRefreshCounter++;
    
    // Region 1: Temperature and Humidity data values only
    display.setPartialWindow(90, 55, 110, 45);
    
    // First pass: clear
    display.firstPage();
    do {
        display.fillRect(90, 55, 110, 45, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw
    display.firstPage();
    do {
        display.fillRect(90, 55, 110, 45, GxEPD_WHITE);
        printEnvironmentalStatus();
    } while (display.nextPage());
    
    // Region 2: Pressure and Altitude data values only
    display.setPartialWindow(80, 100, 100, 45);
    
    // First pass: clear
    display.firstPage();
    do {
        display.fillRect(80, 100, 100, 45, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw
    display.firstPage();
    do {
        display.fillRect(80, 100, 100, 45, GxEPD_WHITE);
        printEnvironmentalStatus();
    } while (display.nextPage());
    
    // Region 3: Battery rectangle
    display.setPartialWindow(5, 175, 190, 21);
    
    // First pass: clear
    display.firstPage();
    do {
        display.fillRect(5, 175, 190, 21, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw battery section
    display.firstPage();
    do {
        // Redraw battery section
        display.fillRoundRect(5, 175, 190, 21, 5, GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(15, 190);
        display.println("Battery: " + convertBattery(lastSensorsData.lastBatteryData));
        display.setTextColor(GxEPD_BLACK);
    } while (display.nextPage());
}

// Environmental screen data printer
void Display::printEnvironmentalStatus() {
    // Draw decorative header line under title
    display.drawLine(10, 40, 190, 40, GxEPD_BLACK);
    
    // Print line by line environmental data with icons/markers
    display.setFont(&FreeMonoBold9pt7b);
    
    int startY = 65;
    int lineSpacing = 25;
    int iconX = 10;
    int textX = 25;  // Closer to icons (was 30)
    
#ifdef BME_ENABLE
    // Temperature with thermometer icon (simple box)
    display.fillRect(iconX, startY - 13, 3, 12, GxEPD_BLACK);
    display.fillCircle(iconX + 1, startY + 1, 3, GxEPD_BLACK);
    display.setCursor(textX, startY);
    display.println("Temp: " + convertTemperature(lastSensorsData.lastBMEData));
    
    // Humidity with droplet shape (circle)
    display.fillCircle(iconX + 1, startY + lineSpacing - 4, 4, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing);
    display.println("Hum:  " + convertHumidity(lastSensorsData.lastBMEData));
    
    // Altitude with mountain icon (triangle) - centered with text
    display.drawLine(iconX + 1, startY + lineSpacing * 3 - 5, iconX - 4, startY + lineSpacing * 3, GxEPD_BLACK);
    display.drawLine(iconX + 1, startY + lineSpacing * 3 - 5, iconX + 6, startY + lineSpacing * 3, GxEPD_BLACK);
    display.drawLine(iconX - 4, startY + lineSpacing * 3, iconX + 6, startY + lineSpacing * 3, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * 3);
    display.println("Alt: " + convertAltitude(lastSensorsData.lastBMEData));

    // Pressure with gauge icon (arc shape using lines)
    display.drawCircle(iconX + 1, startY + lineSpacing * 2 - 4, 5, GxEPD_BLACK);
    display.drawLine(iconX + 1, startY + lineSpacing * 2 - 4, iconX + 4, startY + lineSpacing * 2 - 7, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * 2);
    display.println("Pres:" + convertPressure(lastSensorsData.lastBMEData, true, true));
#endif

    // Draw battery section with filled background
    display.fillRoundRect(5, 175, 190, 21, 5, GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(15, 190);
    display.println("Battery: " + convertBattery(lastSensorsData.lastBatteryData));
    display.setTextColor(GxEPD_BLACK);
}

// Routines for updating the Pollutants information screen (full or partial)
void Display::showPollutantsScreen() {
    logg("Preparing pollutants screen");

    if (lastScreenData.previousScreen == SCREEN_MODE::POLLUTANTS &&
        partialRefreshCounter < FULL_REFRESH_INTERVAL) {
        partialPollutantsScreen();
    } else {
        fullPollutantsScreen();
    }
    logg("Rendered pollutants screen");
}

void Display::fullPollutantsScreen() {
    loggValue("Full refresh", "Pollution");
    
    // Reset counter on full refresh
    partialRefreshCounter = 0;

    display.setFullWindow();
    display.firstPage();

    do {
        // Write title
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X(PollutantsScreenText[0]), 30);
        display.print(PollutantsScreenText[0]);

        printPollutantsStatus();
    } while (display.nextPage());
}

void Display::partialPollutantsScreen() {
    loggValue("Partial refresh", "Pollution");
    
    partialRefreshCounter++;
    
    // Split into 2 regions to reduce greying
    // Region 1: Top half of pollutants data (CO2, CO, NO2, NH3)
    display.setPartialWindow(80, 50, 170, 80);
    
    // First pass: clear
    display.firstPage();
    do {
        display.fillRect(80, 50, 170, 80, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw
    display.firstPage();
    do {
        display.fillRect(80, 50, 170, 80, GxEPD_WHITE);
        printPollutantsStatus();
    } while (display.nextPage());
    
    // Region 2: Bottom half of pollutants data (PM1, PM2.5, PM10)
    display.setPartialWindow(80, 130, 170, 70);
    
    // First pass: clear
    display.firstPage();
    do {
        display.fillRect(80, 130, 170, 70, GxEPD_WHITE);
    } while (display.nextPage());
    
    // Second pass: redraw
    display.firstPage();
    do {
        display.fillRect(80, 130, 170, 70, GxEPD_WHITE);
        printPollutantsStatus();
    } while (display.nextPage());
}

// Pollutants screen data printer
void Display::printPollutantsStatus() {
    // Draw decorative header line under title
    display.drawLine(10, 40, 190, 40, GxEPD_BLACK);
    
    // Print line by line pollutants data with warning boxes
    display.setFont(&FreeMonoBold9pt7b);
    
    int startY = 60;
    int lineSpacing = 22;
    int boxX = 8;
    int textX = 30;
    
    int currentLine = 0;
    
#ifdef CO2_ENABLE
    // CO2 with alert box icon
    display.drawRect(boxX, startY + lineSpacing * currentLine - 10, 12, 12, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("CO2: " + convertCO2(lastSensorsData.lastCO2Data));
    currentLine++;
#endif

#ifdef MICS_ENABLE
    // CO with filled circle
    display.fillCircle(boxX + 6, startY + lineSpacing * currentLine - 4, 5, GxEPD_BLACK);
    display.fillCircle(boxX + 6, startY + lineSpacing * currentLine - 4, 2, GxEPD_WHITE);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("CO:  " + convertCO(lastSensorsData.lastMICSData));
    currentLine++;
    
    // NO2 with double box
    display.drawRect(boxX, startY + lineSpacing * currentLine - 10, 5, 12, GxEPD_BLACK);
    display.drawRect(boxX + 7, startY + lineSpacing * currentLine - 10, 5, 12, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("NO2: " + convertNO2(lastSensorsData.lastMICSData));
    currentLine++;
    
    // NH3 with triangle
    display.drawLine(boxX + 6, startY + lineSpacing * currentLine - 10, boxX + 1, startY + lineSpacing * currentLine + 2, GxEPD_BLACK);
    display.drawLine(boxX + 6, startY + lineSpacing * currentLine - 10, boxX + 11, startY + lineSpacing * currentLine + 2, GxEPD_BLACK);
    display.drawLine(boxX + 1, startY + lineSpacing * currentLine + 2, boxX + 11, startY + lineSpacing * currentLine + 2, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("NH3: " + convertNH3(lastSensorsData.lastMICSData));
    currentLine++;
#endif

#ifdef PM_ENABLE
    // PM1 with small dots pattern
    display.fillCircle(boxX + 3, startY + lineSpacing * currentLine - 6, 2, GxEPD_BLACK);
    display.fillCircle(boxX + 9, startY + lineSpacing * currentLine - 2, 2, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("PM1: " + convertPM1(lastSensorsData.lastPMData));
    currentLine++;
    
    // PM2.5 with medium dots
    display.fillCircle(boxX + 3, startY + lineSpacing * currentLine - 6, 3, GxEPD_BLACK);
    display.fillCircle(boxX + 9, startY + lineSpacing * currentLine - 2, 2, GxEPD_BLACK);
    display.setCursor(textX, startY + lineSpacing * currentLine);
    display.println("PM25:" + convertPM2_5(lastSensorsData.lastPMData));
    currentLine++;
    
    // PM10 with larger dots
    display.fillCircle(boxX + 3, startY + lineSpacing * currentLine - 6, 4, GxEPD_BLACK);
    display.fillCircle(boxX + 10, startY + lineSpacing * currentLine - 2, 3, GxEPD_BLACK);
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