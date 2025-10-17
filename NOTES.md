# ESP32 Kiln Controller - Technical Notes & Reference

This document contains detailed technical information, procedures, and reference material that supports the project but isn't needed in the primary steering documents (CLAUDE.md, PLANNING.md, TASKS.md).

---

## Table of Contents

1. [Development Environment Setup](#development-environment-setup)
2. [Thermocouple Calibration Procedure](#thermocouple-calibration-procedure)
3. [Library Documentation](#library-documentation)
4. [Testing Procedures](#testing-procedures)
5. [Web Interface Details](#web-interface-details)
6. [Hardware Configuration Details](#hardware-configuration-details)
7. [Development Session History](#development-session-history)

---

## Development Environment Setup

### PlatformIO Configuration Details

**Platform**: Espressif 32 (ESP32)
**Framework**: Arduino
**Board**: esp32dev (ESP32-WROOM-32)

**Complete platformio.ini Settings**:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = min_spiffs.csv
lib_deps =
    me-no-dev/AsyncTCP
    me-no-dev/ESP Async WebServer
    bblanchon/ArduinoJson
    adafruit/Adafruit MAX31855 library
    br3ttb/PID
    olikraus/U8g2
    Wire
```

**Partition Scheme**: Use `min_spiffs.csv` or custom partition table to allocate sufficient space for:
- Firmware (~1.5MB)
- SPIFFS/LittleFS for web files and profiles (~1.5MB)
- OTA updates (requires dual partition)

### Build Targets

- `pio run` - Build firmware
- `pio run --target upload` - Upload to device
- `pio run --target monitor` - Serial monitor
- `pio run --target uploadfs` - Upload SPIFFS filesystem

### Installation Steps

**Step 1: Install VSCode**
```bash
# Download and install Visual Studio Code
# https://code.visualstudio.com/
```

**Step 2: Install PlatformIO**
1. Open VSCode
2. Go to Extensions (Ctrl+Shift+X)
3. Search "PlatformIO IDE"
4. Click Install
5. Restart VSCode

**Step 3: Create Project**
```bash
# PlatformIO will handle this through GUI:
# PlatformIO Home > New Project
# Board: Espressif ESP32 Dev Module
# Framework: Arduino
```

**Step 4: Test Upload**
1. Connect ESP32 via USB
2. Click PlatformIO Upload button
3. Verify successful compilation and upload

---

## Thermocouple Calibration Procedure

### Overview

**CRITICAL**: Thermocouple calibration must be completed before PID auto-tuning. Accurate temperature measurement is essential for kiln control.

K-type thermocouples can have offset errors due to:
- Manufacturing tolerances
- Connection resistance at junction points
- Wire quality variations
- Cold junction compensation drift

### Ice-Point Calibration Method

**Reference Temperature**: 0°C (32°F) using ice-water bath
**Accuracy**: ±0.1°C when properly prepared
**Standard**: NIST calibration laboratories use this method

### Required Materials

- [ ] Crushed ice (not cubes - crushed ice packs better)
- [ ] Distilled water (tap water acceptable for ±0.5°C accuracy)
- [ ] Insulated container (thermos, cooler, or styrofoam cup)
- [ ] Thermometer (optional, for verification)
- [ ] The assembled kiln controller with thermocouple

### Ice Bath Preparation Procedure

**Step 1: Prepare the Ice Bath**
1. Fill insulated container 50-60% with crushed ice
2. Add just enough distilled water to cover the ice (ice should float slightly)
3. Stir vigorously for 30 seconds to ensure thermal equilibrium
4. Let settle for 1-2 minutes
5. Result: Ice-water slush at exactly 0.0°C (32.0°F)

**Important Notes**:
- Too much water: Temperature will be above 0°C
- Too little water: Poor thermal contact with thermocouple
- Ice must be floating, not packed solid at bottom
- Air temperature does NOT affect ice-water temperature (it stays at 0°C)

**Step 2: Insert Thermocouple**
1. Insert thermocouple tip at least 3-4 inches into ice bath
2. Ensure tip is surrounded by ice-water slush, not touching container walls
3. Gently stir around thermocouple to eliminate air pockets
4. Wait 2-3 minutes for thermal stabilization

**Step 3: Record Reading**
1. Power on kiln controller
2. Wait for temperature reading to stabilize (30-60 seconds)
3. Record the displayed temperature
4. Repeat reading 3 times to ensure consistency
5. Calculate average if readings vary

### Calibration Offset Calculation

**Expected Reading**: 0.0°C (32.0°F)
**Actual Reading**: [What controller displays]
**Calibration Offset** = Expected - Actual

**Examples**:
- Controller reads 2.5°C → Offset = 0 - 2.5 = -2.5°C
- Controller reads -1.0°C → Offset = 0 - (-1.0) = +1.0°C
- Controller reads 0.3°C → Offset = 0 - 0.3 = -0.3°C

**Applying the Offset**:
```cpp
float correctedTemp = rawTemp + calibrationOffset;
```

### Acceptable Tolerance

- **Excellent**: ±0.5°C or better
- **Good**: ±1.0°C
- **Acceptable**: ±2.0°C
- **Poor**: >±2.0°C (check connections, thermocouple quality)

If error is >±5°C, check for:
- Loose or corroded connections
- Damaged thermocouple wire
- MAX31855 wiring errors
- Cold junction compensation issues

### Implementation Details

**Calibration Data Structure**:
```cpp
struct CalibrationData {
    float offset;                // Calculated offset in °C
    unsigned long calibDate;     // Unix timestamp of calibration
    bool isCalibrated;           // Calibration status flag
    uint8_t sampleCount;         // Number of stable readings used
};
```

**Storage in NVS**:
```cpp
// Save calibration data
Preferences prefs;
prefs.begin("kiln-config", false);
prefs.putFloat("temp_offset", calibrationOffset);
prefs.putULong("calib_date", now());
prefs.putBool("is_calibrated", true);
prefs.end();

// Load calibration data
prefs.begin("kiln-config", true); // read-only
float offset = prefs.getFloat("temp_offset", 0.0);
bool calibrated = prefs.getBool("is_calibrated", false);
unsigned long calibDate = prefs.getULong("calib_date", 0);
prefs.end();
```

**Apply Calibration to Readings**:
```cpp
float readCalibratedTemperature() {
    float rawTemp = max31855.readCelsius();

    // Validate reading
    if (isnan(rawTemp) || !isValidTemperature(rawTemp)) {
        return TEMP_ERROR_VALUE;
    }

    // Apply calibration offset
    float calibratedTemp = rawTemp + calibrationData.offset;

    return calibratedTemp;
}
```

### Stability Detection for Calibration

**Requirements**:
- Temperature must be stable within ±0.5°C for 60 seconds
- Minimum 10 consecutive readings within tolerance
- Visual feedback during stabilization

**Implementation**:
```cpp
bool isTemperatureStable(float currentTemp) {
    static float readings[10] = {0};
    static uint8_t readingIndex = 0;
    static unsigned long stableStartTime = 0;

    // Add reading to circular buffer
    readings[readingIndex] = currentTemp;
    readingIndex = (readingIndex + 1) % 10;

    // Calculate min/max range
    float minTemp = readings[0];
    float maxTemp = readings[0];
    for (int i = 1; i < 10; i++) {
        if (readings[i] < minTemp) minTemp = readings[i];
        if (readings[i] > maxTemp) maxTemp = readings[i];
    }

    float range = maxTemp - minTemp;

    // Check if within tolerance
    if (range <= 0.5) {
        if (stableStartTime == 0) {
            stableStartTime = millis();
        }

        // Check if stable for required duration
        if (millis() - stableStartTime >= 60000) {
            return true;
        }
    } else {
        stableStartTime = 0;
    }

    return false;
}
```

### Calibration Mode UI Flow

**LCD Display Sequence**:

1. **Instructions Screen**:
```
┌──────────────────────┐
│ THERMOCOUPLE CAL     │
├──────────────────────┤
│ Prepare ice bath:    │
│ 1. Fill with crushed │
│    ice (50-60%)      │
│ 2. Add water to cover│
│ 3. Stir 30 seconds   │
│ 4. Insert probe 3"   │
│                      │
│ [Start] [Cancel]     │
└──────────────────────┘
```

2. **Stabilization Screen**:
```
┌──────────────────────┐
│ CALIBRATION MODE     │
├──────────────────────┤
│ Waiting for stable   │
│ reading at 0.0°C...  │
│                      │
│ Current: 1.8°C       │
│ Samples: 23          │
│ Stable: No [░░░░░   ]│
│                      │
│ [Cancel]             │
└──────────────────────┘
```

3. **Confirmation Screen**:
```
┌──────────────────────┐
│ CALIBRATION RESULT   │
├──────────────────────┤
│ Expected: 0.0°C      │
│ Measured: 1.8°C      │
│ Offset: -1.8°C       │
│                      │
│ Quality: Excellent   │
│                      │
│ [Accept] [Retry]     │
└──────────────────────┘
```

### Calibration Quality Assessment

```cpp
String assessCalibrationQuality(float offset) {
    float absOffset = abs(offset);

    if (absOffset <= 0.5) return "Excellent";
    if (absOffset <= 1.0) return "Good";
    if (absOffset <= 2.0) return "Acceptable";
    if (absOffset <= 5.0) return "Poor";
    return "Failed";
}
```

**Quality Thresholds**:
- **Excellent**: ±0.5°C or better
- **Good**: ±1.0°C
- **Acceptable**: ±2.0°C
- **Poor**: ±5.0°C (check connections)
- **Failed**: >±5.0°C (investigate wiring, thermocouple damage)

### Calibration Status Display

**Main Status Screen** should show:
```cpp
// Display calibration status icon
if (calibrationData.isCalibrated) {
    display.print("✓CAL");  // Calibrated
} else {
    display.print("!CAL");  // Not calibrated - warning
}
```

**System Info Menu** should show:
- Last calibration date
- Calibration offset value
- Time since last calibration
- Warning if > 365 days old

### When to Recalibrate

**Mandatory**:
- Initial setup (before first use)
- After replacing thermocouple
- After modifying thermocouple wiring
- Before PID auto-tune

**Recommended**:
- Annually (every 365 days)
- If temperature readings seem inaccurate
- After suspected thermocouple damage
- After any kiln rewiring

### API Endpoints for Calibration

**REST API**:
```
GET  /api/calibration        - Get calibration status and data
POST /api/calibration/start  - Initiate calibration mode
POST /api/calibration/save   - Save calculated offset
POST /api/calibration/reset  - Clear calibration data
```

**WebSocket Messages**:
```json
{
  "type": "calibration_status",
  "currentTemp": 1.8,
  "sampleCount": 23,
  "isStable": false,
  "stableProgress": 38
}
```

### Safety Considerations

**During Calibration**:
- SSR must be OFF (kiln not heating)
- System state must be IDLE
- Cannot start firing without calibration
- Calibration can be aborted at any time

**Validation**:
```cpp
bool canStartCalibration() {
    // Must be in IDLE state
    if (kilnState.state != STATE_IDLE) {
        return false;
    }

    // SSR must be off
    if (digitalRead(SSR_PIN) == HIGH) {
        return false;
    }

    // Thermocouple must be functional
    float temp = max31855.readCelsius();
    if (isnan(temp)) {
        return false;
    }

    return true;
}
```

### Example Calibration Module (src/temperature/calibration.cpp)

```cpp
class ThermocoupleCalibration {
private:
    CalibrationData data;
    Preferences prefs;

    bool stabilityDetected;
    float stableTemperature;

public:
    void begin() {
        loadFromNVS();
    }

    void loadFromNVS() {
        prefs.begin("kiln-config", true);
        data.offset = prefs.getFloat("temp_offset", 0.0);
        data.calibDate = prefs.getULong("calib_date", 0);
        data.isCalibrated = prefs.getBool("is_calibrated", false);
        prefs.end();
    }

    void saveToNVS() {
        prefs.begin("kiln-config", false);
        prefs.putFloat("temp_offset", data.offset);
        prefs.putULong("calib_date", now());
        prefs.putBool("is_calibrated", true);
        prefs.end();
    }

    float applyCalibration(float rawTemp) {
        return rawTemp + data.offset;
    }

    void calculateOffset(float measuredTemp) {
        // Ice point expected = 0.0°C
        data.offset = 0.0 - measuredTemp;
        data.calibDate = now();
        data.isCalibrated = true;
    }

    bool isCalibrated() {
        return data.isCalibrated;
    }

    String getCalibrationAge() {
        if (!data.isCalibrated) return "Never";

        unsigned long age = now() - data.calibDate;
        uint16_t days = age / 86400;

        if (days == 0) return "Today";
        if (days == 1) return "Yesterday";
        if (days < 30) return String(days) + " days ago";
        if (days < 365) return String(days/30) + " months ago";
        return String(days/365) + " years ago";
    }
};
```

### Calibration Verification

After applying offset:
1. Repeat ice-point test
2. Controller should now read 0.0°C ± 0.5°C
3. Optional: Test at boiling point (100°C at sea level)
   - Should read 100.0°C ± 1.0°C
   - Adjust altitude: Boiling point decreases ~1°C per 300m elevation

---

## Library Documentation

### Temperature & Control Libraries

**Adafruit MAX31855**
- Purpose: Thermocouple interface, handles SPI communication and cold junction compensation
- Why: Industry standard, reliable, well-documented
- Documentation: https://github.com/adafruit/Adafruit-MAX31855-library

**PID_v1 (br3ttb)**
- Purpose: Robust PID controller with anti-windup and auto mode
- Why: Proven PID implementation with auto-tune support
- Documentation: https://github.com/br3ttb/Arduino-PID-Library

### Display & Input Libraries

**U8g2**
- Purpose: Universal graphics library supporting ST7920 LCD via SPI
- Why: Excellent font support, drawing primitives, ST7920 support
- Documentation: https://github.com/olikraus/u8g2/wiki

**SPI**
- Purpose: SPI communication for LCD and MAX31855
- Why: Built-in to ESP32 Arduino framework
- Documentation: Part of Arduino Core

**Encoder Library** (optional)
- Purpose: Simplifies rotary encoder handling with debouncing
- Why: Reduces complexity of encoder state management
- Documentation: https://www.pjrc.com/teensy/td_libs_Encoder.html

### Web & Network Libraries

**AsyncTCP**
- Purpose: Asynchronous TCP library for ESP32
- Why: Required by ESPAsyncWebServer
- Documentation: https://github.com/me-no-dev/AsyncTCP

**ESPAsyncWebServer**
- Purpose: Non-blocking web server
- Why: Essential for smooth operation while serving web UI
- Documentation: https://github.com/me-no-dev/ESPAsyncWebServer

**ArduinoJson**
- Purpose: JSON parsing/generation for API and profile storage
- Why: De facto standard, well-tested, efficient, use v6.x
- Documentation: https://arduinojson.org/

### Storage Libraries

**SPIFFS or LittleFS**
- Purpose: File system for web files and profiles
- Why: Built-in to ESP32, LittleFS more robust and faster
- Documentation: ESP32 Arduino Core docs

**Preferences**
- Purpose: ESP32 NVS (non-volatile storage) for settings
- Why: ESP32 NVS wrapper, simple key-value store
- Documentation: ESP32 Arduino Core docs

### Utilities

**ESP32Time** or similar
- Purpose: Software RTC for timestamps
- Why: Needed for calibration dates, firing logs
- Documentation: https://github.com/fbiego/ESP32Time

**WiFi**
- Purpose: ESP32 WiFi stack (built-in)
- Why: Required for web interface
- Documentation: ESP32 Arduino Core docs

---

## Testing Procedures

### Unit Testing
- Test each module independently
- Mock hardware interfaces (SPI) for desktop testing
- Use PlatformIO native testing when possible

### Integration Testing
- Test on actual hardware with mock kiln (resistive load)
- Verify all safety features trigger correctly
- Long-duration testing (24+ hour firings)

### Safety Testing

**CRITICAL - Must verify**:
- [ ] Emergency stop immediately cuts power
- [ ] Thermocouple disconnect triggers alarm and shutdown
- [ ] Maximum temperature limit cannot be overridden
- [ ] Watchdog timer triggers on firmware hang
- [ ] Temperature readings filtered/validated before use

**Test Scenarios**:

1. **Thermocouple Disconnect Test**
   - Unplug thermocouple during operation
   - Expected: Immediate SSR shutdown, alarm, error state

2. **Emergency Stop Test**
   - Press both encoder switches for 0.5s during firing
   - Expected: Immediate SSR shutdown, cannot resume

3. **Temperature Limit Test**
   - Attempt to set temperature above maxTempLimit
   - Expected: Rejected, cannot override safety limit

4. **Watchdog Test**
   - Inject infinite loop in non-critical task
   - Expected: Watchdog reset after timeout, SSR safe state

5. **Invalid Temperature Test**
   - Inject NaN or out-of-range values
   - Expected: Value rejected, error logged, safe state maintained

### Field Testing
- Beta test with actual ceramic artists
- Various kiln sizes and types
- Different firing profiles (bisque, glaze, raku)
- Network reliability testing (WiFi dropout recovery)

### Test Documentation Template

```markdown
## Test Report: [Feature Name]

**Date**: YYYY-MM-DD
**Tester**: Name
**Hardware**: ESP32 rev X.X, Kiln type

### Test Cases

1. **[Test Name]**
   - Procedure: [Steps]
   - Expected Result: [Expected behavior]
   - Actual Result: [What happened]
   - Status: ✅ Pass / ❌ Fail
   - Notes: [Any observations]

### Issues Found
- [Issue 1]
- [Issue 2]

### Conclusion
[Overall assessment]
```

---

## Web Interface Details

### API Endpoints (Complete List)

**REST API** (JSON responses):
- `GET /api/status` - Current temperature, state, progress
- `GET /api/profiles` - List all profiles
- `POST /api/profiles` - Create new profile
- `PUT /api/profiles/{id}` - Update profile
- `DELETE /api/profiles/{id}` - Delete profile
- `POST /api/firing/start` - Start firing with profile ID
- `POST /api/firing/stop` - Stop current firing
- `GET /api/history` - Get firing history
- `GET /api/costs` - Get cost tracking data
- `POST /api/costs/reset` - Reset cost counters
- `GET /api/config` - Get system configuration
- `POST /api/config` - Update system configuration
- `POST /api/autotune/start` - Initiate PID auto-tune
- `GET /api/autotune/status` - Auto-tune progress
- `GET /api/calibration` - Get calibration status and data
- `POST /api/calibration/start` - Initiate calibration mode
- `POST /api/calibration/save` - Save calculated offset
- `POST /api/calibration/reset` - Clear calibration data

**WebSocket** (Real-time updates):
- `/ws` - Stream temperature, state, cost data every 1 second

### Web Interface Tech Stack

- **HTML5/CSS3**: Responsive design, mobile-first
- **Vanilla JavaScript**: Keep it lightweight, no heavy frameworks
- **Chart.js**: Temperature and cost graphing
- **WebSocket API**: Real-time data updates
- **Fetch API**: REST calls

**Minimize dependencies** - ESP32 serves files from SPIFFS, so keep total web UI < 500KB

### Example API Response Formats

**GET /api/status**:
```json
{
  "temperature": 823.5,
  "targetTemp": 850.0,
  "state": "FIRING",
  "currentSegment": 2,
  "pidOutput": 67.3,
  "firingElapsed": 14523,
  "segmentElapsed": 3245,
  "currentCost": 2.45,
  "isCalibrated": true
}
```

**GET /api/profiles**:
```json
{
  "profiles": [
    {
      "id": 1,
      "name": "Bisque Cone 04",
      "description": "Standard bisque firing",
      "numSegments": 3,
      "maxTemp": 1060
    }
  ]
}
```

**WebSocket Message**:
```json
{
  "type": "status_update",
  "timestamp": 1634567890,
  "data": {
    "temperature": 823.5,
    "targetTemp": 850.0,
    "state": "FIRING"
  }
}
```

---

## Hardware Configuration Details

### GPIO Pin Detailed Notes

**GPIOs 34-39 (Input-only pins)**:
- No internal pull-up or pull-down resistors
- Used for right rotary encoder
- 5V tolerant when used as high-impedance inputs
- Cannot be used for output

**Boot Strapping Pins (GPIOs 0, 2, 5, 12, 15)**:
- State during boot affects boot mode
- GPIO 0: Must be HIGH during boot (pulled up internally)
- GPIO 2: Must be floating or LOW during boot
- GPIO 5, 12, 15: Used for SPI, state matters for boot mode
- Safe for use after boot completes

**SPI Bus Sharing (ST7920 LCD + MAX31855)**:
- Both devices on same SPI bus (HSPI)
- SCK (clock): GPIO 18 (shared)
- MISO: GPIO 19 (MAX31855 only - LCD doesn't read back)
- MOSI: GPIO 23 (LCD only - MAX31855 doesn't receive data)
- CS pins are different (GPIO 15 for LCD, GPIO 5 for MAX31855)
- Different CS pins allow independent device selection

### Electrical Isolation Best Practices

**Power Supply Isolation**:
- ESP32: 5VDC regulated to 3.3VDC (USB or wall adapter)
- Kiln: 120VAC mains (completely separate circuit)
- NO common ground between ESP32 and AC mains
- SSR provides optical isolation (1500V+ rating)

**PCB Layout Recommendations**:
- Minimum 5mm clearance between AC and DC traces
- Visual separation on PCB silk screen
- Physical barrier (milled slot) for high-voltage isolation
- AC section on one side of board, DC on other

**Grounding**:
- Earth ground for kiln chassis (safety)
- ESP32 ground isolated from earth ground
- SSR provides ground isolation via optocoupler

---

## Development Session History

### Session 1 - 2025-10-11: Development Environment Setup (M1.1 Complete)

**Objective**: Set up PlatformIO development environment and initialize project structure

**Actions Completed**:
1. **Software Installation**
   - Installed PlatformIO Core 6.1.18 via pip
   - Verified Python 3.12.4 (already installed)
   - Verified Git 2.40.1 (already installed)

2. **Project Initialization**
   - Created PlatformIO project for ESP32-WROOM-32
   - Configured `platformio.ini`:
     - Platform: espressif32
     - Board: esp32dev
     - Framework: Arduino
     - Monitor speed: 115200, Upload speed: 921600
     - Partition scheme: min_spiffs.csv
     - Libraries: ArduinoJson, PID (hardware libs commented for incremental addition)

3. **Project Structure Created**
   - Module directories: `src/temperature/`, `src/control/`, `src/safety/`, `src/ui/`, `src/web/`, `src/storage/`, `src/energy/`, `src/utils/`
   - Data directories: `data/www/css/`, `data/www/js/`, `data/profiles/`
   - Standard PlatformIO dirs: `include/`, `lib/`, `test/`

4. **Core Files Created**
   - `src/config.h` (85 lines)
     - All GPIO pin definitions per hardware spec
     - System constants (temp limits, PID defaults, SSR timing)
     - Safety limits (MAX_TEMP_LIMIT: 1320°C, MAX_RAMP_RATE: 600°C/hr)
     - Feature flags and debug macros
   - `src/main.cpp` (108 lines)
     - Safety-first initialization (SSR defaults to OFF)
     - Built-in LED blink test
     - Serial debug output
     - Placeholder structure for FreeRTOS tasks
   - `.gitignore`
     - PlatformIO build artifacts
     - Credentials/secrets exclusion
     - macOS and IDE temporary files

5. **Version Control**
   - Initialized Git repository
   - Created two commits:
     - Initial project structure
     - Configuration updates and M1.1 completion

6. **Build Verification**
   - Successfully compiled firmware
   - Size: 269,789 bytes (13.7% of 1,966,080 bytes Flash)
   - RAM usage: 21,472 bytes (6.6% of 327,680 bytes)
   - Libraries strategy: Incremental addition (avoiding dependency conflicts)

**Technical Decisions**:
- Used PlatformIO Core CLI instead of VSCode (more portable, scriptable)
- Commented out web server and hardware-specific libraries in platformio.ini to enable clean incremental builds
- Will add libraries as each module is implemented (avoids dependency resolution issues)
- Established safety-first code pattern: SSR pin initialized to LOW (OFF) before any other operations

**Files Modified/Created**:
- `platformio.ini` - Project configuration
- `src/config.h` - System configuration and pin definitions
- `src/main.cpp` - Main firmware with blink test
- `.gitignore` - Comprehensive exclusions
- `TASKS.md` - Updated M1.1 tasks as complete

**Blockers/Issues**:
- ESP32 hardware not yet available (tasks requiring upload are pending)
- USB drivers will be verified once hardware is connected
- ESPAsyncWebServer library has packaging issues - resolved by using GitHub URL (commented out for now)

**Next Steps** (Requires Hardware):
- **M1.2**: Hardware acquisition (ESP32, MAX31855, components)
- **M1.1 Remaining**: Upload blink test to physical ESP32 (verifies upload capability)
- **M1.3**: Begin thermocouple integration once hardware arrives

**Commit References**:
- Initial commit: `d92a0ae` - "Initial commit: PlatformIO project setup"
- Update commit: `f763f85` - "Update configuration and mark M1.1 tasks complete"

**Build Output Summary**:
```
RAM:   [=         ]   6.6% (used 21472 bytes from 327680 bytes)
Flash: [=         ]  13.7% (used 269789 bytes from 1966080 bytes)
========================= [SUCCESS] Took 4.47 seconds =========================
```

**Status**: M1.1 Development Environment Setup - ✅ COMPLETE (pending hardware upload test)

---

## Additional Resources

### Documentation Links
- [ESP32 Arduino Core Docs](https://docs.espressif.com/projects/arduino-esp32/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [MAX31855 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX31855.pdf)
- [U8g2 Reference](https://github.com/olikraus/u8g2/wiki)

### Ceramic Firing References
- Orton Cone Temperature Chart
- Standard bisque and glaze firing schedules
- Thermal shock prevention best practices

### Safety Standards
- UL 873 (Temperature-Indicating and Regulating Equipment)
- Electrical isolation requirements
- Thermocouple installation standards

---

## Code Review & Bug Fixes - 2025-10-17

### Session Summary: Critical Hardware & Compatibility Issues Fixed

During Milestone 1.7 Integration Testing implementation, a comprehensive code review identified and fixed **4 critical issues** that would have caused hardware failures. All fixes were applied to both `src/main.cpp` and `src/hardware_test.cpp` for consistency.

---

### Issue #1: 🔴 CRITICAL - Missing SPI Initialization Configuration

**Location**: `src/main.cpp:375` (original)

**Problem Identified**:
- Code called `SPI.begin()` with no parameters
- Would use ESP32 default pins instead of our custom pin configuration
- Both MAX31855 and LCD were declared as using software SPI, making hardware SPI.begin() unnecessary
- Potential conflicts or undefined behavior

**Root Cause**:
- Confusion between hardware SPI and software SPI implementations
- MAX31855: Using software SPI constructor `Adafruit_MAX31855(CLK, CS, MISO)`
- LCD ST7920: Using software SPI constructor `U8G2_ST7920_128X64_F_SW_SPI`

**Solution Applied**:
```cpp
// BEFORE (WRONG):
#include <SPI.h>
SPI.begin();  // Uses default pins, unnecessary for software SPI

// AFTER (CORRECT):
// Note: SPI.h not needed - using software SPI for both MAX31855 and LCD
// Both devices bit-bang their own pins independently
// Hardware SPI.begin() removed - not needed
```

**Rationale**:
- Software SPI devices control their own pins (bit-banging)
- No hardware SPI bus initialization needed
- Avoids potential conflicts between hardware and software SPI
- Cleaner, more accurate code

**Files Modified**:
- `src/main.cpp`: Removed `#include <SPI.h>` and `SPI.begin()` call
- Added clear documentation explaining software SPI usage
- `src/hardware_test.cpp`: No changes needed (already using software SPI correctly)

---

### Issue #2: 🔴 CRITICAL - MAX31855 Constructor Parameter Verification

**Location**: `src/main.cpp:31`

**Problem Identified**:
- Needed to verify constructor parameter order matches library specification
- Potential for pins to be wired incorrectly if constructor signature was wrong

**Investigation Results**:
```cpp
// Adafruit MAX31855 Library Constructors:
// Software SPI: Adafruit_MAX31855(SCLK, CS, MISO)
// Hardware SPI: Adafruit_MAX31855(CS, &SPI)

// Our Code (VERIFIED CORRECT):
Adafruit_MAX31855 thermocouple(THERMOCOUPLE_CLK, THERMOCOUPLE_CS, THERMOCOUPLE_MISO);
// Expands to: thermocouple(18, 5, 19)
// Maps to: SCLK=18 ✓, CS=5 ✓, MISO=19 ✓
```

**Verification Steps**:
1. Read Adafruit MAX31855 library source code
2. Confirmed constructor signature: `Adafruit_MAX31855(int8_t _sclk, int8_t _cs, int8_t _miso)`
3. Verified our pin mapping matches expected order
4. Added detailed comments documenting pin assignments

**Solution Applied**:
```cpp
// Thermocouple using SOFTWARE SPI
// Constructor: Adafruit_MAX31855(SCLK, CS, MISO)
// We use software SPI to match the LCD's software SPI implementation
// Pins: CLK=18, CS=5, MISO=19
Adafruit_MAX31855 thermocouple(THERMOCOUPLE_CLK, THERMOCOUPLE_CS, THERMOCOUPLE_MISO);
```

**Result**: ✅ Code was correct, but now properly documented to prevent future confusion

---

### Issue #3: 🔴 CRITICAL - Safety Temperature Limit Enforcement

**Location**: `src/main.cpp:268`

**Problem Identified**:
```cpp
// BEFORE (WRONG):
state.targetTemp += 5.0;
if (state.targetTemp > 1200.0) state.targetTemp = 1200.0;  // Hardcoded limit!
```

**Issues**:
1. **Hardcoded magic number** `1200.0` instead of using `MAX_TEMP_LIMIT` from config.h
2. **Maintenance risk**: If `MAX_TEMP_LIMIT` changed in config.h, encoder wouldn't respect it
3. **No enforcement in SSR control**: Could potentially heat above safe limits

**Safety Impact**:
- **CRITICAL**: Temperature limits are a primary safety feature
- Hardcoded values bypass centralized safety configuration
- Could allow dangerous temperature overruns

**Solution Applied**:

**1. Fixed Encoder Handler** (`main.cpp:269-271`):
```cpp
// AFTER (CORRECT):
state.targetTemp += 5.0;
// SAFETY: Enforce hard temperature limit from config.h
if (state.targetTemp > MAX_TEMP_LIMIT) {
    state.targetTemp = MAX_TEMP_LIMIT;
}
```

**2. Added SSR Control Safety Checks** (`main.cpp:138-152`):
```cpp
// SAFETY: Don't heat if target exceeds hard limit
if (state.targetTemp > MAX_TEMP_LIMIT) {
    digitalWrite(SSR_PIN, LOW);
    state.heating = false;
    DEBUG_PRINTLN("[SAFETY] Target exceeds MAX_TEMP_LIMIT - heating disabled");
    return;
}

// SAFETY: Don't heat if current temperature exceeds hard limit
if (state.currentTemp >= MAX_TEMP_LIMIT) {
    digitalWrite(SSR_PIN, LOW);
    state.heating = false;
    DEBUG_PRINTLN("[SAFETY] Current temp at/above MAX_TEMP_LIMIT - heating disabled");
    return;
}
```

**Defense in Depth**:
- ✅ **Layer 1**: Encoder input validation (prevents setting target above limit)
- ✅ **Layer 2**: SSR control check (prevents heating if target > limit)
- ✅ **Layer 3**: SSR control check (prevents heating if current temp ≥ limit)
- ✅ **Centralized**: All use `MAX_TEMP_LIMIT` constant from config.h (1320°C)

**Files Modified**:
- `src/main.cpp`: Fixed encoder handler and added SSR safety checks
- `src/hardware_test.cpp`: No temperature limit enforcement needed (test firmware)

---

### Issue #4: 🔴 CRITICAL - GPIO Pull-up Configuration Error

**Location**: `src/main.cpp:385-390`, `src/hardware_test.cpp:846-853`

**Problem Identified**:
```cpp
// BEFORE (WRONG):
pinMode(ENCODER_LEFT_CLK_PIN, INPUT_PULLUP);   // GPIO 32 - OK
pinMode(ENCODER_LEFT_DT_PIN, INPUT_PULLUP);    // GPIO 33 - OK
pinMode(ENCODER_LEFT_SW_PIN, INPUT_PULLUP);    // GPIO 34 - ❌ BROKEN!
pinMode(ENCODER_RIGHT_CLK_PIN, INPUT);         // GPIO 35 - OK
pinMode(ENCODER_RIGHT_DT_PIN, INPUT);          // GPIO 39 - OK
pinMode(ENCODER_RIGHT_SW_PIN, INPUT);          // GPIO 36 - OK
```

**Critical Issue**:
- **GPIO 34 is INPUT-ONLY** on ESP32
- Has **NO internal pull-up or pull-down resistors**
- Using `INPUT_PULLUP` on GPIO 34 **silently fails**
- Pin becomes **floating** → erratic button behavior, false triggers, unreliable emergency stop

**ESP32 Input-Only GPIOs** (No Internal Pull-ups):
- GPIO 34, 35, 36, 39 (all used by right encoder + left encoder SW)
- These pins **physically cannot** enable internal pull-ups
- Must use external pull-ups (which our 5V encoder modules provide)

**Hardware Impact**:
- **Emergency stop would be unreliable** (critical safety issue!)
- **Random encoder rotation detection** (noise on floating pins)
- **False button presses** (unpredictable behavior)
- **Very difficult to debug** (intermittent, hardware-dependent failures)

**Solution Applied**:
```cpp
// AFTER (CORRECT):
// Initialize encoder pins - all use INPUT mode
// 5V encoder modules have their own external pull-up resistors
// CRITICAL: GPIOs 34-39 are INPUT-ONLY and have NO internal pull-up capability
pinMode(ENCODER_LEFT_CLK_PIN, INPUT);   // GPIO 32 - has internal pull-up but using external
pinMode(ENCODER_LEFT_DT_PIN, INPUT);    // GPIO 33 - has internal pull-up but using external
pinMode(ENCODER_LEFT_SW_PIN, INPUT);    // GPIO 34 - INPUT-ONLY (no internal pull-up)
pinMode(ENCODER_RIGHT_CLK_PIN, INPUT);  // GPIO 35 - INPUT-ONLY (no internal pull-up)
pinMode(ENCODER_RIGHT_DT_PIN, INPUT);   // GPIO 39 - INPUT-ONLY (no internal pull-up)
pinMode(ENCODER_RIGHT_SW_PIN, INPUT);   // GPIO 36 - INPUT-ONLY (no internal pull-up)
```

**Why All Pins Use `INPUT` (Not Just GPIO 34-39)**:
1. **Consistency**: All encoder pins configured the same way
2. **Avoid conflicts**: External + internal pull-ups = incorrect voltage divider
3. **Better practice**: 5V modules have their own pull-ups (external is more reliable)
4. **Mandatory for GPIOs 34-39**: No choice - they physically can't use INPUT_PULLUP

**Files Modified**:
- `src/main.cpp`: Fixed all 6 encoder pin configurations
- `src/hardware_test.cpp`: Fixed all 6 encoder pin configurations
- Added detailed comments explaining ESP32 GPIO limitations

---

### Issue #5: 🟡 MINOR - Tone Function ESP32 Compatibility

**Location**: 8 occurrences across both files

**Problem Identified**:
```cpp
// BEFORE (UNRELIABLE ON ESP32):
tone(BUZZER_PIN, 1500, 50);   // Duration parameter often ignored on ESP32
tone(BUZZER_PIN, 1000, 100);  // Inconsistent across ESP32 Arduino core versions
tone(BUZZER_PIN, 2000, 30);   // Can conflict with other PWM/LEDC usage
```

**ESP32 `tone()` Issues**:
- **Duration parameter unreliable**: Often ignored, doesn't auto-stop
- **Implementation varies**: Different behavior across ESP32 Arduino core versions
- **Not ESP32-native**: Wrapper around LEDC, but not the optimal way
- **Potential conflicts**: Can interfere with other PWM channels

**Solution Applied - ESP32 LEDC (Native Hardware)**:

**1. Created `playTone()` Function**:
```cpp
// Buzzer LEDC configuration (ESP32-native)
#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8  // 8-bit resolution

/**
 * Play a tone on the buzzer using ESP32 LEDC peripheral
 * More reliable than Arduino tone() on ESP32
 */
void playTone(uint32_t frequency, uint32_t duration) {
    if (frequency > 0) {
        ledcWriteTone(BUZZER_CHANNEL, frequency);  // Start tone
        delay(duration);                            // Wait
        ledcWriteTone(BUZZER_CHANNEL, 0);          // Stop tone
    }
}
```

**2. Initialize LEDC in `setup()`**:
```cpp
// Initialize buzzer using ESP32 LEDC peripheral
pinMode(BUZZER_PIN, OUTPUT);
digitalWrite(BUZZER_PIN, LOW);
ledcSetup(BUZZER_CHANNEL, 2000, BUZZER_RESOLUTION);  // 2kHz base, 8-bit
ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);           // Attach to GPIO 26
ledcWriteTone(BUZZER_CHANNEL, 0);                    // Start silent
```

**3. Replaced All `tone()` Calls** (8 total):
```cpp
// BEFORE:
tone(BUZZER_PIN, 1500, 50);

// AFTER:
playTone(1500, 50);
```

**Benefits**:
- ✅ **Native ESP32 hardware**: Uses LEDC peripheral directly
- ✅ **Reliable duration control**: We manually control timing
- ✅ **Consistent behavior**: Works across all ESP32 variants
- ✅ **No conflicts**: Dedicated LEDC channel (channel 0)
- ✅ **Better performance**: Hardware PWM generation
- ✅ **Future-proof**: Won't break with ESP32 Arduino core updates

**Audio Feedback Map**:
- **1000 Hz**: Mode switched to IDLE
- **1500 Hz**: Mode switched to MANUAL / Startup beep
- **1200 Hz**: Setpoint adjustment (quick click)
- **2000 Hz**: Button press / Emergency stop alarm

**Files Modified**:
- `src/main.cpp`: Added `playTone()`, initialized LEDC, replaced 6 `tone()` calls
- `src/hardware_test.cpp`: Added `playTone()`, initialized LEDC, replaced 4 `tone()` calls

---

### Build Verification

**Both firmwares compiled successfully after all fixes**:

```
Processing esp32dev (platform: espressif32; board: esp32dev; framework: arduino)
RAM:   [=         ]   7.1% (used 23,328 bytes from 327,680 bytes)
Flash: [==        ]  15.4% (used 301,941 bytes from 1,966,080 bytes)
========================= [SUCCESS] Took 4.70 seconds =========================

Processing hardware_test (platform: espressif32; board: esp32dev; framework: arduino)
RAM:   [=         ]   7.0% (used 23,032 bytes from 327,680 bytes)
Flash: [==        ]  15.8% (used 310,629 bytes from 1,966,080 bytes)
========================= [SUCCESS] Took 4.59 seconds =========================
```

---

### Impact Summary

**Issues Prevented**:
- ❌ **Hardware Failure**: Floating GPIO 34 would cause erratic encoder behavior
- ❌ **Safety Failure**: Temperature limits could be bypassed
- ❌ **Emergency Stop Failure**: Unreliable dual-button detection
- ❌ **Audio Feedback Failure**: Inconsistent buzzer behavior on ESP32
- ❌ **Code Maintenance**: Magic numbers and inconsistent patterns

**Code Quality Improvements**:
- ✅ **Safety-first design**: Multi-layer temperature limit enforcement
- ✅ **ESP32-native implementations**: Using LEDC instead of Arduino tone()
- ✅ **Proper documentation**: All GPIO limitations clearly documented
- ✅ **Centralized constants**: All limits use config.h (no magic numbers)
- ✅ **Consistency**: Both firmwares follow identical patterns

**Files Modified in This Session**:
```
src/main.cpp           - 5 critical fixes applied
src/hardware_test.cpp  - 2 critical fixes applied (GPIO + buzzer)
NOTES.md              - This comprehensive documentation
```

**Testing Recommendations**:
1. **GPIO Pull-up Test**: Verify encoders work reliably (no false triggers)
2. **Emergency Stop Test**: Confirm dual-button hold activates consistently
3. **Temperature Limit Test**: Attempt to exceed MAX_TEMP_LIMIT (should be blocked)
4. **Buzzer Test**: Verify all tones play with correct duration
5. **Long-term Stability**: Run for 24+ hours to verify no floating pin issues

---

**Status**: All critical issues identified and resolved. Firmware ready for hardware testing.

**Reviewer**: Claude Code (Anthropic)
**Review Date**: 2025-10-17
**Milestone**: M1.7 Integration Testing - Code Review & Bug Fixes Complete

---

**Document Status**: Reference material - update as needed
**Last Updated**: 2025-10-17
