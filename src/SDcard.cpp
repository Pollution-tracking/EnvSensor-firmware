#include <SDcard.h>

// Construct SD card
SDcard::SDcard() {
}

// Routine to initialize SD card
void SDcard::init() {
    logg("Initializing SD card...");
    if (!SD.begin(SD_CS_PIN, SPI, 4000000)) {
        logg("SD card initialization failed!");
    } else {
        logg("SD card initialized");
    }
}