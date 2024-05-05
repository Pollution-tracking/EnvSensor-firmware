#include "Modules/Display.h"

extern RTC_DATA_ATTR bool bleEnabled;
extern RTC_DATA_ATTR bool bleConnected;
extern RTC_DATA_ATTR uint8_t currScreenMode;
extern RTC_DATA_ATTR uint8_t prevScreenMode;
extern RTC_DATA_ATTR char _temperature[12];
extern RTC_DATA_ATTR char _humidity[12];
extern RTC_DATA_ATTR char _pressure[12];
extern RTC_DATA_ATTR char _altitude[12];
extern RTC_DATA_ATTR char _co2[12];
extern RTC_DATA_ATTR char _pm1[12];
extern RTC_DATA_ATTR char _pm2_5[12];
extern RTC_DATA_ATTR char _pm10[12];

// Construct display
Display::Display() : display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
    // Set screen mode
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
}

void Display::updateSensorsStats(String *data) {
    for (int i = 0; i < NR_VALUES; i++) {
        convertData(i, data[i]);
    }
}

void Display::convertData(int idx, String data) {
    switch (idx) {
        case BME_TEMPERATURE_INDEX:
            convertTemperature(data);
            break;
        case BME_HUMIDITY_INDEX:
            convertHumidity(data);
            break;
        case BME_PRESSURE_INDEX:
            convertPressure(data);
            break;
        case BME_ALTITUDE_INDEX:
            convertAltitude(data);
            break;
        case CO2_CO2_INDEX:
            convertCO2(data);
            break;
        case PM_PM1_INDEX:
            convertPM1(data);
            break;
        case PM_PM2_5_INDEX:
            convertPM2_5(data);
            break;
        case PM_PM10_INDEX:
            convertPM10(data);
            break;
        default:
            break;
    }
}

void Display::updateBluetoothStats(bool enabled, bool connected) {
    bleEnabled = enabled;
    bleConnected = connected;
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

// Routine to get screen mode
uint8_t Display::getScreenMode() {
    return currScreenMode;
}

void Display::updateScreen(SCREENUPDATE update) {
    if (update == SCREENUPDATE::GENERAL || update == SCREENUPDATE::SENSORS) {
        checkSensorsScreen();
    }
    
    if (update == SCREENUPDATE::GENERAL || update == SCREENUPDATE::BLUETOOTH) {
        checkBluetoothScreen();
    }
}

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

void Display::updateBluetoothScreen() {
    logg("[Display] Updating bluetooth screen");

    if (prevScreenMode == SCREENMODE::BLUETOOTH) {
        partialBluetoothScreen();
    } else {
        fullBluetoothScreen();
    }
}

void Display::fullBluetoothScreen() {
    logg("-> Full bluetooth screen");
    
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);

    // Write title
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(centerText_X("Bluetooth"), 30);
    display.print("Bluetooth");

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
    logg("-> Partial bluetooth screen");
    
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

void Display::updateSensorsScreen() {
    logg("[Display] Updating sensors screen");

    if (prevScreenMode == SCREENMODE::SENSORS) {
        partialSensorsScreen();
    } else {
        fullSensorsScreen();
    }
}

void Display::fullSensorsScreen() {
    logg("-> Full sensors screen");

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
    logg("-> Partial sensors screen");
    
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

void Display::convertTemperature(String data) {
    int temp_val = data.toInt();

    switch (temp_val) {
        case READ_ERROR:
            strcpy(_temperature, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_temperature, "sensor err\0");
            break;
        default:
            strcpy(_temperature, String(String(temp_val) + " C").c_str());
            break;
    }
}

void Display::convertHumidity(String data) {
    int humidity_val = data.toInt();

    switch (humidity_val) {
        case READ_ERROR:
            strcpy(_humidity, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_humidity, "sensor err\0");
            break;
        default:
            strcpy(_humidity, String(String(humidity_val) + " %").c_str());
            break;
    }
}

void Display::convertPressure(String data) {
    int pressure_val = data.toInt();

    switch (pressure_val) {
        case READ_ERROR:
            strcpy(_pressure, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pressure, "sensor err\0");
            break;
        default:
            strcpy(_pressure, String(String(pressure_val) + " hPa").c_str());
            break;
    }
}

void Display::convertAltitude(String data) {
    int altitude_val = data.toInt();

    switch (altitude_val) {
        case READ_ERROR:
            strcpy(_altitude, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_altitude, "sensor err\0");
            break;
        default:
            strcpy(_altitude, String(String(altitude_val) + " m").c_str());
            break;
    }
}

void Display::convertCO2(String data) {
    int co2_val = data.toInt();

    switch (co2_val) {
        case READ_ERROR:
            strcpy(_co2, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_co2, "sensor err\0");
            break;
        default:
            strcpy(_co2, String(String(co2_val) + " ppm").c_str());
            break;
    }
}

void Display::convertPM1(String data) {
    int pm1_val = data.toInt();

    switch (pm1_val) {
        case READ_ERROR:
            strcpy(_pm1, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm1, "sensor err\0");
            break;
        default:
            strcpy(_pm1, String(String(pm1_val) + " ug/m3").c_str());
            break;
    }
}

void Display::convertPM2_5(String data) {
    int pm2_5_val = data.toInt();

    switch (pm2_5_val) {
        case READ_ERROR:
            strcpy(_pm2_5, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm2_5, "sensor err\0");
            break;
        default:
            strcpy(_pm2_5, String(String(pm2_5_val) + " ug/m3").c_str());
            break;
    }
}

void Display::convertPM10(String data) {
    int pm10_val = data.toInt();

    switch (pm10_val) {
        case READ_ERROR:
            strcpy(_pm10, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm10, "sensor err\0");
            break;
        default:
            strcpy(_pm10, String(String(pm10_val) + " ug/m3").c_str());
            break;
    }
}
uint16_t Display::centerText_X(String text) {
    int16_t x_text, y_text;
    uint16_t w_text, h_text, x_centered, y_centered;

    display.getTextBounds(text, 0, 0, &x_text, &y_text, &w_text, &h_text);
    x_centered = ((display.width() - w_text) / 2) - x_text;

    return x_centered;
}

void Display::printSensorsStatus() {
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
}

void Display::printBLEStatus() {
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(bleEnabled ? "Yes" : "No");
    display.setCursor(centerText_X(line1), 90);
    display.print(line1);

    String line2 = "Connected: " + String(bleConnected ? "Yes" : "No");
    display.setCursor(centerText_X(line2), 120);
    display.print(line2);
}