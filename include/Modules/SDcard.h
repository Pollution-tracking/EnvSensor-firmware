#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Resources/pins.h>
#include <Logger/logger.h>
#include <Sensors/sensor.h>
#include <Resources/Constants/storage_constants.h>
#include <Resources/RTC_values.h>

class SDcard {
    public:
        SDcard();
        ~SDcard();
        void init();
        bool isInitialised();
        bool hasCard();
        bool haveHistoricalData();
        bool writeHistoricalData(String data);
        String getHistoricalData();
    private:
        bool _initialised = false;
        bool _hasCard = false;
        int _storedDataCount = 0;
        int _sentDataCount = 0;
        bool loadStats(); // loads stats from file into _storedDataCount/_sentDataCount
        bool saveStats(); // saves _storedDataCount/_sentDataCount to file
        bool fileExists(String path);
        bool writeData(String path, String data);
        bool writeToFile(String path, String message);
        bool appendToFile(String path, String message);
        bool deleteFile(String path);
};

#endif // SD_CARD_H