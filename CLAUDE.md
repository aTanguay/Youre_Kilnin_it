# ESP32 Kiln Controller Project

## Project Overview

This is an affordable, ESP32-based PID kiln controller for small 120VAC ceramic kilns. The controller provides professional-grade temperature control with auto-tuning, web-based monitoring, energy cost tracking, and comprehensive safety features at ~$90 BOM cost versus $300-800 commercial alternatives.

**Target Users**: Home ceramic artists, small studios, hobbyists, educational institutions

**Critical Requirements**: Safety-first design, precise temperature control (±2°C), reliable operation, intuitive user interface

## Development Environment

### PlatformIO Configuration

**Platform**: Espressif 32 (ESP32)  
**Framework**: Arduino  
**Board**: esp32dev (ESP32-WROOM-32)

**Key platformio.ini Settings**:
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

## Hardware Architecture

### Core Components

| Component | Interface | GPIO Pins | Notes |
|-----------|-----------|-----------|-------|
| MAX31855 (Thermocouple) | SPI | CS: 5, CLK: 18, MISO: 19 | K-type thermocouple amplifier |
| SSR Control | GPIO Output | GPIO 25 | PWM/digital control, optoisolated |
| OLED Display (SSD1306) | I2C | SDA: 21, SCL: 22 | 128x64 resolution |
| Rotary Encoder | GPIO Input | CLK: 32, DT: 33, SW: 34 | With push button |
| Secondary Button | GPIO Input | GPIO 35 | Additional user input |
| Emergency Stop | GPIO Input | GPIO 36 (VP) | NC switch, pull-up enabled |
| Piezo Buzzer | GPIO Output | GPIO 26 | Audio feedback |
| Status LEDs | GPIO Output | Power: 27, WiFi: 14, Error: 12 | Optional indicators |

**Important**: 
- GPIOs 34-39 are input-only (no pull-up/pull-down)
- Avoid GPIOs 0, 2, 5, 12, 15 (boot strapping pins)
- SPI, I2C use default pins for reliability

### Electrical Isolation

**CRITICAL SAFETY**: ESP32 (3.3V DC) must be electrically isolated from AC mains (120VAC)
- SSR control through optocoupler
- Separate power supplies for logic and kiln
- Physical separation on PCB between AC and DC sections

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

## Key Libraries and Dependencies

### Temperature & Control
- **Adafruit MAX31855**: Thermocouple interface, handles SPI communication and cold junction compensation
- **PID_v1 (br3ttb)**: Robust PID controller with anti-windup and auto mode

### Display & Input
- **U8g2**: Universal graphics library for OLED, excellent for SSD1306, includes fonts and drawing primitives
- **Wire**: I2C communication (built-in)
- **Encoder Library** (optional): Simplifies rotary encoder handling with debouncing

### Web & Network
- **AsyncTCP**: Asynchronous TCP library for ESP32
- **ESPAsyncWebServer**: Non-blocking web server, essential for smooth operation while serving web UI
- **ArduinoJson**: JSON parsing/generation for API and profile storage, use v6.x

### Storage
- **SPIFFS or LittleFS**: File system for web files and profiles
- **Preferences**: ESP32 NVS (non-volatile storage) for settings

### Utilities
- **ESP32Time** or similar: Software RTC for timestamps
- **WiFi** (built-in): ESP32 WiFi stack

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

4. **Emergency Stop**
   - Monitor NC switch on GPIO 36
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

## Web Interface Guidelines

### API Endpoints

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

**WebSocket** (Real-time updates):
- `/ws` - Stream temperature, state, cost data every 1 second

### Web Interface Tech Stack

- **HTML5/CSS3**: Responsive design, mobile-first
- **Vanilla JavaScript**: Keep it lightweight, no heavy frameworks
- **Chart.js**: Temperature and cost graphing
- **WebSocket API**: Real-time data updates
- **Fetch API**: REST calls

**Minimize dependencies** - ESP32 serves files from SPIFFS, so keep total web UI < 500KB

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

## Testing Strategy

### Unit Testing
- Test each module independently
- Mock hardware interfaces (SPI, I2C) for desktop testing
- Use PlatformIO native testing when possible

### Integration Testing
- Test on actual hardware with mock kiln (resistive load)
- Verify all safety features trigger correctly
- Long-duration testing (24+ hour firings)

### Safety Testing
**CRITICAL - Must verify**:
- Emergency stop immediately cuts power
- Thermocouple disconnect triggers alarm and shutdown
- Maximum temperature limit cannot be overridden
- Watchdog timer triggers on firmware hang
- Temperature readings filtered/validated before use

### Field Testing
- Beta test with actual ceramic artists
- Various kiln sizes and types
- Different firing profiles (bisque, glaze, raku)
- Network reliability testing (WiFi dropout recovery)

## Development Workflow

1. **Feature Branch**: Create branch from `main`
2. **Implement**: Code new feature or fix
3. **Test**: Verify on hardware
4. **Document**: Update comments and docs
5. **Review**: Self-review for safety implications
6. **Merge**: Merge to `main` when stable

### Version Control
- **Tag releases**: v1.0.0, v1.1.0, etc. (semantic versioning)
- **Commit messages**: Clear, descriptive
  - "Add PID auto-tune relay method"
  - "Fix: Thermocouple read timeout on noisy power"
  - "Safety: Add max firing duration check"

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

- **Temperature Read**: < 50ms per read
- **PID Calculation**: < 10ms per cycle
- **Display Update**: < 100ms to refresh
- **Web Response**: < 200ms for API calls
- **WebSocket Latency**: < 100ms for real-time updates
- **Memory Usage**: < 200KB heap free during operation
- **Flash Usage**: < 80% of available (leave room for OTA)

## Resources & References

### Documentation
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

## Project Management Files

This project uses structured planning and task management files:

### PLANNING.md
Contains overall project roadmap, architecture decisions, and long-term goals.

**⚠️ CRITICAL**: Always read `PLANNING.md` at the start of every new conversation

### TASKS.md
Contains the current task list with priorities, status, and implementation details.

**Workflow Requirements**:
1. **Check `TASKS.md` before starting your work** - ensures you're working on the right priorities
2. **Mark completed tasks immediately** - keep status current for project tracking
3. **Add newly discovered tasks** - document new requirements or issues as they arise

## Support & Communication

- **Issues**: Document bugs and feature requests systematically in TASKS.md
- **Questions**: Assume Claude Code may need clarification on hardware specifics
- **Safety**: Always escalate safety concerns immediately
- **Testing**: Document test results, especially for safety features

---

## Quick Start for Claude Code Sessions

**MANDATORY FIRST STEPS** for every new Claude Code session:

1. **Read Context Files**:
   - ✅ Read `CLAUDE.MD` (this file) - project overview and technical specs
   - ✅ Read `PLANNING.md` - current project roadmap and decisions
   - ✅ Read `TASKS.md` - current work priorities and task status

2. **Before Starting Work**:
   - Check TASKS.md for current priorities
   - Understand which task you're working on
   - Consider dependencies and safety implications

3. **During Development**:
   - Follow naming conventions and code style from this document
   - Maintain safety-first approach
   - Test on hardware when possible

4. **After Completing Work**:
   - Mark completed tasks in TASKS.md immediately
   - Add any newly discovered tasks to TASKS.md
   - Document any architectural decisions in PLANNING.md
   - Update "Current Status" checklist below

**Key Reminders**:
- **Priority**: Safety is paramount - all changes must maintain safety features
- **Environment**: PlatformIO with Arduino framework
- **Testing**: All code should be testable on hardware before deployment
- **Before making changes**: Consider safety implications, validate inputs, maintain fail-safe defaults

**Current Status**: [Update this section as project progresses]
- [ ] Core temperature reading implemented
- [ ] PID controller functional
- [ ] Web interface basics
- [ ] Safety features complete
- [ ] Energy tracking implemented
- [ ] Field testing completed