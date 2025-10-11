# ESP32 Kiln Controller - Product Requirements Document

## 1. Product Overview

### 1.1 Product Vision
An affordable, precise, and safe ESP32-based kiln controller designed for small-scale ceramic artists, hobbyists, and educational institutions. The controller provides professional-grade PID temperature control with auto-tuning capabilities, web-based monitoring, and comprehensive safety features at a fraction of the cost of commercial solutions.

### 1.2 Target Market
- Home ceramic artists and pottery hobbyists
- Small studios with 120VAC kilns
- Educational institutions (schools, community centers)
- Makers and DIY enthusiasts
- Users upgrading legacy mechanical kiln controllers

### 1.3 Key Differentiators
- Highly affordable compared to commercial controllers ($50-100 BOM vs $300-800 retail)
- Modern web interface accessible from any device
- Open architecture allowing customization
- PID auto-tuning for optimal performance
- Visual temperature graphing on local display

## 2. Core Features & Requirements

### 2.1 Temperature Control
**Priority: Critical**

- **PID Control**: Implements proportional-integral-derivative control algorithm for precise temperature regulation
- **PID Auto-Tune**: Ziegler-Nichols or Relay method for automatic PID parameter optimization
- **Temperature Range**: 0°C to 1300°C (32°F to 2372°F)
- **Accuracy**: ±2°C at steady state
- **Control Resolution**: 0.1°C display, 1°C control increments
- **SSR Cycling**: Configurable cycle time (default 2-4 seconds for optimal SSR lifespan and temperature control)

### 2.2 Firing Profiles
**Priority: Critical**

- **Profile Storage**: Minimum 10 locally stored profiles in flash memory
- **Profile Parameters**:
  - Multi-segment ramps (up to 8 segments per profile)
  - Target temperatures per segment
  - Ramp rates (°C/hour or °F/hour)
  - Soak/hold times at target temperatures
  - Profile name and description
- **Profile Management**:
  - Create, edit, delete profiles via web interface
  - Upload profiles via JSON file
  - Export profiles for backup/sharing
  - Pre-loaded common firing schedules (Bisque, Glaze, Raku, etc.)

### 2.3 Safety Features
**Priority: Critical**

- **Hardware Safety**:
  - Maximum temperature limit (hard-coded 1320°C fail-safe)
  - User-configurable maximum temperature per firing
  - Physical emergency stop integration (NC circuit)
  - Thermocouple failure detection (open circuit, short circuit)
  - SSR failure detection (temperature not responding to output)
  
- **Software Safety**:
  - Watchdog timer to detect firmware hang
  - Maximum rate-of-rise detection (prevents thermal shock)
  - Maximum firing duration timeout
  - Overshoot protection during heating
  - Sensor noise filtering and validation
  - Graceful degradation on sensor error
  - Safe state on power loss (SSR off)

- **User Alerts**:
  - Audible alarm on critical errors
  - Flashing display on warnings
  - Web notification system
  - Error logging with timestamps

### 2.4 Web Interface
**Priority: High**

- **Access**: ESP32 creates WiFi access point or connects to existing network
- **Features**:
  - Real-time temperature monitoring with live graph
  - Current firing status and progress
  - **Real-time cost tracking** for active firing
  - **Cost dashboard** showing billing cycle, annual, and lifetime costs
  - Profile selection and management
  - PID parameter viewing and manual adjustment
  - Auto-tune initiation and monitoring
  - Historical firing logs with downloadable data (CSV) **including cost per firing**
  - **Cost reports** with monthly/yearly summaries and graphs
  - System configuration (WiFi, safety limits, calibration, **electricity rates**)
  - Firmware update capability (OTA)
  - Mobile-responsive design

- **Authentication**: Password-protected admin functions

### 2.5 Local Display & Controls
**Priority: High**

- **Display Type**: 128x64 OLED (SSD1306) or 20x4 character LCD with I2C
- **Display Content**:
  - Current temperature (large font)
  - Target temperature
  - Time remaining (estimated)
  - **Current firing cost** (updating in real-time)
  - Real-time temperature graph (last 30-60 minutes)
  - Firing progress bar
  - Current segment information
  - Power usage (watts or % duty cycle)
  - Status messages and alerts

- **Input Controls**:
  - **Primary**: Rotary encoder with integrated push button
    - Rotate: Navigate menus, adjust values
    - Press: Select/confirm
  - **Secondary**: Button left of display
    - Quick access to firing start/stop
    - Back/cancel in menus
  - **Audio Feedback**: Piezo buzzer for button presses, alerts, and firing milestones

### 2.6 Energy & Cost Tracking
**Priority: High**

- **Power Monitoring**:
  - Track kiln on-time percentage (SSR duty cycle)
  - Calculate energy consumption based on user-configured kiln wattage
  - Real-time power usage display (watts, kWh)

- **Cost Calculation**:
  - User-configurable electricity rate ($/kWh or local currency)
  - **Current Firing Cost**: Live cost accumulation for active firing
  - **Billing Cycle Cost**: Total cost since last billing cycle reset
  - **Annual Cost**: Running total for calendar year
  - **Lifetime Cost**: Total since initial setup or manual reset

- **Cost Configuration**:
  - Kiln wattage input (default: 1800W for typical 120VAC kiln)
  - Electricity rate with support for tiered/time-of-use rates
  - Currency symbol configuration
  - Billing cycle start date (1-31 of month)
  - Automatic monthly reset option

- **Cost Display & Reporting**:
  - Cost shown on local display during firing
  - Cost summary on web dashboard
  - Historical cost per firing in logs
  - Monthly/yearly cost reports with graphs
  - Estimated cost projection for in-progress firings
  - Export cost data as CSV for accounting

### 2.7 Data Logging
**Priority: Medium**

- **Log Data**: Temperature, target, output percentage, power usage, cost, timestamp
- **Storage**: Local flash memory (circular buffer, ~24 hours at 1-second intervals)
- **Export**: Download via web interface as CSV with cost data included
- **Optional**: MicroSD card slot for extended logging

## 3. Hardware Specifications

### 3.1 Core Components

| Component | Specification | Notes |
|-----------|--------------|-------|
| Microcontroller | ESP32 (ESP32-WROOM-32) | Dual-core, WiFi, adequate GPIO |
| Thermocouple | K-type, -200°C to 1350°C | High-temp ceramic insulation |
| Thermocouple Amplifier | MAX31855 or MAX6675 | SPI interface, cold junction compensation |
| Solid State Relay | 25A-40A, zero-cross, 120VAC | Fotek SSR-25DA or similar, heat sink required |
| Display | 128x64 OLED (SSD1306) | I2C interface, high contrast |
| Rotary Encoder | EC11 or similar | With integrated push button |
| Buzzer | Piezo buzzer, 3-5V | Active or passive |
| Power Supply | 5V 2A regulated | Buck converter from 120VAC or external adapter |
| Enclosure | Plastic project box | DIN rail mountable optional |

### 3.2 Additional Hardware Components

- **Buttons**: 2x tactile push buttons (rotary encoder + secondary)
- **Emergency Stop**: NC switch input with pull-up
- **Status LEDs**: 2-3 LEDs (Power, WiFi, Error/Firing)
- **Terminals**: Screw terminals for AC input, SSR output, thermocouple
- **Protection**: Varistor/MOV on AC input, fuse

### 3.3 PCB Considerations

- Clear separation of low-voltage DC and high-voltage AC sections
- Optical isolation between ESP32 and SSR (optocoupler)
- Proper clearance and creepage for AC traces
- Test points for debugging
- Mounting holes for standoffs

## 4. Software Architecture

### 4.1 Firmware Structure

```
Main Loop (FreeRTOS Tasks):
├── Temperature Reading Task (100ms)
├── PID Control Task (1s or SSR cycle time)
├── Display Update Task (250ms)
├── Web Server Task (async)
├── User Input Task (50ms)
├── Safety Monitor Task (500ms)
├── Energy/Cost Tracking Task (1s)
└── Data Logging Task (1-60s configurable)
```

### 4.2 Key Software Modules

- **PID Controller**: Configurable Kp, Ki, Kd with anti-windup
- **Auto-Tune**: Relay or Ziegler-Nichols method implementation
- **Profile Engine**: State machine for multi-segment firing schedules
- **Energy Monitor**: Tracks SSR duty cycle and calculates power consumption
- **Cost Calculator**: Real-time cost accumulation with billing cycle management
- **Web Server**: AsyncWebServer library for ESP32
- **File System**: SPIFFS or LittleFS for profile and config storage
- **OTA Updates**: ESP32 OTA library for firmware updates
- **Calibration**: Offset and gain adjustment for thermocouple
- **Preferences**: Non-volatile storage for user settings (including electricity rates)

### 4.3 Development Environment

- **IDE**: Arduino IDE or PlatformIO (VSCode)
- **Framework**: Arduino framework for ESP32
- **Key Libraries**:
  - ESP32 Arduino Core
  - AsyncWebServer
  - ArduinoJson
  - MAX31855 driver library
  - PID_v1 or custom PID implementation
  - U8g2 (for OLED) or LiquidCrystal_I2C

## 5. User Interface Requirements

### 5.1 Local Display Navigation

```
Main Screen
├── View Current Firing
│   ├── Temperature Graph
│   ├── Firing Details
│   └── Cost (Current Firing)
├── Start New Firing
│   ├── Select Profile
│   ├── Confirm Parameters
│   └── Begin
├── Cost Summary
│   ├── Billing Cycle Cost
│   ├── Annual Cost
│   ├── Lifetime Cost
│   └── Reset Options
├── Settings
│   ├── WiFi Setup
│   ├── Cost Configuration
│   │   ├── Electricity Rate
│   │   ├── Kiln Wattage
│   │   └── Billing Cycle Date
│   ├── Calibration
│   ├── Safety Limits
│   └── Display Options
└── System Info
    ├── Firmware Version
    ├── Network Status
    └── Last Firing Log
```

### 5.2 Web Interface Pages

1. **Dashboard** - Current temperature, status, live graph, **current firing cost**
2. **Profiles** - List, create, edit, delete firing profiles
3. **Start Firing** - Select profile, **estimated cost preview**, and initiate
4. **History** - Past firings with downloadable data **including cost per firing**
5. **Cost Tracking** - Dedicated page for cost analytics:
   - Current billing cycle total with days remaining
   - Monthly cost history (bar chart)
   - Annual cost projection
   - Lifetime statistics
   - Cost per firing comparison
   - Manual reset buttons for billing cycle/annual/lifetime
6. **PID Settings** - View/adjust PID parameters, run auto-tune
7. **Configuration** - WiFi, safety limits, calibration, units, **electricity rates & billing**
8. **About** - Firmware version, system info, documentation links

### 5.3 Audio Feedback Events

- Button press: Short beep
- Menu navigation: Click
- Firing start: Ascending tone
- Target reached: Double beep
- Segment transition: Single beep
- Firing complete: Musical chime
- Error/warning: Rapid beeping or alarm tone

## 6. Safety & Compliance

### 6.1 Electrical Safety

- Proper isolation between low voltage (ESP32) and high voltage (120VAC)
- Fused AC input
- Strain relief on all external connections
- Clear labeling of high voltage components
- Instructions for proper grounding

### 6.2 Thermal Safety

- Temperature limits cannot exceed physical component ratings
- Automatic shutdown on sensor failure
- User cannot override critical safety limits via interface
- Warning labels about kiln surface temperatures

### 6.3 Software Safety

- Default to safe state (SSR off) on any error
- Timeout protection (maximum 48-hour firing duration)
- Rate-of-rise limits to prevent thermal shock
- Redundant safety checks in multiple code paths

## 7. Additional Feature Ideas

### 7.1 Near-Term Enhancements

1. **Cool-Down Monitoring**: Continue monitoring temperature after firing ends until kiln reaches safe temperature (e.g., 50°C)

2. **Power Failure Recovery**: Detect unexpected shutdowns and allow user to resume or abort firing

3. **Pre-Heat Soak**: Optional low-temperature hold before main firing begins (for moisture removal)

4. **Cone Firing Mode**: Support for cone-based firing (equivalent temperatures for Orton cones)

5. **Mobile Notifications**: Push notifications via services like Pushover or Telegram for firing milestones **and cost thresholds**

6. **Multi-Thermocouple Support**: Hardware provision for 2-3 thermocouples for improved accuracy or larger kilns

7. **Time-of-Use Optimization**: Alert user to cheaper electricity times for scheduling firings

8. **Cost Budget Alerts**: Set cost limits and receive warnings when approaching threshold

### 7.2 Future Considerations

1. **Home Automation Integration**: MQTT support for Home Assistant, OpenHAB integration

2. **Cloud Logging**: Optional upload of firing data to cloud service for analysis

3. **Machine Learning**: Predict optimal PID parameters based on kiln characteristics and historical data

4. **Atmosphere Control**: Support for gas kiln applications with oxygen sensor integration

5. **220VAC Version**: Hardware variant for larger kilns with appropriate safety certifications

6. **Mobile App**: Dedicated iOS/Android app for enhanced mobile experience

7. **Community Profile Sharing**: Online repository of user-contributed firing profiles

8. **Advanced Graphing**: Exportable graphs with overlay comparison of multiple firings

## 8. Development Phases

### Phase 1: Proof of Concept (2-4 weeks)
- Basic ESP32 + MAX31855 + SSR circuit on breadboard
- Simple PID control implementation
- Display current temperature on OLED
- Manual temperature setpoint adjustment

### Phase 2: Core Features (4-6 weeks)
- PCB design and fabrication
- Rotary encoder menu system
- Profile storage and execution
- Basic web interface
- Safety features implementation

### Phase 3: Refinement (3-4 weeks)
- PID auto-tune implementation
- Enhanced web interface
- Data logging and export
- Audio feedback
- Field testing and calibration

### Phase 4: Production Ready (2-3 weeks)
- Enclosure design
- Documentation (user manual, wiring diagrams)
- Installation guide
- Final testing and certification prep

## 9. Success Metrics

- **Accuracy**: ±2°C at steady state temperature control
- **Cost Tracking Accuracy**: Within ±5% of actual metered power consumption
- **Reliability**: 100 consecutive firings without failure
- **Safety**: Zero uncontrolled over-temperature events in testing
- **Usability**: User can start a firing in <60 seconds
- **Cost**: Total BOM under $100 in single quantities
- **Community**: Open-source project with active user contributions

## 10. Bill of Materials (Estimated)

| Item | Qty | Unit Cost | Total |
|------|-----|-----------|-------|
| ESP32 Development Board | 1 | $6 | $6 |
| MAX31855 Module | 1 | $8 | $8 |
| K-Type Thermocouple | 1 | $10 | $10 |
| SSR 40A | 1 | $12 | $12 |
| 128x64 OLED Display | 1 | $5 | $5 |
| Rotary Encoder | 1 | $2 | $2 |
| Piezo Buzzer | 1 | $1 | $1 |
| Buttons, LEDs, misc | - | $5 | $5 |
| Power Supply 5V 2A | 1 | $8 | $8 |
| Enclosure | 1 | $15 | $15 |
| PCB (prototype) | 1 | $10 | $10 |
| Terminals, Hardware | - | $8 | $8 |
| **Total Estimated BOM** | | | **~$90** |

*Costs are estimates and will vary based on supplier, quantity, and market conditions.*

## 11. Documentation Requirements

- User manual with step-by-step setup guide
- Wiring diagrams for thermocouple and SSR installation
- Web interface user guide with screenshots
- Safety warnings and precautions
- Troubleshooting guide
- Developer documentation for firmware customization
- API documentation for web interface

## 12. Testing Plan

- **Unit Testing**: Each software module tested independently
- **Integration Testing**: Full system testing with simulated kiln load
- **Safety Testing**: Deliberately trigger all safety scenarios
- **Thermal Testing**: Long-duration firings at various temperatures
- **User Acceptance Testing**: Beta testing with actual ceramic artists
- **Documentation Testing**: New users follow instructions without assistance

---

## Notes

This PRD provides a comprehensive roadmap for developing a professional-grade kiln controller at an affordable price point. The modular design allows for incremental development and future expansion. Safety features are paramount and should never be compromised for cost or convenience.