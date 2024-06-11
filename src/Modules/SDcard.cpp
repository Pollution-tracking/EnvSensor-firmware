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
}

bool SDcard::isInitialised() {
    return _initialised;
}

bool SDcard::writeHistoricalData(String data) {
    loggWithContext("Storing to file", "HistoricalData");

    return writeData(dataPath, data);
}

bool SDcard::haveHistoricalData() {
    if (!isInitialised()) {
        return false;
    }

    return fileExists(dataPath);
}

bool SDcard::deleteHistoricalData() {
    if (!isInitialised()) {
        return false;
    }

    loggWithContext("Deleting file", "HistoricalData");

    return deleteFile(dataPath);
}

bool SDcard::writeData(String path, String data) {
    if(!fileExists(path)) {
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
    File file = SD.open(path, FILE_WRITE);
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
    File file = SD.open(path, FILE_APPEND);
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