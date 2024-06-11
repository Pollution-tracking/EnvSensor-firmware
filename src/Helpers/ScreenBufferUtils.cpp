#include "Helpers/ScreenBufferUtils.hpp"

void convertData(int idx, String data) {
    switch (idx) {
        case BATTERY_INDEX:
            convertBattery(data);
            break;
        case BME_TEMPERATURE_INDEX:
            convertTemperature(data);
            break;
        case BME_HUMIDITY_INDEX:
            convertHumidity(data);
            break;
        case BME_PRESSURE_INDEX:
            convertPressure(data);
            break;
        case BME_ALTITUDE_INDEX:
            convertAltitude(data);
            break;
        case CO2_CO2_INDEX:
            convertCO2(data);
            break;
        case PM_PM1_INDEX:
            convertPM1(data);
            break;
        case PM_PM2_5_INDEX:
            convertPM2_5(data);
            break;
        case PM_PM10_INDEX:
            convertPM10(data);
            break;
        default:
            break;
    }
}

void convertBattery(String data) {
    float batt_val = data.toFloat();

    switch ((int)batt_val) {
        case READ_ERROR:
            strcpy(_battery, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_battery, "sensor err\0");
            break;
        default:
            strcpy(_battery, String(String(batt_val) + " V").c_str());
            break;
    }
}

void convertTemperature(String data) {
    int temp_val = data.toInt();

    switch (temp_val) {
        case READ_ERROR:
            strcpy(_temperature, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_temperature, "sensor err\0");
            break;
        default:
            strcpy(_temperature, String(String(temp_val) + " C").c_str());
            break;
    }
}

void convertHumidity(String data) {
    int humidity_val = data.toInt();

    switch (humidity_val) {
        case READ_ERROR:
            strcpy(_humidity, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_humidity, "sensor err\0");
            break;
        default:
            strcpy(_humidity, String(String(humidity_val) + " %").c_str());
            break;
    }
}

void convertPressure(String data) {
    int pressure_val = data.toInt();

    switch (pressure_val) {
        case READ_ERROR:
            strcpy(_pressure, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pressure, "sensor err\0");
            break;
        default:
            strcpy(_pressure, String(String(pressure_val) + " hPa").c_str());
            break;
    }
}

void convertAltitude(String data) {
    int altitude_val = data.toInt();

    switch (altitude_val) {
        case READ_ERROR:
            strcpy(_altitude, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_altitude, "sensor err\0");
            break;
        default:
            strcpy(_altitude, String(String(altitude_val) + " m").c_str());
            break;
    }
}

void convertCO2(String data) {
    int co2_val = data.toInt();

    switch (co2_val) {
        case READ_ERROR:
            strcpy(_co2, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_co2, "sensor err\0");
            break;
        default:
            strcpy(_co2, String(String(co2_val) + " ppm").c_str());
            break;
    }
}

void convertPM1(String data) {
    int pm1_val = data.toInt();

    switch (pm1_val) {
        case READ_ERROR:
            strcpy(_pm1, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm1, "sensor err\0");
            break;
        default:
            strcpy(_pm1, String(String(pm1_val) + " ug/m3").c_str());
            break;
    }
}

void convertPM2_5(String data) {
    int pm2_5_val = data.toInt();

    switch (pm2_5_val) {
        case READ_ERROR:
            strcpy(_pm2_5, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm2_5, "sensor err\0");
            break;
        default:
            strcpy(_pm2_5, String(String(pm2_5_val) + " ug/m3").c_str());
            break;
    }
}

void convertPM10(String data) {
    int pm10_val = data.toInt();

    switch (pm10_val) {
        case READ_ERROR:
            strcpy(_pm10, "read error\0");
            break;
        case SENSOR_ERROR:
            strcpy(_pm10, "sensor err\0");
            break;
        default:
            strcpy(_pm10, String(String(pm10_val) + " ug/m3").c_str());
            break;
    }
}