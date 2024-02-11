#include "Button.h"

Button *Button::instance_left;
Button *Button::instance_right;

void IRAM_ATTR Button::ISR_button_left_low() {
    instance_left->ISR_button_low();
}

void IRAM_ATTR Button::ISR_button_left_high() {
    instance_left->ISR_button_high();
}

void IRAM_ATTR Button::ISR_button_right_low() {
    instance_right->ISR_button_low();
}

void IRAM_ATTR Button::ISR_button_right_high() {
    instance_right->ISR_button_high();
}

// ISR routine
void IRAM_ATTR Button::ISR_button_low() {
    uint64_t interruptTime = millis();
    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Button being pressed
        buttonPressTime = interruptTime;

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;

        // Change interrupt to rising edge
        switch (this->button) {
            case BUTTONS::LEFT:
                attachInterrupt(pin, &Button::ISR_button_left_high, RISING);
                break;
            case BUTTONS::RIGHT:
                attachInterrupt(pin, &Button::ISR_button_right_high, RISING);
                break;
        }
    }
}

void IRAM_ATTR Button::ISR_button_high() {
    uint64_t interruptTime = millis();
    // If interrupts come faster than debounce time, assume it's a bounce and ignore
    if (interruptTime - lastInterruptTime > DEBOUNCE_TIME) {
        // Button being released
        buttonPressTime = interruptTime - buttonPressTime;

        // Store last treated interrupt time
        lastInterruptTime = interruptTime;

        // Set button state
        if (buttonPressTime < LONG_PRESS_THRESHOLD) {
            buttonState = BUTTON_STATES::SHORT_PRESS;
        } else {
            buttonState = BUTTON_STATES::LONG_PRESS;
        }

        // Change interrupt to falling edge
        switch (this->button) {
            case BUTTONS::LEFT:
                attachInterrupt(pin, &Button::ISR_button_left_low, FALLING);
                break;
            case BUTTONS::RIGHT:
                attachInterrupt(pin, &Button::ISR_button_right_low, FALLING);
                break;
        }
    }
}

// Construct button object
Button::Button(uint8_t pin) : pin(pin) {
    lastInterruptTime = 0;
    buttonPressTime = 0;
    buttonState = BUTTON_STATES::NO_PRESS;
}

// Set pin type and attach interrupt (parameter defines which button is being initialized)
void Button::init(BUTTONS button) {
    this->button = button;
    pinMode(pin, INPUT);
    switch (button) {
        case BUTTONS::LEFT:
            instance_left = this;
            attachInterrupt(pin, &Button::ISR_button_left_low, FALLING);
            break;
        case BUTTONS::RIGHT:
            instance_right = this;
            attachInterrupt(pin, &Button::ISR_button_right_low, FALLING);
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

// Return button press time
uint64_t Button::getButtonPressTime() {
    return buttonPressTime;
}