# ESP32 Kiln Controller - AI Assistant Guide

## Quick Reference

**Project**: Affordable ESP32-based PID kiln controller (~$90 BOM)
**Platform**: ESP32-WROOM-32, PlatformIO, Arduino Framework
**Safety**: Fail-safe design, redundant checks, ±2°C temperature control

**⚠️ READ FIRST**: `PLANNING.md` for architecture decisions, `TASKS.md` for current priorities

## Build Commands

- `pio run` - Build | `pio run --target upload` - Upload | `pio run --target monitor` - Monitor
- See `platformio.ini` for library dependencies and configuration

## Hardware Architecture

### Core Components

| Component | Interface | GPIO Pins | Notes |
|-----------|-----------|-----------|-------|
| MAX31855 (Thermocouple) | SPI | CS: 5, CLK: 18, MISO: 19 | K-type thermocouple amplifier |
| SSR Control | GPIO Output | GPIO 25 | PWM/digital control, optoisolated |
| TFT Display (ILI9341) | SPI | CS: 15, DC: 2, MOSI: 23, SCK: 18, RST: 4 | 240x320 color, 2.4" or 2.8" |
| Rotary Encoder (Left) | GPIO Input | CLK: 32, DT: 33, SW: 34 | 5V module, 20 pulses/rev, navigation/time |
| Rotary Encoder (Right) | GPIO Input | CLK: 35, DT: 39, SW: 36 | 5V module, 20 pulses/rev, values/temp |
| Emergency Stop | Both Encoder SW | Press both SW simultaneously (0.5s hold) | Dual-button safety, no dedicated GPIO |
| Piezo Buzzer | GPIO Output | GPIO 26 | Audio feedback |
| Status LEDs | GPIO Output | Power: 27, WiFi: 14, Error: 12 | Optional indicators |

**GPIO Notes**:
- GPIOs 34-39: Input-only (right encoder uses these)
- GPIO 2: Used for TFT DC (data/command select), pulled down at boot (safe)
- TFT/MAX31855 share SPI bus (different CS pins)
- Emergency stop: Both encoder SW pressed simultaneously for 0.5s

**⚠️ CRITICAL SAFETY**: AC mains isolated from ESP32 via SSR optocoupler, separate power supplies

## Software Architecture

### FreeRTOS Task Structure

```cpp
void setup() {
    // Hardware initialization
    xTaskCreatePinnedToCore(tempReadTask, "TempRead", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(pidControlTask, "PIDControl", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(displayTask, "Display", 8192, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(userInputTask, "UserInput", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(safetyMonitorTask, "Safety", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(energyTrackTask, "Energy", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(dataLogTask, "DataLog", 4096, NULL, 1, NULL, 1);
}
```

**Task Priorities** (higher = more important):
- Priority 3: Safety Monitor (most critical)
- Priority 2: Temperature Reading, PID Control
- Priority 1: Display, Input, Energy, Logging

**Task Timing**:
- Temperature Reading: 100ms cycle
- PID Control: 1s cycle (matches SSR cycle time)
- Display Update: 250ms cycle
- User Input: 50ms cycle (debouncing)
- Safety Monitor: 500ms cycle
- Energy Tracking: 1s cycle
- Data Logging: 1-60s (user configurable)

### Module Organization

```
src/
├── main.cpp                 # Setup, task creation, main loop
├── config.h                 # Pin definitions, constants, feature flags
├── temperature/
│   ├── thermocouple.cpp     # MAX31855 interface
│   ├── thermocouple.h
│   └── calibration.cpp      # Offset/gain calibration
├── control/
│   ├── pid_controller.cpp   # PID implementation
│   ├── pid_controller.h
│   ├── autotune.cpp         # Relay/ZN auto-tuning
│   └── profile_engine.cpp   # Multi-segment firing profiles
├── safety/
│   ├── safety_monitor.cpp   # Watchdog, limits, error detection
│   └── emergency_stop.cpp   # E-stop handling
├── ui/
│   ├── display.cpp          # OLED/LCD rendering
│   ├── menu_system.cpp      # Rotary encoder navigation
│   ├── input_handler.cpp    # Button debouncing
│   └── buzzer.cpp           # Audio feedback
├── web/
│   ├── web_server.cpp       # AsyncWebServer setup
│   ├── api_handlers.cpp     # REST API endpoints
│   ├── websocket.cpp        # Real-time data streaming
│   └── auth.cpp             # Authentication
├── storage/
│   ├── profiles.cpp         # Profile CRUD operations
│   ├── preferences.cpp      # Settings storage (EEPROM/NVS)
│   └── data_logger.cpp      # Temperature/cost logging
├── energy/
│   ├── power_monitor.cpp    # SSR duty cycle tracking
│   └── cost_calculator.cpp  # Energy cost calculations
└── utils/
    ├── timing.cpp           # Non-blocking timing utilities
    └── helpers.cpp          # Common utility functions

data/
├── www/                     # Web interface files (SPIFFS)
│   ├── index.html
│   ├── dashboard.html
│   ├── profiles.html
│   ├── costs.html
│   ├── config.html
│   ├── css/
│   │   └── style.css
│   └── js/
│       ├── main.js
│       ├── charts.js
│       └── websocket.js
└── profiles/                # Default firing profiles (JSON)
    ├── bisque_cone04.json
    ├── glaze_cone6.json
    └── raku.json
```

## Key Libraries

| Library | Purpose |
|---------|---------|
| Adafruit MAX31855 | Thermocouple SPI interface |
| PID_v1 (br3ttb) | PID controller with anti-windup |
| TFT_eSPI | ILI9341 color TFT graphics driver |
| ESPAsyncWebServer | Non-blocking web server |
| ArduinoJson | JSON for profiles/config |
| Preferences | ESP32 NVS storage |

## Core Data Structures

### Firing Profile
```cpp
struct ProfileSegment {
    float targetTemp;        // Target temperature (°C)
    float rampRate;          // Rate (°C/hour), 0 = full power
    uint16_t soakTime;       // Hold time at target (minutes)
};

struct FiringProfile {
    char name[32];
    char description[64];
    uint8_t numSegments;
    ProfileSegment segments[8];
    float maxTemp;           // Safety limit for this profile
    bool isActive;
};
```

### System State
```cpp
enum SystemState {
    STATE_IDLE,
    STATE_PREHEATING,
    STATE_FIRING,
    STATE_SOAKING,
    STATE_COOLING,
    STATE_COMPLETE,
    STATE_ERROR,
    STATE_EMERGENCY_STOP
};

struct KilnState {
    SystemState state;
    float currentTemp;
    float targetTemp;
    float pidOutput;         // 0-100%
    uint8_t currentSegment;
    unsigned long segmentStartTime;
    unsigned long firingStartTime;
    bool sensorError;
    bool emergencyStop;
};
```

### Energy Tracking
```cpp
struct EnergyData {
    float currentFiringKWh;
    float currentFiringCost;
    float billingCycleCost;
    float annualCost;
    float lifetimeCost;
    unsigned long billingCycleStart;
    unsigned long annualStart;
};
```

### Configuration
```cpp
struct SystemConfig {
    // Temperature
    float tempOffset;        // Calibration offset
    float tempGain;          // Calibration gain
    bool useFahrenheit;
    
    // PID
    float kp, ki, kd;
    float pidSampleTime;     // seconds
    
    // Safety
    float maxTempLimit;      // Hard limit (1320°C)
    float maxRampRate;       // °C/hour
    uint32_t maxFiringDuration; // seconds
    
    // Energy
    uint16_t kilnWattage;    // Watts
    float electricityRate;   // $/kWh
    uint8_t billingCycleDay; // 1-31
    char currencySymbol[4];
    
    // Network
    char ssid[32];
    char password[64];
    char apPassword[32];
    bool useAccessPoint;
};
```

## Critical Safety Requirements

### Hardware Safety Checks (Every 500ms)
1. **Thermocouple Validation**
   - Check for open circuit (reading > 1400°C indicates disconnection)
   - Check for short circuit (reading < -100°C)
   - Check for excessive noise (rapid fluctuations)

2. **Temperature Limits**
   - Current temp < maxTempLimit (1320°C hard coded)
   - Current temp < profile maxTemp
   - Rate of rise < maxRampRate

3. **Timeout Protection**
   - Firing duration < maxFiringDuration (default 48 hours)
   - Watchdog timer reset every cycle

4. **Emergency Stop (Dual Button Press)**
   - Monitor both encoder switches simultaneously
   - Require 0.5 second hold to trigger (prevents accidental activation)
   - Immediate SSR shutoff on activation

### Software Safety Patterns

**Always Default to Safe State**:
```cpp
// GOOD: Fail-safe default
pinMode(SSR_PIN, OUTPUT);
digitalWrite(SSR_PIN, LOW);  // SSR off

// BAD: Could start with SSR on
pinMode(SSR_PIN, OUTPUT);
```

**Watchdog Timer**:
```cpp
void setup() {
    esp_task_wdt_init(10, true);  // 10 second timeout, panic on trigger
    esp_task_wdt_add(NULL);       // Add current task
}

void loop() {
    esp_task_wdt_reset();  // Must be called regularly
}
```

**Temperature Validation**:
```cpp
bool isValidTemperature(float temp) {
    // Reject impossible readings
    if (temp < -50 || temp > 1400) return false;
    
    // Reject excessive change (>100°C/sec impossible)
    if (abs(temp - lastTemp) > 100) return false;
    
    // Check for NaN
    if (isnan(temp)) return false;
    
    return true;
}
```

**Error Recovery**:
```cpp
void handleSensorError() {
    // Immediate safe state
    digitalWrite(SSR_PIN, LOW);
    
    // Set system state
    kilnState.state = STATE_ERROR;
    kilnState.sensorError = true;
    
    // Alert user
    buzzer.alarm();
    display.showError("SENSOR FAILURE");
    
    // Log error
    logError("Thermocouple error detected");
    
    // Cannot recover automatically - requires user intervention
}
```

## PID Control Implementation

### PID Tuning Approach

**Initial Conservative Values**:
```cpp
// Start with conservative PID values
float kp = 5.0;   // Proportional gain
float ki = 0.5;   // Integral gain  
float kd = 1.0;   // Derivative gain
```

**Auto-Tune Process** (Relay Method):
1. Heat kiln at 50% power until oscillation occurs
2. Measure oscillation period (Pu) and amplitude
3. Calculate PID parameters using Ziegler-Nichols rules
4. Apply calculated values and fine-tune

**Temperature-Dependent Tuning** (Advanced):
- Low temps (< 400°C): More aggressive (faster response)
- Mid temps (400-800°C): Standard tuning
- High temps (> 800°C): Conservative (prevent overshoot)

### SSR Control Strategy

**Time-Proportional Output**:
```cpp
// SSR cycle time: 2-4 seconds
const unsigned long SSR_CYCLE_MS = 2000;

void updateSSR(float pidOutput) {
    // pidOutput is 0-100%
    unsigned long onTime = (SSR_CYCLE_MS * pidOutput) / 100;
    unsigned long offTime = SSR_CYCLE_MS - onTime;
    
    digitalWrite(SSR_PIN, HIGH);
    delay(onTime);
    digitalWrite(SSR_PIN, LOW);
    delay(offTime);
}
```

**Note**: Use non-blocking timing in actual implementation!

## Thermocouple Calibration

**CRITICAL**: Must calibrate before PID auto-tune. See `PLANNING.md` for detailed ice-point calibration procedure.

**Implementation Checklist**:
- Store offset in NVS: `Preferences prefs; prefs.putFloat("temp_offset", offset);`
- Apply: `calibratedTemp = rawTemp + calibrationData.offset;`
- Quality thresholds: Excellent (±0.5°C), Good (±1.0°C), Acceptable (±2.0°C)
- Recalibrate: Initial setup, after thermocouple replacement, annually

**API Endpoints**: `/api/calibration` (GET), `/api/calibration/start` (POST)

**Key Safety Check**:
```cpp
bool canStartCalibration() {
    return (kilnState.state == STATE_IDLE) &&
           (digitalRead(SSR_PIN) == LOW) &&
           (!isnan(max31855.readCelsius()));
}
```

## Web Interface API

**REST Endpoints**: `/api/status`, `/api/profiles`, `/api/firing/{start|stop}`, `/api/costs`, `/api/config`, `/api/autotune/{start|status}`, `/api/calibration`

**WebSocket**: `/ws` - 1s interval updates (temp, state, cost)

**Tech**: HTML5/CSS3, Vanilla JS, Chart.js, WebSocket API. Keep total < 500KB for SPIFFS.

## Coding Standards

### Naming Conventions
- **Functions**: camelCase - `readTemperature()`, `updateDisplay()`
- **Variables**: camelCase - `currentTemp`, `pidOutput`
- **Constants**: UPPER_SNAKE_CASE - `MAX_TEMP_LIMIT`, `SSR_PIN`
- **Classes**: PascalCase - `PIDController`, `ProfileEngine`
- **Files**: snake_case - `pid_controller.cpp`, `web_server.h`

### Code Style
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Comments**: Doxygen-style for functions, inline for complex logic
- **Line Length**: 100 characters max

### Error Handling
```cpp
// Always validate inputs
float readTemperature() {
    float temp = max31855.readCelsius();
    
    if (isnan(temp)) {
        logError("Thermocouple read returned NaN");
        return TEMP_ERROR_VALUE;
    }
    
    if (!isValidTemperature(temp)) {
        logError("Invalid temperature reading: " + String(temp));
        return TEMP_ERROR_VALUE;
    }
    
    return temp;
}
```

### Memory Management
- **Avoid dynamic allocation**: Use fixed-size arrays, no `new`/`malloc` in embedded systems
- **Stack usage**: Monitor stack depth, FreeRTOS stack overflow detection
- **Global vs local**: Minimize globals, prefer passing by reference

## Safety Testing Checklist

**CRITICAL - Must verify before deployment**:
- [ ] Emergency stop immediately cuts power
- [ ] Thermocouple disconnect triggers alarm and shutdown
- [ ] Maximum temperature limit cannot be overridden
- [ ] Watchdog timer triggers on firmware hang
- [ ] Temperature readings validated before use

## Git Workflow

**Commit message format**: "Add/Fix/Safety: [description]"
**Tag releases**: Semantic versioning (v1.0.0, v1.1.0, etc.)

## Common Pitfalls to Avoid

1. **Blocking Operations**: Never use `delay()` in critical paths - use non-blocking timing
2. **Integer Overflow**: Use `unsigned long` for `millis()` comparisons
3. **Floating Point**: Be careful with equality checks, use epsilon comparison
4. **Global State**: Protect shared variables with mutexes in multi-task environment
5. **Watchdog**: Don't forget to reset watchdog timer in long operations
6. **SPIFFS**: Check return values, handle file system full gracefully
7. **WiFi**: Handle disconnections, don't assume stable connection
8. **SSR Heat**: Monitor SSR temperature, thermal shutdown possible

## Performance Targets

Temp read <50ms | PID calc <10ms | Display <100ms | API <200ms | WebSocket <100ms | Heap >200KB free | Flash <80%

---

## AI Assistant Session Workflow

**MANDATORY FIRST STEPS**:
1. Read `PLANNING.md` (architecture, design decisions, calibration procedure)
2. Read `TASKS.md` (current priorities and status)
3. Understand task context before coding

**During Development**:
- Follow naming conventions and safety patterns above
- Mark tasks complete in TASKS.md immediately
- Document architectural decisions in PLANNING.md
- Safety-first: validate inputs, fail-safe defaults, test on hardware

**Session History**: See end of `PLANNING.md` for development session logs