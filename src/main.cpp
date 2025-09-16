#include <Arduino.h>
#include <time.h>
#include "AstronomyCalculator.h"
#include "GPSManager.h"

GPSManager gpsManager;
bool timeSet = false;
time_t gpsTime = 0;
double latitude = 40.7128;   // Default NYC
double longitude = -74.0060;

void setupGPS() {
    Serial.println("Initializing GPS...");
    gpsManager.begin();

    // Try to get GPS fix for 60 seconds
    unsigned long startTime = millis();
    while (millis() - startTime < 60000) {
        gpsManager.update();

        if (gpsManager.hasValidFix()) {
            Serial.println("GPS fix acquired!");
            latitude = gpsManager.getLatitude();
            longitude = gpsManager.getLongitude();

            // Set system time from GPS using new method
            if (gpsManager.setSystemTime()) {
                timeSet = true;
                gpsTime = gpsManager.getUnixTimestamp();
                Serial.printf("Location: %.4f, %.4f\n", latitude, longitude);
                return;
            }
        }

        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nGPS fix timeout - using default location and system time");
    gpsManager.setDefaultLocation();
    latitude = gpsManager.getLatitude();
    longitude = gpsManager.getLongitude();

    // Use current system time if GPS fails
    gpsTime = time(nullptr);
    timeSet = (gpsTime > 0);
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Astronomy Calculator v" VERSION_STRING);
    Serial.printf("Build: v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    Serial.println();

    setupGPS();
}

void loop() {
    // Continue updating GPS data
    gpsManager.update();

    // Update location and time if GPS has valid fix
    if (gpsManager.hasValidFix()) {
        latitude = gpsManager.getLatitude();
        longitude = gpsManager.getLongitude();

        // Update system time if GPS time is available
        time_t newGpsTime = gpsManager.getUnixTimestamp();
        if (newGpsTime != gpsTime) {
            gpsTime = newGpsTime;
            if (gpsManager.setSystemTime()) {
                timeSet = true;
            }
        }
    }

    if (timeSet) {
        time_t now = time(nullptr);

        // Create astronomy calculator
        AstronomyCalculator astro(latitude, longitude, now);

        // Display results every hour
        static unsigned long lastDisplay = 0;
        if (millis() - lastDisplay >= 3600000 || lastDisplay == 0) {
            lastDisplay = millis();

            Serial.println("\n=== Astronomy Data ===");
            Serial.printf("Location: %.4f, %.4f\n", latitude, longitude);
            Serial.printf("Sunrise: %s %s Length: %d minutes\n", astro.sunRiseTodayHHMM.c_str(), astro.sunSetTodayHHMM.c_str(), astro.minutesSunVisible);
            
            if (astro.isMoonVisible) {
                Serial.println("Moon is currently visible!");
                Serial.printf("Moon Rise: %s - %s Phase: %s\n", 
                    astro.lastMoonRiseHHMM.c_str(), 
                    astro.nextMoonSetHHMM.c_str(), 
                    astro.moonPhase().c_str()
                );
            } else {
                Serial.println("Moon is not currently visible");
                Serial.printf("Moon Rise: %s (Tmrw) - %s Phase: %s\n", 
                    astro.lastMoonRiseHHMM.c_str(), 
                    astro.nextMoonSetHHMM.c_str(), 
                    astro.moonPhase().c_str()
                );
            }
        }
    } else {
        Serial.println("Waiting for time sync...");
    }

    delay(1000);
}