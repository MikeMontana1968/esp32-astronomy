#include <iostream>
#include <cassert>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>
#include "AstronomyCalculator.h"

class AstronomyTest {
private:
    struct TestLocation {
        std::string name;
        double latitude;
        double longitude;
    };

    struct TestDate {
        std::string name;
        int year, month, day;
        time_t timestamp;
    };

    struct ExpectedResults {
        std::string sunriseTime;    // HHMM format
        std::string sunsetTime;     // HHMM format
        double sunAzimuthAtRise;    // degrees
        int dayLengthMinutes;       // total minutes of sunlight
        std::string location;
        std::string date;
    };

    // Test locations
    std::vector<TestLocation> locations = {
        {"New York", 40.7128, -74.0060},
        {"Baltimore", 39.2904, -76.6122},
        {"Los Angeles", 34.0522, -118.2437},
        {"Miami", 25.7617, -80.1918}
    };

    // Test dates
    std::vector<TestDate> testDates;

    // Expected results from published astronomical data
    std::vector<ExpectedResults> expectedResults = {
        // January 1, 2026
        {"0719", "1651", 122.0, 567, "New York", "Jan 1, 2026"},
        {"0714", "1656", 118.0, 582, "Baltimore", "Jan 1, 2026"},
        {"0659", "1655", 117.0, 596, "Los Angeles", "Jan 1, 2026"},
        {"0703", "1758", 112.0, 655, "Miami", "Jan 1, 2026"},

        // June 15, 2026 (near summer solstice)
        {"0527", "2008", 62.0, 881, "New York", "Jun 15, 2026"},
        {"0534", "2001", 65.0, 867, "Baltimore", "Jun 15, 2026"},
        {"0541", "2006", 61.0, 865, "Los Angeles", "Jun 15, 2026"},
        {"0621", "2003", 68.0, 822, "Miami", "Jun 15, 2026"},

        // December 25, 2026 (near winter solstice)
        {"0725", "1646", 123.0, 561, "New York", "Dec 25, 2026"},
        {"0720", "1651", 119.0, 571, "Baltimore", "Dec 25, 2026"},
        {"0657", "1650", 118.0, 593, "Los Angeles", "Dec 25, 2026"},
        {"0708", "1753", 113.0, 645, "Miami", "Dec 25, 2026"}
    };

    time_t createTimestamp(int year, int month, int day) {
        struct tm timeinfo = {};
        timeinfo.tm_year = year - 1900;
        timeinfo.tm_mon = month - 1;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = 12; // Noon
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        timeinfo.tm_isdst = -1; // Let system determine DST

        return mktime(&timeinfo);
    }

    bool timeWithinTolerance(const std::string& actual, const std::string& expected, int toleranceMinutes = 5) {
        if (actual.empty() || expected.empty()) return false;

        int actualHour = std::stoi(actual.substr(0, 2));
        int actualMin = std::stoi(actual.substr(2, 2));
        int actualTotalMin = actualHour * 60 + actualMin;

        int expectedHour = std::stoi(expected.substr(0, 2));
        int expectedMinute = std::stoi(expected.substr(2, 2));
        int expectedTotalMin = expectedHour * 60 + expectedMinute;

        return std::abs(actualTotalMin - expectedTotalMin) <= toleranceMinutes;
    }

    bool angleWithinTolerance(double actual, double expected, double toleranceDegrees = 3.0) {
        return std::abs(actual - expected) <= toleranceDegrees;
    }

    bool minutesWithinTolerance(int actual, int expected, int toleranceMinutes = 10) {
        return std::abs(actual - expected) <= toleranceMinutes;
    }

public:
    AstronomyTest() {
        // Initialize test dates
        testDates = {
            {"Jan 1, 2026", 2026, 1, 1, createTimestamp(2026, 1, 1)},
            {"Jun 15, 2026", 2026, 6, 15, createTimestamp(2026, 6, 15)},
            {"Dec 25, 2026", 2026, 12, 25, createTimestamp(2026, 12, 25)}
        };
    }

    void runAllTests() {
        std::cout << "=== AstronomyCalculator Unit Tests ===" << std::endl;
        #ifdef VERSION_STRING
        std::cout << "Version: " << VERSION_STRING << std::endl;
        #endif
        std::cout << "Testing against published astronomical data" << std::endl;
        std::cout << std::endl;

        int totalTests = 0;
        int passedTests = 0;

        // Run location/date combination tests
        for (const auto& location : locations) {
            for (const auto& date : testDates) {
                totalTests++;
                if (runLocationDateTest(location, date)) {
                    passedTests++;
                }
            }
        }

        // Additional functionality tests
        std::cout << "=== Functionality Tests ===" << std::endl;
        totalTests++;
        if (testMoonPhaseCalculation()) passedTests++;

        totalTests++;
        if (testBoundaryConditions()) passedTests++;

        totalTests++;
        if (testConsistencyChecks()) passedTests++;

        // Print summary
        std::cout << "=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passedTests << "/" << totalTests << " tests" << std::endl;

        float successRate = (float)passedTests / totalTests * 100.0f;
        std::cout << "Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%" << std::endl;

        if (passedTests == totalTests) {
            std::cout << "🎉 ALL TESTS PASSED!" << std::endl;
        } else {
            std::cout << "❌ Some tests failed. Check implementation." << std::endl;
        }
    }

private:
    bool runLocationDateTest(const TestLocation& loc, const TestDate& date) {
        std::cout << "Testing " << loc.name << " on " << date.name << "..." << std::endl;

        // Find expected results for this location/date combination
        ExpectedResults expected;
        bool foundExpected = false;

        for (const auto& result : expectedResults) {
            if (result.location == loc.name && result.date == date.name) {
                expected = result;
                foundExpected = true;
                break;
            }
        }

        if (!foundExpected) {
            std::cout << "  ❌ No expected results found for " << loc.name << " on " << date.name << std::endl;
            return false;
        }

        // Create calculator and run test
        AstronomyCalculator astro(loc.latitude, loc.longitude, date.timestamp);

        bool allPassed = true;

        // Test sunrise time
        if (!timeWithinTolerance(astro.sunRiseTodayHHMM, expected.sunriseTime)) {
            std::cout << "  ❌ Sunrise: expected " << expected.sunriseTime << ", got " << astro.sunRiseTodayHHMM << std::endl;
            allPassed = false;
        } else {
            std::cout << "  ✅ Sunrise: " << astro.sunRiseTodayHHMM << std::endl;
        }

        // Test sunset time
        if (!timeWithinTolerance(astro.sunSetTodayHHMM, expected.sunsetTime)) {
            std::cout << "  ❌ Sunset: expected " << expected.sunsetTime << ", got " << astro.sunSetTodayHHMM << std::endl;
            allPassed = false;
        } else {
            std::cout << "  ✅ Sunset: " << astro.sunSetTodayHHMM << std::endl;
        }

        // Test sun azimuth at rise
        if (!angleWithinTolerance(astro.sunAzimuthAtRise, expected.sunAzimuthAtRise)) {
            std::cout << "  ❌ Sun azimuth at rise: expected " << std::fixed << std::setprecision(1)
                      << expected.sunAzimuthAtRise << "°, got " << astro.sunAzimuthAtRise << "°" << std::endl;
            allPassed = false;
        } else {
            std::cout << "  ✅ Sun azimuth at rise: " << std::fixed << std::setprecision(1) << astro.sunAzimuthAtRise << "°" << std::endl;
        }

        // Test day length
        if (!minutesWithinTolerance(astro.minutesSunVisible, expected.dayLengthMinutes)) {
            std::cout << "  ❌ Day length: expected " << expected.dayLengthMinutes
                      << " min, got " << astro.minutesSunVisible << " min" << std::endl;
            allPassed = false;
        } else {
            std::cout << "  ✅ Day length: " << astro.minutesSunVisible << " minutes" << std::endl;
        }

        std::cout << std::endl;
        return allPassed;
    }

    bool testMoonPhaseCalculation() {
        std::cout << "Testing moon phase calculation..." << std::endl;

        // Test known new moon date (approximate)
        time_t newMoonDate = createTimestamp(2026, 1, 13); // Known new moon
        AstronomyCalculator astroNewMoon(40.7128, -74.0060, newMoonDate);
        std::string newMoonPhase = astroNewMoon.moonPhase();

        if (newMoonPhase != "New Moon" && newMoonPhase != "Waxing Crescent") {
            std::cout << "  ❌ Expected phase near New Moon on Jan 13, 2026, got: " << newMoonPhase << std::endl;
            return false;
        }

        std::cout << "  ✅ Moon phase calculation working" << std::endl;
        return true;
    }

    bool testBoundaryConditions() {
        std::cout << "Testing boundary conditions..." << std::endl;

        time_t testDate = createTimestamp(2026, 6, 21); // Summer solstice
        AstronomyCalculator astroNorth(65.0, -150.0, testDate); // Northern Alaska

        // In northern latitudes during summer, days should be very long
        if (astroNorth.minutesSunVisible < 1000) { // > 16 hours
            std::cout << "  ❌ Expected very long day in northern Alaska in summer, got "
                      << astroNorth.minutesSunVisible << " minutes" << std::endl;
            return false;
        }

        std::cout << "  ✅ Boundary conditions handled correctly" << std::endl;
        return true;
    }

    bool testConsistencyChecks() {
        std::cout << "Testing internal consistency..." << std::endl;

        time_t testDate = createTimestamp(2026, 3, 20); // Spring equinox
        AstronomyCalculator astro(40.7128, -74.0060, testDate);

        // At equinox, day length should be close to 12 hours (720 minutes)
        int expectedEquinoxLength = 720; // 12 hours
        if (std::abs(astro.minutesSunVisible - expectedEquinoxLength) > 30) {
            std::cout << "  ❌ Day length at equinox should be ~12 hours, got "
                      << astro.minutesSunVisible << " minutes" << std::endl;
            return false;
        }

        std::cout << "  ✅ Internal consistency checks passed" << std::endl;
        return true;
    }
};

// Native test runner
int main() {
    try {
        AstronomyTest test;
        test.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}