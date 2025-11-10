#ifndef BUTTON_CONSTANTS_H
#define BUTTON_CONSTANTS_H

#include <Arduino.h>

// Each bit represents one of the buttons
enum BUTTONS {
    NO_BUTTON,
    BUTTON_LEFT,
    BUTTON_CENTER,
    BUTTON_RIGHT,
    BUTTON_SLEEP,
    INIT_BUTTONS,
    BUTTON_PRESSED
};

// 500ms debounce time
const unsigned long debounce_time = 500;


#endif // BUTTON_CONSTANTS_H