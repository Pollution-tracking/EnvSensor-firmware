#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include "pins.h"
#include <logger.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDcard {
    public:
        SDcard();
        void init();
    private:
};

#endif // SDCARD_H