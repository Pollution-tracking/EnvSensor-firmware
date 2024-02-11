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
    private:
        uint8_t pin;
        uint64_t lastInterruptTime;
        uint64_t buttonPressTime;
        uint8_t buttonDigitalState;
        uint8_t buttonState;
        void IRAM_ATTR ISR_button();
        static Button *instance_left;
        static Button *instance_right;
        static void IRAM_ATTR ISR_button_left();
        static void IRAM_ATTR ISR_button_right();
};

#endif // BUTTON_H