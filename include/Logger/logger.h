#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

extern bool loggingEnabled;

void logg(String message);
void forcePrint();

#endif // LOGGER_H