#include "Modules/SDcard.h"

#define logg(message) loggWithObj(message, "SD")
#define loggValue(message, value) loggWithCtx(message, "SD", value)

// Construct SD card
SDcard::SDcard() {
    _storedDataCount = 0;
    _sentDataCount = 0;
}

// Destruct SD card
SDcard::~SDcard() { }

// Routine to initialize SD card
void SDcard::init() {
    logg(INITIALIZING);

    bool ret = SD.begin(SD_CS_PIN, SPI, 4000000);

    // Wait for module to start
	delay(100);

    if (ret == true) {
        logg(INITIALIZED);

        _initialised = true;

        if (SD.cardType() != CARD_NONE) {
            logg("Card type: " + String(SD.cardType()));
            _hasCard = true;
            if (!this->fileExists(STATS_PATH)) {
                this->saveStats();
            }
            loadStats();
        } else {
             logg("No card attached");
            _hasCard = false;
        }
    } else {
        logg(NOT_INITIALIZED);
        _initialised = false;
        _hasCard = false;
    }
}

bool SDcard::isInitialised() {
    return _initialised;
}

bool SDcard::hasCard() {
    return _hasCard;
}

bool SDcard::haveHistoricalData() {
    bool res = loadStats();
    return res && _storedDataCount > _sentDataCount;
}

// Routine to write historical data updates to SD card
bool SDcard::writeHistoricalData(String data, String header) {
    loggValue("Storing to file", "HistoricalData");
    
    bool res = true;
    bool isNewFile = !this->haveHistoricalData();
    
    if (isNewFile) {
        // Overwrite or create file
        if (header.length() > 0) {
            res &= writeToFile(DATA_PATH, header);
            res &= appendToFile(DATA_PATH, data);
        } else {
            res &= writeToFile(DATA_PATH, data);
        }
    } else {
        res &= appendToFile(DATA_PATH, data);
    }
    
    if (res) {
        res &= loadStats();
        _storedDataCount++;
        res &= saveStats();
    }

    return res;
}

String SDcard::getHistoricalData() {
    int lineIdx = 0;
    int newLinesRead = 0;
    String result = "";

    loadStats();
    if (_sentDataCount >= _storedDataCount) {
        return NO_MORE_DATA;
    }

    // Read data file
    File dataFile = SD.open(DATA_PATH, FILE_READ);
    if (!dataFile) {
        loggValue("Error opening data file", "HistoricalData");
        return ERROR_READING;
    }

    // Skip already sent lines
    while(lineIdx < _sentDataCount && dataFile.available()) {
        dataFile.readStringUntil('\n');
        lineIdx++;
    }

    // Read a batch of new lines
    while (dataFile.available() && lineIdx < _storedDataCount && newLinesRead < MAX_LINES_TO_RETRIEVE) {
        String line = dataFile.readStringUntil('\n');
        if (line.length() > 0) {
            result += line + "\n";
            newLinesRead++;
        }
        lineIdx++;
    }
    dataFile.close();

    // Update sentDataCount
    _sentDataCount = lineIdx;
    saveStats();

    // Send NO_MORE_DATA if nothing new was read
    if (result.length() == 0) {
        return NO_MORE_DATA;
    }

    return result;
}

bool SDcard::writeData(String path, String data) {
    if (!this->haveHistoricalData()) {
        return writeToFile(path, data);
    } else {
        return appendToFile(path, data);
    }
}

bool SDcard::fileExists(String path) {
    File file = SD.open(path);

    if (!file) {
        return false;
    }
    file.close();

    return true;
}

bool SDcard::writeToFile(String path, String message) {
    // Open file for writing
    File file = SD.open(path, FILE_WRITE, true);
    if (!file) {
        return false;
    }

    // Format message
    if (message.length() == 0 || message.charAt(message.length() - 1) != '\n') {
        message += "\n";
    }
    
    // Write to file
    if (!file.print(message)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SDcard::appendToFile(String path, String message) {
    // Open file for appending
    File file = SD.open(path, FILE_APPEND, true);
    if (!file) {
        return false;
    }

    // Format message
    if (message.length() == 0 || message.charAt(message.length() - 1) != '\n') {
        message += "\n";
    }
    
    // Write to file
    if (!file.print(message)) {
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

bool SDcard::loadStats() {
    _storedDataCount = 0;
    _sentDataCount = 0;

    File statsFile = SD.open(STATS_PATH, FILE_READ);
    if (statsFile) {
        while (statsFile.available()) {
            String line = statsFile.readStringUntil('\n');
            if (line.startsWith("storedDataCount:")) {
                _storedDataCount = line.substring(line.indexOf(":") + 1).toInt();
            } else if (line.startsWith("sentDataCount:")) {
                _sentDataCount = line.substring(line.indexOf(":") + 1).toInt();
            }
        }
        statsFile.close();
        return true;
    }

    logg("Could not load stats file");
    return false;
}

bool SDcard::saveStats() {
    File statsFile = SD.open(STATS_PATH, FILE_WRITE, true);
    if (statsFile) {
        statsFile.print("storedDataCount: ");
        statsFile.println(_storedDataCount);
        statsFile.print("sentDataCount: ");
        statsFile.println(_sentDataCount);
        statsFile.close();

        return true;
    }

    logg("Could not save stats file");
    return false;
}