#include "AstronomyCalculator.h"
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor - performs all calculations
AstronomyCalculator::AstronomyCalculator(double lat, double lng, time_t unixTime)
    : latitude(lat), longitude(lng), timestamp(unixTime) {
    
    julianDay = toJulianDay(unixTime);
    
    // Convert timestamp to local hour of day
    struct tm* timeinfo = localtime(&unixTime);
    localHour = timeinfo->tm_hour + timeinfo->tm_min / 60.0 + timeinfo->tm_sec / 3600.0;
    
    // Calculate sun data
    double sunriseTime = calcSunrise(julianDay, latitude, longitude);
    double sunsetTime = calcSunset(julianDay, latitude, longitude);
    
    sunRiseTodayHHMM = formatTime(sunriseTime);
    sunSetTodayHHMM = formatTime(sunsetTime);
    
    // Calculate sun timing relative to current time
    double hoursSinceSunrise = localHour - sunriseTime;
    double hoursSinceSunset = localHour - sunsetTime;
    double hoursUntilSunset = sunsetTime - localHour;
    double hoursUntilSunrise = (sunriseTime + 24.0) - localHour; // Next day
    
    // Handle negative values and convert to minutes
    minutesSinceSunRise = (hoursSinceSunrise >= 0) ? hoursToMinutes(hoursSinceSunrise) : -1;
    minutesSinceSunSet = (hoursSinceSunset >= 0) ? hoursToMinutes(hoursSinceSunset) : -1;
    minutesUntilSunSet = (hoursUntilSunset >= 0 && hoursSinceSunset < 0) ? hoursToMinutes(hoursUntilSunset) : -1;
    minutesUntilSunRise = (localHour > sunriseTime && localHour > sunsetTime) ? hoursToMinutes(hoursUntilSunrise) : -1;
    
    minutesSunVisible = (sunsetTime > sunriseTime) ? hoursToMinutes(sunsetTime - sunriseTime) : 0;
    
    // Calculate sun position at rise
    sunAltitudeAtRise = calcSunAzEl(julianDay, sunriseTime, latitude, longitude, &sunAzimuthAtRise);
    
    // Calculate moon data
    double moonriseToday = calcMoonrise(julianDay, latitude, longitude);
    double moonsetToday = calcMoonset(julianDay, latitude, longitude);
    double moonriseYesterday = calcMoonrise(julianDay - 1.0, latitude, longitude);
    double moonsetYesterday = calcMoonset(julianDay - 1.0, latitude, longitude);
    double moonriseTomorrow = calcMoonrise(julianDay + 1.0, latitude, longitude);
    double moonsetTomorrow = calcMoonset(julianDay + 1.0, latitude, longitude);
    
    // Determine current moon visibility
    isMoonVisible = isMoonCurrentlyVisible();
    
    // Find most recent moonrise/moonset
    double lastMoonRise = -999, lastMoonSet = -999;
    if (moonriseToday >= 0 && moonriseToday <= localHour) {
        lastMoonRise = moonriseToday;
    } else if (moonriseYesterday >= 0) {
        lastMoonRise = moonriseYesterday;
    }
    
    if (moonsetToday >= 0 && moonsetToday <= localHour) {
        lastMoonSet = moonsetToday;
    } else if (moonsetYesterday >= 0) {
        lastMoonSet = moonsetYesterday;
    }
    
    // Find next moonrise/moonset
    double nextMoonRise = -999, nextMoonSet = -999;
    if (moonriseToday >= 0 && moonriseToday > localHour) {
        nextMoonRise = moonriseToday;
    } else if (moonriseTomorrow >= 0) {
        nextMoonRise = moonriseTomorrow;
    }
    
    if (moonsetToday >= 0 && moonsetToday > localHour) {
        nextMoonSet = moonsetToday;
    } else if (moonsetTomorrow >= 0) {
        nextMoonSet = moonsetTomorrow;
    }
    
    // Convert to member variables
    minutesSinceLastMoonRise = (lastMoonRise > -999) ? hoursToMinutes(localHour - lastMoonRise + ((lastMoonRise > localHour) ? 24.0 : 0.0)) : -1;
    minutesSinceLastMoonSet = (lastMoonSet > -999) ? hoursToMinutes(localHour - lastMoonSet + ((lastMoonSet > localHour) ? 24.0 : 0.0)) : -1;
    minutesUntilNextMoonRise = (nextMoonRise > -999) ? hoursToMinutes(nextMoonRise - localHour + ((nextMoonRise < localHour) ? 24.0 : 0.0)) : -1;
    minutesUntilNextMoonSet = (nextMoonSet > -999) ? hoursToMinutes(nextMoonSet - localHour + ((nextMoonSet < localHour) ? 24.0 : 0.0)) : -1;
    
    // Format time strings with conditional logic
    nextMoonRiseHHMM = (isMoonVisible || nextMoonRise <= -999) ? "" : formatTime(fmod(nextMoonRise + 24.0, 24.0));
    nextMoonSetHHMM = (nextMoonSet <= -999) ? "" : formatTime(fmod(nextMoonSet + 24.0, 24.0));
    lastMoonRiseHHMM = (lastMoonRise <= -999) ? "" : formatTime(fmod(lastMoonRise + 24.0, 24.0));
    lastMoonSetHHMM = (lastMoonSet <= -999) ? "" : formatTime(fmod(lastMoonSet + 24.0, 24.0));
    
    // Calculate moon visibility duration
    if (lastMoonRise > -999 && nextMoonSet > -999) {
        double visibleDuration = nextMoonSet - lastMoonRise;
        if (visibleDuration < 0) visibleDuration += 24.0;
        minutesMoonVisible = hoursToMinutes(visibleDuration);
    } else {
        minutesMoonVisible = 0;
    }
    
    // Calculate moon position at rise
    if (nextMoonRise > -999) {
        moonAltitudeAtRise = calcMoonAzEl(julianDay, nextMoonRise, latitude, longitude, &moonAzimuthAtRise);
    } else if (lastMoonRise > -999) {
        moonAltitudeAtRise = calcMoonAzEl(julianDay, lastMoonRise, latitude, longitude, &moonAzimuthAtRise);
    } else {
        moonAltitudeAtRise = moonAzimuthAtRise = -1;
    }
}

// Convert Unix timestamp to Julian Day
double AstronomyCalculator::toJulianDay(time_t unixTime) {
    return (unixTime / 86400.0) + 2440587.5;
}

// Calculate solar declination using NOAA algorithm
double AstronomyCalculator::calcSunDeclination(double jd) {
    double n = jd - 2451545.0;
    double L = fmod(280.460 + 0.9856474 * n, 360.0);
    double g = fmod(357.528 + 0.9856003 * n, 360.0) * M_PI / 180.0;
    double lambda = (L + 1.915 * sin(g) + 0.020 * sin(2 * g)) * M_PI / 180.0;
    double epsilon = 23.439 * M_PI / 180.0;
    
    double delta = asin(sin(epsilon) * sin(lambda));
    return delta * 180.0 / M_PI;
}

// Calculate equation of time
double AstronomyCalculator::calcSunEquationOfTime(double jd) {
    double n = jd - 2451545.0;
    double L = fmod(280.460 + 0.9856474 * n, 360.0);
    double g = fmod(357.528 + 0.9856003 * n, 360.0) * M_PI / 180.0;
    double lambda = L + 1.915 * sin(g) + 0.020 * sin(2 * g);
    
    double epsilon = 23.439;
    double tanE2 = tan((epsilon/2.0) * M_PI / 180.0);
    tanE2 *= tanE2;
    
    double y = tanE2;
    double e = 0.016708634 - n * 0.000000042037;
    double cos2L = cos(2.0 * L * M_PI / 180.0);
    double sin4L = sin(4.0 * L * M_PI / 180.0);
    double cos4L = cos(4.0 * L * M_PI / 180.0);
    double sinG = sin(g);
    double sin2G = sin(2.0 * g);
    
    double E = 4.0 * (y * sin(2.0 * L * M_PI / 180.0) - 2.0 * e * sinG + 
                     4.0 * e * y * sinG * cos2L - 0.5 * y * y * sin4L - 
                     1.25 * e * e * sin2G);
    
    return E * 180.0 / M_PI;
}

// Calculate hour angle for sunrise/sunset
double AstronomyCalculator::calcHourAngleSunrise(double lat, double solarDec) {
    double latRad = lat * M_PI / 180.0;
    double sdRad = solarDec * M_PI / 180.0;
    
    double HAarg = (cos(90.833 * M_PI / 180.0) / (cos(latRad) * cos(sdRad))) - tan(latRad) * tan(sdRad);
    
    if (HAarg < -1.0 || HAarg > 1.0) {
        return -999; // No sunrise/sunset
    }
    
    double HA = acos(HAarg);
    return HA * 180.0 / M_PI;
}

// Calculate sunrise time
double AstronomyCalculator::calcSunrise(double jd, double latitude, double longitude) {
    double solarDec = calcSunDeclination(jd);
    double hourAngle = calcHourAngleSunrise(latitude, solarDec);
    
    if (hourAngle <= -999) return -1;
    
    double timeUTC = 12.0 - hourAngle / 15.0 - longitude / 15.0;
    double eqTime = calcSunEquationOfTime(jd);
    timeUTC += eqTime / 60.0;
    
    // Convert to local time (simplified - assumes standard time zone)
    double localTime = timeUTC + longitude / 15.0;
    return fmod(localTime + 24.0, 24.0);
}

// Calculate sunset time
double AstronomyCalculator::calcSunset(double jd, double latitude, double longitude) {
    double solarDec = calcSunDeclination(jd);
    double hourAngle = calcHourAngleSunrise(latitude, solarDec);
    
    if (hourAngle <= -999) return -1;
    
    double timeUTC = 12.0 + hourAngle / 15.0 - longitude / 15.0;
    double eqTime = calcSunEquationOfTime(jd);
    timeUTC += eqTime / 60.0;
    
    // Convert to local time (simplified)
    double localTime = timeUTC + longitude / 15.0;
    return fmod(localTime + 24.0, 24.0);
}

// Calculate sun altitude and azimuth at given time
double AstronomyCalculator::calcSunAzEl(double jd, double hour, double lat, double lng, double* azimuth) {
    double solarDec = calcSunDeclination(jd);
    double eqTime = calcSunEquationOfTime(jd);
    
    double solarTimeFix = eqTime + 4.0 * lng;
    double trueSolarTime = hour * 60.0 + solarTimeFix;
    double hourAngle = trueSolarTime / 4.0 - 180.0;
    
    double latRad = lat * M_PI / 180.0;
    double decRad = solarDec * M_PI / 180.0;
    double haRad = hourAngle * M_PI / 180.0;
    
    double elevation = asin(sin(latRad) * sin(decRad) + cos(latRad) * cos(decRad) * cos(haRad)) * 180.0 / M_PI;
    
    double azDenom = cos(latRad) * sin(elevation * M_PI / 180.0);
    double azRad;
    if (fabs(azDenom) > 0.001) {
        azRad = (sin(latRad) * cos(elevation * M_PI / 180.0) - sin(decRad)) / azDenom;
        if (azRad > 1.0) azRad = 1.0;
        if (azRad < -1.0) azRad = -1.0;
        azRad = acos(azRad);
        if (hourAngle > 0.0) {
            azRad = 2.0 * M_PI - azRad;
        }
    } else {
        azRad = (lat > 0.0) ? M_PI : 0.0;
    }
    
    *azimuth = azRad * 180.0 / M_PI;
    return elevation;
}

// Simplified moon position calculation
double AstronomyCalculator::calcMoonPosition(double jd, double* moonRA, double* moonDec) {
    double n = jd - 2451545.0;
    double L = fmod(218.316 + 13.176396 * n, 360.0);
    double M = fmod(134.963 + 13.064993 * n, 360.0) * M_PI / 180.0;
    double F = fmod(93.272 + 13.229350 * n, 360.0) * M_PI / 180.0;
    
    double longitude = L + 6.289 * sin(M);
    double latitude = 5.128 * sin(F);
    
    // Convert to RA/Dec (simplified)
    double epsilon = 23.439 * M_PI / 180.0;
    double lonRad = longitude * M_PI / 180.0;
    double latRad = latitude * M_PI / 180.0;
    
    *moonRA = atan2(sin(lonRad) * cos(epsilon) - tan(latRad) * sin(epsilon), cos(lonRad)) * 180.0 / M_PI;
    if (*moonRA < 0) *moonRA += 360.0;
    *moonDec = asin(sin(latRad) * cos(epsilon) + cos(latRad) * sin(epsilon) * sin(lonRad)) * 180.0 / M_PI;
    
    return longitude;
}

// Calculate moon phase angle
double AstronomyCalculator::calcMoonPhaseAngle(double jd) {
    double n = jd - 2451545.0;
    double L = fmod(218.316 + 13.176396 * n, 360.0);
    double M = fmod(134.963 + 13.064993 * n, 360.0);
    double Msun = fmod(357.529 + 0.98560028 * n, 360.0);
    double D = fmod(297.850 + 12.190749 * n, 360.0);
    
    // Phase angle (illumination)
    double phaseAngle = 180.0 - D - 6.289 * sin(M * M_PI / 180.0) + 2.1 * sin(Msun * M_PI / 180.0);
    return normalizeAngle(phaseAngle);
}

// Calculate moonrise (simplified)
double AstronomyCalculator::calcMoonrise(double jd, double latitude, double longitude) {
    double moonRA, moonDec;
    calcMoonPosition(jd, &moonRA, &moonDec);
    
    // Use same method as sun but with moon's position
    double latRad = latitude * M_PI / 180.0;
    double decRad = moonDec * M_PI / 180.0;
    
    double HAarg = (cos(90.567 * M_PI / 180.0) / (cos(latRad) * cos(decRad))) - tan(latRad) * tan(decRad);
    
    if (HAarg < -1.0 || HAarg > 1.0) {
        return -1; // No moonrise
    }
    
    double HA = acos(HAarg) * 180.0 / M_PI;
    double riseTime = (moonRA - HA) / 15.0 + longitude / 15.0;
    
    return fmod(riseTime + 24.0, 24.0);
}

// Calculate moonset (simplified)
double AstronomyCalculator::calcMoonset(double jd, double latitude, double longitude) {
    double moonRA, moonDec;
    calcMoonPosition(jd, &moonRA, &moonDec);
    
    // Use same method as sun but with moon's position
    double latRad = latitude * M_PI / 180.0;
    double decRad = moonDec * M_PI / 180.0;
    
    double HAarg = (cos(90.567 * M_PI / 180.0) / (cos(latRad) * cos(decRad))) - tan(latRad) * tan(decRad);
    
    if (HAarg < -1.0 || HAarg > 1.0) {
        return -1; // No moonset
    }
    
    double HA = acos(HAarg) * 180.0 / M_PI;
    double setTime = (moonRA + HA) / 15.0 + longitude / 15.0;
    
    return fmod(setTime + 24.0, 24.0);
}

// Calculate moon altitude and azimuth
double AstronomyCalculator::calcMoonAzEl(double jd, double hour, double lat, double lng, double* azimuth) {
    double moonRA, moonDec;
    calcMoonPosition(jd, &moonRA, &moonDec);
    
    // Convert to local hour angle
    double lst = hour + lng / 15.0;
    double hourAngle = (lst * 15.0) - moonRA;
    
    double latRad = lat * M_PI / 180.0;
    double decRad = moonDec * M_PI / 180.0;
    double haRad = hourAngle * M_PI / 180.0;
    
    double elevation = asin(sin(latRad) * sin(decRad) + cos(latRad) * cos(decRad) * cos(haRad)) * 180.0 / M_PI;
    
    double azRad = atan2(sin(haRad), cos(haRad) * sin(latRad) - tan(decRad) * cos(latRad));
    *azimuth = fmod(azRad * 180.0 / M_PI + 180.0, 360.0);
    
    return elevation;
}

// Check if moon is currently visible
bool AstronomyCalculator::isMoonCurrentlyVisible() {
    double moonRA, moonDec;
    calcMoonPosition(julianDay, &moonRA, &moonDec);
    
    double moonAz;
    double moonAlt = calcMoonAzEl(julianDay, localHour, latitude, longitude, &moonAz);
    
    return moonAlt > 0.0; // Above horizon
}

// Format time as HHMM string
std::string AstronomyCalculator::formatTime(double hour) {
    if (hour < 0) return "";
    
    int h = (int)hour;
    int m = (int)((hour - h) * 60.0);
    
    // Handle rounding
    if (m >= 60) {
        h++;
        m = 0;
    }
    if (h >= 24) h -= 24;
    
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d%02d", h, m);
    return std::string(buffer);
}

// Convert hours to minutes
int AstronomyCalculator::hoursToMinutes(double hours) {
    return (int)(hours * 60.0 + 0.5); // Round to nearest minute
}

// Normalize angle to 0-360 degrees
double AstronomyCalculator::normalizeAngle(double angle) {
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

// Get moon phase description
std::string AstronomyCalculator::moonPhase() {
    double phaseAngle = calcMoonPhaseAngle(julianDay);
    
    if (phaseAngle < 22.5 || phaseAngle >= 337.5) return "New Moon";
    else if (phaseAngle < 67.5) return "Waxing Crescent";
    else if (phaseAngle < 112.5) return "First Quarter";
    else if (phaseAngle < 157.5) return "Waxing Gibbous";
    else if (phaseAngle < 202.5) return "Full Moon";
    else if (phaseAngle < 247.5) return "Waning Gibbous";
    else if (phaseAngle < 292.5) return "Last Quarter";
    else return "Waning Crescent";
}