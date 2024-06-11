#ifndef SCREENBUFFERUTILS_HPP
#define SCREENBUFFERUTILS_HPP

#include <Arduino.h>
#include "Resources/constants.h"
#include <Resources/RTC_values.hpp>

void convertData(int idx, String data);
void convertBattery(String data);
void convertTemperature(String data);
void convertHumidity(String data);
void convertPressure(String data);
void convertAltitude(String data);
void convertCO2(String data);
void convertPM1(String data);
void convertPM2_5(String data);
void convertPM10(String data);

#endif // SCREENBUFFERUTILS_HPP