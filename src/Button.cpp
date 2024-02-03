#include "Button.h"

Button *Button::instance_left;
Button *Button::instance_right;

void IRAM_ATTR Button::ISR_button_left() {
    instance_left->ISR_button();
}

void IRAM_ATTR Button::ISR_button_right() {
    instance_right->ISR_button();
}

// ISR routine
void IRAM_ATTR Button::ISR_button() {
    uint64_t interruptTime = millis();
    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        if (buttonDigitalState == HIGH) {
            // Button being pushed
            buttonPressTime = interruptTime;
            buttonDigitalState = LOW;
        } else {
            // Button being released
            buttonPressTime = interruptTime - buttonPressTime;
            buttonDigitalState = HIGH;

            // Decide if it was a short or long press
            if (buttonPressTime >= LONG_PRESS_THRESHOLD) {
                buttonState = BUTTON_STATES::LONG_PRESS;
            } else {
                buttonState = BUTTON_STATES::SHORT_PRESS;
            }
        }

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;
    }
}

// Construct button object
Button::Button(uint8_t pin) : pin(pin) {
    lastInterruptTime = 0;
    buttonPressTime = 0;
    buttonDigitalState = HIGH;
    buttonState = BUTTON_STATES::NO_PRESS;
}

// Set pin type and attach interrupt (parameter defines which button is being initialized)
void Button::init(BUTTONS button) {
    pinMode(pin, INPUT);
    switch (button) {
        case BUTTONS::LEFT:
            instance_left = this;
            attachInterrupt(pin, &Button::ISR_button_left, CHANGE);
            break;
        case BUTTONS::RIGHT:
            instance_right = this;
            attachInterrupt(pin, &Button::ISR_button_right, CHANGE);
            break;
    }
}

// Return button state
uint8_t Button::getButtonState() {
    return buttonState;
}

// Reset button state
void Button::resetButtonState() {
    buttonState = BUTTON_STATES::NO_PRESS;
}