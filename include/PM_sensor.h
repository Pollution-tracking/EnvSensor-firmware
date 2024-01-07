#ifndef PM_SENSOR_H
#define PM_SENSOR_H

#include <Arduino.h>
#include "PMserial.h"
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include <BLECharacteristic.h>


class PMSensor {
  public:
    PMSensor(BLECharacteristic* pm1Characteristic,
             BLECharacteristic* pm2_5Characteristic,
             BLECharacteristic* pm10Characteristic);
    ~PMSensor();
    void update();
    void init();
    uint16_t getPM1();
    uint16_t getPM2_5();
    uint16_t getPM10();
    bool errorPM = false;
    bool sensorFound = false;
  private:
    SerialPM *pms;
    uint16_t pm1;
    uint16_t pm2_5;
    uint16_t pm10;
    BLECharacteristic* pm1Characteristic;
    BLECharacteristic* pm2_5Characteristic;
    BLECharacteristic* pm10Characteristic;
    void updateCharacteristics();
    void checkErrors(SerialPM::STATUS status);
};

#endif // PM_SENSOR_H