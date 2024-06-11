#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include "Resources/pins.h"
#include "Resources/constants.h"
#include <Logger/logger.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDcard {
    public:
        SDcard();
        ~SDcard();
        void init();
        bool isInitialised();
        bool haveHistoricalData();
        bool deleteHistoricalData();
        bool writeHistoricalData(String data);
    private:
        bool _initialised = false;
        bool fileExists(String path);
        bool writeData(String path, String data);
        bool writeToFile(String path, String message);
        bool appendToFile(String path, String message);
        bool deleteFile(String path);
};

#endif // SDCARD_H