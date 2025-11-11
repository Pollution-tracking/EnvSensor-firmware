#include <Modules/DisplayTask.h>
#include <configs.h>
#include "freertos/semphr.h"

#define DISPLAY_TASK_STACK_SIZE 4096
#define DISPLAY_QUEUE_LENGTH 10

static void displayTask(void *pvParameters);

static QueueHandle_t displayQueue;
static SemaphoreHandle_t displayBusySemaphore;

TaskHandle_t displayTaskHandle;
Display display;

void createDisplayTask() {
    displayQueue = xQueueCreate(DISPLAY_QUEUE_LENGTH, sizeof(DisplayCommand));
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        1,
        &displayTaskHandle,
        0
    );
    displayBusySemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(displayBusySemaphore); // Initially available
}

void sendDisplayCommand(const DisplayCommand& command) {
    xQueueSend(displayQueue, &command, portMAX_DELAY);
}

static void displayTask(void *pvParameters) {
    DisplayCommand receivedCommand;

    for (;;) {
        if (xQueueReceive(displayQueue, &receivedCommand, portMAX_DELAY) == pdPASS) {
            // Take semaphore to signal busy
            xSemaphoreTake(displayBusySemaphore, portMAX_DELAY);

            switch (receivedCommand.cmd) {
                case CMD_INIT:
                    display.init();
                    break;
                case CMD_SET_MODE:
                    display.setScreenMode(receivedCommand.payload.screen_mode);
                    break;
                case CMD_CHANGE_RIGHT:
                    display.changeScreenRight();
                    break;
                case CMD_CHANGE_LEFT:
                    display.changeScreenLeft();
                    break;
                case CMD_REFRESH:
                    display.refreshScreen(receivedCommand.payload.screen_refresh);
                    break;
            }

            // Give semaphore to signal idle
            xSemaphoreGive(displayBusySemaphore);
        }
    }
}

void waitForDisplayIdle() {
    // Wait for the display task to finish its current operation
    xSemaphoreTake(displayBusySemaphore, portMAX_DELAY);
    xSemaphoreGive(displayBusySemaphore);
}
