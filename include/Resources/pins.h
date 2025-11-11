#ifndef PINS_H
#define PINS_H

// MH-Z19 Sensor
#define CO2_RX_PIN 41
#define CO2_TX_PIN 42

// PMS Sensor
#define PM_RX_PIN 1
#define PM_TX_PIN 2

// GPIO buttons
#define BUTTON_LEFT_PIN     18
#define BUTTON_CENTER_PIN   45
#define BUTTON_RIGHT_PIN    3

// Wakeup pins from deep sleep (center unavailable)
const uint64_t BUTTONS_MASK = (uint64_t)1 << BUTTON_LEFT_PIN |
                              (uint64_t)1 << BUTTON_RIGHT_PIN;

// Display
#define DISPLAY_CS_PIN   21
#define DISPLAY_DC_PIN   6
#define DISPLAY_RST_PIN  5
#define DISPLAY_BUSY_PIN 4

// SD Card
#define SD_CS_PIN 10

// SPI
#define _MOSI_PIN 11
#define _CLK_PIN  12
#define _MISO_PIN 13

// I2C
#define _SDA_PIN 8
#define _SCL_PIN 9

// Buck-Boost
#define BUCK_EN_PIN 7

// Battery level
#define BATTERY_PIN 15

#endif // PINS_H