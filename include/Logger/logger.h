#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

extern bool loggingEnabled;

void logg(String message);
void loggWithBase(String message, String base);
void loggWithContext(String message, String context, String base);
void forcePrint();

#endif // LOGGER_H