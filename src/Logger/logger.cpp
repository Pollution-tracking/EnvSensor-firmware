#include <Logger/logger.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

const int OBJECT_WIDTH = 12; // Width for the [OBJECT] part

static SemaphoreHandle_t logMutex;

void initLogger() {
  logMutex = xSemaphoreCreateMutex();
}

static String getPadded(String text, int width) {
  String paddedText = text;
  for (int i = text.length(); i < width; i++) {
    paddedText += " ";
  }
  return paddedText;
}

void logg(String message) {
#ifdef LOGGING_ENABLED
  if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
    Serial.print(message + "\n");
    xSemaphoreGive(logMutex);
  }
#endif
}

void loggWithObj(String message, String object) {
#ifdef LOGGING_ENABLED
  String objStr = "[" + object + "]";
  String logMessage = getPadded(objStr, OBJECT_WIDTH);
  logMessage += "  "; // 2 spaces for separation
  logMessage += message;
  
  if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
    Serial.print(logMessage + "\n");
    xSemaphoreGive(logMutex);
  }
#endif
}

void loggWithCtx(String message, String object, String context) {
#ifdef LOGGING_ENABLED
  String objStr = "[" + object + "]";
  String logMessage = getPadded(objStr, OBJECT_WIDTH);
  logMessage += "  "; // 2 spaces for separation
  String ctxStr = "[" + context + "]";
  logMessage += getPadded(ctxStr, OBJECT_WIDTH);
  logMessage += "  "; // 2 spaces for separation
  logMessage += message;

  if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
    Serial.print(logMessage + "\n");
    xSemaphoreGive(logMutex);
  }
#endif
}

void forcePrint() {
#ifdef LOGGING_ENABLED
  if (xSemaphoreTake(logMutex, portMAX_DELAY) == pdTRUE) {
    Serial.flush();
    xSemaphoreGive(logMutex);
  }
#endif
}
