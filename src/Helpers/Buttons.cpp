#include "Helpers/Buttons.hpp"

#define logg(message) loggWithBase(message, "BUTTONS")
#define loggWithContext(message, context) loggWithContext(message, context, "BUTTONS")

uint64_t lastInterruptTime;
uint8_t pressed_button = BUTTONS::NO_BUTTON;

// Button left ISR
void IRAM_ATTR ISR_button_left() {
    pressed_button = BUTTONS::BUTTON_LEFT;
}

// Button center ISR
void IRAM_ATTR ISR_button_center() {
    pressed_button = BUTTONS::BUTTON_CENTER;
}

// Button right ISR
void IRAM_ATTR ISR_button_right() {
    pressed_button = BUTTONS::BUTTON_RIGHT;
}

void init_buttons() {
    pinMode(BUTTON_LEFT_PIN, INPUT);
    pinMode(BUTTON_CENTER_PIN, INPUT);
    pinMode(BUTTON_RIGHT_PIN, INPUT);

    attachInterrupt(BUTTON_LEFT_PIN, ISR_button_left, ONHIGH);
    attachInterrupt(BUTTON_CENTER_PIN, ISR_button_center, ONHIGH);
    attachInterrupt(BUTTON_RIGHT_PIN, ISR_button_right, ONHIGH);

    logg("Initialized");
}

void handle_button_readings() {
    // Early exit if no button was pressed
    if (pressed_button == BUTTONS::NO_BUTTON) {
        return;
    }

    // Left button press
    check_left_button();

    // Center button press
    check_center_button();

    // Right button press
    check_right_button();

    // Clear flag
    pressed_button = BUTTONS::NO_BUTTON;

    // Restart timer for reenabling sleep mode if needed
    if (sleepUtils.is_cooldown_enabled()) {
        restart_timer_reenable_sleep();
    }
}

void check_left_button() {
    if (pressed_button == BUTTONS::BUTTON_LEFT) {
        treat_left_button();
    }
}

void check_center_button() {
    if (pressed_button == BUTTONS::BUTTON_CENTER) {
        treat_center_button();
    }
}

void check_right_button() {
    if (pressed_button == BUTTONS::BUTTON_RIGHT) {
        treat_right_button();
    }
}

void treat_left_button() {
    loggWithContext("Pressed", "Left");
    display.changeScreenLeft();

    // Update display
    display.updateScreen();
}

void treat_center_button() {
    loggWithContext("Pressed", "Center");
    // Check if screen is interactive (blueooth screen)
    if (display.getScreenMode() == SCREENMODE::BLUETOOTH) {
        loggWithContext("Acting", "Center");
        // Change BLE state
        if (bluetoothModule.isEnabled()) {
            sleepUtils.allow_sleep();
            bluetoothModule.disable();
        } else {
            sleepUtils.disable_sleep(true);
            sleepUtils.disable_cooldown();
            bluetoothModule.enable();
        }

        // Update display
        display.updateScreen(SCREENUPDATE::BLUETOOTH);
    }
}

void treat_right_button() {
    loggWithContext("Pressed", "Right");
    display.changeScreenRight();

    // Update display
    display.updateScreen();
}

void virtual_press_button(uint8_t button) {
    logg("Virtual button press");
    pressed_button = button;
}