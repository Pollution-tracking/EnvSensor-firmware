#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// BLE server name
const std::string bleServerName = "ESP32_EnvMonitor";

// UUIDs
#define SERVICE_UUID (BLEUUID((uint16_t)0x181A))
#define CLIENT_CHARACTERISTIC_CONFIG_UUID (BLEUUID((uint16_t)0x2902))
#define TEMPERATURE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6E))
#define CARBON_DIOXIDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2B8C))
#define PM1_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD5))
#define PM2_5_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD6))
#define PM10_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BD7))
#define GAS_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2BE7)) //???
#define HUMIDITY_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6F))
#define PRESSURE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2A6D))
#define ALTITUDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2AB3))

// BLE descriptor values
const std::string temperatureDescriptorValue   = "Temperature";
const std::string carbonDioxideDescriptorValue = "Carbon dioxide";
const std::string pm1DescriptorValue           = "PM1";
const std::string pm2_5DescriptorValue         = "PM2.5";
const std::string pm10DescriptorValue          = "PM10";
const std::string gasDescriptorValue           = "Gas resistance";
const std::string humidityDescriptorValue      = "Humidity";
const std::string pressureDescriptorValue      = "Pressure";
const std::string altitudeDescriptorValue      = "Altitude";
// MH-Z19 sensor read error
const uint16_t CO2_ERROR = 0xFFFF;

// Time between sensor reads (in us -> chosen to be prime numbers)
const uint64_t WAIT_TIME_READ_SENSORS   = 20000009;
const uint64_t WAIT_TIME_REENABLE_SLEEP = 5000003;

// Button times (in ms)
const uint16_t DEBOUNCE_TIME        = 100;
const uint16_t LONG_PRESS_THRESHOLD = 1000;

// Button debounce time (in ms)
const uint16_t DEBOUNCE_TIME = 500;

// Sea level pressure (in h200Pa)
const float seaLevel = 1013.25;

// Each bit represents one of the sensors to be read
namespace SENSORS {
    const uint8_t NO_SENSOR  = 0b000;
    const uint8_t SENSOR_PM  = 0b001;
    const uint8_t SENSOR_CO2 = 0b010;
    const uint8_t SENSOR_BME = 0b100;
}

// Each bit represents one of the buttons
namespace BUTTON_STATES {
    const uint8_t NO_PRESS    = 0b00;
    const uint8_t SHORT_PRESS = 0b01;
    const uint8_t LONG_PRESS  = 0b10;
};

enum class BUTTONS {
    LEFT,
    RIGHT
};

namespace SCREENMODE {
    const uint8_t MAIN_SCREEN  = 0b10000000;
    const uint8_t BLUETOOTH    = 0b01000000;
    const uint8_t TEMPERATURE  = 0b00100000;
    const uint8_t PM           = 0b00010000;
    const uint8_t CO2          = 0b00001000;
    const uint8_t HUMIDITY     = 0b00000100;
    const uint8_t PRESSURE     = 0b00000010;
    const uint8_t ALTITUDE     = 0b00000001;
};

namespace SCREENMODE {
    const uint8_t MAIN_SCREEN  = 0b10000000;
    const uint8_t BLUETOOTH    = 0b01000000;
    const uint8_t TEMPERATURE  = 0b00100000;
    const uint8_t PM           = 0b00010000;
    const uint8_t CO2          = 0b00001000;
    const uint8_t HUMIDITY     = 0b00000100;
    const uint8_t PRESSURE     = 0b00000010;
    const uint8_t ALTITUDE     = 0b00000001;
};

struct BMEData {
    int32_t temperature;
    int32_t pressure;
    int32_t humidity;
    int32_t gas;
    float altitude;
};

#endif // CONSTANTS_H