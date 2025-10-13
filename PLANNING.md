# ESP32 Kiln Controller - Planning & Architecture

## Project Vision

### Mission Statement
Democratize professional-grade kiln control by creating an affordable, open-source ESP32-based controller that delivers precision temperature management, comprehensive safety features, and modern connectivity at a fraction of commercial controller costs.

### Core Objectives
1. **Affordability**: Target ~$90 BOM cost vs $300-800 commercial alternatives
2. **Safety**: Implement fail-safe design with redundant safety checks
3. **Precision**: Achieve ±2°C temperature control accuracy
4. **Accessibility**: Intuitive interface for non-technical ceramic artists
5. **Transparency**: Open-source design enabling community improvements
6. **Sustainability**: Cost tracking promotes energy awareness

### Success Criteria
- 100 consecutive firings without safety incidents
- Temperature accuracy within ±2°C at steady state
- User can initiate firing in under 60 seconds
- Cost tracking accuracy within ±5% of actual consumption
- Active community adoption and contributions

## Target Users

### Primary
- **Home Ceramic Artists**: Hobbyists upgrading mechanical controllers or building first kiln
- **Small Studios**: 1-3 person operations with budget constraints
- **Educational Institutions**: Schools, community centers teaching ceramics

### Secondary
- **Makers/DIY Enthusiasts**: Technical users interested in embedded systems
- **Kiln Builders**: Custom kiln manufacturers seeking affordable control solutions
- **International Markets**: Regions where commercial controllers are prohibitively expensive

### User Needs
- Reliable, set-and-forget operation
- Confidence in safety systems
- Visibility into firing progress from anywhere
- Understanding of operating costs
- Ability to save and reuse proven firing schedules

## Architecture Overview

### System Architecture (High-Level)

```
┌─────────────────────────────────────────────────────────────┐
│                        USER INTERFACES                       │
├──────────────┬──────────────────┬──────────────────────────┤
│ Local LCD    │  Web Dashboard   │  Mobile Browser          │
│ + Rotary Enc │  (WiFi)          │  (Responsive)            │
└──────────────┴──────────────────┴──────────────────────────┘
                            ▲
                            │
┌───────────────────────────┼───────────────────────────────┐
│                      ESP32 CORE                            │
├────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐ │
│  │   FreeRTOS   │  │  Web Server  │  │  WiFi Manager   │ │
│  │   Scheduler  │  │   (Async)    │  │   (AP/Client)   │ │
│  └──────────────┘  └──────────────┘  └─────────────────┘ │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐ │
│  │              APPLICATION LAYER                        │ │
│  ├──────────────┬──────────────┬──────────────────────┬─┤ │
│  │ Profile      │ PID Control  │ Safety Monitor       │ │ │
│  │ Engine       │ + Auto-tune  │ (Watchdog/Limits)    │ │ │
│  ├──────────────┼──────────────┼──────────────────────┼─┤ │
│  │ Energy/Cost  │ Data Logger  │ User Input Handler   │ │ │
│  │ Tracker      │              │ (Menu System)        │ │ │
│  └──────────────┴──────────────┴──────────────────────┴─┘ │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐ │
│  │              HARDWARE ABSTRACTION LAYER              │ │
│  ├──────────────┬──────────────┬──────────────────────┬─┤ │
│  │ Thermocouple │ SSR Control  │ Display Driver       │ │ │
│  │ (SPI)        │ (GPIO PWM)   │ (SPI - ST7920)       │ │ │
│  ├──────────────┼──────────────┼──────────────────────┼─┤ │
│  │ Rotary Enc   │ Buttons      │ Buzzer               │ │ │
│  │ (GPIO Int)   │ (GPIO Int)   │ (GPIO PWM)           │ │ │
│  └──────────────┴──────────────┴──────────────────────┴─┘ │
└────────────────────────────────────────────────────────────┘
                            ▲
                            │
┌───────────────────────────┼───────────────────────────────┐
│                    PHYSICAL LAYER                          │
├────────────────┬───────────────────┬───────────────────────┤
│ K-Type         │  Solid State      │  120VAC Kiln          │
│ Thermocouple   │  Relay (40A)      │  (1800W typical)      │
│ + MAX31855     │  + Heat Sink      │                       │
└────────────────┴───────────────────┴───────────────────────┘
```

### Data Flow

**Temperature Control Loop** (Primary):
```
Thermocouple → MAX31855 → ESP32 (SPI Read) → 
Temperature Validation → PID Algorithm → 
SSR Duty Cycle → Kiln Heating → (feedback loop)
```

**Safety Monitor Loop** (Parallel):
```
System State → Safety Checks (Temp Limits, Sensor Health, Timeouts) →
If Safe: Continue | If Unsafe: Emergency Shutdown + Alert
```

**User Interaction Flow**:
```
User Input (Rotary/Button/Web) → Input Handler → 
State Machine Update → Display/Web Update → Audio Feedback
```

**Energy Tracking Flow**:
```
SSR On-Time Tracking → Power Calculation (W = Wattage × Duty%) →
Energy Integration (kWh) → Cost Calculation ($ = kWh × Rate) →
Storage (Current/Billing/Annual/Lifetime)
```

### Architectural Principles

1. **Safety by Design**: Fail-safe defaults, redundant checks, immediate shutdown on error
2. **Separation of Concerns**: Hardware abstraction, business logic, UI separated
3. **Real-Time Responsiveness**: FreeRTOS tasks prioritized by criticality
4. **Non-Blocking Operations**: Async web server, no blocking delays in control loops
5. **Testability**: Hardware abstraction enables unit testing without physical devices
6. **Modularity**: Components can be developed and tested independently

### Key Architectural Decisions

| Decision | Rationale | Trade-offs |
|----------|-----------|------------|
| ESP32 vs ESP8266 | Dual-core, better performance, more GPIO | Slightly higher cost (~$2) |
| FreeRTOS Tasks vs Loop | Better real-time control, prioritization | More complex code structure |
| AsyncWebServer vs Standard | Non-blocking allows smooth control during web access | Requires async programming patterns |
| SPIFFS vs LittleFS | LittleFS more robust, faster | May need partition table adjustment |
| Single Thermocouple | Adequate for small kilns, simpler | Multi-zone kilns need upgrade path |
| SSR vs Mechanical Relay | Silent, long-lasting, no arcing | Requires heat sinking |
| ST7920 LCD vs OLED | Larger size, lower cost, better visibility | Slightly higher power consumption |
| JSON Profiles | Human-readable, standard format | Larger file size vs binary |

## Technology Stack

### Hardware Platform

**Microcontroller**: ESP32-WROOM-32
- **Why**: Dual-core 240MHz, WiFi/Bluetooth, 34 GPIO pins, mature ecosystem
- **Alternatives Considered**: 
  - ESP8266: Too limited (single core, fewer pins)
  - Arduino Mega: No WiFi, outdated platform
  - Raspberry Pi: Overkill, higher cost, less real-time capable

**Temperature Sensor**: K-Type Thermocouple + MAX31855
- **Why**: K-type standard for kilns (-200°C to 1350°C), MAX31855 has cold junction compensation
- **Alternatives Considered**:
  - MAX6675: Lower resolution (12-bit vs 14-bit)
  - Direct ADC: Requires complex cold junction compensation

**Power Control**: Solid State Relay (40A, Zero-Cross)
- **Why**: Silent, long-lasting (millions of cycles), no contact wear, low EMI
- **Alternatives Considered**:
  - Mechanical Relay: Noisy, limited lifespan, contact arcing
  - TRIAC Circuit: More complex, safety concerns with DIY builds

**Display**: 128x64 LCD Graphic Module (ST7920, SPI)
- **Why**: Larger physical size (better visibility), lower cost than OLED, full graphics capability with blue backlight, proven reliability
- **Advantages**:
  - Bigger display area improves readability in studio environment
  - Lower cost (~$5-8 vs $10-12 for OLED)
  - Blue backlight provides excellent contrast
  - Widely available from multiple suppliers
  - Full graphics mode (128x64 dots) supports charts and custom layouts
- **Alternatives Considered**:
  - 128x64 OLED (SSD1306): Smaller, higher cost, but lower power
  - 20x4 Character LCD: Character-only (no graphs), limited flexibility
  - TFT Color Display: Higher cost, unnecessary for application

**User Input**: Dual Rotary Encoders
- **Specifications**: Two 5V rotary encoders, 20 pulses per revolution each (detents)
- **Pins per encoder**: CLK (Pin A), DT (Pin B), SW (select switch), + (5V), GND
- **Why Dual Encoders**:
  - **Intuitive profile editing**: Direct manipulation of both time (X-axis) and temperature (Y-axis) simultaneously
  - **Improved UX**: Left encoder for navigation/time, right encoder for values/temperature
  - **Menu efficiency**: Two-axis control for complex parameter adjustment
  - **Professional feel**: Similar to audio mixing consoles, synthesizers
  - **Tactile feedback**: Physical encoders more satisfying than single encoder + buttons
- **Voltage Consideration**: 5V output is safe for ESP32 3.3V GPIO (high-impedance inputs tolerate 5V)
- **Use Cases**:
  - **Profile Editor**: Left = segment time, Right = target temperature
  - **Menu Navigation**: Left = scroll items, Right = adjust values
  - **Live Control**: Left = navigate modes, Right = adjust setpoint
- **Alternatives Considered**:
  - Single encoder + buttons: Less intuitive for 2D editing (time + temp profiles)
  - Touchscreen: More expensive, less reliable in dusty studio environment
  - Button Matrix: Many buttons needed, less elegant

### Software Stack

**Development Environment**
- **IDE**: PlatformIO (VSCode Extension)
  - **Why**: Superior to Arduino IDE - better library management, IntelliSense, debugging, build system
  - Professional development experience while maintaining Arduino compatibility

**Framework**: Arduino for ESP32
- **Why**: Large community, extensive libraries, rapid prototyping
- **Alternatives Considered**:
  - ESP-IDF: Steeper learning curve, less community support for hobbyist project
  - MicroPython: Lower performance for real-time control applications

**Operating System**: FreeRTOS
- **Why**: Built into ESP32, true multi-tasking, priority-based scheduling
- **Note**: Transparent when using Arduino framework

**Key Libraries**

| Library | Purpose | Why This One |
|---------|---------|--------------|
| ESPAsyncWebServer | Web UI hosting | Non-blocking, essential for smooth operation |
| AsyncTCP | TCP foundation | Required by AsyncWebServer |
| ArduinoJson | Profile/config storage | De facto standard, well-tested, efficient |
| Adafruit MAX31855 | Thermocouple interface | Industry standard, reliable, well-documented |
| PID_v1 (br3ttb) | Temperature control | Proven PID implementation with auto-tune support |
| U8g2 | ST7920 LCD graphics | Universal graphics library, supports ST7920 SPI, excellent fonts |
| Preferences | Settings storage | ESP32 NVS wrapper, simple key-value store |

**Web Technologies**
- **HTML5/CSS3**: Semantic markup, responsive design, CSS Grid/Flexbox
- **Vanilla JavaScript**: No framework overhead, direct WebSocket/Fetch API usage
- **Chart.js**: Lightweight, canvas-based graphing for temperature/cost visualization
- **WebSocket**: Real-time bidirectional communication for live updates

### Firmware Architecture

**Programming Paradigm**: Hybrid
- Object-oriented for major components (PIDController class, ProfileEngine class)
- Procedural for hardware interaction and simple utilities
- Task-based for concurrent operations (FreeRTOS)

**Memory Management**
- Static allocation preferred (no `new`/`malloc` in production code)
- Fixed-size buffers and arrays
- Circular buffers for logging (prevents memory leaks)

**State Management**
- Global state structure for system state (kilnState, energyData, config)
- Protected by mutexes where shared across tasks
- Single source of truth principle

**Error Handling**
- Return codes for critical functions
- Error logging to serial and file system
- Graceful degradation where possible
- Fail-safe shutdown for critical errors

## Required Tools

### Hardware Tools

**Development & Testing**
- [ ] ESP32 Development Board (ESP32-WROOM-32)
- [ ] USB-C or Micro-USB cable (depending on dev board)
- [ ] Breadboard and jumper wires
- [ ] MAX31855 Thermocouple Amplifier breakout board
- [ ] K-Type thermocouple (high-temp rated)
- [ ] 12864 LCD Display Module (128x64 dots, ST7920 controller, blue backlight)
- [ ] Two rotary encoder modules (5V, 20 pulses/revolution, with push button each)
- [ ] Piezo buzzer
- [ ] LEDs (3-5) and 220Ω resistors
- [ ] Solid State Relay (40A, zero-cross) + heat sink
- [ ] 5V power supply (2A minimum)
- [ ] Multimeter (DMM)
- [ ] Logic analyzer or oscilloscope (optional but helpful)

**For Testing Kiln Load**
- [ ] Resistive load (1500-1800W) or actual small kiln
- [ ] Kill-A-Watt meter or similar for power verification
- [ ] Thermocouple simulator or heat source for testing

**PCB Production** (Later Phase)
- [ ] PCB design software (KiCad recommended - free, open-source)
- [ ] PCB fabrication service account (JLCPCB, PCBWay, OSH Park)

**Enclosure & Assembly**
- [ ] Project enclosure (plastic, DIN-rail mountable)
- [ ] Screw terminals for AC connections
- [ ] Standoffs and mounting hardware
- [ ] Heat shrink tubing
- [ ] Wire strippers, crimpers, screwdrivers
- [ ] Label maker (optional but professional)

### Software Tools

**Essential**
- [ ] **Visual Studio Code** (IDE)
  - Download: https://code.visualstudio.com/
- [ ] **PlatformIO Extension** for VSCode
  - Install from VSCode Extensions Marketplace
- [ ] **Git** (Version control)
  - Download: https://git-scm.com/
- [ ] **Python 3.x** (Required by PlatformIO)
  - Download: https://www.python.org/

**Drivers**
- [ ] **CP2102 or CH340 USB-Serial Driver** (for ESP32 dev board)
  - Usually auto-installed, but download from Silicon Labs or WCH if needed

**Optional but Recommended**
- [ ] **Arduino IDE 2.x** (Backup development environment)
- [ ] **Postman** or **Insomnia** (API testing)
- [ ] **Chrome/Firefox DevTools** (Web interface debugging)
- [ ] **Serial Monitor Alternative**: PuTTY, CoolTerm, or screen (Linux/Mac)
- [ ] **WebSocket Testing Tool**: Websocket King, wscat

**Documentation & Design**
- [ ] **Markdown Editor**: Typora, MarkText, or VSCode
- [ ] **KiCad** (PCB design - free, open-source)
  - Download: https://www.kicad.org/
- [ ] **Fritzing** (Circuit diagrams for documentation)
  - Download: https://fritzing.org/
- [ ] **Draw.io** or **Excalidraw** (Architecture diagrams)

**Testing & Debugging**
- [ ] **ESP32 Exception Decoder** (PlatformIO tool)
- [ ] **ESP32 Sketch Data Upload** (For SPIFFS/LittleFS)

### Development Environment Setup

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

**Step 4: Configure platformio.ini**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
board_build.partitions = min_spiffs.csv

lib_deps = 
    me-no-dev/AsyncTCP@^1.1.1
    me-no-dev/ESP Async WebServer@^1.2.3
    bblanchon/ArduinoJson@^6.21.3
    adafruit/Adafruit MAX31855 library@^1.4.0
    br3ttb/PID@^1.2.1
    olikraus/U8g2@^2.34.22
```

**Step 5: Test Upload**
1. Connect ESP32 via USB
2. Click PlatformIO Upload button
3. Verify successful compilation and upload

## Thermocouple Calibration Procedure

### Overview

Accurate temperature measurement is critical for kiln control. K-type thermocouples and their associated wiring can introduce measurement errors due to:
- Manufacturing tolerances in the thermocouple
- Connection resistance at junction points
- Wire quality variations
- Cold junction compensation errors

**The ice-point calibration method** provides a simple, accurate way to determine the offset error at 0°C (32°F), which can then be applied across the entire temperature range.

### Why Ice-Point Calibration?

- **Known Reference**: Ice water at 0°C provides a stable, easily reproducible reference point
- **Simple Setup**: Requires only ice, water, and an insulated container
- **Accurate**: Properly prepared ice bath is accurate to ±0.1°C
- **Industry Standard**: Used by NIST and calibration laboratories worldwide

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

### Storage in Firmware

The calibration offset should be stored in non-volatile storage (NVS/Preferences):

```cpp
// During calibration
Preferences prefs;
prefs.begin("kiln-config", false);
prefs.putFloat("temp_offset", calibrationOffset);
prefs.end();

// During normal operation
Preferences prefs;
prefs.begin("kiln-config", true); // read-only
float offset = prefs.getFloat("temp_offset", 0.0); // default 0.0
float correctedTemp = thermocouple.readCelsius() + offset;
```

### Calibration Verification

After applying offset:
1. Repeat ice-point test
2. Controller should now read 0.0°C ± 0.5°C
3. Optional: Test at boiling point (100°C at sea level)
  - Should read 100.0°C ± 1.0°C
  - Adjust altitude: Boiling point decreases ~1°C per 300m elevation

### When to Recalibrate

- [ ] Initial setup (required)
- [ ] After replacing thermocouple
- [ ] After modifying thermocouple wiring
- [ ] Annually (recommended)
- [ ] If temperature readings seem inaccurate
- [ ] After any suspected thermocouple damage

### User Interface Considerations

**Calibration Mode Menu**:
1. "Start Calibration" → Instructions displayed
2. "Prepare ice bath (see manual)"
3. "Insert thermocouple into ice bath"
4. "Wait for stable reading..."
5. Display: "Current: 2.3°C | Expected: 0.0°C"
6. "Apply offset? -2.3°C" → [Yes/No]
7. "Calibration saved! Verify: [Run test]"

**Display During Calibration**:
```
┌──────────────────────┐
│ CALIBRATION MODE     │
├──────────────────────┤
│ Insert probe into    │
│ ice bath (0°C)       │
│                      │
│ Reading: 2.3°C       │
│ Samples: 12          │
│ Stable: Yes          │
│                      │
│ [Accept] [Cancel]    │
└──────────────────────┘
```

### Important Notes

- **MUST be done before PID auto-tune**: PID tuning depends on accurate temperature readings
- **Single-point calibration**: This method assumes linear error across temperature range (valid for thermocouples)
- **Cold junction**: MAX31855 handles cold junction compensation automatically
- **Two-point calibration**: Advanced users can perform both ice-point (0°C) and boiling-point (100°C) calibration to calculate both offset and gain
- **Safety**: Calibration should be performed with kiln OFF and cool

## Development Phases

### Phase 1: Hardware Proof of Concept (Weeks 1-2)
**Goal**: Validate core hardware functionality and calibrate thermocouple

**Tasks**:
- [ ] Breadboard ESP32 + MAX31855 + thermocouple
- [ ] Test temperature reading accuracy
- [ ] **Perform thermocouple ice-point calibration** (see Calibration Procedure below)
- [ ] Store calibration offset in NVS
- [ ] Implement basic SSR control (on/off)
- [ ] Display temperature on 12864 LCD (ST7920)
- [ ] Verify rotary encoder input

**Deliverable**: Working breadboard prototype with calibrated thermocouple reading and SSR control

### Phase 2: Core Control System (Weeks 3-5)
**Goal**: Implement temperature control and safety

**Tasks**:
- [ ] Implement PID controller
- [ ] Add manual setpoint adjustment
- [ ] Develop safety monitoring system
- [ ] Implement emergency stop
- [ ] Create basic menu system
- [ ] Add audio feedback

**Deliverable**: System can maintain set temperature safely

### Phase 3: Profile System (Weeks 6-7)
**Goal**: Multi-segment firing profiles

**Tasks**:
- [ ] Design profile data structure
- [ ] Implement profile engine (state machine)
- [ ] Create profile storage (SPIFFS/LittleFS)
- [ ] Build profile selection UI
- [ ] Test common firing schedules

**Deliverable**: Execute multi-segment firings automatically

### Phase 4: Web Interface (Weeks 8-10)
**Goal**: Remote monitoring and control

**Tasks**:
- [ ] Set up AsyncWebServer
- [ ] Build responsive HTML/CSS interface
- [ ] Implement REST API endpoints
- [ ] Add WebSocket for real-time updates
- [ ] Create profile management UI
- [ ] Implement authentication

**Deliverable**: Full-featured web dashboard

### Phase 5: Energy Tracking (Weeks 11-12)
**Goal**: Cost monitoring and reporting

**Tasks**:
- [ ] Implement power calculation
- [ ] Add cost tracking (current/billing/annual/lifetime)
- [ ] Create cost configuration UI
- [ ] Build cost analytics page
- [ ] Add CSV export

**Deliverable**: Comprehensive energy and cost tracking

### Phase 6: PID Auto-Tune (Weeks 13-14)
**Goal**: Automated PID optimization

**Prerequisites**:
- [ ] **Thermocouple MUST be calibrated** (ice-point method completed)
- [ ] Verify temperature readings accurate to ±1°C

**Tasks**:
- [ ] Implement relay auto-tune method
- [ ] Add Ziegler-Nichols calculations
- [ ] Create auto-tune UI with calibration check
- [ ] Test with different kiln loads
- [ ] Store optimal parameters per kiln

**Deliverable**: One-click PID optimization with calibrated temperature sensing

### Phase 7: PCB Design (Weeks 15-16)
**Goal**: Production-ready hardware

**Tasks**:
- [ ] Schematic design in KiCad
- [ ] PCB layout with AC/DC isolation
- [ ] Design review for safety
- [ ] Order prototype PCBs
- [ ] Assembly and testing

**Deliverable**: Custom PCB replacing breadboard

### Phase 8: Field Testing (Weeks 17-20)
**Goal**: Real-world validation

**Tasks**:
- [ ] Beta testing with ceramic artists
- [ ] Long-duration reliability testing
- [ ] Safety system validation
- [ ] Bug fixing and refinement
- [ ] Documentation creation

**Deliverable**: Production-ready firmware and hardware

### Phase 9: Documentation & Release (Weeks 21-22)
**Goal**: Open-source release

**Tasks**:
- [ ] User manual with setup guide
- [ ] Wiring diagrams
- [ ] Video tutorials
- [ ] GitHub repository setup
- [ ] Community launch

**Deliverable**: Public release with complete documentation

## Key Design Decisions

### Decision Log

**Decision 001**: Use Single Thermocouple Initially
- **Date**: 2025-10-11
- **Context**: Small 120VAC kilns vs larger multi-zone kilns
- **Decision**: Single thermocouple for initial version, hardware provision for expansion
- **Rationale**: Adequate for target market, reduces cost and complexity
- **Status**: Approved

**Decision 002**: Zero-Cross SSR vs Random-Fire SSR
- **Date**: 2025-10-11
- **Context**: EMI concerns and electrical noise
- **Decision**: Specify zero-cross SSRs only
- **Rationale**: Lower EMI, less interference with thermocouple readings
- **Status**: Approved

**Decision 003**: Display Selection - ST7920 LCD vs OLED
- **Date**: 2025-10-11 (Updated: 2025-10-11)
- **Context**: Display capabilities, cost, and visibility requirements
- **Decision**: 12864 LCD Module with ST7920 controller (SPI) as primary display
- **Rationale**:
  - Larger physical size improves readability in studio environment
  - Lower cost (~$5-8 vs $10-12 for OLED)
  - Full graphics mode (128x64 dots) supports temperature graphs and custom UI
  - Blue backlight provides excellent visibility
  - U8g2 library provides excellent ST7920 support
  - More cost-effective without sacrificing functionality
- **Status**: Approved

**Decision 004**: PlatformIO vs Arduino IDE
- **Date**: 2025-10-11
- **Context**: Development environment choice
- **Decision**: PlatformIO with Arduino framework
- **Rationale**: Better tooling, library management, and IntelliSense while maintaining Arduino ecosystem
- **Status**: Approved

**Decision 005**: Embedded Cost Tracking
- **Date**: 2025-10-11
- **Context**: User request for energy cost monitoring
- **Decision**: Implement comprehensive cost tracking (current firing, billing cycle, annual, lifetime)
- **Rationale**: Valuable feature for users, no additional hardware required (uses SSR duty cycle)
- **Status**: Approved

**Decision 006**: Dual Rotary Encoders
- **Date**: 2025-10-13
- **Context**: User interface design for profile editing and menu navigation
- **Decision**: Use two rotary encoders instead of single encoder + button
- **Rationale**:
  - Intuitive 2D control for firing profile editing (time X-axis, temperature Y-axis)
  - Direct manipulation feels more natural than sequential adjustment
  - Professional UX similar to audio/synth equipment
  - Left encoder: navigation/time, Right encoder: values/temperature
  - More engaging tactile experience
  - Minimal cost increase (~$3 for second encoder)
- **Status**: Approved

**Decision 007**: Dual-Button Emergency Stop (No Dedicated GPIO)
- **Date**: 2025-10-13
- **Context**: Emergency stop implementation for safety
- **Decision**: Emergency stop triggered by pressing both encoder switches simultaneously for 0.5 seconds, instead of dedicated emergency stop button
- **Rationale**:
  - **Prevents accidental activation**: Requires deliberate two-handed action, much harder to trigger accidentally than single button
  - **Saves cost**: Eliminates ~$2-3 emergency stop button component
  - **Saves GPIO pin**: GPIO 13 now available for future expansion (LED, sensor, etc.)
  - **Dual-action safety pattern**: Similar to professional safety systems (fighter jets, industrial equipment) that require multiple simultaneous actions
  - **Appropriate for target kilns**: Small, attended 120VAC kilns where user can simply unplug as backup emergency measure
  - **Simpler hardware**: Fewer components to wire, test, and potentially fail
  - **Professional UX**: Deliberate dual-button press feels more controlled than panic-hitting single button
- **Implementation**: Both encoder SW pins monitored simultaneously, 500ms hold time required before triggering emergency shutdown
- **Status**: Approved

### Open Questions

**Question**: Should we support 220VAC kilns in Version 1?
- **Status**: Deferred to V2 - focus on 120VAC for initial release
- **Next Steps**: Design V1 with upgrade path in mind

**Question**: MicroSD card for extended logging vs SPIFFS only?
- **Status**: Optional feature - provide header pins for easy addition
- **Impact**: Minimal - can be added without firmware changes

**Question**: Bluetooth support for mobile app?
- **Status**: Future consideration - WiFi sufficient for MVP
- **Note**: ESP32 has BLE capability available if needed

**Question**: Multiple PID parameter sets for different temperature ranges?
- **Status**: Investigate during auto-tune development
- **Research**: Temperature-dependent tuning may improve performance

## Risk Assessment

### Technical Risks

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Thermocouple EMI/noise | Medium | High | Use zero-cross SSR, shielded cable, software filtering |
| SSR overheating | Medium | Medium | Specify adequate heat sink, thermal monitoring in docs |
| WiFi reliability | Medium | Low | Local controls always functional, WiFi is monitoring only |
| Memory constraints | Low | Medium | Monitor heap usage, optimize web assets, use compression |
| Real-time deadline miss | Low | High | FreeRTOS priorities, watchdog timer, minimize ISR work |

### Safety Risks

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Thermocouple failure | Medium | Critical | Redundant detection (open/short circuit), immediate shutdown |
| Software hang | Low | Critical | Hardware watchdog timer, automatic reset and safe state |
| Over-temperature | Low | Critical | Multiple limit checks, hardware maximum, cannot be overridden |
| User configuration error | Medium | Medium | Validation on all inputs, sane defaults, confirmation dialogs |
| Power failure during firing | Medium | Medium | Document recovery procedure, log all firings |

### Project Risks

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Scope creep | High | Medium | Defined MVP, feature requests to backlog, phased approach |
| Component availability | Medium | Medium | Multiple vendor options, design with common parts |
| Community adoption | Medium | Low | Focus on documentation, easy setup, responsive support |
| Regulatory compliance | Low | Medium | Research UL/CE requirements early, design for compliance |

## Future Enhancements

### Near-Term (V1.x Updates)
- [ ] Cool-down monitoring with notification when safe to open
- [ ] Power failure recovery (resume or abort)
- [ ] Pre-heat soak for moisture removal
- [ ] Cone firing mode (Orton cone equivalents)
- [ ] Push notifications (Pushover, Telegram)
- [ ] Time-of-use electricity rate optimization

### Medium-Term (V2.0)
- [ ] 220VAC support for larger kilns
- [ ] Multi-thermocouple support (2-3 sensors)
- [ ] Multi-zone control (top/bottom independent heating)
- [ ] MicroSD card for extended logging
- [ ] Bluetooth LE for direct mobile connection
- [ ] Advanced graphing with overlay comparisons

### Long-Term (V3.0+)
- [ ] MQTT/Home Assistant integration
- [ ] Cloud logging and analytics
- [ ] Machine learning for PID optimization
- [ ] Atmosphere control (gas kiln, oxygen sensor)
- [ ] Mobile app (iOS/Android)
- [ ] Community profile repository

## Success Metrics & KPIs

### Technical Metrics
- Temperature control accuracy: ±2°C ✓
- Cost tracking accuracy: ±5% of actual ✓
- System uptime: >99.9% (excluding planned maintenance) ✓
- Web interface response time: <200ms ✓
- Firmware OTA update success rate: >95% ✓

### User Experience Metrics
- Time to first firing: <15 minutes from unboxing ✓
- Time to start firing: <60 seconds ✓
- User error rate: <5% of firing attempts ✓
- Safety incidents: 0 ✓

### Project Metrics
- BOM cost: <$100 ✓
- Community contributions: >10 contributors in first year
- GitHub stars: >500 in first year
- Production units deployed: >100 in first year

## Resources & References

### Technical Documentation
- ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
- MAX31855 Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/MAX31855.pdf
- PID Control Theory: https://en.wikipedia.org/wiki/PID_controller
- Ziegler-Nichols Tuning: https://en.wikipedia.org/wiki/Ziegler–Nichols_method

### Ceramic Firing Resources
- Orton Cone Temperature Chart: https://www.ortonceramic.com/
- Skutt Kiln Firing Schedules: https://skutt.com/firing-techniques/
- Electric Kiln Best Practices

### Safety Standards
- UL 873: Temperature-Indicating and Regulating Equipment
- IEC 61010: Safety requirements for electrical equipment
- Thermocouple Installation Standards

### Development Resources
- PlatformIO Documentation: https://docs.platformio.org/
- ESP32 Arduino Core: https://docs.espressif.com/projects/arduino-esp32/
- FreeRTOS Documentation: https://www.freertos.org/
- AsyncWebServer Library: https://github.com/me-no-dev/ESPAsyncWebServer

### Community
- ESP32 Forum: https://esp32.com/
- Arduino Forum: https://forum.arduino.cc/
- r/embedded (Reddit)
- r/esp32 (Reddit)

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-11 | Initial | Created comprehensive planning document |

---

**Next Review Date**: [Update after major milestone or quarterly]

**Document Status**: Living document - update as project evolves