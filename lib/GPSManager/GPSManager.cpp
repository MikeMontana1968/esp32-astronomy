#include "GPSManager.h"
#include <sys/time.h>

GPSManager::GPSManager(uint8_t rxPin, uint8_t txPin) {
    Serial.println("GPSManager::GPSManager()");
    this->rxPin = rxPin;
    this->txPin = txPin;
    useDefaults = false;
    gpsSerial = nullptr;
    timeSet = false;
    showNEMAData = false;
}

GPSManager::~GPSManager() {
    Serial.println("GPSManager::~GPSManager()");
    if (gpsSerial) {
        delete gpsSerial;
    }
}

void GPSManager::begin() {
    Serial.println("GPSManager::begin()");
    Serial.printf("GPS using pins: RX=%d, TX=%d\n", rxPin, txPin);
    gpsSerial = new SoftwareSerial(rxPin, txPin);
    gpsSerial->begin(9600);
}

void GPSManager::update() {
    static unsigned long last_timestamp = 0;
    // Serial.println("GPSManager::update()"); // Commented out - called frequently
    if (gpsSerial && gpsSerial->available()) {
        String buffer = "";
        while (gpsSerial->available()) {
            char c = gpsSerial->read();
            buffer += c;
        }
        if (buffer.length() > 0) {
            if (showNEMAData) {
                Serial.printf("[%d]: %s\n", buffer.length(), buffer.c_str());
            }
            for (int i = 0; i < buffer.length(); i++) {
                if (gps.encode(buffer[i])) {
                // unsigned long now = GPSManager::getUnixTimestamp();
                // if(now != last_timestamp) {
                //     last_timestamp = now;
                //     Serial.print("*** GPSManager::update() - ");
                //     Serial.printf("%02d:%02d:%02d",
                //         gps.time.hour(),
                //         gps.time.minute(),
                //         gps.time.second()
                //     );
                //     Serial.println();
                // }
                }
            }
        }
    }
}

bool GPSManager::hasValidFix() {
    //Serial.println("GPSManager::hasValidFix()");
    bool result = !useDefaults && gps.location.isValid() && gps.date.isValid() && gps.time.isValid();
    //Serial.print("GPSManager::hasValidFix() returning: ");
    //Serial.println(result);
    return result;
}

void GPSManager::setDefaultLocation() {
    Serial.println("GPSManager::setDefaultLocation()");
    useDefaults = true;
}

float GPSManager::getLatitude() {
    //Serial.println("GPSManager::getLatitude()");
    float result = useDefaults ? defaultLat : (float)gps.location.lat();
    // Serial.print("GPSManager::getLatitude() returning: ");
    // Serial.println(result);
    return result;
}

float GPSManager::getLongitude() {
    //Serial.println("GPSManager::getLongitude()");
    float result = useDefaults ? defaultLng : (float)gps.location.lng();
    // Serial.print("GPSManager::getLongitude() returning: ");
    // Serial.println(result);
    return result;
}

float GPSManager::getAltitude() {
    //Serial.println("GPSManager::getAltitude()");
    float result = useDefaults ? defaultAlt : (float)gps.altitude.meters();
    // Serial.print("GPSManager::getAltitude() returning: ");
    // Serial.println(result);
    return result;
}

int GPSManager::getYear() {
    //Serial.println("GPSManager::getYear()");
    int result = useDefaults ? 2025 : gps.date.year();
    //Serial.print("GPSManager::getYear() returning: ");
    //Serial.println(result);
    return result;
}

int GPSManager::getMonth() {
    //Serial.println("GPSManager::getMonth()");
    int result = useDefaults ? 8 : gps.date.month();
    //Serial.print("GPSManager::getMonth() returning: ");
    ///Serial.println(result);
    return result;
}

int GPSManager::getDay() {
    //Serial.println("GPSManager::getDay()");
    int result = useDefaults ? 27 : gps.date.day();
    //Serial.print("GPSManager::getDay() returning: ");
    //Serial.println(result);
    return result;
}

int GPSManager::getHour() {
    //Serial.println("GPSManager::getHour()");
    int result = useDefaults ? 12 : gps.time.hour();
    //Serial.print("GPSManager::getHour() returning: ");
    //Serial.println(result);
    return result;
}

int GPSManager::getMinute() {
    //Serial.println("GPSManager::getMinute()");
    int result = useDefaults ? 0 : gps.time.minute();
    //Serial.print("GPSManager::getMinute() returning: ");
    //Serial.println(result);
    return result;
}

int GPSManager::getSecond() {
    //Serial.println("GPSManager::getSecond()");
    int result = useDefaults ? 0 : gps.time.second();
    //Serial.print("GPSManager::getSecond() returning: ");
    //Serial.println(result);
    return result;
}

unsigned long GPSManager::getUnixTimestamp() {
    int year = getYear();
    int month = getMonth();
    int day = getDay();
    int hour = getHour();
    int minute = getMinute();
    int second = getSecond();

    // Calculate days since Unix epoch (1970-01-01)
    unsigned long days = 0;

    // Add days for complete years since 1970
    for (int y = 1970; y < year; y++) {
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
            days += 366; // Leap year
        } else {
            days += 365;
        }
    }

    // Days in each month (non-leap year)
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Adjust February for leap year
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[1] = 29;
    }

    // Add days for complete months in current year
    for (int m = 1; m < month; m++) {
        days += daysInMonth[m - 1];
    }

    // Add days in current month (minus 1 since we count from day 1)
    days += day - 1;

    // Convert to seconds and add time components
    unsigned long timestamp = days * 86400UL + hour * 3600UL + minute * 60UL + second;

    return timestamp;
}

int GPSManager::getTimezoneOffset() {
    float longitude = getLongitude();

    // Basic timezone calculation: longitude / 15 degrees per hour
    // Round to nearest hour
    int timezoneOffset = round(longitude / 15.0);

    // Add 1 hour for Daylight Saving Time if applicable
    if (isDST()) {
        timezoneOffset += 1;
    }

    // Clamp to reasonable timezone range (-12 to +14)
    if (timezoneOffset < -12) timezoneOffset = -12;
    if (timezoneOffset > 14) timezoneOffset = 14;

    return timezoneOffset;
}

bool GPSManager::isDST() {
    int month = getMonth();
    int day = getDay();

    // Simple DST rules for US (2nd Sunday in March to 1st Sunday in November)
    // This is a simplified version - just use months for basic detection

    if (month >= 4 && month <= 10) {
        // April through October - definitely DST
        return true;
    } else if (month == 3) {
        // March - DST starts 2nd Sunday (roughly day 8-14)
        return day >= 8;
    } else if (month == 11) {
        // November - DST ends 1st Sunday (roughly day 1-7)
        return day <= 7;
    }

    // December, January, February - no DST
    return false;
}

bool GPSManager::setSystemTime() {
    unsigned long gpsTime = getUnixTimestamp();
    if (gpsTime == 0) {
        return false;
    }

    // Calculate timezone offset based on longitude and DST
    int timezoneOffset = getTimezoneOffset();

    // Apply timezone offset to GPS UTC time
    time_t localTime = gpsTime + (timezoneOffset * 3600);

    // Set system time
    struct timeval tv;
    tv.tv_sec = localTime;
    tv.tv_usec = 0;

    if (settimeofday(&tv, NULL) == 0) {
        Serial.printf("System time set from GPS: %lu (UTC+%d)\n", gpsTime, timezoneOffset);
        Serial.printf("DST: %s\n", isDST() ? "Yes" : "No");
        timeSet = true;
        return true;
    }

    return false;
}

bool GPSManager::initializeWithFix(unsigned long timeoutMs) {
    Serial.println("Initializing GPS...");
    begin();

    // Try to get GPS fix for specified timeout
    unsigned long startTime = millis();
    while (millis() - startTime < timeoutMs) {
        update();

        if (hasValidFix()) {
            Serial.println("GPS fix acquired!");

            // Set system time from GPS
            if (setSystemTime()) {
                Serial.printf("Location: %.4f, %.4f\n", getLatitude(), getLongitude());
                return true;
            }
        }

        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nGPS fix timeout - using default location");
    setDefaultLocation();
    return false;
}

bool GPSManager::isTimeSet() const {
    return timeSet;
}

void GPSManager::setTimeSet(bool value) {
    timeSet = value;
}

bool GPSManager::getShowNEMAData() const {
    return showNEMAData;
}

void GPSManager::setShowNEMAData(bool value) {
    showNEMAData = value;
}