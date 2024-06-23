#include <Arduino.h>

#include "esp_sleep.h"
#include "esp_wifi.h"
#include "hal/wdt_hal.h"
#include "soc/rtc.h"

#include <Resources/Pins.h>
#include <Resources/Constants.h>
#include <Resources/RTC_values.hpp>

#include <Helpers/Buttons.hpp>
#include <Helpers/Sensors.hpp>
#include <Helpers/Timers.hpp>
#include <Helpers/SleepUtils.hpp>

#include <Logger/Logger.h>

#include <Sensors/CO2_sensor.h>
#include <Sensors/PM_sensor.h>
#include <Sensors/BME_sensor.h>
#include <Sensors/Adapter/SensorsReadAdapter.h>

#include <Modules/Display.h>
#include <Modules/Bluetooth_module.h>
#include <Modules/RTC.h>

#define logg(message) loggWithBase(message, "MAIN")
#define loggWithContext(message, context) loggWithContext(message, context, "MAIN")

// Global variables
Bluetooth_module bluetoothModule; // Bluetooth module
RTC rtc; // RTC module
SDcard sdcard; // SD card
SensorsReadAdapter sensorsReadAdapter(&bluetoothModule, &sdcard, &rtc); // Adapter that handles sensor data
CO2Sensor co2Sensor; // CO2 sensor
PMSensor pmSensor; // PM sensor
BMESensor bmeSensor; // BME sensor
Display display; // Display

SleepUtils sleepUtils;

// Function prototypes
void enter_sleep_mode();
void exit_sleep_mode();
void configure_wakeup_sources();
void treat_wakeup_reason();
void act_on_wakeup_button();
void handle_bluetooth_updates();
void send_historical_data();
void configure_watchdog();

void setup() {
	// Disable WiFi
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_stop();

	// Configure serial
	Serial.begin(115200);

	// Increase watchdog timer to 1.5s
	configure_watchdog();
	
	// Configure SPI
	SPI.begin(_CLK_PIN, _MISO_PIN, _MOSI_PIN, SD_CS_PIN);

	// Configure ADC
	analogReadResolution(ADC_RESOLUTION);

	// Set buck-boost converter pin as output
	pinMode(BUCK_EN_PIN, OUTPUT);
	// Set battery voltage pin as input
	pinMode(BATTERY_PIN, INPUT);

	// Treat wakeup reason (also initializes modules)
	treat_wakeup_reason();
}

void loop() {
	// Check if there is a sensor to read
	handle_sensor_readings();

	// Check if there is a button pressed
	handle_button_readings();

	// Check if there is a bluetooth status update
	handle_bluetooth_updates();

	// Check if there is historical data to send
	send_historical_data();

	// Check if we can sleep
	if (sleepUtils.is_sleep_allowed()) {
		enter_sleep_mode();
	}

	// Reactivate modules if sleep is over
	if (sleepUtils.is_sleep_over()) {
		exit_sleep_mode();
	}
}

void configure_watchdog() {
	wdt_hal_context_t rtc_wdt_ctx;
    wdt_hal_init(&rtc_wdt_ctx, WDT_RWDT, 0, false);
    uint32_t stage_timeout_ticks = (uint32_t)(1500ULL * rtc_clk_slow_freq_get_hz() / 1000ULL);
    wdt_hal_write_protect_disable(&rtc_wdt_ctx);
    wdt_hal_config_stage(&rtc_wdt_ctx, WDT_STAGE0, stage_timeout_ticks, WDT_STAGE_ACTION_RESET_SYSTEM);
    wdt_hal_config_stage(&rtc_wdt_ctx, WDT_STAGE1, stage_timeout_ticks, WDT_STAGE_ACTION_RESET_RTC);
    wdt_hal_write_protect_enable(&rtc_wdt_ctx);
}

void handle_bluetooth_updates() {
	if (bluetoothModule.getStatusUpdates() != BLE_STATUS::NO_UPDATE) {
		// Update display
		if (bluetoothModule.getStatusUpdates() & BLE_STATUS::CLIENT_UPDATE) {
			display.updateScreen(SCREENUPDATE::BLUETOOTH);
			bluetoothModule.acknowledgeStatusUpdates(BLE_STATUS::CLIENT_UPDATE);
		}

		// Fetch timestamp
		if (bluetoothModule.getStatusUpdates() & BLE_STATUS::TIMESTAMP_UPDATE) {
			rtc.setFromTimestamp(bluetoothModule.getTimestamp());
			bluetoothModule.acknowledgeStatusUpdates(BLE_STATUS::TIMESTAMP_UPDATE);
		}
	}
}

void send_historical_data() {
	if (sensorsReadAdapter.ableToSendHistoricalData()) {
		// Temporarily disable sensors activity
		pause_active_timers();

		// Change screen to sending screen
		display.showSendingScreen();

		// Send historical data
		sensorsReadAdapter.sendHistoricalData();

		// Revert screen to previous state
		display.updateScreen(SCREENUPDATE::GENERAL);

		// Reenable sensors activity
		enable_paused_timers();
	}
}

void enter_sleep_mode() {
  	logg("Entering deep sleep mode");

	// Change sensors state to sleep
	pmSensor.sleep();
  
	// Enable deep sleep wakes
	configure_wakeup_sources();

	// Disable timers
	disable_active_timers();

	// Flush debug messages
	forcePrint();

	// Enter sleep mode
	esp_deep_sleep_start();
}

void exit_sleep_mode() {
	logg("Exiting deep sleep mode");

	// Turn on sensors and initialize after heating up
	init_timer_init_sensors();

	// Mark sleep cycle as completed
	sleepUtils.mark_sleep_treated();

	// Disable timer for cooldown after sleep wake-up caused by buttons
	disable_timer_reenable_sleep();

	// Re-initialize modules
	sensorsReadAdapter.init();
	
	// Show loading screen
	display.showLoadingScreen();
}

void configure_wakeup_sources() {
  // Timer wakeup for sensor readings
  esp_sleep_enable_timer_wakeup(WAIT_TIME_PREPARE_SENSORS);
  // ext1 wakeup for button presses
  esp_sleep_enable_ext1_wakeup(BUTTONS_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void treat_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
	case ESP_SLEEP_WAKEUP_TIMER:
		loggWithContext("Wakeup caused by timer", "wakeup_reason");

		// Read sensors, go back to sleep (BLE is off)
		init_sensors();            // Initialize sensors
		display.init();            // Initialize display
		sensorsReadAdapter.init(); // Initialize SD card (BLE is still off)
		init_timer_read_sensors(); // Initialize timer for sensor readings
		force_read_sensors_interrupt(); // Force sensor readings
		sleepUtils.allow_sleep_after_read(); // Enable sleep mode after sensor readings
		break;

	case ESP_SLEEP_WAKEUP_EXT1:
		loggWithContext("Wakeup caused by button press", "wakeup_reason");

		// Momentarily disable sleep mode (until cooldown is over)
		sleepUtils.disable_sleep();
		sleepUtils.enable_cooldown();
		init_timer_reanable_sleep();

		act_on_wakeup_button(); // Determine which button was pressed
		display.init();         // Initialize display
		init_buttons();         // Initialize buttons
		break;

	default:
		loggWithContext("Wakeup was not caused by timer nor GPIO", "wakeup_reason");

		// Normal boot
		init_timer_init_sensors();   // Initialize sensors after heating up
		display.init();              // Initialize display
		display.showLoadingScreen(); // Show loading screen
		sensorsReadAdapter.init();   // Initialize SD card and BLE
		init_buttons();              // Initialize buttons
		break;
  }
}

void act_on_wakeup_button() {
	uint64_t wakeup_pin = esp_sleep_get_ext1_wakeup_status();
	wakeup_pin = log(wakeup_pin)/log(2);

	if (wakeup_pin == BUTTON_LEFT_PIN) {
		virtual_press_button(BUTTONS::BUTTON_LEFT);
	} else if (wakeup_pin == BUTTON_CENTER_PIN) {
		virtual_press_button(BUTTONS::BUTTON_CENTER);
	} else if (wakeup_pin == BUTTON_RIGHT_PIN) {
		virtual_press_button(BUTTONS::BUTTON_RIGHT);
	}
}