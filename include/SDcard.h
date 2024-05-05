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
        bool isInitialised();
        bool writeData(String path, String data);
    private:
        bool _initialised = false;
        bool fileExists(String path);
        bool writeToFile(String path, String message);
        bool appendToFile(String path, String message);
};

#endif // SDCARD_H