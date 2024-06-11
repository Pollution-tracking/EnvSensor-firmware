#ifndef SENSORS_READ_ADAPTER_H
#define SENSORS_READ_ADAPTER_H

#include <Arduino.h>

#include <vector>

#include <Resources/Constants.h>

#include <Logger/Logger.h>

#include <Sensors/BME_sensor.h>
#include <Sensors/CO2_sensor.h>
#include <Sensors/PM_sensor.h>

#include <Modules/SDcard.h>
#include <Modules/Bluetooth_module.h>
#include <Modules/RTC.h>

class SensorsReadAdapter {
    public:
        SensorsReadAdapter(Bluetooth_module *bluetoothModule, SDcard *sdcard, RTC *rtc);
        ~SensorsReadAdapter();
        void init();
        void updateCO2Sensor(CO2Data co2data);
        void updateBMESensor(BMEData bmedata);
        void updatePMSensor(PMData pmdata);
        void updateBatteryStatus(float voltage);
        void storeData();
        bool ableToSendHistoricalData();
        void sendHistoricalData();
        String* getData();
    private:
        RTC *rtc;
        SDcard *sdcard;
        Bluetooth_module *bluetoothModule;
        BMEData bmedata;
        CO2Data co2data;
        PMData pmdata;
        // Format: [Timestamp]hh:mm:ss:dd:mm:yyy,[Batt]Voltage,[BME]Temperature,[BME]Humidity,[BME]Pressure,[BME]Gas,[BME]Altitude,[CO2]CO2,[PM]PM1,[PM]PM2.5,[PM]PM10
        String csvData[NR_VALUES];
        void prepareCSVData();
        String convertDataToCSV();
        void handleCSVData(String csvData);
        std::vector<String> decodeCSVData(String str);
};

#endif // SENSORS_READ_ADAPTER_H