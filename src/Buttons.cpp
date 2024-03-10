#include "Buttons.hpp"

uint64_t lastInterruptTime;
uint8_t pressed_button = BUTTONS::NO_BUTTON;

// Button left ISR
void IRAM_ATTR ISR_button_left() {
    uint64_t interruptTime = millis();
    
    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Button being pressed
        pressed_button |= BUTTONS::BUTTON_LEFT;

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;
    }
}

// Button center ISR
void IRAM_ATTR ISR_button_center() {
    uint64_t interruptTime = millis();
    
    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Button being pressed
        pressed_button |= BUTTONS::BUTTON_CENTER;

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;
    }
}

// Button right ISR
void IRAM_ATTR ISR_button_right() {
    uint64_t interruptTime = millis();

    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Button being pressed
        pressed_button |= BUTTONS::BUTTON_RIGHT;

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;
    }
}

void init_buttons() {
    pinMode(BUTTON_LEFT_PIN, INPUT);
    pinMode(BUTTON_CENTER_PIN, INPUT);
    pinMode(BUTTON_RIGHT_PIN, INPUT);

    attachInterrupt(BUTTON_LEFT_PIN, ISR_button_left, FALLING);
    attachInterrupt(BUTTON_CENTER_PIN, ISR_button_center, FALLING);
    attachInterrupt(BUTTON_RIGHT_PIN, ISR_button_right, FALLING);
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

    // Restart timer for reenabling sleep mode if needed
    if (sleepUtils.is_cooldown_enabled()) {
        restart_timer_reenable_sleep();
    }

    // Update display
    display.updateScreen();
}

void check_left_button() {
    if (pressed_button & BUTTONS::BUTTON_LEFT) {
        // Clear flag
        pressed_button &= ~BUTTONS::BUTTON_LEFT;

        treat_left_button();
    }
}

void check_center_button() {
    if (pressed_button & BUTTONS::BUTTON_CENTER) {
        // Clear flag
        pressed_button &= ~BUTTONS::BUTTON_CENTER;

        treat_center_button();
    }
}

void check_right_button() {
    if (pressed_button & BUTTONS::BUTTON_RIGHT) {
        // Clear flag
        pressed_button &= ~BUTTONS::BUTTON_RIGHT;

        treat_right_button();
    }
}

void treat_left_button() {
    logg("Button Left press");
    display.changeScreenLeft();
}

void treat_center_button() {
    logg("Button Center press");
    // Check if screen is interactive (blueooth screen)
    if (display.getScreenMode() == SCREENMODE::BLUETOOTH) {
        // Change BLE state
        if (bluetoothModule.isEnabled()) {
            bluetoothModule.disable();
            sleepUtils.allow_sleep();
        } else {
            bluetoothModule.enable();
            sleepUtils.disable_sleep();
            // When reenabling BLE after sleep, disable cooldown and reactivate timer for sensors
            sleepUtils.disable_cooldown();
            disable_timer_reenable_sleep();
            init_timer_read_sensors();
        }
    }
}

void treat_right_button() {
    logg("Button Right press");
    display.changeScreenRight();
}