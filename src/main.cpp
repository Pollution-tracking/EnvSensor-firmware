#include <Arduino.h>

#include "esp_wifi.h"

#include <driver/gpio.h>
#include <configs.h>

#include <Logger/logger.h>

#include <Resources/pins.h>
#include <Resources/Constants/battery_constants.h>
#include <Resources/Software/DataHandler.h>

#include <Modules/Bluetooth.h>
#include <Modules/BoardUtilities.h>
#include <Modules/Sensors.h>
#include <Modules/Buttons.h>
#include <Modules/Timers.h>
#include <Modules/SDcard.h>
#include <Modules/RTC.h>
#include <Modules/DisplayTask.h>

#ifdef PM_ENABLE
#include <Sensors/PM_sensor.h>
#endif
#ifdef BME_ENABLE
#include <Sensors/BME_sensor.h>
#endif
#ifdef CO2_ENABLE
#include <Sensors/CO2_sensor.h>
#endif
#ifdef MICS_ENABLE
#include <Sensors/MICS_sensor.h>
#endif
#include <Sensors/Battery.h>

#define logg(message) loggWithObj(message, "MAIN")

// Global variables
Bluetooth bluetooth; // Bluetooth module
SDcard sdcard;       // SD card module
RTC rtc;             // RTC module
BoardUtilities boardUtilities; // Board utilities
#ifdef PM_ENABLE
PMSensor pmSensor; // PM sensor
#endif
#ifdef CO2_ENABLE
CO2Sensor co2Sensor; // CO2 sensor
#endif
#ifdef BME_ENABLE
BMESensor bmeSensor; // BME sensor
#endif
#ifdef MICS_ENABLE
MICSSensor micsSensor; // MICS sensor
#endif
Battery battery; // Battery monitor

// Forward declarations
void treatDisplay();
void treatSensors();
void treatButtons();
void treatTimers();
void treatBluetooth();
void treatSleep();

void setup() {
	// Disable WiFi
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_stop();

	// Disable JTAG on GPIO3 to use it as a regular pin
	gpio_pulldown_dis(GPIO_NUM_3);

	// Configure serial for debugging
	Serial.begin(115200);
	initLogger();

	// Set buck-boost converter pin as output
	pinMode(BUCK_EN_PIN, OUTPUT);
	// Set battery voltage pin as input
	pinMode(BATTERY_PIN, INPUT);

	// Configure SPI
	SPI.begin(_CLK_PIN, _MISO_PIN, _MOSI_PIN, SD_CS_PIN);
	// Configure I2C
	Wire.begin(_SDA_PIN, _SCL_PIN);
	// Configure ADC
	analogReadResolution(ADC_RESOLUTION);

	// Initialize RTC
	rtc.init();
	// Initialize SD card
	sdcard.init();
	// Create display task
	createDisplayTask();
	// Initialize display
	DisplayCommand cmd_init = {CMD_INIT, {}};
	sendDisplayCommand(cmd_init);

    // Treat wakeup reason
    boardUtilities.treatWakeupReason();
}

void loop() {
    // Check if board config needs to be treated
    if (board_config.to_treat) {
		board_config.to_treat = false;
        logg("Treating board config changes");

		// Check if display needs to be handled
		if (board_config.screen != SCREEN_MODE::NO_SCREEN) {
			treatDisplay();
		}

		// Check if timers need to be handled
		if (board_config.timer_type != TIMER_TYPES::NO_TIMER) {
			treatTimers();
		}

		// Check if sensors need to be handled
		if (board_config.sensors_state != SENSORS::NO_ACTION) {
			treatSensors();
		}

		// Check if board needs to go to sleep
		if (board_config.board_state != SLEEP_STATE::AWAKE) {
			treatSleep();
		}

		// Check if buttons need to be handled
		if (board_config.buttons_state != BUTTONS::NO_BUTTON) {
			treatButtons();
		}

		// Check if Bluetooth needs to be handled
		if (board_config.ble_state != BLE::NO_UPDATE) {
			treatBluetooth();
		}
    }
}

void treatDisplay() {
	logg("Handling display state change");

	DisplayCommand cmd_set_mode = {CMD_SET_MODE, {}};
	cmd_set_mode.payload.screen_mode = board_config.screen;
	sendDisplayCommand(cmd_set_mode);

	// Reset screen state after handling
	board_config.screen = SCREEN_MODE::NO_SCREEN;
}

void treatSensors() {
	logg("Handling sensors state change");
	SENSORS executed;

	executed = handleSensorsState(board_config.sensors_state);

	if (executed == SENSORS::READ_SENSORS) {
		// When reading from deep sleep wakeup, let the board go back to sleep afterwards
		if (board_config.board_state == SLEEP_STATE::FROM_SLEEP) {
			board_config.board_state = SLEEP_STATE::TO_SLEEP;
			board_config.to_treat = true;
		}

		// Process collected data
		handleLiveData();
		// Signal display to refresh shown values
		DisplayCommand cmd_refresh = {CMD_REFRESH, {}};
		cmd_refresh.payload.screen_refresh = SCREEN_REFRESH::SENSORS;
		sendDisplayCommand(cmd_refresh);
	} else if (executed == SENSORS::HEATED_SENSORS) {
		// Start sensor reading timer
		configureTimer(TIMER_MODES::T_ACTIVE, TIMER_TYPES::T_READ);
		// Stop sensor heat timer
		configureTimer(TIMER_MODES::T_DISABLE, TIMER_TYPES::T_HEAT);
		// Signal display to show sensors screen (no one else changes the screen)
		DisplayCommand cmd_set_mode = {CMD_SET_MODE, {}};
		cmd_set_mode.payload.screen_mode = SCREEN_MODE::SENSORS;
		sendDisplayCommand(cmd_set_mode);
	}

	// Reset sensors state after handling
	board_config.sensors_state = SENSORS::NO_ACTION;
}

void treatButtons() {
	logg("Handling buttons state change");
	BUTTONS executed;

	executed = handleButtonsState(board_config.buttons_state);

	if (executed == BUTTONS::BUTTON_PRESSED) {
		// Reset timer re-enable sleep if active
		bool sleep_active = (getTimerMode(TIMER_TYPES::T_SLEEP) == TIMER_MODES::T_ACTIVE);
		if (sleep_active) {
			configureTimer(TIMER_MODES::T_RESTART, TIMER_TYPES::T_SLEEP);
		}

		switch (board_config.buttons_state) {
			case BUTTONS::BUTTON_LEFT: {
				// Handle left button press
				logg("LEFT button pressed");

				// Signal display to change screen
				DisplayCommand cmd_left = {CMD_CHANGE_LEFT, {}};
				sendDisplayCommand(cmd_left);
				break;
			}
			case BUTTONS::BUTTON_CENTER: {
				// Handle center button press
				logg("CENTER button pressed");

				// Interact with Bluetooth if on Bluetooth screen
				if (lastScreenData.currentScreen == SCREEN_MODE::BLUETOOTH) {
					// Toggle Bluetooth state
					bluetooth.toggle();
					// Toggle sleep state
					boardUtilities.toggleSleepState();
					// Signal display to refresh shown values
					DisplayCommand cmd_refresh = {CMD_REFRESH, {}};
					cmd_refresh.payload.screen_refresh = SCREEN_REFRESH::BLUETOOTH;
					sendDisplayCommand(cmd_refresh);
				}
				break;
			}
			case BUTTONS::BUTTON_RIGHT: {
				// Handle right button press
				logg("RIGHT button pressed");

				// Signal display to change screen
				DisplayCommand cmd_right = {CMD_CHANGE_RIGHT, {}};
				sendDisplayCommand(cmd_right);
				break;
			}
			default: {
				break;
			}
		}
	}

	// Reset buttons state after handling
	board_config.buttons_state = BUTTONS::NO_BUTTON;
}

void treatTimers() {
	logg("Handling timer state change");

	// Set desired timer configuration
	configureTimer(board_config.timer_mode, board_config.timer_type);

	// Reset timer config after handling
	board_config.timer_type = TIMER_TYPES::NO_TIMER;
}

void treatBluetooth() {
	logg("Handling Bluetooth state change");

	switch (board_config.ble_state) {
		case BLE::INIT_BLE: {
			// Initialize Bluetooth
			bluetooth.enable();
			break;
		}
		case BLE::CLIENT_UPDATE: {
			// Handle client connection update
			logg("Bluetooth connection updated");
			// Signal display to refresh shown values
			DisplayCommand cmd_refresh = {CMD_REFRESH, {}};
			cmd_refresh.payload.screen_refresh = SCREEN_REFRESH::BLUETOOTH;
			sendDisplayCommand(cmd_refresh);
			// Try syncing stored data
			handleHistoricalData();
			break;
		}
		case BLE::TIMESTAMP_UPDATE: {
			// Handle timestamp update
			logg("Bluetooth timestamp updated: " + bluetooth.getTimestamp());
			// Update RTC with received timestamp
			rtc.setFromTimestamp(bluetooth.getTimestamp());
			break;
		}	
		default: {
			break;
		}
	}

	// Reset Bluetooth state after handling
	board_config.ble_state = BLE::NO_UPDATE;
}

void treatSleep() {
	logg("Handling sleep state change");

	switch (board_config.board_state) {
		case SLEEP_STATE::TO_SLEEP:
			logg("Entering deep sleep mode");
			// Enable deep sleep wakeup sources
			boardUtilities.configureWakeupSources();
			// Change sensors state to sleep
			pmSensor.sleep();
			// Disable timers
			disable_all_timers();
			// Wait for display task to finish
			waitForDisplayIdle();
			// Flush debug messages
			forcePrint();
			// Enter deep sleep
			esp_deep_sleep_start();
			break;

		case SLEEP_STATE::AWAKENING:
			logg("Normal operation after deep sleep");
			// Set new operation mode
			board_config.board_state = SLEEP_STATE::AWAKE;
			// Disable sleep re-enable timer
			configureTimer(TIMER_MODES::T_DISABLE, TIMER_TYPES::T_SLEEP);
			// Initialize sensors
            board_config.sensors_state = SENSORS::INIT_SENSORS;
			// Start sensor reading timer
			configureTimer(TIMER_MODES::T_ACTIVE, TIMER_TYPES::T_READ);

			board_config.to_treat = true;
			break;

		default:
			break;
	}
}
