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

struct MeanTemperature {
private:
    float temperature = 0;
    int contributions = 0;
public:
    void addTemperature(float temp) {
        // Reset computation if we have 2 contributions
        if (contributions == 2) {
            temperature = 0;
            contributions = 0;
        }

        // Hackish way to handle wrong CO2 temperature reading (UGLY!)
        if (temp == 0 || (int)temp == READ_ERROR || (int)temp == SENSOR_ERROR) {
            temperature *= 2;
        } else {
            temperature += temp;
        }
        contributions++;
    }

    float getMeanTemperature() {
        // Avoid division by zero
        if (contributions == 0) {
            return 0;
        }

        return temperature / contributions;
    }
};

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
        float batteryData;
        MeanTemperature meanTemperature;
        // Format: [Timestamp]hh:mm:ss:dd:mm:yyy,[Batt]Voltage,[BME]Temperature,[BME]Humidity,[BME]Pressure,[BME]Gas,[BME]Altitude,[CO2]CO2,[PM]PM1,[PM]PM2.5,[PM]PM10
        String csvData[NR_VALUES];
        void prepareCSVData();
        String convertDataToCSV();
        void handleCSVData(String csvData);
        std::vector<String> decodeCSVData(String str);
};

#endif // SENSORS_READ_ADAPTER_H