#include "Resources/Software/ScreenBufferUtils.h"

static String formatNumber(float floatValue) {
    // Check if decimal part is zero
    if (floatValue == (int)floatValue) {
        return String((int)floatValue);
    } else {
        // Check if last digit is 0 (e.g., 25.10 -> 25.1)
        int intPart = (int)floatValue;
        int decimalPart = (int)((floatValue - intPart) * 100 + 0.5);
        if (decimalPart % 10 == 0) {
            return String(floatValue, 1);
        } else {
            return String(floatValue, 2);
        }
    }
}

static String formatSensorValue(int32_t value, const String& unit, bool includeUnit, bool integerOnly) {
    switch (value) {
        case READ_ERROR:
            return READ_ERROR_STRING;
        case SENSOR_ERROR:
            return SENSOR_ERROR_STRING;
        case NO_DATA:
            return NO_DATA_STRING;
        default:
            String result;
            
            if (integerOnly) {
                result = String((int)(value / 100.0 + 0.5));
            } else {
                result = formatNumber(value / 100.0);
            }
            
            if (includeUnit) {
                result += unit;
            }
            
            return result;
    }
}

String convertBattery(BatteryData data, bool includeUnit) {
    switch ((int)data.voltage) {
        case NO_DATA:
            return NO_DATA_STRING;
        default:
            break;
    }

    String result = formatNumber(data.voltage / 100.0);
    
    if (includeUnit) {
        result += " V";
    }
    
    return result;
}

#ifdef BME_ENABLE
String convertTemperature(BMEData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.temperature, " C", includeUnit, integerOnly);
}

String convertHumidity(BMEData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.humidity, " %", includeUnit, integerOnly);
}

String convertPressure(BMEData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.pressure, " hPa", includeUnit, integerOnly);
}

String convertAltitude(BMEData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.altitude, " m", includeUnit, integerOnly);
}
#endif // BME_ENABLE

#ifdef CO2_ENABLE
String convertCO2(CO2Data data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.co2, " ppm", includeUnit, integerOnly);
}
#endif // CO2_ENABLE

#ifdef PM_ENABLE
String convertPM1(PMData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.pm1, " ug/m3", includeUnit, integerOnly);
}

String convertPM2_5(PMData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.pm2_5, " ug/m3", includeUnit, integerOnly);
}

String convertPM10(PMData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.pm10, " ug/m3", includeUnit, integerOnly);
}
#endif // PM_ENABLE

#ifdef MICS_ENABLE
String convertCO(MICSData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.co, " ppm", includeUnit, integerOnly);
}

String convertNO2(MICSData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.no2, " ppm", includeUnit, integerOnly);
}

String convertNH3(MICSData data, bool includeUnit, bool integerOnly) {
    return formatSensorValue(data.nh3, " ppm", includeUnit, integerOnly);
}
#endif // MICS_ENABLE
