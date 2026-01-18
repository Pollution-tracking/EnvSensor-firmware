#include "Resources/Software/ScreenBufferUtils.h"

static String formatSensorValue(int32_t value, const String& unit) {
    switch (value) {
        case READ_ERROR:
            return READ_ERROR_STRING;
        case SENSOR_ERROR:
            return SENSOR_ERROR_STRING;
        case NO_DATA:
            return NO_DATA_STRING;
        default:
            return String(value / 100.0, 2) + unit;
    }
}

String convertBattery(BatteryData data) {
    switch ((int)data.voltage) {
        case NO_DATA:
            return NO_DATA_STRING;
        default:
            break;
    }

    return String(String(data.voltage / 100.0, 2) + " V");
}

#ifdef BME_ENABLE
String convertTemperature(BMEData data) {
    return formatSensorValue(data.temperature, " C");
}

String convertHumidity(BMEData data) {
    return formatSensorValue(data.humidity, " %");
}

String convertPressure(BMEData data) {
    return formatSensorValue(data.pressure, " hPa");
}

String convertAltitude(BMEData data) {
    return formatSensorValue(data.altitude, " m");
}
#endif // BME_ENABLE

#ifdef CO2_ENABLE
String convertCO2(CO2Data data) {
    return formatSensorValue(data.co2, " ppm");
}
#endif // CO2_ENABLE

#ifdef PM_ENABLE
String convertPM1(PMData data) {
    return formatSensorValue(data.pm1, " ug/m3");
}

String convertPM2_5(PMData data) {
    return formatSensorValue(data.pm2_5, " ug/m3");
}

String convertPM10(PMData data) {
    return formatSensorValue(data.pm10, " ug/m3");
}
#endif // PM_ENABLE

#ifdef MICS_ENABLE
String convertCO(MICSData data) {
    return formatSensorValue(data.co, " ppm");
}

String convertNO2(MICSData data) {
    return formatSensorValue(data.no2, " ppm");
}

String convertNH3(MICSData data) {
    return formatSensorValue(data.nh3, " ppm");
}
#endif // MICS_ENABLE
