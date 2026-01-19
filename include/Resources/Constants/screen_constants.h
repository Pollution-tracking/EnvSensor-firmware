#ifndef SCREEN_CONSTANTS_H
#define SCREEN_CONSTANTS_H

#include <Arduino.h>

// Partial refresh configuration
// After this many partial refreshes, a full refresh will be performed to prevent ghosting
const uint8_t FULL_REFRESH_INTERVAL = 5;

enum class SCREEN_REFRESH {
    GENERAL,
    BLUETOOTH,
    SENSORS
};

enum class SCREEN_MODE {
    NO_SCREEN = -1,
    LOADING,
    BLUETOOTH,
    SENSORS,
    ENVIRONMENTAL,
    POLLUTANTS,
    SENDING,
    HEATING
};

const String LoadingScreenText[2] = {
    "Welcome",
    "CityAirQ!"
};

const String HeatingScreenText[2] = {
    "Sensors",
    "heating"
};

const String SendingScreenText[2] = {
    "Syncing",
    "historical data"
};

const String BluetoothScreenText[3] = {
    "Bluetooth",
    "Press button to",
    "toggle BLE"
};

const String SensorsScreenText[1] = {
    "Sensors"
};

const String EnvironmentalScreenText[1] = {
    "Ambient"
};

const String PollutantsScreenText[1] = {
    "Pollution"
};

// ============================================================================
// SCREEN LAYOUT CONSTANTS
// ============================================================================

// Common display dimensions
const uint16_t DISPLAY_WIDTH = 200;
const uint16_t DISPLAY_HEIGHT = 200;

// --- BLUETOOTH SCREEN LAYOUT ---
namespace BluetoothLayout {
    // Icon position
    const uint16_t ICON_CENTER_X = 100;
    const uint16_t ICON_CENTER_Y = 53;
    const uint16_t ICON_HEIGHT = 16;
    const uint16_t ICON_WIDTH = 8;
    
    // Server name box
    const uint16_t NAME_BOX_X = 15;
    const uint16_t NAME_BOX_Y = 70;
    const uint16_t NAME_BOX_W = 170;
    const uint16_t NAME_BOX_H = 25;
    const uint16_t NAME_BOX_RADIUS = 5;
    const uint16_t NAME_TEXT_Y = 88;
    
    // Status box
    const uint16_t STATUS_BOX_X = 3;
    const uint16_t STATUS_BOX_Y = 105;
    const uint16_t STATUS_BOX_W = 194;
    const uint16_t STATUS_BOX_H = 50;
    const uint16_t STATUS_BOX_RADIUS = 5;
    const uint16_t STATUS_LINE1_Y = 125;
    const uint16_t STATUS_LINE2_Y = 147;
    
    // Footer
    const uint16_t FOOTER_LINE_Y = 160;
    const uint16_t FOOTER_TEXT1_Y = 177;
    const uint16_t FOOTER_TEXT2_Y = 193;
}

// --- SENSORS SCREEN LAYOUT ---
namespace SensorsLayout {
    // Main box
    const uint16_t BOX_X = 5;
    const uint16_t BOX_Y = 35;
    const uint16_t BOX_W = 190;
    const uint16_t BOX_H = 165;
    const uint16_t BOX_RADIUS = 8;
    
    // Grid layout
    const uint16_t START_Y = 55;
    const uint16_t LINE_SPACING = 20;
    const uint16_t LINE_HEIGHT = 18;
    
    // Partial refresh regions
    const uint16_t PARTIAL_X = 10;
    const uint16_t PARTIAL_W = 180;
}

// --- ENVIRONMENTAL SCREEN LAYOUT ---
namespace EnvironmentalLayout {
    // Header line
    const uint16_t HEADER_LINE_Y = 40;
    
    // Data layout
    const uint16_t START_Y = 65;
    const uint16_t LINE_SPACING = 25;
    const uint16_t ICON_X = 10;
    const uint16_t TEXT_X = 25;
    const uint16_t DATA_START_X = 80;  // Where numeric values begin
    
    // Battery section
    const uint16_t BATTERY_X = 5;
    const uint16_t BATTERY_Y = 175;
    const uint16_t BATTERY_W = 190;
    const uint16_t BATTERY_HEIGHT = 21;
    const uint16_t BATTERY_RADIUS = 5;
    const uint16_t BATTERY_TEXT_X = 15;
    const uint16_t BATTERY_TEXT_Y = 190;
    
    // Partial refresh regions
    const uint16_t TEMP_HUM_REGION_X = 90;
    const uint16_t TEMP_HUM_REGION_Y = 55;
    const uint16_t TEMP_HUM_REGION_W = 110;
    const uint16_t TEMP_HUM_REGION_H = 45;
    
    const uint16_t PRESS_ALT_REGION_X = 80;
    const uint16_t PRESS_ALT_REGION_Y = 100;
    const uint16_t PRESS_ALT_REGION_W = 100;
    const uint16_t PRESS_ALT_REGION_H = 45;
}

// --- POLLUTANTS SCREEN LAYOUT ---
namespace PollutantsLayout {
    // Header line
    const uint16_t HEADER_LINE_Y = 40;
    
    // Data layout
    const uint16_t START_Y = 60;
    const uint16_t LINE_SPACING = 22;
    const uint16_t BOX_X = 8;
    const uint16_t TEXT_X = 30;
    const uint16_t DATA_START_X = 80;  // Where numeric values begin
    
    // Partial refresh regions
    const uint16_t TOP_REGION_X = 80;
    const uint16_t TOP_REGION_Y = 50;
    const uint16_t TOP_REGION_W = 170;
    const uint16_t TOP_REGION_H = 80;
    
    const uint16_t BOTTOM_REGION_X = 80;
    const uint16_t BOTTOM_REGION_Y = 130;
    const uint16_t BOTTOM_REGION_W = 170;
    const uint16_t BOTTOM_REGION_H = 70;
}

// --- LOADING SCREEN LAYOUT ---
namespace LoadingLayout {
    // Border coordinates
    const uint16_t BORDER_OUTER = 10;
    const uint16_t BORDER_INNER = 13;
    const uint16_t BORDER_RIGHT_OUTER = 190;
    const uint16_t BORDER_RIGHT_INNER = 187;
    const uint16_t BORDER_TOP_END = 50;
    const uint16_t BORDER_BOTTOM_START = 145;
    const uint16_t BORDER_BOTTOM = 190;
    const uint16_t BORDER_BOTTOM_INNER = 187;
    
    // Text positions
    const uint16_t TEXT1_Y = 90;
    const uint16_t TEXT2_Y = 130;
}

// --- HEATING SCREEN LAYOUT ---
namespace HeatingLayout {
    // Icon position
    const uint16_t ICON_X = 90;
    const uint16_t ICON_Y = 30;
    
    // Text positions
    const uint16_t TEXT1_Y = 110;
    const uint16_t TEXT2_Y = 145;
    
    // Progress bar
    const uint16_t PROGRESS_X = 40;
    const uint16_t PROGRESS_Y = 165;
    const uint16_t PROGRESS_W = 120;
    const uint16_t PROGRESS_H = 12;
    const uint16_t PROGRESS_RADIUS = 6;
    const uint16_t PROGRESS_FILL_RADIUS = 4;
}

// --- SENDING SCREEN LAYOUT ---
namespace SendingLayout {
    // Cloud icon position
    const uint16_t CLOUD_X = 85;
    const uint16_t CLOUD_Y = 50;
    
    // Text positions
    const uint16_t TEXT1_Y = 100;
    const uint16_t TEXT2_Y = 130;
    
    // Dots positions
    const uint16_t DOTS_Y = 155;
}

#endif // SCREEN_CONSTANTS_H