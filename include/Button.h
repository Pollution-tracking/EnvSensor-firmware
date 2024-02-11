#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "constants.h"

class Button {
    public:
        Button(uint8_t pin);
        void init(BUTTONS button);
        uint8_t getButtonState();
        void resetButtonState();
        uint64_t getButtonPressTime();
    private:
        uint8_t pin;
        BUTTONS button;
        uint64_t lastInterruptTime;
        uint64_t buttonPressTime;
        uint8_t buttonState;
        void IRAM_ATTR ISR_button_low();
        void IRAM_ATTR ISR_button_high();
        static Button *instance_left;
        static Button *instance_right;
        static void IRAM_ATTR ISR_button_left_low();
        static void IRAM_ATTR ISR_button_left_high();
        static void IRAM_ATTR ISR_button_right_low();
        static void IRAM_ATTR ISR_button_right_high();
};

#endif // BUTTON_H