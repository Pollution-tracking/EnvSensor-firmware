#include <Logger/logger.h>

const int OBJECT_WIDTH = 12; // Width for the [OBJECT] part

void logg(String message) {
#ifdef LOGGING_ENABLED
  Serial.println(message);
#endif
}

static void printPadded(String text, int width) {
  Serial.print(text);
  for (int i = text.length(); i < width; i++) {
    Serial.print(" ");
  }
}

void loggWithObj(String message, String object) {
#ifdef LOGGING_ENABLED
  String objStr = "[" + object + "]";
  printPadded(objStr, OBJECT_WIDTH);
  Serial.print("  "); // 2 spaces for separation
  Serial.println(message);
#endif
}

void loggWithCtx(String message, String object, String context) {
#ifdef LOGGING_ENABLED
  String objStr = "[" + object + "]";
  printPadded(objStr, OBJECT_WIDTH);
  Serial.print("  "); // 2 spaces for separation
  String ctxStr = "[" + context + "]";
  printPadded(ctxStr, OBJECT_WIDTH);
  Serial.print("  "); // 2 spaces for separation
  Serial.println(message);
#endif
}

void forcePrint() {
#ifdef LOGGING_ENABLED
  Serial.flush();
#endif
}