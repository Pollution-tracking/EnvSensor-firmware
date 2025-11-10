#include "Modules/BoardUtilities.h"

#define logg(message) loggWithObj(message, "BOARD")
#define loggValue(message, value) loggWithCtx(message, "BOARD", value)

// Construct Board Utilities module
BoardUtilities::BoardUtilities() {
}

// Destruct Board Utilities module
BoardUtilities::~BoardUtilities() {
}

void BoardUtilities::configureWakeupSources() {
    // Timer wakeup for sensor readings
    esp_sleep_enable_timer_wakeup(WAIT_TIME_PREPARE_SENSORS);
    // ext1 wakeup for button presses
    esp_sleep_enable_ext1_wakeup(BUTTONS_MASK, ESP_EXT1_WAKEUP_ANY_HIGH);
}

// Routine to treat the wakeup reason
void BoardUtilities::treatWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_TIMER:
            loggValue("Wakeup caused by timer", "wakeup_reason");
            // Initialize sensors
            board_config.sensors_state = SENSORS::INIT_SENSORS;
            // Start sensor reading timer
            board_config.timer_type = TIMER_TYPES::T_READ;
            board_config.timer_mode = TIMER_MODES::T_ACTIVE;
            // Set board state to partially awake from sleep, but will go back
            board_config.board_state = SLEEP_STATE::FROM_SLEEP;
            break;

        case ESP_SLEEP_WAKEUP_EXT1:
            loggValue("Wakeup caused by button press", "wakeup_reason");
            // Initialize buttons and act on press
            board_config.buttons_state = BUTTONS::BUTTON_SLEEP;
            // Start sleep re-enable timer
            board_config.timer_type = TIMER_TYPES::T_SLEEP;
            board_config.timer_mode = TIMER_MODES::T_ACTIVE;
            // Set board state to partially awake from sleep, but will go back
            board_config.board_state = SLEEP_STATE::FROM_SLEEP;
            break;

        default:
            loggValue("Wakeup was not caused by timer nor GPIO", "wakeup_reason");
            // Display welcome screen
            board_config.screen = SCREEN_MODE::LOADING;
            // Initialize sensors
            board_config.sensors_state = SENSORS::INIT_SENSORS;
            // Initialize buttons
            board_config.buttons_state = BUTTONS::INIT_BUTTONS;
            // Initialize Bluetooth
            board_config.ble_state = BLE::INIT_BLE;
            // Start sensor heating timer
            board_config.timer_type = TIMER_TYPES::T_HEAT;
            board_config.timer_mode = TIMER_MODES::T_ACTIVE;
            // Set board state to awake
            board_config.board_state = SLEEP_STATE::AWAKE;
            break;
    }

    board_config.to_treat = true;
}

void BoardUtilities::enableBuck() {
    digitalWrite(BUCK_EN_PIN, HIGH);
    logg("Buck-boost converter enabled");
}

void BoardUtilities::disableBuck() {
    digitalWrite(BUCK_EN_PIN, LOW);
    logg("Buck-boost converter disabled");
}

void BoardUtilities::toggleSleepState() {
    switch (board_config.board_state) {
        case SLEEP_STATE::AWAKE:
            board_config.board_state = SLEEP_STATE::TO_SLEEP;
            logg("Board is set to go to sleep");
            break;
        case SLEEP_STATE::FROM_SLEEP:
            board_config.board_state = SLEEP_STATE::AWAKENING;
            logg("Board is set to recover from sleep");
            break;
        default:
            logg("Cannot toggle from this sleep state!");
            return;
    }

    board_config.to_treat = true;
}
