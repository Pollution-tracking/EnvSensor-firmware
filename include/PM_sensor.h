#ifndef PM_SENSOR_H
#define PM_SENSOR_H

#include <Arduino.h>
#include "PMserial.h"
#include "pins.h"
#include "constants.h"
#include <logger.h>
#include "Bluetooth_module.h"
#include "Sensor.h"

class PMSensor : public Sensor{
  public:
    PMSensor(Bluetooth_module *bluetoothModule);
    ~PMSensor();
    bool sensorFound() override;
    bool sensorError() override;
    void update();
    void init();
    uint16_t getPM1();
    uint16_t getPM2_5();
    uint16_t getPM10();
  private:
    SerialPM *pms;
    uint16_t pm1;
    uint16_t pm2_5;
    uint16_t pm10;
    Bluetooth_module *bluetoothModule;
    bool _errorPM = false;
    bool _sensorFound = false;
    void updateCharacteristics();
    void checkErrors(SerialPM::STATUS status);
};

#endif // PM_SENSOR_H