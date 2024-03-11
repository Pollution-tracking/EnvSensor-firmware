#include "Display.h"

extern RTC_DATA_ATTR bool bleEnabled;

// Construct display
Display::Display(BMESensor *bmeSensor, PMSensor *pmSensor, CO2Sensor *co2Sensor, Bluetooth_module *bluetoothModule)
    : display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
    // Set screen mode
    if (currScreenMode == SCREENMODE::NO_SCREEN) {
        currScreenMode = SCREENMODE::SENSORS;
    }
    // Set sensors
    this->bmeSensor = bmeSensor;
    this->pmSensor = pmSensor;
    this->co2Sensor = co2Sensor;
    this->bluetoothModule = bluetoothModule;
}

// Routine to initialize display
void Display::init() {
    display.init(0, true, 2, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    display.setRotation(2);
    display.setTextColor(GxEPD_BLACK);
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
    logg("Updating bluetooth screen");

    if (prevScreenMode == SCREENMODE::BLUETOOTH) {
        partialBluetoothScreen();
    } else {
        fullBluetoothScreen();
    }
}

void Display::fullBluetoothScreen() {
    logg("Full bluetooth screen");
    
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
    logg("Partial bluetooth screen");
    
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
    logg("Updating sensors screen");

    if (prevScreenMode == SCREENMODE::SENSORS) {
        partialSensorsScreen();
    } else {
        fullSensorsScreen();
    }
}

void Display::fullSensorsScreen() {
    logg("Full sensors screen");

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
    logg("Partial sensors screen");
    
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

String Display::readTemperature() {
    logg("---> " + String(String(bmeSensor->getTemperature()) + " C ") + checkSensorInitialised(bmeSensor) + checkSensorConnection(bmeSensor) + checkSensorError(bmeSensor));
    if (checkSensorInitialised(bmeSensor)) {
        if (!checkSensorConnection(bmeSensor)) {
            strcpy(_temperature, "sensor err\0");
        } else if (checkSensorError(bmeSensor)) {
            strcpy(_temperature, "read error\0");
        } else {
            strcpy(_temperature, String(String(bmeSensor->getTemperature()) + " C").c_str());
        }
    }

    return String(_temperature);
}

String Display::readHumidity() {
    if (checkSensorInitialised(bmeSensor)) {
        if (!checkSensorConnection(bmeSensor)) {
            strcpy(_humidity, "sensor err\0");
        } else if (checkSensorError(bmeSensor)) {
            strcpy(_humidity, "read error\0");
        } else {
            strcpy(_humidity, String(String(bmeSensor->getHumidity()) + " %").c_str());
        }
    }
    
    return String(_humidity);
}

String Display::readPressure() {
    if (checkSensorInitialised(bmeSensor)) {
        if (!checkSensorConnection(bmeSensor)) {
            strcpy(_pressure, "sensor err\0");
        } else if (checkSensorError(bmeSensor)) {
            strcpy(_pressure, "read error\0");
        } else {
            strcpy(_pressure, String(String(bmeSensor->getPressure()) + " hPa").c_str());
        }
    }
    
    return String(_pressure);
}

String Display::readAltitude() {
    if (checkSensorInitialised(bmeSensor)) {
        if (!checkSensorConnection(bmeSensor)) {
            strcpy(_altitude, "sensor err\0");
        } else if (checkSensorError(bmeSensor)) {
            strcpy(_altitude, "read error\0");
        } else {
            strcpy(_altitude, String(String(static_cast<int>(bmeSensor->getAltitude())) + " m").c_str());
        }
    }
    
    return String(_altitude);
}

String Display::readCO2() {
    if(checkSensorInitialised(co2Sensor)) {
        if (!checkSensorConnection(co2Sensor)) {
            strcpy(_co2, "sensor err\0");
        } else if (checkSensorError(co2Sensor)) {
            strcpy(_co2, "read error\0");
        } else {
            strcpy(_co2, String(String(co2Sensor->getCO2()) + " ppm").c_str());
        }
    }

    return String(_co2);
}

String Display::readPM1() {
    if(checkSensorInitialised(pmSensor)) {
        if (!checkSensorConnection(pmSensor)) {
            strcpy(_pm1, "sensor err\0");
        } else if (checkSensorError(pmSensor)) {
            strcpy(_pm1, "read error\0");
        } else {
            strcpy(_pm1, String(String(pmSensor->getPM1()) + " ug/m3").c_str());
        }
    }

    return String(_pm1);
}

String Display::readPM2_5() {
    if(checkSensorInitialised(pmSensor)) {
        if (!checkSensorConnection(pmSensor)) {
            strcpy(_pm2_5, "sensor err\0");
        } else if (checkSensorError(pmSensor)) {
            strcpy(_pm2_5, "read error\0");
        } else {
            strcpy(_pm2_5, String(String(pmSensor->getPM2_5()) + " ug/m3").c_str());
        }
    }

    return String(_pm2_5);
}

String Display::readPM10() {
    if(checkSensorInitialised(pmSensor)) {
        if (!checkSensorConnection(pmSensor)) {
            strcpy(_pm10, "sensor err\0");
        } else if (checkSensorError(pmSensor)) {
            strcpy(_pm10, "read error\0");
        } else {
            strcpy(_pm10, String(String(pmSensor->getPM10()) + " ug/m3").c_str());
        }
    }

    return String(_pm10);
}

bool Display::checkSensorInitialised(Sensor *sensor) {
    return sensor->sensorInitialised();
}

bool Display::checkSensorConnection(Sensor *sensor) {
    return sensor->sensorFound();
}

bool Display::checkSensorError(Sensor *sensor) {
    return sensor->sensorError();
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
    String line1 = "Temperature: " + readTemperature();
    display.println(line1);
    String line2 = "Humidity: " + readHumidity();
    display.println(line2);
    String line3 = "Pressure: " + readPressure();
    display.println(line3);
    String line4 = "Altitude: " + readAltitude();
    display.println(line4);
    String line5 = "CO2: " + readCO2();
    display.println(line5);
    String line6 = "PM1: " + readPM1();
    display.println(line6);
    String line7 = "PM2.5: " + readPM2_5();
    display.println(line7);
    String line8 = "PM10: " + readPM10();
    display.println(line8);
}

void Display::printBLEStatus() {
    display.setFont(&FreeMonoBold12pt7b);
    String line1 = "Enabled: " + String(bluetoothModule->isEnabled() ? "Yes" : "No");
    display.setCursor(centerText_X(line1), 90);
    display.print(line1);

    String line2 = "Connected: " + String(bluetoothModule->isConnected() ? "Yes" : "No");
    display.setCursor(centerText_X(line2), 120);
    display.print(line2);
}