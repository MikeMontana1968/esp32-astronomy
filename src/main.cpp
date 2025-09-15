#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "AstronomyCalculator.h"

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Location (example: your city)
const double LATITUDE = 40.7128;   // NYC
const double LONGITUDE = -74.0060;

void setupWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("WiFi connected!");
}

void setupTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for time sync...");
    
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nTime synchronized!");
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Astronomy Calculator v" VERSION_STRING);
    Serial.printf("Build: v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    Serial.println();

    setupWiFi();
    setupTime();
}

void loop() {
    time_t now = time(nullptr);
    
    // Create astronomy calculator
    AstronomyCalculator astro(LATITUDE, LONGITUDE, now);
    
    // Display results
    Serial.println("\n=== Astronomy Data ===");
    Serial.printf("Sunrise: %s\n", astro.sunRiseTodayHHMM.c_str());
    Serial.printf("Sunset: %s\n", astro.sunSetTodayHHMM.c_str());
    Serial.printf("Moon Phase: %s\n", astro.moonPhase().c_str());
    Serial.printf("Day Length: %d minutes\n", astro.minutesSunVisible);
    
    if (astro.isMoonVisible) {
        Serial.println("Moon is currently visible!");
    } else {
        Serial.println("Moon is not currently visible");
    }
    
    // Wait 1 hour before next calculation
    delay(3600000);
}