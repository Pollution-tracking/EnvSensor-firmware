#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <Arduino.h>
#include <Modules/Display.h>

typedef enum {
    CMD_INIT,
    CMD_SET_MODE,
    CMD_CHANGE_RIGHT,
    CMD_CHANGE_LEFT,
    CMD_REFRESH
} DisplayCommandType;

typedef struct {
    DisplayCommandType cmd;
    union {
        SCREEN_MODE screen_mode;
        SCREEN_REFRESH screen_refresh;
    } payload;
} DisplayCommand;

void createDisplayTask();
void sendDisplayCommand(const DisplayCommand& command);
void waitForDisplayIdle();

#endif // DISPLAY_TASK_H
