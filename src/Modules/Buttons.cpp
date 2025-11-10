#include "Modules/Buttons.h"

#define logg(message) loggWithObj(message, "BUTTONS")
#define loggValue(message, value) loggWithCtx(message, "BUTTONS", value)

static volatile unsigned long last_interrupt_time = 0;

// Button left ISR
void IRAM_ATTR ISR_button_left() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > debounce_time) {
        last_interrupt_time = interrupt_time;
        board_config.buttons_state = BUTTONS::BUTTON_LEFT;
        board_config.to_treat = true;
    }
}

// Button center ISR
void IRAM_ATTR ISR_button_center() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > debounce_time) {
        last_interrupt_time = interrupt_time;
        board_config.buttons_state = BUTTONS::BUTTON_CENTER;
        board_config.to_treat = true;
    }
}

// Button right ISR
void IRAM_ATTR ISR_button_right() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > debounce_time) {
        last_interrupt_time = interrupt_time;
        board_config.buttons_state = BUTTONS::BUTTON_RIGHT;
        board_config.to_treat = true;
    }
}

// GPIO button initialization
static void init_buttons() {
    logg(INITIALIZING);

    pinMode(BUTTON_LEFT_PIN, INPUT);
    pinMode(BUTTON_CENTER_PIN, INPUT);
    pinMode(BUTTON_RIGHT_PIN, INPUT);

    attachInterrupt(BUTTON_LEFT_PIN, ISR_button_left, RISING);
    attachInterrupt(BUTTON_CENTER_PIN, ISR_button_center, RISING);
    attachInterrupt(BUTTON_RIGHT_PIN, ISR_button_right, RISING);

    logg(INITIALIZED);
}

// Main loop handler for buttons events (returns completed action)
BUTTONS handleButtonsState(const BUTTONS state) {
    // Early exit if no action is required
    if (state == BUTTONS::NO_BUTTON) {
        goto default_exit;
    }

    if (state == BUTTONS::INIT_BUTTONS) {
        init_buttons();

        return BUTTONS::INIT_BUTTONS;
    }

    if (state == BUTTONS::BUTTON_SLEEP) {
        init_buttons();
        act_on_wakeup_button();
    }

    // Some button was pressed
    return BUTTONS::BUTTON_PRESSED;

default_exit:
    return BUTTONS::NO_BUTTON;
}

void act_on_wakeup_button() {
	uint64_t wakeup_pin = esp_sleep_get_ext1_wakeup_status();
	wakeup_pin = log(wakeup_pin)/log(2);

	if (wakeup_pin == BUTTON_LEFT_PIN) {
		board_config.buttons_state = BUTTONS::BUTTON_LEFT;
	} else if (wakeup_pin == BUTTON_CENTER_PIN) {
		board_config.buttons_state = BUTTONS::BUTTON_CENTER;
	} else if (wakeup_pin == BUTTON_RIGHT_PIN) {
		board_config.buttons_state = BUTTONS::BUTTON_RIGHT;
	}
}