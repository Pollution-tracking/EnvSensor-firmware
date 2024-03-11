#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <pins.h>

// BLE server name
const std::string bleServerName = "ESP32_EnvMonitor";

// UUIDs
#define SERVICE_UUID                       (BLEUUID((uint16_t)0x181A))
#define CLIENT_CHARACTERISTIC_CONFIG_UUID  (BLEUUID((uint16_t)0x2902))
#define TEMPERATURE_CHARACTERISTIC_UUID    (BLEUUID((uint16_t)0x2A6E))
#define CARBON_DIOXIDE_CHARACTERISTIC_UUID (BLEUUID((uint16_t)0x2B8C))
#define PM1_CHARACTERISTIC_UUID            (BLEUUID((uint16_t)0x2BD5))
#define PM2_5_CHARACTERISTIC_UUID          (BLEUUID((uint16_t)0x2BD6))
#define PM10_CHARACTERISTIC_UUID           (BLEUUID((uint16_t)0x2BD7))
#define GAS_CHARACTERISTIC_UUID            (BLEUUID((uint16_t)0x2BE7)) //???
#define HUMIDITY_CHARACTERISTIC_UUID       (BLEUUID((uint16_t)0x2A6F))
#define PRESSURE_CHARACTERISTIC_UUID       (BLEUUID((uint16_t)0x2A6D))
#define ALTITUDE_CHARACTERISTIC_UUID       (BLEUUID((uint16_t)0x2AB3))

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
const uint64_t WAIT_TIME_REENABLE_SLEEP = 8000003;

// Button times (in ms)
const uint16_t DEBOUNCE_TIME = 250;

const uint64_t BUTTONS_MASK = (uint64_t)1 << BUTTON_LEFT_PIN | (uint64_t)1 << BUTTON_CENTER_PIN | (uint64_t)1 << BUTTON_RIGHT_PIN;

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
namespace BUTTONS {
    const uint8_t NO_BUTTON     = 0b00000000;
    const uint8_t BUTTON_LEFT   = 0b11000000;
    const uint8_t BUTTON_CENTER = 0b00011000;
    const uint8_t BUTTON_RIGHT  = 0b00000011;
};

namespace SCREENMODE {
    const uint8_t NO_SCREEN = 0b00;
    const uint8_t BLUETOOTH = 0b10;
    const uint8_t SENSORS   = 0b01;
};

enum class SCREENUPDATE {
    GENERAL,
    BLUETOOTH,
    SENSORS
};

struct BMEData {
    int32_t temperature;
    int32_t pressure;
    int32_t humidity;
    int32_t gas;
    float altitude;
};

struct SleepUtils {
private:
    bool allowSleep    = false; // Enable sleep mode
    bool sleepCooldown = false; // Cooldown after sleep
    bool sleepOver     = false; // Sleep mode over
    
public:
    void allow_sleep() {
        allowSleep = true;
    }

    void disable_sleep(bool sleep_over = false) {
        allowSleep = false;
        sleepOver = sleep_over;
    }

    bool is_sleep_allowed() {
        return allowSleep;
    }

    void enable_cooldown() {
        sleepCooldown = true;
    }

    void disable_cooldown() {
        sleepCooldown = false;
    }

    bool is_cooldown_enabled() {
        return sleepCooldown;
    }

    bool is_sleep_over() {
        return sleepOver;
    }

    void mark_sleep_treated() {
        sleepOver = false;
    }
};

#endif // CONSTANTS_H