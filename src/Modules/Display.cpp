#include "Modules/Display.h"

#define logg(message) loggWithBase(message, "DISPLAY")
#define loggWithContext(message, context) loggWithContext(message, context, "DISPLAY")

// Construct display
Display::Display() : display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
    // Set initial screen mode
    if (currScreenMode == SCREENMODE::NO_SCREEN) {
        currScreenMode = SCREENMODE::SENSORS;
    }
}

// Destruct display
Display::~Display() { }

// Routine to initialize display
void Display::init() {
    this->display.init(0, true, 2, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    this->display.setRotation(2);
    this->display.setTextColor(GxEPD_BLACK);

    logg("Initialized");
}

// Routine to show the loading screen
void Display::showLoadingScreen() {
    logg("Loading screen");
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(centerText_X("Sensors"), 90);
    display.print("Sensors");
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X("heating"), 130);
    display.print("heating");

    display.display(false);
}

// Routine to change screen to the right
void Display::changeScreenRight() {
    prevScreenMode = currScreenMode;
    if (currScreenMode == SCREENMODE::SENSORS) {
        currScreenMode = SCREENMODE::BLUETOOTH;
    } else {
        currScreenMode = currScreenMode >> 1;
    }
}

// Routine to change screen to the left
void Display::changeScreenLeft() {
    prevScreenMode = currScreenMode;
    if (currScreenMode == SCREENMODE::BLUETOOTH) {
        currScreenMode = SCREENMODE::SENSORS;
    } else {
        currScreenMode = currScreenMode << 1;
    }
}

// Routine to update a certain page of the display
void Display::updateScreen(SCREENUPDATE update) {
    if (update == SCREENUPDATE::GENERAL || update == SCREENUPDATE::SENSORS) {
        checkSensorsScreen();
    }
    
    if (update == SCREENUPDATE::GENERAL || update == SCREENUPDATE::BLUETOOTH) {
        checkBluetoothScreen();
    }
}

// Routines to check if the current screen needs to be refreshed with the updates received
void Display::checkSensorsScreen() {
    if (currScreenMode == SCREENMODE::SENSORS) {
        updateSensorsScreen();
        prevScreenMode = SCREENMODE::SENSORS;
        display.powerOff();
    }
}

void Display::checkBluetoothScreen() {
    if (currScreenMode == SCREENMODE::BLUETOOTH) {
        updateBluetoothScreen();
        prevScreenMode = SCREENMODE::BLUETOOTH;
        display.powerOff();
    }
}

// Routines for updating the Bluetooth information screen (full or partial)
void Display::updateBluetoothScreen() {
    loggWithContext("Updating screen", "Bluetooth");

    if (prevScreenMode == SCREENMODE::BLUETOOTH) {
        partialBluetoothScreen();
    } else {
        fullBluetoothScreen();
    }
}

void Display::fullBluetoothScreen() {
    logg("Full refresh");
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X("Bluetooth"), 30);
    display.print("Bluetooth");

    // Write BLE server name
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X(bleServerName), 55);
    display.print(bleServerName);

    printBLEStatus();

    // Write instructions
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(centerText_X("Press button to"), 170);
    display.print("Press button to");
    display.setCursor(centerText_X("toggle BLE"), 190);
    display.print("toggle BLE");

    display.display(false);
}

void Display::partialBluetoothScreen() {
    logg("Partial refresh");
    
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do {
        // Cover previous text
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    do {
        // Write title
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X("Bluetooth"), 30);
        display.print("Bluetooth");

        // Write BLE server name
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(centerText_X(bleServerName), 55);
        display.print(bleServerName);

         // Print new text
        printBLEStatus();
        
        // Write instructions
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(centerText_X("Press button to"), 170);
        display.print("Press button to");
        display.setCursor(centerText_X("toggle BLE"), 190);
        display.print("toggle BLE");
    } while (display.nextPage());
}

// Routines for updating the Sensors information screen (full or partial)
void Display::updateSensorsScreen() {
    loggWithContext("Updating screen", "Sensors");

    if (prevScreenMode == SCREENMODE::SENSORS) {
        partialSensorsScreen();
    } else {
        fullSensorsScreen();
    }
}

void Display::fullSensorsScreen() {
    logg("Full refresh");

    display.setFullWindow();
    display.firstPage();

    do {
        // Write title
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X("Sensors"), 30);
        display.print("Sensors");

        printSensorsStatus();
    } while (display.nextPage());
}

void Display::partialSensorsScreen() {
    logg("Partial refresh");
    
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.firstPage();
    do {
        // Cover previous text
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    do {
        // Re-print title
        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(centerText_X("Sensors"), 30);
        display.print("Sensors");
        // Print new text
        printSensorsStatus();
    } while (display.nextPage());
}

// Routine to center text on the X axis
uint16_t Display::centerText_X(String text) {
    int16_t x_text, y_text;
    uint16_t w_text, h_text, x_centered, y_centered;

    display.getTextBounds(text, 0, 0, &x_text, &y_text, &w_text, &h_text);
    x_centered = ((display.width() - w_text) / 2) - x_text;

    return x_centered;
}

// Decodes the sensor's data to be displayed
void Display::updateSensorsStats(String *data) {
    for (int i = 0; i < NR_VALUES; i++) {
        convertData(i, data[i]);
    }
}

// Routine to get screen mode
uint8_t Display::getScreenMode() {
    return currScreenMode;
}

// Sensors screen data printer
void Display::printSensorsStatus() {
    // Print line by line sensor data
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 50);
    String line1 = "Temperature: " + String(_temperature);
    display.println(line1);
    String line2 = "Humidity: " + String(_humidity);
    display.println(line2);
    String line3 = "Pressure: " + String(_pressure);
    display.println(line3);
    String line4 = "Altitude: " + String(_altitude);
    display.println(line4);
    String line5 = "CO2: " + String(_co2);
    display.println(line5);
    String line6 = "PM1: " + String(_pm1);
    display.println(line6);
    String line7 = "PM2.5: " + String(_pm2_5);
    display.println(line7);
    String line8 = "PM10: " + String(_pm10);
    display.println(line8);

    // Print battery voltage on the bottom right corner
    display.setFont(&FreeMono9pt7b);
    display.setCursor(40, 195);
    display.println("Battery: " + String(_battery));
}

// BLE screen data printer
void Display::printBLEStatus() {
    // Print action status
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(bleEnabled ? "Yes" : "No");
    display.setCursor(centerText_X(line1), 90);
    display.print(line1);

    // Print connection status
    String line2 = "Connected: " + String(bleConnected ? "Yes" : "No");
    display.setCursor(centerText_X(line2), 120);
    display.print(line2);
}