#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

class GPSManager {
public:
    GPSManager();
    ~GPSManager();

    void begin();
    void update();
    bool hasValidFix();
    void setDefaultLocation();

    float getLatitude();
    float getLongitude();
    float getAltitude();
    int getYear();
    int getMonth();
    int getDay();
    int getHour();
    int getMinute();
    int getSecond();

    unsigned long getUnixTimestamp();
    int getTimezoneOffset();
    bool isDST();

private:
    TinyGPSPlus gps;
    SoftwareSerial* gpsSerial;
    bool useDefaults;

    // Default location: East Northport, NY
    float defaultLat = 40.5169;
    float defaultLng = -74.4063;
    float defaultAlt = 0.0;
};

#endif