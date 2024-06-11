#include <Logger/logger.h>

bool loggingEnabled = true; // Enable/disable logging

void logg(String message) {
  if (loggingEnabled) {
    Serial.println(message);
  }
}

void loggWithBase(String message, String base) {
  if (loggingEnabled) {
    Serial.print("[" + base + "] ");
    Serial.println(message);
  }
}

void loggWithContext(String message, String context, String base) {
  if (loggingEnabled) {
    Serial.print("[" + base + "] {" + context + "} ");
    Serial.println(message);
  }

}

void forcePrint() {
  Serial.flush();
}