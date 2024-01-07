#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

extern bool loggingEnabled;

void logg(String message);

#endif // LOGGER_H