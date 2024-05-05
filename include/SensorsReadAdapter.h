#ifndef SENSORS_READ_ADAPTER_H
#define SENSORS_READ_ADAPTER_H

#include <Arduino.h>
#include <logger.h>
#include <constants.h>
#include "SDcard.h"
#include <Bluetooth_module.h>
#include <BME_sensor.h>
#include <CO2_sensor.h>
#include <PM_sensor.h>

class SensorsReadAdapter {
    public:
        SensorsReadAdapter(Bluetooth_module *bluetoothModule, SDcard *sdcard);
        ~SensorsReadAdapter();
        void init();
        void updateCO2Sensor(CO2Data co2data);
        void updateBMESensor(BMEData bmedata);
        void updatePMSensor(PMData pmdata);
        void storeData();
        String* getData();
    private:
        SDcard *sdcard;
        Bluetooth_module *bluetoothModule;
        BMEData bmedata;
        CO2Data co2data;
        PMData pmdata;
        // Format: [BME]Temperature,[BME]Humidity,[BME]Pressure,[BME]Gas,[BME]Altitude,[CO2]CO2,[PM]PM1,[PM]PM2.5,[PM]PM10
        String csvData[NR_VALUES];
        void initCSVData();
        String convertDataToCSV();
};

#endif // SENSORS_READ_ADAPTER_H