#include "Display.h"

// Construct display
Display::Display(BMESensor *bmeSensor, PMSensor *pmSensor, CO2Sensor *co2Sensor, Bluetooth_module *bluetoothModule)
    : io(SPI, DISPLAY_CS_PIN, DISPLAY_DC_PIN, DISPLAY_RST_PIN),
      display(io, DISPLAY_RST_PIN, DISPLAY_BUSY_PIN) {
    // Set screen mode
    currScreenMode = SCREENMODE::MAIN_SCREEN;
    prevScreenMode = SCREENMODE::ALTITUDE;
    // Set sensors
    this->bmeSensor = bmeSensor;
    this->pmSensor = pmSensor;
    this->co2Sensor = co2Sensor;
    this->bluetoothModule = bluetoothModule;
}

// Routine to initialize display
void Display::init() {
    display.init(0); // disable serial logs
    display.setRotation(2);
    display.fillScreen(GxEPD_WHITE);
    display.update();
    display.powerDown();
}

// Routine to change screen to the right
void Display::changeScreenRight() {
    prevScreenMode = currScreenMode;
    if (currScreenMode == SCREENMODE::ALTITUDE) {
        currScreenMode = SCREENMODE::MAIN_SCREEN;
    } else {
        currScreenMode = currScreenMode >> 1;
    }
}

// Routine to change screen to the left
void Display::changeScreenLeft() {
    prevScreenMode = currScreenMode;
    if (currScreenMode == SCREENMODE::MAIN_SCREEN) {
        currScreenMode = SCREENMODE::ALTITUDE;
    } else {
        currScreenMode = currScreenMode << 1;
    }
}

// Routine to get screen mode
uint8_t Display::getScreenMode() {
    return currScreenMode;
}

// Routine to update screen
void Display::updateScreen() {
    if (currScreenMode == SCREENMODE::MAIN_SCREEN) {
        updateMainScreen();
        prevScreenMode = SCREENMODE::MAIN_SCREEN;
    } else if (currScreenMode == SCREENMODE::BLUETOOTH) {
        updateBluetoothScreen();
        prevScreenMode = SCREENMODE::BLUETOOTH;
    } else if (currScreenMode == SCREENMODE::TEMPERATURE) {
        updateTemperatureScreen();
        prevScreenMode = SCREENMODE::TEMPERATURE;
    } else if (currScreenMode == SCREENMODE::PM) {
        updatePMScreen();
        prevScreenMode = SCREENMODE::PM;
    } else if (currScreenMode == SCREENMODE::CO2) {
        updateCO2Screen();
        prevScreenMode = SCREENMODE::CO2;
    } else if (currScreenMode == SCREENMODE::HUMIDITY) {
        updateHumidityScreen();
        prevScreenMode = SCREENMODE::HUMIDITY;
    } else if (currScreenMode == SCREENMODE::PRESSURE) {
        updatePressureScreen();
        prevScreenMode = SCREENMODE::PRESSURE;
    } else if (currScreenMode == SCREENMODE::ALTITUDE) {
        updateAltitudeScreen();
        prevScreenMode = SCREENMODE::ALTITUDE;
    }
}

void Display::updateMainScreen() {
    // No need to update
    if (prevScreenMode == SCREENMODE::MAIN_SCREEN) {
        return;
    }

    logg("Updating main screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(45, 80);
    display.println("Home");
    display.setCursor(15, 130);
    display.println("screen");
    display.update();
    display.powerDown();
}

void Display::updateBluetoothScreen() {
    logg("Updating bluetooth screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
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

    if (prevScreenMode == SCREENMODE::BLUETOOTH) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updateTemperatureScreen() {
    logg("Updating temperature screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(20, 30);
    display.println("Temperature");

    // Check if BME sensor is connected
    if (!bmeSensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if BME sensor has an error
    if (bmeSensor->errorBME) {
        showReadError();
        return;
    }

    // Get temperature value
    uint32_t temperature = bmeSensor->getTemperature();

    display.setCursor(50, 130);
    display.setFont(&FreeMonoBold24pt7b);
    display.println(String(temperature) + " C");

    if (prevScreenMode == SCREENMODE::TEMPERATURE) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updatePMScreen() {
    logg("Updating PM screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(70, 30);
    display.println("PM");

    // Check if PM sensor is connected
    if (!pmSensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if PM sensor has an error
    if (pmSensor->errorPM) {
        showReadError();
        return;
    }

    // Get PM values
    uint16_t pm1 = pmSensor->getPM1();
    uint16_t pm2_5 = pmSensor->getPM2_5();
    uint16_t pm10 = pmSensor->getPM10();

    display.setCursor(0, 70);
    display.setFont(&FreeMonoBold12pt7b);
    display.print("PM1: ");
    display.setFont(&FreeMonoBold18pt7b);
    display.println(String(pm1));
    display.setFont(&FreeMonoBold12pt7b);
    display.print("PM2.5: ");
    display.setFont(&FreeMonoBold18pt7b);
    display.println(String(pm2_5));
    display.setFont(&FreeMonoBold12pt7b);
    display.print("PM10: ");
    display.setFont(&FreeMonoBold18pt7b);
    display.println(String(pm10));

    display.setFont(&FreeMonoBold12pt7b);
    display.println("Measuring unit");
    display.setCursor(60, display.getCursorY() - 4);
    display.println("ug/m3");

    if (prevScreenMode == SCREENMODE::PM) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updateCO2Screen() {
    logg("Updating CO2 screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(60, 30);
    display.println("CO2");

    // Check if CO2 sensor is connected
    if (!co2Sensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if CO2 sensor has an error
    if (co2Sensor->errorCO2) {
        showReadError();
        return;
    }

    // Get CO2 value
    uint16_t co2 = co2Sensor->getCO2();

    display.setCursor(0, 130);
    display.println(String(co2) + " ppm");

    if (prevScreenMode == SCREENMODE::CO2) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updateHumidityScreen() {
    logg("Updating humidity screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(10, 30);
    display.println("Humidity");

    // Check if BME sensor is connected
    if (!bmeSensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if BME sensor has an error
    if (bmeSensor->errorBME) {
        showReadError();
        return;
    }

    // Get humidity value
    uint32_t humidity = bmeSensor->getHumidity();

    display.setCursor(50, 130);
    display.setFont(&FreeMonoBold24pt7b);
    display.println(String(humidity) + " %");

    if (prevScreenMode == SCREENMODE::HUMIDITY) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updatePressureScreen() {
    logg("Updating pressure screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(10, 30);
    display.println("Pressure");

    // Check if BME sensor is connected
    if (!bmeSensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if BME sensor has an error
    if (bmeSensor->errorBME) {
        showReadError();
        return;
    }

    // Get pressure value
    uint32_t pressure = bmeSensor->getPressure();

    display.setCursor(0, 130);
    display.setFont(&FreeMonoBold24pt7b);
    display.println(String(pressure) + " hPa");

    if (prevScreenMode == SCREENMODE::PRESSURE) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::updateAltitudeScreen() {
    logg("Updating altitude screen");

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(15, 30);
    display.println("Altitude");

    // Check if BME sensor is connected
    if (!bmeSensor->sensorFound) {
        showSensorError();
        return;
    }

    // Check if BME sensor has an error
    if (bmeSensor->errorBME) {
        showReadError();
        return;
    }

    // Get altitude value
    float altitude = bmeSensor->getAltitude();

    display.setCursor(0, 130);
    display.setFont(&FreeMonoBold24pt7b);
    display.println(String(altitude) + " m");

    if (prevScreenMode == SCREENMODE::ALTITUDE) {
        // TODO implement partial update
    }
    display.update();
    display.powerDown();
}

void Display::showSensorError() {
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(40, 100);
    display.println("Sensor");
    display.setCursor(50, 130);
    display.println("error");
    display.update();
    display.powerDown();
}

void Display::showReadError() {
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(60, 100);
    display.println("Read");
    display.setCursor(50, 130);
    display.println("error");
    display.update();
    display.powerDown();
}

// Routine to test display
void Display::test() {
    logg("Testing display");
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 50);
    display.println("Hello world!");
    display.update();
    display.powerDown();
}