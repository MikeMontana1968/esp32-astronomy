#include <Arduino.h>
#include <time.h>
#include "AstronomyCalculator.h"
#include "GPSManager.h"
#include "HasTimeExpired.h"

GPSManager gpsManager;
HasTimeExpired fiveSecondTimer(5000); // 5 second timer
time_t gpsTime = 0;
double latitude = 40.7128;   // Default NYC
double longitude = -74.0060;

void setupGPS() {
    if (gpsManager.initializeWithFix(100000)) { // 60 second timeout
        // GPS fix acquired and system time set
        gpsTime = gpsManager.getUnixTimestamp();
    } else {
        // GPS fix timeout - use current system time if available
        gpsTime = time(nullptr);
        gpsManager.setTimeSet(gpsTime > 0);
    }

    // Update location variables
    latitude = gpsManager.getLatitude();
    longitude = gpsManager.getLongitude();
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
            gpsManager.setSystemTime();
        }
    } else {
        setupGPS(); // Try to reinitialize GPS if no valid fix
    }

    if (gpsManager.isTimeSet()) {
        time_t now = time(nullptr);

        // Create astronomy calculator
        AstronomyCalculator astro(latitude, longitude, now);

        // Display results every hour
        if (fiveSecondTimer.hasIntervalExpired()) {

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