#include "Display.h"

// Construct display
Display::Display(BMESensor *bmeSensor, PMSensor *pmSensor, CO2Sensor *co2Sensor, Bluetooth_module *bluetoothModule)
    : display(GxEPD2_DRIVER_CLASS(DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN)) {
    // Set screen mode
    currScreenMode = SCREENMODE::SENSORS;
    prevScreenMode = SCREENMODE::BLUETOOTH;
    // Set sensors
    this->bmeSensor = bmeSensor;
    this->pmSensor = pmSensor;
    this->co2Sensor = co2Sensor;
    this->bluetoothModule = bluetoothModule;
}

// Routine to initialize display
void Display::init() {
    display.init(0, true, 10, false, SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    display.setRotation(2);
    display.setTextColor(GxEPD_BLACK);
    updateScreen();
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

void Display::updateScreen() {
    if (currScreenMode == SCREENMODE::BLUETOOTH) {
        updateBluetoothScreen();
        prevScreenMode = SCREENMODE::BLUETOOTH;
    } else if (currScreenMode == SCREENMODE::SENSORS) {
        updateSensorsScreen();
        prevScreenMode = SCREENMODE::SENSORS;
    }

    display.powerOff();
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
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(0, 30);
    display.println("Bluetooth");

    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(0, 100);
    String line1 = "Enabled: " + String(bluetoothModule->isEnabled() ? "Yes" : "No");
    display.println(line1);
    display.setCursor(0, 130);
    String line2 = "Connected: " + String(bluetoothModule->isConnected() ? "Yes" : "No");
    display.println(line2);

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 170);
    display.println(" Press button to");
    display.setCursor(45, 190);
    display.println("toggle BLE");

    display.display(false);
}

void Display::partialBluetoothScreen() {
    logg("Partial bluetooth screen");
    
    display.setPartialWindow(0, 80, display.width(), 80);
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);

    do {
        display.setCursor(0, 100);
        String line1 = "Enabled: " + String(bluetoothModule->isEnabled() ? "Yes" : "No");
        display.println(line1);
        display.setCursor(0, 130);
        String line2 = "Connected: " + String(bluetoothModule->isConnected() ? "Yes" : "No");
        display.println(line2);
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
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(30, 20);
    display.println("Sensors");

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(0, 45);

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

    display.display(false);
}

void Display::partialSensorsScreen() {
    logg("Partial sensors screen");
    
    display.setPartialWindow(0, 30, display.width(), 160);
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);

    do {
        display.setCursor(0, 45);
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
    } while (display.nextPage());
}

String Display::readTemperature() {
    if (!checkSensorConnection(bmeSensor)) {
        return String("sensor err");
    } else if (checkSensorError(bmeSensor)) {
        return String("read error");
    } else {
        return String(String(bmeSensor->getTemperature()) + " C");
    }
}

String Display::readHumidity() {
    if (!checkSensorConnection(bmeSensor)) {
        return String("sensor err");
    } else if (checkSensorError(bmeSensor)) {
        return String("read error");
    } else {
        return String(String(bmeSensor->getHumidity()) + " %");
    }
}

String Display::readPressure() {
    if (!checkSensorConnection(bmeSensor)) {
        return String("sensor err");
    } else if (checkSensorError(bmeSensor)) {
        return String("read error");
    } else {
        return String(String(bmeSensor->getPressure()) + " hPa");
    }
}

String Display::readAltitude() {
    if (!checkSensorConnection(bmeSensor)) {
        return String("sensor err");
    } else if (checkSensorError(bmeSensor)) {
        return String("read error");
    } else {
        return String(String(bmeSensor->getAltitude()) + " m");
    }
}

String Display::readCO2() {
    if (!checkSensorConnection(co2Sensor)) {
        return String("sensor err");
    } else if (checkSensorError(co2Sensor)) {
        return String("read error");
    } else {
        return String(String(co2Sensor->getCO2()) + " ppm");
    }
}

String Display::readPM1() {
    if (!checkSensorConnection(pmSensor)) {
        return String("sensor err");
    } else if (checkSensorError(pmSensor)) {
        return String("read error");
    } else {
        return String(String(pmSensor->getPM1()) + " ug/m3");
    }
}

String Display::readPM2_5() {
    if (!checkSensorConnection(pmSensor)) {
        return String("sensor err");
    } else if (checkSensorError(pmSensor)) {
        return String("read error");
    } else {
        return String(String(pmSensor->getPM2_5()) + " ug/m3");
    }
}

String Display::readPM10() {
    if (!checkSensorConnection(pmSensor)) {
        return String("sensor err");
    } else if (checkSensorError(pmSensor)) {
        return String("read error");
    } else {
        return String(String(pmSensor->getPM10()) + " ug/m3");
    }
}

bool Display::checkSensorConnection(Sensor *sensor) {
    return sensor->sensorFound();
}

bool Display::checkSensorError(Sensor *sensor) {
    return sensor->sensorError();
}