# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based astronomy calculator project that provides real-time solar and lunar calculations using NOAA algorithms. The project uses PlatformIO and the Arduino framework for ESP32 development.

**Current Version:** 0.1.0
**Status:** Initial development with basic functionality and test framework

## Build and Development Commands

```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Clean build
pio run --target clean

# Monitor serial output
pio device monitor --baud 115200

# Run tests
pio test

# List available targets
pio run --list-targets

# Check for dependency updates
pio pkg update

# List connected devices
pio device list
```

## Architecture

### Core Components

- **AstronomyCalculator Class** (`lib/AstronomyCalculator/`): The main calculation engine implementing NOAA solar and lunar algorithms
  - Solar calculations: sunrise, sunset, azimuth, altitude with ±1-2 minute accuracy
  - Lunar calculations: moon phases, moonrise, moonset, visibility with ±10-30 minutes accuracy
  - Performance optimized: ~100 microseconds per calculation on ESP32

- **Main Application** (`src/main.cpp`): ESP32 application that:
  - Connects to WiFi for NTP time synchronization
  - Creates AstronomyCalculator instances with current location and time
  - Displays astronomy data via serial output every hour
  - Requires WiFi credentials and location coordinates configuration

### Key Features

- **Real-time calculations**: Uses current time and GPS coordinates
- **High accuracy**: NOAA-based algorithms for solar calculations
- **Performance optimized**: Designed for ESP32 microcontroller constraints
- **Time synchronization**: WiFi-based NTP sync for accurate timing
- **Comprehensive data**: Provides timing, angles, visibility, and duration data

### Configuration Requirements

Before building, update `src/main.cpp` with:
- WiFi credentials (`ssid`, `password`)
- Location coordinates (`LATITUDE`, `LONGITUDE`)
- Timezone offset if needed

### Hardware Requirements

- ESP32 development board (any variant)
- WiFi connection for time synchronization
- USB connection for programming and serial monitoring

## Testing

The project includes comprehensive test suite in `test/test_astronomy.cpp` with:
- Multiple test locations (NYC, London, Sydney, etc.)
- Various dates throughout the year
- Accuracy validation against known astronomical data
- Performance benchmarking

Run tests with `pio test` to validate calculations.

## Project History

See `claude-development-notes.md` for detailed development history and technical insights from the original collaboration with Claude AI.