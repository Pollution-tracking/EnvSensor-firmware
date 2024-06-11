#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <BLECharacteristic.h>

#include <Resources/Pins.h>

const uint32_t CHARACTERISTIC_PROPERTIES = BLECharacteristic::PROPERTY_READ |
                                           BLECharacteristic::PROPERTY_NOTIFY;
// UUIDs 0000xxxx-0000-1000-8000-00805F9B34FB
#define SERVICE_UUID                        (BLEUUID((uint16_t)0x181A))
#define CLIENT_CHARACTERISTIC_CONFIG_UUID   (BLEUUID((uint16_t)0x2902))
#define TEMPERATURE_CHARACTERISTIC_UUID     (BLEUUID((uint16_t)0x2A6E))
#define CARBON_DIOXIDE_CHARACTERISTIC_UUID  (BLEUUID((uint16_t)0x2B8C))
#define PM1_CHARACTERISTIC_UUID             (BLEUUID((uint16_t)0x2BD5))
#define PM2_5_CHARACTERISTIC_UUID           (BLEUUID((uint16_t)0x2BD6))
#define PM10_CHARACTERISTIC_UUID            (BLEUUID((uint16_t)0x2BD7))
#define GAS_CHARACTERISTIC_UUID             (BLEUUID((uint16_t)0x2BE7))
#define HUMIDITY_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2A6F))
#define PRESSURE_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2A6D))
#define ALTITUDE_CHARACTERISTIC_UUID        (BLEUUID((uint16_t)0x2AB3))
#define BATTERY_CHARACTERISTIC_UUID         (BLEUUID((uint16_t)0x2A76))
#define TIMESTAMP_CHARACTERISTIC_UUID       (BLEUUID((uint16_t)0x2A79))

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
const std::string batteryDescriptorValue       = "Battery";
const std::string timestampDescriptorValue     = "Timestamp";

// Battery voltage monitor
const uint8_t BATTERY_VOLTAGE_DIVIDER = 2;
const float BATTERY_MAX_VOLTAGE       = 3.3;
const float BATTERY_CORRECTION_FACTOR = 1.115;
const uint8_t ADC_RESOLUTION          = 12;
const uint16_t ADC_MAX_VALUE          = (1 << ADC_RESOLUTION) - 1;

// Sensor errors
const int32_t READ_ERROR   = INT32_MIN;
const int32_t SENSOR_ERROR = INT32_MAX;
const char SENSOR_ERROR_STRING[] = "Sensor err\0";
const char READ_ERROR_STRING[]   = "Read err\0";

// Time errors
const uint8_t TIME_ERROR_8   = 255;
const uint16_t TIME_ERROR_16 = 65535;

// CSV data values
const int NR_VALUES             = 11;
const int TIMESTAMP_INDEX       = 0;
const int BATTERY_INDEX         = 1;
const int BME_TEMPERATURE_INDEX = 2;
const int BME_HUMIDITY_INDEX    = 3;
const int BME_PRESSURE_INDEX    = 4;
const int BME_GAS_INDEX         = 5;
const int BME_ALTITUDE_INDEX    = 6;
const int CO2_CO2_INDEX         = 7;
const int PM_PM1_INDEX          = 8;
const int PM_PM2_5_INDEX        = 9;
const int PM_PM10_INDEX         = 10;
const String dataPath = "/SensorsData.csv";

// Time between sensor reads (in us -> chosen to be prime numbers)
const uint64_t WAIT_TIME_READ_SENSORS   = 20000009;
const uint64_t WAIT_TIME_REENABLE_SLEEP = 8000003;

// Button times (in ms)
const uint16_t DEBOUNCE_TIME = 250;

const uint64_t BUTTONS_MASK = (uint64_t)1 << BUTTON_LEFT_PIN |
                              (uint64_t)1 << BUTTON_CENTER_PIN |
                              (uint64_t)1 << BUTTON_RIGHT_PIN;

// Sea level pressure (in h200Pa)
const float seaLevel = 1013.25;

// Each bit represents one of the sensors to be read
namespace SENSORS {
    const uint8_t NO_SENSOR    = 0b0000;
    const uint8_t ALL_SENSORS  = 0b1111;
    const uint8_t SENSOR_PM    = 0b0001;
    const uint8_t SENSOR_CO2   = 0b0010;
    const uint8_t SENSOR_BME   = 0b0100;
    const uint8_t BATTERY      = 0b1000;
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

namespace BLE_STATUS {
    const uint8_t CLIENT_UPDATE     = 0b01;
    const uint8_t TIMESTAMP_UPDATE  = 0b10;
    const uint8_t NO_UPDATE         = 0b00;
};

enum class SCREENUPDATE {
    GENERAL,
    BLUETOOTH,
    SENSORS
};

#endif // CONSTANTS_H