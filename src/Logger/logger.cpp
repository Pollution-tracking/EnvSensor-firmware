#include <Logger/logger.h>

bool loggingEnabled = true; // Enable/disable logging

void logg(String message) {
  if (loggingEnabled) {
    Serial.println(message);
  }
}

void forcePrint() {
  Serial.flush();
}