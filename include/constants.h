#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// BLE server name
const std::string bleServerName = "ESP32_EnvMonitor";

// MH-Z19 sensor read error
const uint16_t CO2_ERROR = 0xFFFF;

// Time between sensor reads (in us -> chosen to be prime numbers)
const uint64_t WAIT_TIME_READ_SENSORS = 20000009;

// Sea level pressure (in h200Pa)
const float seaLevel = 1013.25;

// Each bit represents one of the sensors to be read
namespace SENSORS {
    const uint8_t NO_SENSOR = 0b000;
    const uint8_t SENSOR_PM = 0b001;
    const uint8_t SENSOR_CO2 = 0b010;
    const uint8_t SENSOR_BME = 0b100;
}

// Each bit represents one of the buttons
namespace BUTTONS {
    const uint8_t NO_BUTTON = 0b000;
    const uint8_t BUTTON_B = 0b001;
    const uint8_t BUTTON_Y = 0b010;
    const uint8_t BUTTON_R = 0b100;
};

struct BMEData {
    int32_t temperature;
    int32_t pressure;
    int32_t humidity;
    int32_t gas;
    float altitude;
};

#endif // CONSTANTS_H