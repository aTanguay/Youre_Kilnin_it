# ESP32 Kiln Controller - Task List

**Last Updated**: 2025-10-13
**Current Milestone**: M1 - Hardware Proof of Concept
**Status**: Development environment setup complete, calibration tasks added, ready for hardware integration

## Task Status Legend
- [ ] Not Started
- [→] In Progress
- [✓] Completed
- [⚠] Blocked/Issues
- [!] High Priority
- [?] Needs Discussion

---

## Milestone 1: Hardware Proof of Concept (M1)
**Goal**: Validate core hardware functionality on breadboard  
**Target**: Week 1-2  
**Status**: Not Started

### 1.1 Development Environment Setup
- [✓] Install Visual Studio Code (using PlatformIO Core CLI instead)
- [✓] Install PlatformIO extension (PlatformIO Core 6.1.18 installed)
- [✓] Install Python 3.x (PlatformIO dependency) (Python 3.12.4 already installed)
- [ ] Install ESP32 USB drivers (CP2102/CH340) - Will test when hardware is connected
- [✓] Create new PlatformIO project for ESP32
- [✓] Configure platformio.ini with initial settings (libraries commented for incremental addition)
- [ ] Test upload to ESP32 with "blink" example - Requires physical ESP32 hardware
- [✓] Set up Git repository and .gitignore
- [✓] Create initial project folder structure (src/ modules, data/, include/, lib/)
- [✓] Verify project compiles successfully (269KB firmware, 6.6% RAM, 13.7% Flash)

### 1.2 Hardware Acquisition
- [ ] Order/acquire ESP32 development board (ESP32-WROOM-32)
- [ ] Order/acquire MAX31855 thermocouple amplifier breakout
- [ ] Order/acquire K-type thermocouple (high-temp rated)
- [ ] Order/acquire 12864 LCD Display Module (128x64 dots, ST7920 controller, blue backlight)
- [ ] Order/acquire rotary encoder module (5V, 20 pulses/revolution, with push button)
- [ ] Order/acquire tactile push buttons (2-3)
- [ ] Order/acquire piezo buzzer
- [ ] Order/acquire LEDs and resistors
- [ ] Order/acquire solid state relay (40A, zero-cross)
- [ ] Order/acquire SSR heat sink
- [ ] Order/acquire breadboard and jumper wires
- [ ] Order/acquire 5V 2A power supply

### 1.3 Basic Thermocouple Reading
- [ ] Wire MAX31855 to ESP32 (SPI: CS=5, CLK=18, MISO=19)
- [ ] Install Adafruit MAX31855 library
- [ ] Write basic thermocouple reading code
- [ ] Test reading room temperature
- [ ] Implement cold junction compensation validation
- [ ] Test with heat source (lighter, candle)
- [ ] Add temperature validation function (range check)
- [ ] Add error detection (open/short circuit)
- [ ] Display raw temperature readings to serial monitor
- [ ] Document expected temperature range and accuracy

**Thermocouple Calibration Tasks:**
- [ ] Implement calibration offset storage in NVS (Preferences library)
- [ ] Create function to apply calibration offset to readings
- [ ] Implement stability detection for calibration (readings within ±0.5°C for 60 seconds)
- [ ] Add calibration data structure (offset, calibration_date, is_calibrated flag)
- [ ] Test uncalibrated vs calibrated temperature readings
- [ ] Document calibration procedure in code comments

### 1.4 SSR Control Implementation
- [ ] Wire SSR control to GPIO 25
- [ ] Add optocoupler circuit for isolation
- [ ] Write basic SSR on/off control code
- [ ] Test SSR switching with LED load
- [ ] Test SSR switching with resistive load (if available)
- [ ] Implement time-proportional SSR control (PWM simulation)
- [ ] Set default SSR cycle time (2-4 seconds)
- [ ] Verify zero-cross switching with oscilloscope (optional)
- [ ] Add SSR duty cycle calculation functions
- [ ] Test various duty cycles (25%, 50%, 75%, 100%)

### 1.5 LCD Display Integration (ST7920)
- [ ] Wire LCD to ESP32 (SPI: CS=15, MOSI=23, SCK=18 shared with MAX31855)
- [ ] Install U8g2 library (supports ST7920)
- [ ] Initialize LCD in code (ST7920 constructor)
- [ ] Display "Hello World" test
- [ ] Create temperature display layout
- [ ] Display current temperature (large font)
- [ ] Add target temperature display
- [ ] Test different font sizes and styles
- [ ] Implement screen refresh logic (250ms)
- [ ] Handle display errors gracefully

### 1.6 Rotary Encoder Input
- [ ] Wire rotary encoder (5V to ESP32 5V, GND to GND, CLK to GPIO32, DT to GPIO33, SW to GPIO34)
- [ ] Implement encoder interrupt handlers
- [ ] Test clockwise rotation detection
- [ ] Test counter-clockwise rotation detection
- [ ] Test push button detection
- [ ] Add debouncing logic for encoder
- [ ] Add debouncing logic for button
- [ ] Create simple value adjustment test (increment/decrement)
- [ ] Test encoder responsiveness at different speeds

### 1.7 Integration Testing
- [ ] Display live temperature on LCD
- [ ] Use rotary encoder to adjust target temperature setpoint
- [ ] Display both current and target temperature
- [ ] Manually control SSR based on setpoint (on/off control)
- [ ] Add status LED indicating heating/idle
- [ ] Test complete hardware loop for 1+ hour
- [ ] Document any hardware issues or limitations
- [ ] Take photos/videos of working prototype
- [ ] Update PLANNING.md with any design changes

**Calibration Verification:**
- [!] Perform ice-point thermocouple calibration (mandatory before M2)
- [ ] Verify calibration is saved correctly to NVS
- [ ] Test temperature reading accuracy after calibration
- [ ] Document calibration results (offset value, date)

---

## Milestone 2: Core Control System (M2)
**Goal**: Implement PID temperature control and safety monitoring  
**Target**: Week 3-5  
**Status**: Not Started

### 2.1 PID Controller Implementation
- [ ] Install PID library (br3ttb/PID)
- [ ] Create PID controller instance with initial parameters
- [ ] Set initial PID values (Kp=5.0, Ki=0.5, Kd=1.0)
- [ ] Configure PID sample time (1 second)
- [ ] Configure PID output limits (0-100%)
- [ ] Implement PID compute loop
- [ ] Convert PID output to SSR duty cycle
- [ ] Test PID control with low temperature setpoint (50°C)
- [ ] Test PID control with medium temperature (200°C)
- [ ] Monitor and log PID performance (overshoot, settling time)
- [ ] Create PID parameter adjustment interface
- [ ] Document PID tuning process

### 2.2 Safety System Foundation
- [ ] Define system safety limits in config.h
  - [ ] MAX_TEMP_LIMIT = 1320°C (hard coded)
  - [ ] MAX_RAMP_RATE = 600°C/hour
  - [ ] MAX_FIRING_DURATION = 48 hours
- [ ] Create safety monitoring task (FreeRTOS)
- [ ] Implement temperature limit checking
- [ ] Implement ramp rate calculation and checking
- [ ] Implement firing duration timeout
- [ ] Create emergency shutdown function
- [ ] Test emergency shutdown (simulate over-temp)
- [ ] Add safety error logging
- [ ] Display safety errors on OLED
- [ ] Ensure SSR defaults to OFF on any error

### 2.3 Thermocouple Error Detection
- [ ] Implement open circuit detection (temp > 1400°C)
- [ ] Implement short circuit detection (temp < -100°C)
- [ ] Implement NaN detection
- [ ] Implement noise filtering (excessive fluctuation check)
- [ ] Add consecutive error count (require 3 bad reads before alarm)
- [ ] Create thermocouple error state
- [ ] Test disconnected thermocouple scenario
- [ ] Test shorted thermocouple scenario
- [ ] Add thermocouple status indicator on display
- [ ] Log thermocouple errors with timestamps

### 2.4 Emergency Stop Implementation
- [ ] Wire emergency stop button to GPIO 36 (input-only pin)
- [ ] Configure GPIO with internal pull-up
- [ ] Implement NC (normally closed) switch logic
- [ ] Create emergency stop interrupt handler
- [ ] Test emergency stop activation
- [ ] Ensure immediate SSR shutdown on activation
- [ ] Display emergency stop status
- [ ] Require manual reset after emergency stop
- [ ] Log emergency stop events
- [ ] Test emergency stop during active heating

### 2.5 Watchdog Timer
- [ ] Enable ESP32 hardware watchdog timer
- [ ] Set watchdog timeout to 10 seconds
- [ ] Add watchdog reset calls in main loop
- [ ] Test watchdog trigger on intentional hang
- [ ] Verify system resets and SSR stays off
- [ ] Add watchdog reset to all FreeRTOS tasks
- [ ] Log watchdog resets for debugging
- [ ] Test watchdog with various system loads

### 2.6 Menu System Development
- [ ] Design menu structure (state machine)
- [ ] Create menu item data structure
- [ ] Implement menu navigation with rotary encoder
- [ ] Create main menu screen
  - [ ] View Current Status
  - [ ] Set Temperature
  - [ ] Start/Stop Heating
  - [ ] Settings
  - [ ] System Info
  - [ ] Calibrate Thermocouple (new)
- [ ] Implement menu scrolling
- [ ] Implement menu selection (push button)
- [ ] Implement back button functionality (secondary button)
- [ ] Add menu timeout (return to main after 30s inactivity)
- [ ] Test full menu navigation

**Calibration UI Tasks:**
- [ ] Create "Calibrate Thermocouple" menu screen
- [ ] Display ice bath preparation instructions (step-by-step)
- [ ] Show current raw temperature reading during calibration
- [ ] Implement stability indicator (visual feedback when stable)
- [ ] Display calculated offset after calibration
- [ ] Add confirmation prompt to save calibration
- [ ] Show last calibration date on status screen
- [ ] Add "Calibration Required" warning if never calibrated
- [ ] Implement calibration reset option in settings
- [ ] Test complete calibration workflow on LCD

### 2.7 Audio Feedback System
- [ ] Wire piezo buzzer to GPIO 26
- [ ] Implement tone generation function (PWM)
- [ ] Create different tones for different events
  - [ ] Button press: Short beep (50ms)
  - [ ] Menu navigation: Click (20ms)
  - [ ] Heating start: Ascending tone
  - [ ] Target reached: Double beep
  - [ ] Error: Rapid beeping alarm
- [ ] Add volume control (PWM duty cycle)
- [ ] Add audio enable/disable setting
- [ ] Test all audio feedback scenarios
- [ ] Ensure audio doesn't block critical operations

### 2.8 Manual Temperature Control
- [ ] Create "Manual Mode" menu option
- [ ] Allow setpoint adjustment via rotary encoder (1°C increments)
- [ ] Display current vs target temperature
- [ ] Implement start/stop heating controls
- [ ] Add PID status display (output percentage)
- [ ] Show time at temperature
- [ ] Add manual override safety checks
- [ ] Test holding various temperatures (100°C, 200°C, 500°C)
- [ ] Log manual firing sessions

### 2.9 Status LED Implementation
- [ ] Wire status LEDs (Power=27, WiFi=14, Error=12)
- [ ] Implement LED control functions
- [ ] Power LED: Solid on when powered
- [ ] WiFi LED: Blink during connection, solid when connected
- [ ] Error LED: Blink on warnings, solid on critical errors
- [ ] Test LED patterns
- [ ] Add LED brightness control
- [ ] Document LED meaning in user manual

---

## Milestone 3: Profile System (M3)
**Goal**: Multi-segment firing profile execution  
**Target**: Week 6-7  
**Status**: Not Started

### 3.1 Data Structure Design
- [ ] Define ProfileSegment struct (target temp, ramp rate, soak time)
- [ ] Define FiringProfile struct (name, description, segments array)
- [ ] Define SystemState enum (IDLE, FIRING, SOAKING, etc.)
- [ ] Define KilnState struct (current state, temps, segment info)
- [ ] Create global state variables
- [ ] Add mutex protection for shared state
- [ ] Document data structures in code comments

### 3.2 File System Setup
- [ ] Configure SPIFFS or LittleFS in platformio.ini
- [ ] Create data/ folder for file system
- [ ] Initialize file system in code
- [ ] Test file system mount/unmount
- [ ] Implement file system error handling
- [ ] Create profiles/ directory in file system
- [ ] Test file creation, reading, writing
- [ ] Implement file system space checking

### 3.3 Profile Storage Format
- [ ] Define JSON schema for firing profiles
- [ ] Install ArduinoJson library
- [ ] Create example profile JSON files
  - [ ] Bisque firing (Cone 04)
  - [ ] Glaze firing (Cone 6)
  - [ ] Low-fire glaze (Cone 06)
- [ ] Implement profile serialization (struct to JSON)
- [ ] Implement profile deserialization (JSON to struct)
- [ ] Test save profile to file
- [ ] Test load profile from file
- [ ] Handle corrupted profile files gracefully

### 3.4 Profile Engine (State Machine)
- [ ] Create profile execution state machine
- [ ] Implement IDLE state
- [ ] Implement PREHEATING state (initial ramp)
- [ ] Implement RAMPING state (controlled ramp to target)
- [ ] Implement SOAKING state (hold at temperature)
- [ ] Implement COOLING state (passive cool-down)
- [ ] Implement COMPLETE state
- [ ] Implement ERROR state
- [ ] Add state transition logic
- [ ] Test state machine with simple 2-segment profile

### 3.5 Ramp Rate Control
- [ ] Calculate required temperature change per second
- [ ] Implement ramp rate limiting in PID controller
- [ ] Test slow ramp (50°C/hour)
- [ ] Test medium ramp (150°C/hour)
- [ ] Test fast ramp (300°C/hour)
- [ ] Add "full power" option (ramp rate = 0)
- [ ] Display current ramp rate on OLED
- [ ] Adjust PID aggressiveness based on ramp rate

### 3.6 Soak Time Management
- [ ] Implement soak timer
- [ ] Define temperature tolerance for "at target" (±5°C)
- [ ] Start soak timer when target reached
- [ ] Display soak time remaining
- [ ] Handle temperature drops during soak (restart timer)
- [ ] Test various soak durations (10min, 30min, 1hour)
- [ ] Add audio notification when soak complete

### 3.7 Profile Management UI
- [ ] Create "Profiles" menu
- [ ] List available profiles (from file system)
- [ ] Implement profile selection
- [ ] Display profile details (segments, total time estimate)
- [ ] Add "Delete Profile" option
- [ ] Add "Rename Profile" option
- [ ] Create profile pagination (if >10 profiles)
- [ ] Add confirmation dialogs for destructive actions

### 3.8 Profile Execution UI
- [ ] Create "Start Firing" screen
- [ ] Display selected profile name
- [ ] Show estimated total time
- [ ] Show estimated cost (if configured)
- [ ] Add confirmation prompt
- [ ] Implement firing progress display
  - [ ] Current segment number (e.g., "Segment 2 of 5")
  - [ ] Current temperature vs target
  - [ ] Time in current segment
  - [ ] Overall progress bar
  - [ ] Estimated time remaining
- [ ] Add "Abort Firing" option
- [ ] Implement safe abort (gradual cool-down)

### 3.9 Integration Testing
- [ ] Test complete bisque firing profile
- [ ] Test complete glaze firing profile
- [ ] Test profile with 8 segments (maximum)
- [ ] Test profile abort mid-firing
- [ ] Test power cycle during firing (manual recovery)
- [ ] Verify all state transitions work correctly
- [ ] Log complete firing from start to finish
- [ ] Validate timing accuracy (±5 minutes for multi-hour firings)

---

## Milestone 4: Web Interface (M4)
**Goal**: Remote monitoring and control via web browser  
**Target**: Week 8-10  
**Status**: Not Started

### 4.1 WiFi Configuration
- [ ] Implement WiFi connection manager
- [ ] Create Access Point (AP) mode
- [ ] Set AP SSID (e.g., "KilnController-XXXX")
- [ ] Set AP password
- [ ] Create captive portal for initial setup
- [ ] Implement WiFi client mode (connect to existing network)
- [ ] Store WiFi credentials in Preferences (NVS)
- [ ] Add WiFi reconnection logic
- [ ] Display WiFi status on OLED
- [ ] Update WiFi status LED

### 4.2 Web Server Setup
- [ ] Install AsyncTCP library
- [ ] Install ESPAsyncWebServer library
- [ ] Create web server instance (port 80)
- [ ] Configure CORS headers
- [ ] Implement basic index.html route
- [ ] Test web server access from browser
- [ ] Add server error handling
- [ ] Implement request logging

### 4.3 Static File Serving
- [ ] Create data/www/ folder structure
- [ ] Create index.html (landing page)
- [ ] Create style.css (global styles)
- [ ] Create main.js (core JavaScript)
- [ ] Upload files to SPIFFS/LittleFS
- [ ] Configure web server to serve static files
- [ ] Test file serving in browser
- [ ] Implement GZIP compression for HTML/CSS/JS

### 4.4 REST API - Status Endpoints
- [ ] Create /api/status endpoint (GET)
  - [ ] Return current temperature
  - [ ] Return target temperature
  - [ ] Return system state
  - [ ] Return PID output
  - [ ] Return firing progress
  - [ ] Return calibration status (is_calibrated, last_calibration_date)
- [ ] Create /api/config endpoint (GET)
  - [ ] Return system configuration
- [ ] Create /api/calibration endpoint (GET)
  - [ ] Return calibration data (offset, date, status)
- [ ] Test API endpoints with browser/Postman
- [ ] Add JSON response formatting
- [ ] Implement error responses (404, 500)

### 4.5 REST API - Profile Management
- [ ] Create /api/profiles endpoint (GET) - list all profiles
- [ ] Create /api/profiles/{id} endpoint (GET) - get single profile
- [ ] Create /api/profiles endpoint (POST) - create new profile
- [ ] Create /api/profiles/{id} endpoint (PUT) - update profile
- [ ] Create /api/profiles/{id} endpoint (DELETE) - delete profile
- [ ] Implement JSON profile validation
- [ ] Test CRUD operations via API
- [ ] Add authentication checks (later milestone)

### 4.6 REST API - Firing Control
- [ ] Create /api/firing/start endpoint (POST)
  - [ ] Accept profile ID
  - [ ] Validate profile exists
  - [ ] Start firing
  - [ ] Return success/error
- [ ] Create /api/firing/stop endpoint (POST)
  - [ ] Stop current firing
  - [ ] Initiate safe shutdown
- [ ] Create /api/firing/status endpoint (GET)
  - [ ] Return detailed firing status
  - [ ] Include segment information
  - [ ] Include time estimates
- [ ] Test firing start/stop via API

### 4.7 WebSocket Implementation
- [ ] Add WebSocket handler to server
- [ ] Create /ws endpoint
- [ ] Implement WebSocket connection management
- [ ] Create message format (JSON)
- [ ] Send temperature updates every 1 second
- [ ] Send system state updates
- [ ] Send firing progress updates
- [ ] Test WebSocket with browser console
- [ ] Handle client disconnections gracefully
- [ ] Limit number of concurrent WebSocket connections (max 5)

### 4.8 Dashboard Page (HTML/CSS/JS)
- [ ] Create dashboard.html structure
- [ ] Design responsive layout (mobile-first)
- [ ] Add current temperature display (large, prominent)
- [ ] Add target temperature display
- [ ] Add system status indicator
- [ ] Add calibration status badge (calibrated/uncalibrated)
- [ ] Create real-time temperature graph (Chart.js)
- [ ] Add firing progress section
- [ ] Display current segment information
- [ ] Add PID output gauge/meter
- [ ] Style with CSS (modern, clean design)
- [ ] Test on desktop browser
- [ ] Test on mobile browser (iOS/Android)

### 4.9 Profiles Management Page
- [ ] Create profiles.html structure
- [ ] Display list of available profiles
- [ ] Add "Create New Profile" button
- [ ] Create profile editor form
  - [ ] Profile name input
  - [ ] Profile description input
  - [ ] Segment editor (add/remove/reorder)
  - [ ] Target temperature input
  - [ ] Ramp rate input
  - [ ] Soak time input
- [ ] Implement form validation
- [ ] Add save/cancel buttons
- [ ] Implement profile deletion with confirmation
- [ ] Add profile import (JSON file upload)
- [ ] Add profile export (download JSON)
- [ ] Test profile CRUD operations in browser

### 4.10 Start Firing Page
- [ ] Create start-firing.html structure
- [ ] Add profile selection dropdown
- [ ] Display selected profile details
- [ ] Show estimated firing time
- [ ] Show estimated cost
- [ ] Add safety checklist (user confirmation)
  - [ ] Kiln loaded properly
  - [ ] Ventilation adequate
  - [ ] Area clear of flammables
- [ ] Add "Start Firing" button
- [ ] Implement confirmation dialog
- [ ] Show firing in progress status
- [ ] Redirect to dashboard after start

### 4.11 History/Logs Page
- [ ] Create history.html structure
- [ ] Display list of past firings
- [ ] Show firing details (date, profile, duration, peak temp)
- [ ] Add "View Details" for each firing
- [ ] Create detailed firing view
  - [ ] Temperature graph over time
  - [ ] Segment transitions marked
  - [ ] Duration and statistics
  - [ ] Calibration status at time of firing
- [ ] Add CSV export button
- [ ] Implement date range filtering
- [ ] Add search functionality
- [ ] Test with multiple logged firings

### 4.12 Authentication System
- [ ] Implement basic authentication
- [ ] Create login page
- [ ] Store hashed password in Preferences
- [ ] Create session management (cookies/tokens)
- [ ] Protect admin endpoints
- [ ] Allow read-only access without auth (optional)
- [ ] Add logout functionality
- [ ] Test authentication flow
- [ ] Add "Change Password" feature

---

## Milestone 5: Energy & Cost Tracking (M5)
**Goal**: Comprehensive energy consumption and cost monitoring  
**Target**: Week 11-12  
**Status**: Not Started

### 5.1 Power Calculation System
- [ ] Create power monitoring task (FreeRTOS)
- [ ] Track SSR on-time per cycle
- [ ] Calculate instantaneous power (W = kiln_wattage × duty_cycle)
- [ ] Implement energy integration (kWh accumulation)
- [ ] Store energy data in global state
- [ ] Add mutex protection for energy data
- [ ] Test power calculation accuracy vs Kill-A-Watt meter
- [ ] Add calibration factor if needed

### 5.2 Cost Configuration
- [ ] Define cost configuration data structure
  - [ ] kiln_wattage (default 1800W)
  - [ ] electricity_rate ($/kWh)
  - [ ] currency_symbol
  - [ ] billing_cycle_day (1-31)
- [ ] Create cost configuration in Preferences
- [ ] Implement default values
- [ ] Create configuration UI (web interface)
- [ ] Add input validation (reasonable ranges)
- [ ] Test saving and loading configuration

### 5.3 Cost Tracking Implementation
- [ ] Create EnergyData structure
  - [ ] currentFiringKWh
  - [ ] currentFiringCost
  - [ ] billingCycleCost
  - [ ] annualCost
  - [ ] lifetimeCost
- [ ] Implement cost calculation (cost = kWh × rate)
- [ ] Reset current firing cost when firing starts
- [ ] Accumulate billing cycle cost
- [ ] Accumulate annual cost
- [ ] Accumulate lifetime cost
- [ ] Store accumulated costs in Preferences (persist across reboots)

### 5.4 Billing Cycle Management
- [ ] Store billing cycle start date in Preferences
- [ ] Implement automatic monthly reset check
- [ ] Reset billing cycle cost on cycle boundary
- [ ] Add manual reset option (web interface)
- [ ] Calculate days remaining in cycle
- [ ] Implement "next billing date" calculation
- [ ] Test cycle reset at month boundaries
- [ ] Handle edge cases (Feb 29, month-end variations)

### 5.5 Annual Cost Tracking
- [ ] Store annual start date (Jan 1)
- [ ] Implement automatic annual reset check
- [ ] Reset annual cost on January 1
- [ ] Add manual reset option
- [ ] Calculate year-to-date accumulation
- [ ] Project annual cost based on current usage
- [ ] Test annual rollover

### 5.6 OLED Cost Display
- [ ] Add cost display to firing screen
- [ ] Show current firing cost (live updating)
- [ ] Format cost with currency symbol
- [ ] Create "Cost Summary" menu screen
  - [ ] Billing cycle cost + days remaining
  - [ ] Annual cost
  - [ ] Lifetime cost
  - [ ] Last reset dates
- [ ] Add cost display to menu system
- [ ] Test cost display updates during firing

### 5.7 Web Interface - Cost Configuration
- [ ] Create config.html page (or section)
- [ ] Add kiln wattage input field
- [ ] Add electricity rate input field
- [ ] Add currency symbol selector
- [ ] Add billing cycle day selector (1-31)
- [ ] Implement form validation
- [ ] Add save button
- [ ] Display current configuration
- [ ] Test configuration updates

### 5.8 Web Interface - Cost Dashboard
- [ ] Create costs.html page
- [ ] Display current firing cost (live via WebSocket)
- [ ] Display billing cycle cost
  - [ ] Show cycle start date
  - [ ] Show days remaining
  - [ ] Show cost so far
- [ ] Display annual cost
  - [ ] Show YTD total
  - [ ] Show projected annual
- [ ] Display lifetime cost
- [ ] Create monthly cost history chart (bar chart)
- [ ] Add manual reset buttons (with confirmation)
- [ ] Style for clarity and usability

### 5.9 Cost Data in Firing Logs
- [ ] Add cost fields to firing log structure
- [ ] Store kWh consumed per firing
- [ ] Store cost per firing
- [ ] Update history page to show costs
- [ ] Add cost column to CSV export
- [ ] Calculate average cost per firing
- [ ] Show cost trends over time

### 5.10 Cost Projection & Estimation
- [ ] Implement firing cost estimation (before starting)
- [ ] Base estimate on profile segments and historical data
- [ ] Display estimated cost on "Start Firing" page
- [ ] Compare estimated vs actual cost after firing
- [ ] Improve estimation algorithm over time
- [ ] Add cost budget warning feature (optional)

---

## Milestone 6: PID Auto-Tune (M6)
**Goal**: Automated PID parameter optimization  
**Target**: Week 13-14  
**Status**: Not Started

### 6.1 Auto-Tune Algorithm Research
- [!] **PREREQUISITE: Thermocouple MUST be calibrated before PID auto-tune**
- [ ] Verify thermocouple calibration is complete
- [ ] Research relay auto-tune method
- [ ] Research Ziegler-Nichols tuning rules
- [ ] Document auto-tune algorithm approach
- [ ] Define auto-tune test parameters
- [ ] Create auto-tune state machine design

### 6.2 Relay Auto-Tune Implementation
- [ ] Create auto-tune state machine
- [ ] Implement oscillation detection
- [ ] Measure oscillation period (Pu)
- [ ] Measure oscillation amplitude
- [ ] Calculate ultimate gain (Ku)
- [ ] Apply Ziegler-Nichols PID rules
- [ ] Store calculated PID parameters
- [ ] Test auto-tune at various temperatures (200°C, 500°C, 1000°C)

### 6.3 Auto-Tune Safety
- [ ] Add auto-tune maximum temperature limit
- [ ] Implement auto-tune timeout (30 minutes)
- [ ] Add oscillation detection safety checks
- [ ] Monitor for runaway conditions
- [ ] Add abort option during auto-tune
- [ ] Test safety limits during auto-tune

### 6.4 Auto-Tune UI (OLED)
- [ ] Create "PID Auto-Tune" menu option
- [ ] Display auto-tune instructions
- [ ] Show auto-tune progress
  - [ ] Current phase
  - [ ] Oscillation count
  - [ ] Time elapsed
- [ ] Display results (calculated Kp, Ki, Kd)
- [ ] Prompt user to accept or reject new parameters
- [ ] Show before/after comparison option

### 6.5 Auto-Tune UI (Web)
- [ ] Create auto-tune page or section
- [ ] Add "Start Auto-Tune" button
- [ ] Display auto-tune status (via WebSocket)
- [ ] Show real-time temperature graph during auto-tune
- [ ] Display calculated PID parameters
- [ ] Add accept/reject buttons
- [ ] Show PID parameter history
- [ ] Allow manual PID parameter entry

### 6.6 PID Parameter Storage
- [ ] Store PID parameters in Preferences
- [ ] Create PID parameter history (last 5 sets)
- [ ] Add parameter set naming (e.g., "Auto-Tune 2025-10-11")
- [ ] Implement parameter rollback feature
- [ ] Add factory default restore option

### 6.7 Temperature-Dependent PID (Advanced)
- [ ] Research temperature-dependent tuning
- [ ] Define temperature ranges (low/mid/high)
- [ ] Store multiple PID parameter sets
- [ ] Implement PID parameter switching based on temperature
- [ ] Test performance improvement
- [ ] Add UI to configure temperature-dependent tuning

### 6.8 Auto-Tune Documentation
- [ ] Create user guide for auto-tune process
- [ ] Document best practices (when to auto-tune)
- [ ] Add troubleshooting section
- [ ] Create video tutorial (optional)

---

## Milestone 7: PCB Design (M7)
**Goal**: Production-ready custom PCB  
**Target**: Week 15-16  
**Status**: Not Started

### 7.1 Schematic Design (KiCad)
- [ ] Install KiCad
- [ ] Create new KiCad project
- [ ] Add ESP32 module to schematic
- [ ] Add MAX31855 circuit
- [ ] Add SSR control circuit with optocoupler
- [ ] Add OLED connector
- [ ] Add rotary encoder connector
- [ ] Add button inputs
- [ ] Add piezo buzzer circuit
- [ ] Add status LEDs
- [ ] Add power supply circuit (120VAC to 5VDC)
- [ ] Add screw terminals for AC connections
- [ ] Add screw terminals for thermocouple
- [ ] Add emergency stop input
- [ ] Run electrical rules check (ERC)
- [ ] Peer review schematic

### 7.2 PCB Layout
- [ ] Set board dimensions
- [ ] Define AC and DC zones (clear separation)
- [ ] Place components for optimal layout
- [ ] Route power traces (adequate thickness)
- [ ] Route signal traces
- [ ] Add ground planes (separate AC/DC grounds)
- [ ] Add mounting holes
- [ ] Add silkscreen labels
- [ ] Set trace width/clearance for AC (safety)
- [ ] Add test points
- [ ] Run design rules check (DRC)
- [ ] Generate 3D view for verification

### 7.3 Safety Review
- [ ] Verify AC/DC isolation (minimum 5mm clearance)
- [ ] Verify creepage and clearance distances
- [ ] Check trace widths for current carrying
- [ ] Verify fuse placement
- [ ] Review optocoupler isolation
- [ ] Check grounding scheme
- [ ] Document safety features in schematic
- [ ] Third-party safety review (if possible)

### 7.4 Gerber Generation & Order
- [ ] Generate Gerber files
- [ ] Generate drill files
- [ ] Generate BOM (Bill of Materials)
- [ ] Generate assembly files (if using PCB assembly service)
- [ ] Select PCB manufacturer (JLCPCB, PCBWay, OSH Park)
- [ ] Upload Gerbers for quote
- [ ] Review PCB preview
- [ ] Order 5-10 prototype PCBs
- [ ] Order components from BOM

### 7.5 PCB Assembly
- [ ] Receive PCBs and components
- [ ] Inspect PCBs for manufacturing defects
- [ ] Assemble one PCB (hand soldering)
- [ ] Visual inspection of solder joints
- [ ] Continuity test (multimeter)
- [ ] Test AC/DC isolation (megger test)
- [ ] Power on test (measure voltages)
- [ ] Program ESP32
- [ ] Functional test (all peripherals)

### 7.6 PCB Testing & Validation
- [ ] Test thermocouple reading
- [ ] Test SSR control
- [ ] Test display
- [ ] Test rotary encoder
- [ ] Test all buttons
- [ ] Test buzzer
- [ ] Test LEDs
- [ ] Test emergency stop
- [ ] Test WiFi connectivity
- [ ] Run complete firing profile on PCB
- [ ] Test for 24+ hours continuous operation
- [ ] Document any issues found

### 7.7 PCB Revision (if needed)
- [ ] Document PCB issues
- [ ] Update schematic with fixes
- [ ] Update PCB layout
- [ ] Order revised PCBs
- [ ] Re-test

---

## Milestone 8: Field Testing (M8)
**Goal**: Real-world validation with beta users  
**Target**: Week 17-20  
**Status**: Not Started

### 8.1 Beta Tester Recruitment
- [ ] Identify 3-5 beta testers (ceramic artists)
- [ ] Create beta tester agreement/waiver
- [ ] Provide safety guidelines document
- [ ] Set expectations for feedback
- [ ] Create feedback survey/form

### 8.2 Beta Unit Preparation
- [ ] Assemble 3-5 complete units
- [ ] Flash latest firmware
- [ ] Pre-load common firing profiles
- [ ] Full functional test each unit
- [ ] Create quick start guide
- [ ] Package units safely for shipping
- [ ] Include emergency stop instructions

### 8.3 Beta Testing - Week 1
- [ ] Ship units to beta testers
- [ ] Provide setup support (video call if needed)
- [ ] Monitor remote logs (if telemetry enabled)
- [ ] Collect initial feedback
- [ ] Document any setup issues

### 8.4 Beta Testing - Week 2-3
- [ ] Check in with beta testers weekly
- [ ] Review firing logs
- [ ] Collect usability feedback
- [ ] Identify any bugs or issues
- [ ] Prioritize bug fixes
- [ ] Release firmware updates if needed

### 8.5 Long-Duration Testing
- [ ] Test 24+ hour firings
- [ ] Test system over 100 firing cycles
- [ ] Monitor for memory leaks
- [ ] Monitor for WiFi stability issues
- [ ] Test in various environmental conditions (temperature, humidity)
- [ ] Verify SSR and components don't overheat

### 8.6 Safety Validation
- [ ] Verify all safety features trigger correctly in field
- [ ] Test emergency stop effectiveness
- [ ] Verify thermocouple error detection
- [ ] Verify over-temperature shutdowns
- [ ] Document any safety concerns
- [ ] No safety incidents during beta testing

### 8.7 Accuracy Validation
- [ ] Compare controller temperature to witness cones
- [ ] Validate cost tracking vs actual power bill
- [ ] Check firing duration accuracy
- [ ] Verify PID control stability
- [ ] Measure temperature uniformity in kiln

### 8.8 User Experience Testing
- [ ] Time new user setup
- [ ] Time experienced user to start firing
- [ ] Collect feedback on OLED menu usability
- [ ] Collect feedback on web interface
- [ ] Identify confusing features
- [ ] Document feature requests

### 8.9 Bug Fixing & Refinement
- [ ] Create bug tracking list
- [ ] Prioritize bugs (critical/high/medium/low)
- [ ] Fix critical bugs immediately
- [ ] Fix high priority bugs before release
- [ ] Medium/low bugs go to backlog
- [ ] Release updated firmware to beta testers
- [ ] Re-test after fixes

### 8.10 Beta Testing Report
- [ ] Compile all beta testing feedback
- [ ] Document all issues found and resolved
- [ ] Create lessons learned document
- [ ] Update firmware based on feedback
- [ ] Update documentation based on feedback
- [ ] Decide if ready for public release

---

## Milestone 9: Documentation & Release (M9)
**Goal**: Public open-source release with complete documentation  
**Target**: Week 21-22  
**Status**: Not Started

### 9.1 User Manual
- [ ] Write introduction and overview
- [ ] Create safety warnings section
- [ ] Write hardware assembly guide
- [ ] Write wiring instructions with diagrams
- [ ] Create first-time setup guide
- [ ] Write basic operation instructions
- [ ] Document all menu options
- [ ] Create firing profile guide
- [ ] Write troubleshooting section
- [ ] Add FAQ section
- [ ] Include specifications table
- [ ] Proofread and edit

### 9.2 Wiring Diagrams
- [ ] Create breadboard wiring diagram (Fritzing)
- [ ] Create PCB wiring diagram
- [ ] Diagram AC power connections
- [ ] Diagram thermocouple installation
- [ ] Diagram SSR wiring and heat sink
- [ ] Color-code wires for safety
- [ ] Add clear labels and annotations
- [ ] Export high-resolution images

### 9.3 Video Tutorials
- [ ] Script video content
- [ ] Record assembly video
- [ ] Record wiring video
- [ ] Record first-time setup video
- [ ] Record basic operation video
- [ ] Record profile creation video
- [ ] Record auto-tune tutorial
- [ ] Edit videos
- [ ] Upload to YouTube
- [ ] Add to documentation

### 9.4 Code Documentation
- [ ] Review all code comments
- [ ] Add Doxygen-style function headers
- [ ] Create README.md for code repository
- [ ] Document code architecture
- [ ] Create developer setup guide
- [ ] Document API endpoints
- [ ] Create contribution guidelines
- [ ] Add license file (MIT or Apache 2.0)

### 9.5 GitHub Repository Setup
- [ ] Create GitHub organization or personal repo
- [ ] Initialize repository
- [ ] Upload firmware source code
- [ ] Upload KiCad PCB files
- [ ] Upload 3D printable enclosure files (if available)
- [ ] Upload web interface files
- [ ] Create comprehensive README.md
  - [ ] Project description
  - [ ] Features list
  - [ ] Hardware requirements
  - [ ] Installation guide
  - [ ] Usage instructions
  - [ ] Contributing guidelines
  - [ ] License information
  - [ ] Safety warnings
- [ ] Add screenshots and photos
- [ ] Create issue templates
- [ ] Create pull request template

### 9.6 Bill of Materials (BOM)
- [ ] Create detailed BOM spreadsheet
- [ ] Include part numbers
- [ ] Include supplier links (multiple sources)
- [ ] Include approximate costs
- [ ] Include alternative part options
- [ ] Calculate total BOM cost
- [ ] Add to documentation

### 9.7 Project Website (Optional)
- [ ] Register domain name
- [ ] Create simple project website
- [ ] Add project overview
- [ ] Link to documentation
- [ ] Link to GitHub repository
- [ ] Add photo gallery
- [ ] Add community forum link
- [ ] Add contact information

### 9.8 Community Launch
- [ ] Write launch announcement
- [ ] Post to r/esp32
- [ ] Post to r/pottery and r/Ceramics
- [ ] Post to Arduino forums
- [ ] Post to Hackaday
- [ ] Post to social media (if applicable)
- [ ] Submit to Hackaday projects
- [ ] Submit to Hackster.io

### 9.9 Initial Support
- [ ] Monitor GitHub issues
- [ ] Respond to questions promptly
- [ ] Help users with setup problems
- [ ] Collect feature requests
- [ ] Fix critical bugs immediately
- [ ] Plan first maintenance release (v1.1)

### 9.10 Success Metrics
- [ ] Track GitHub stars
- [ ] Track number of clones/downloads
- [ ] Count community contributions
- [ ] Collect user testimonials
- [ ] Document successful builds
- [ ] Celebrate milestones!

---

## Future Enhancements (Post-Release)

### Features for v1.1
- [ ] Cool-down monitoring
- [ ] Power failure recovery
- [ ] Pre-heat soak option
- [ ] Cone firing mode
- [ ] Push notifications (Pushover/Telegram)
- [ ] Time-of-use rate optimization

### Features for v2.0
- [ ] 220VAC support
- [ ] Multi-thermocouple support
- [ ] Multi-zone control
- [ ] MicroSD card logging
- [ ] Bluetooth LE support
- [ ] Mobile app (iOS/Android)

### Features for v3.0
- [ ] MQTT/Home Assistant integration
- [ ] Cloud logging
- [ ] Machine learning PID optimization
- [ ] Gas kiln / atmosphere control
- [ ] Community profile repository

---

## Notes

- **Mark tasks complete immediately** using [✓]
- **Add newly discovered tasks** to appropriate milestone
- **Use [!]** to mark high-priority tasks
- **Use [⚠]** to flag blocked tasks with explanation
- **Update "Last Updated" date** when making changes
- **Move completed milestones** to an archive section

**Last Updated**: 2025-10-11