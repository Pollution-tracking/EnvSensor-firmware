#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <configs.h>

void logg(String message);
void loggWithObj(String message, String object);
void loggWithCtx(String message, String object, String context);
void forcePrint();

#endif // LOGGER_H