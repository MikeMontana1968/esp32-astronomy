#ifndef ASTRONOMY_CALCULATOR_H
#define ASTRONOMY_CALCULATOR_H

#include <cmath>
#include <ctime>
#include <string>
#include <algorithm>

class AstronomyCalculator {
private:
    // Input parameters
    double latitude;
    double longitude; 
    time_t timestamp;
    
    // Common calculations
    double julianDay;
    double localHour;
    
    // Internal calculation methods
    double toJulianDay(time_t unixTime);
    double calcSunDeclination(double julianDay);
    double calcSunEquationOfTime(double julianDay);
    double calcHourAngleSunrise(double lat, double solarDec);
    double calcSunrise(double julianDay, double latitude, double longitude);
    double calcSunset(double julianDay, double latitude, double longitude);
    double calcSunAzEl(double julianDay, double hour, double lat, double lng, double* azimuth);
    
    // Moon calculation methods
    double calcMoonPosition(double julianDay, double* moonRA, double* moonDec);
    double calcMoonPhaseAngle(double julianDay);
    double calcMoonrise(double julianDay, double latitude, double longitude);
    double calcMoonset(double julianDay, double latitude, double longitude);
    double calcMoonAzEl(double julianDay, double hour, double lat, double lng, double* azimuth);
    
    // Utility methods
    std::string formatTime(double hour);
    std::string formatTimeFromMinutes(int minutes);
    int hoursToMinutes(double hours);
    double normalizeAngle(double angle);
    bool isMoonCurrentlyVisible();

public:
    // Constructor
    AstronomyCalculator(double lat, double lng, time_t unixTime);
    
    // Public member variables - calculated on construction
    bool isMoonVisible;
    int minutesSinceLastMoonRise;
    int minutesSinceLastMoonSet;
    int minutesUntilNextMoonRise;
    int minutesUntilNextMoonSet;
    
    std::string nextMoonRiseHHMM;
    std::string nextMoonSetHHMM;
    std::string lastMoonRiseHHMM;
    std::string lastMoonSetHHMM;
    
    std::string sunRiseTodayHHMM;
    std::string sunSetTodayHHMM;
    int minutesSinceSunRise;
    int minutesSinceSunSet;
    int minutesUntilSunSet;
    int minutesUntilSunRise;
    
    double sunAltitudeAtRise;
    double sunAzimuthAtRise;
    double moonAltitudeAtRise;
    double moonAzimuthAtRise;
    
    int minutesSunVisible;
    int minutesMoonVisible;
    
    // Public methods
    std::string moonPhase();
};

#endif