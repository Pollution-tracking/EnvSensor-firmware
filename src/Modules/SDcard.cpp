#include "Modules/SDcard.h"

// Construct SD card
SDcard::SDcard() {
}

// Routine to initialize SD card
void SDcard::init() {
    if (!SD.begin(SD_CS_PIN, SPI, 4000000)) {
        logg("SD card initialization failed!");
        return;
    }

    logg("SD card module found.");

    if(SD.cardType() == CARD_NONE){
        logg("No SD card attached");
        return;
    }

    logg("SD card initialised.");

    _initialised = true;
}

bool SDcard::isInitialised() {
    return _initialised;
}

bool SDcard::writeData(String path, String data) {
    if(!fileExists(path)){
        return writeToFile(path, data);
    } else {
        return appendToFile(path, data);
    }
}

bool SDcard::fileExists(String path) {
    File file = SD.open(path);

    if(!file){
        return false;
    }
    file.close();

    return true;
}

bool SDcard::writeToFile(String path, String message) {
    // Open file for writing
    File file = SD.open(path, FILE_WRITE);
    if(!file){
        return false;
    }

    // Format message (TODO: timestamp)
    message += "\n";
    
    // Write to file
    if(!file.print(message)){
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SDcard::appendToFile(String path, String message) {
    // Open file for appending
    File file = SD.open(path, FILE_APPEND);
    if(!file){
        return false;
    }

    // Format message (TODO: timestamp)
    message += "\n";
    
    // Write to file
    if(!file.print(message)){
        file.close();
        return false;
    }

    file.close();
    return true;
}