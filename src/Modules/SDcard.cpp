#include "Modules/SDcard.h"

#define logg(message) loggWithBase(message, "SD")
#define loggWithContext(message, context) loggWithContext(message, context, "SD")

// Construct SD card
SDcard::SDcard() { }

// Destruct SD card
SDcard::~SDcard() { }

// Routine to initialize SD card
void SDcard::init() {
    if (!SD.begin(SD_CS_PIN, SPI, 4000000)) {
        logg("Initialization failed!");
        return;
    }

    logg("Module found");

    if(SD.cardType() == CARD_NONE) {
        logg("No card attached");
        return;
    }

    logg("Initialized");

    _initialised = true;
    hasHistoricalData = fileExists(dataPath);
}

bool SDcard::isInitialised() {
    return _initialised;
}

// Routine to write historical data updates to SD card
bool SDcard::writeHistoricalData(String data) {
    loggWithContext("Storing to file", "HistoricalData");

    bool writeRes = writeData(dataPath, data);

    if (writeRes) {
        hasHistoricalData = true;
    }

    return writeRes;
}

// Routine to check if historical data are stored on SD card
bool SDcard::haveHistoricalData() {
    return hasHistoricalData;
}

// Routine to delete all historical data from SD card
bool SDcard::deleteHistoricalData() {
    if (!isInitialised()) {
        return false;
    }

    loggWithContext("Deleting file", "HistoricalData");

    bool deleteRes = deleteFile(dataPath);

    if (deleteRes) {
        hasHistoricalData = false;
    }

    return deleteRes;
}

bool SDcard::writeData(String path, String data) {
    if(!hasHistoricalData) {
        return writeToFile(path, data);
    } else {
        return appendToFile(path, data);
    }
}

bool SDcard::fileExists(String path) {
    File file = SD.open(path);

    if(!file) {
        return false;
    }
    file.close();

    return true;
}

bool SDcard::writeToFile(String path, String message) {
    // Open file for writing
    File file = SD.open(path, FILE_WRITE, true);
    if(!file) {
        return false;
    }

    // Format message
    message += "\n";
    
    // Write to file
    if(!file.print(message)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SDcard::appendToFile(String path, String message) {
    // Open file for appending
    File file = SD.open(path, FILE_APPEND, true);
    if(!file) {
        return false;
    }

    // Format message
    message += "\n";
    
    // Write to file
    if(!file.print(message)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SDcard::deleteFile(String path) {
    if(!SD.remove(path)) {
        return false;
    }

    return true;
}