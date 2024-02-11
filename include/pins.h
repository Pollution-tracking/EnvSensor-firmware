#ifndef PINS_H
#define PINS_H

// MH-Z19 Sensor
#define CO2_RX_PIN 4
#define CO2_TX_PIN 5

// PMS Sensor
#define PM_RX_PIN 3
#define PM_TX_PIN 10

// GPIO buttons
#define BUTTON_LEFT_PIN   18//20
#define BUTTON_RIGHT_PIN  19//21
#define GPIO_BUTTON_LEFT  GPIO_NUM_18
#define GPIO_BUTTON_RIGHT GPIO_NUM_19

// BME680 Sensor
#define BME680_CS_PIN 8

// Display
#define DISPLAY_CS_PIN   9
#define DISPLAY_DC_PIN   2
#define DISPLAY_RST_PIN  1
#define DISPLAY_BUSY_PIN 0

// SD Card
#define SD_CS_PIN 7

// SPI
#define _CLK_PIN  4
#define _MOSI_PIN 6
#define _MISO_PIN 5

#endif // PINS_H