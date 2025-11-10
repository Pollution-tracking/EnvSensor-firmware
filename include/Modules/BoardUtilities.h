#ifndef BOARD_UTILITIES_H
#define BOARD_UTILITIES_H

#include <Arduino.h>

#include <Logger/logger.h>
#include <Resources/Constants/board_constants.h>
#include <Resources/Constants/timers_constants.h>
#include <Resources/RTC_values.h>
#include <Resources/pins.h>

class BoardUtilities {
  public:
    BoardUtilities();
    ~BoardUtilities();
    
    void treatWakeupReason();
    void configureWakeupSources();
    void toggleSleepState();

    void enableBuck();
    void disableBuck();
  private:
};

#endif // BOARD_UTILITIES_H