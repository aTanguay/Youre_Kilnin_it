# Kilnin' It - ESP32 Kiln Controller

**Affordable, open-source PID kiln controller for small ceramic kilns**

Transform your basic 120VAC kiln into a smart, precision-controlled firing system for ~$90 in parts—versus $300-800 for commercial alternatives.

---

## 🎯 Project Overview

**Kilnin' It** is a safety-first, ESP32-based kiln controller designed for ceramic artists, hobbyists, and small studios. Get professional-grade temperature control with web monitoring, energy cost tracking, and programmable firing profiles—all at a fraction of commercial controller costs.

### Key Features

- ✅ **Precise PID Temperature Control** - ±2°C accuracy with auto-tuning
- ✅ **Multi-Segment Firing Profiles** - Bisque, glaze, raku, and custom schedules
- ✅ **Web-Based Monitoring** - Real-time temperature graphs and control from any device
- ✅ **Energy Cost Tracking** - Monitor daily, billing cycle, and lifetime costs
- ✅ **Comprehensive Safety Systems** - Watchdog timers, thermal limits, emergency stop
- ✅ **Thermocouple Calibration** - Ice-point calibration for accuracy
- ✅ **Dual Rotary Encoder UI** - Intuitive simultaneous time/temp control
- ✅ **Open Source Hardware & Software** - Build, modify, and improve

### Target Audience

- Home ceramic artists and potters
- Small pottery studios
- Educational institutions (schools, makerspaces)
- Hobbyists looking to upgrade basic kilns
- Anyone wanting programmable kiln control without the premium price

---

## 📊 Specifications

| Feature | Specification |
|---------|---------------|
| **Microcontroller** | ESP32-WROOM-32 (dual-core, 240MHz) |
| **Temperature Range** | 0-1320°C (32-2408°F) |
| **Temperature Accuracy** | ±2°C (with calibration) |
| **Thermocouple Type** | K-type (included in MAX31855 module) |
| **Control Method** | PID with auto-tuning |
| **SSR Control** | Time-proportional (2s cycle) |
| **Kiln Compatibility** | 120VAC, up to 1800W (15A circuit) |
| **Display** | 128x64 ST7920 LCD (blue backlight) |
| **Input** | Dual rotary encoders + emergency stop |
| **Connectivity** | WiFi (2.4GHz), web interface |
| **Power Supply** | 5V/2A USB or DC adapter |
| **Cost** | ~$90 BOM (vs $300-800 commercial) |

---

## 🛠️ Bill of Materials (BOM)

### Core Components (~$90 total)

| Component | Quantity | Est. Cost | Notes |
|-----------|----------|-----------|-------|
| ESP32-WROOM-32 Dev Board | 1 | $6-8 | 38-pin version |
| MAX31855 Thermocouple Module | 1 | $8-12 | Includes K-type thermocouple |
| Solid State Relay (SSR) | 1 | $8-15 | 25A, 3-32VDC input, 120VAC output |
| ST7920 128x64 LCD Display | 1 | $8-12 | SPI interface, blue backlight |
| Rotary Encoder (5V, 20 pulses) | 2 | $4-6 | With push-button switches |
| Piezo Buzzer (active) | 1 | $1-2 | 5V, audio feedback |
| Status LEDs (RGB or single) | 3 | $1-2 | Power, WiFi, Error indicators |
| 5V Power Supply (2A) | 1 | $5-8 | USB or DC barrel jack |
| Project Enclosure | 1 | $10-15 | Heat-resistant plastic |
| Resistors, wire, connectors | - | $5-10 | Misc electronics |
| **Optional Components** | | | |
| Heat sink for SSR | 1 | $3-5 | Recommended for continuous use |
| Cooling fan (5V) | 1 | $3-5 | For enclosure ventilation |

**Where to Buy**: Amazon, AliExpress, Adafruit, SparkFun, Digi-Key, Mouser

**Safety Note**: SSR must be rated for your kiln's wattage + 25% headroom. For 1500W kiln, use 25A SSR minimum.

---

## 🔌 Hardware Setup

### Wiring Overview

**ESP32 GPIO Connections**:

| Component | GPIO Pins | Interface |
|-----------|-----------|-----------|
| MAX31855 (Thermocouple) | CS: 5, CLK: 18, MISO: 19 | SPI |
| ST7920 LCD | CS: 15, MOSI: 23, SCK: 18 | SPI (shared) |
| Left Rotary Encoder | CLK: 32, DT: 33, SW: 34 | Digital input |
| Right Rotary Encoder | CLK: 35, DT: 39, SW: 36 | Digital input |
| SSR Control | GPIO 25 | Digital output (PWM) |
| Piezo Buzzer | GPIO 26 | Digital output |
| Power LED | GPIO 27 | Digital output |
| WiFi LED | GPIO 14 | Digital output |
| Error LED | GPIO 12 | Digital output |

**Emergency Stop**: Press both encoder buttons simultaneously for 0.5 seconds (prevents accidental activation).

### Assembly Steps

1. **Mount Components in Enclosure**
   - Position LCD on front panel for visibility
   - Mount rotary encoders on front panel (left = navigation/time, right = values/temp)
   - Mount status LEDs on front panel
   - Install ESP32 on standoffs inside enclosure
   - Mount SSR with heat sink (keep separate from ESP32)

2. **Wire Low-Voltage Connections** (3.3V/5V DC)
   - Connect ESP32 to MAX31855 module (SPI)
   - Connect ESP32 to LCD display (SPI, shared bus)
   - Connect rotary encoders to ESP32 (digital inputs)
   - Connect buzzer and LEDs to ESP32 (digital outputs)
   - Connect 5V power supply to ESP32 VIN pin

3. **Wire High-Voltage Connections** (120VAC - **DANGEROUS**)
   - ⚠️ **STOP**: Only proceed if you are qualified to work with AC mains voltage
   - Install SSR between kiln and wall outlet (switched hot wire)
   - Connect SSR control input (low-voltage side) to ESP32 GPIO 25
   - Ensure proper electrical isolation between AC and DC sections
   - Use strain relief, proper wire gauge (14 AWG minimum for 15A)
   - Install in grounded metal enclosure if required by local code

4. **Thermocouple Installation**
   - Route K-type thermocouple into kiln chamber
   - Position tip in center of kiln, away from elements
   - Secure with high-temp ceramic insulators
   - Do not let thermocouple touch metal elements or walls

### Safety Checklist Before Power-On

- [ ] All AC wiring in strain relief, properly insulated
- [ ] SSR mounted with heat sink, adequate ventilation
- [ ] ESP32 and AC sections physically separated
- [ ] Enclosure grounded (if metal)
- [ ] Thermocouple properly secured in kiln
- [ ] Emergency stop tested (press both encoder buttons)
- [ ] Fire extinguisher nearby

**⚠️ WARNING**: Incorrect wiring can cause fire, electric shock, or death. If you are not comfortable working with 120VAC mains voltage, hire a licensed electrician.

---

## 💻 Software Installation

### Prerequisites

- **Python 3.x** (for PlatformIO)
- **Git** (for cloning repository)
- **PlatformIO Core** or **PlatformIO IDE** (VSCode extension)
- **USB-to-Serial drivers** (CP210x or CH340 depending on your ESP32 board)

### Installation Steps

1. **Clone Repository**
   ```bash
   git clone https://github.com/yourusername/kilnin-it.git
   cd kilnin-it
   ```

2. **Install PlatformIO** (if not already installed)
   ```bash
   pip install platformio
   ```

3. **Build Firmware**
   ```bash
   pio run
   ```

4. **Upload to ESP32**
   ```bash
   pio run --target upload
   ```

5. **Upload Web Files to SPIFFS**
   ```bash
   pio run --target uploadfs
   ```

6. **Monitor Serial Output**
   ```bash
   pio device monitor
   ```

### First Boot Configuration

1. **Connect to WiFi**
   - On first boot, ESP32 creates access point: `KilnController-XXXX`
   - Connect with password: `kilnin-it`
   - Navigate to `http://192.168.4.1`
   - Enter your WiFi credentials

2. **Calibrate Thermocouple** (IMPORTANT - do this first!)
   - Navigate to **Settings > Calibration**
   - Follow ice-point calibration procedure (see below)
   - Save calibration offset

3. **Configure System Settings**
   - Set kiln wattage (watts)
   - Set electricity rate ($/kWh)
   - Set temperature units (°C or °F)
   - Set max temperature limit (default: 1320°C)

4. **Run PID Auto-Tune** (Recommended)
   - Navigate to **Settings > Auto-Tune**
   - Follow on-screen instructions
   - Controller will automatically calculate optimal PID values

---

## 🧪 Thermocouple Calibration

**CRITICAL**: Calibrate before first use and annually thereafter.

### Ice-Point Calibration Method

**Reference Temperature**: 0°C (32°F) - NIST standard for thermocouple calibration

**Materials Needed**:
- Crushed ice (50-60% of container volume)
- Distilled or tap water
- Insulated container (Thermos, foam cup)
- Thermocouple to be calibrated

**Procedure**:
1. Fill container 50-60% with crushed ice (small pieces, not cubes)
2. Add water until just covering ice (ice should still float)
3. Stir vigorously for 30 seconds (creates 0°C equilibrium)
4. Insert thermocouple probe 3 inches deep (7.5cm)
5. Wait 60 seconds for reading to stabilize
6. On controller: Navigate to **Settings > Calibration > Start**
7. Wait for "Stable" indicator (temperature within ±0.5°C for 60s)
8. Controller calculates offset automatically (Expected: 0.0°C, Measured: X.X°C)
9. Review calibration quality and accept

**Calibration Quality**:
- **Excellent**: ±0.5°C or better
- **Good**: ±1.0°C
- **Acceptable**: ±2.0°C
- **Poor**: ±5.0°C (check connections, retry)
- **Failed**: >±5.0°C (inspect thermocouple for damage)

---

## 🔥 Creating Firing Profiles

### Profile Structure

Firing profiles consist of multiple **segments**:
- **Ramp Rate**: How fast to heat (°C/hour)
- **Target Temperature**: Temperature to reach
- **Soak Time**: How long to hold at target (minutes)

### Example: Bisque Firing (Cone 04)

| Segment | Ramp Rate | Target Temp | Soak Time | Purpose |
|---------|-----------|-------------|-----------|---------|
| 1 | 50°C/hr | 100°C | 60 min | Water smoking (remove physical water) |
| 2 | 100°C/hr | 500°C | 30 min | Organic burnout (remove carbon) |
| 3 | 150°C/hr | 950°C | 15 min | Final bisque temperature (Cone 04) |

### Creating Profiles via Web Interface

1. Navigate to **Profiles > Create New**
2. Enter profile name and description
3. Add segments (up to 8 per profile)
4. Set safety limit (max temp for this profile)
5. Save profile
6. Test with empty kiln first!

### Pre-Loaded Profiles

- **Bisque Cone 04** (950°C / 1742°F)
- **Glaze Cone 6** (1220°C / 2228°F)
- **Raku** (900°C / 1652°F, fast firing)

---

## 🎮 Using the Controller

### LCD Display Screens

**Main Status Screen**:
```
┌──────────────────────┐
│ FIRING: Bisque 04    │
├──────────────────────┤
│ Current: 842°C       │
│ Target:  950°C       │
│ Segment: 3/3 (Soak)  │
│ Time: 4:32 / 6:45    │
│ Power: 78%  [✓CAL]   │
└──────────────────────┘
```

**Energy Tracking Screen**:
```
┌──────────────────────┐
│ ENERGY COSTS         │
├──────────────────────┤
│ This Firing: $2.34   │
│ Daily: $2.34         │
│ Billing: $18.92      │
│ Annual: $94.50       │
│ Lifetime: $342.18    │
└──────────────────────┘
```

### Rotary Encoder Controls

**Left Encoder** (Navigation/Time):
- Rotate: Navigate menu items, adjust time values
- Press: Select/confirm

**Right Encoder** (Values/Temperature):
- Rotate: Adjust temperature values, change settings
- Press: Edit mode toggle

**Emergency Stop**:
- Press BOTH encoder buttons simultaneously for 0.5 seconds
- Immediately shuts off SSR and enters safe state

### Starting a Firing

1. Load kiln with ceramics
2. Ensure kiln lid is secure
3. On controller: **Menu > Start Firing**
4. Select profile (Left encoder)
5. Confirm start (Press both encoders)
6. Monitor via LCD or web interface

### Stopping a Firing

**Normal Completion**:
- Profile completes automatically
- Controller switches to cooling mode
- Safe to open when temp < 100°C

**Manual Stop**:
- **Menu > Stop Firing** (controlled shutdown)
- Controller ramps down slowly

**Emergency Stop**:
- Press both encoder buttons for 0.5s
- Immediate shutoff (thermal shock risk!)

---

## 🌐 Web Interface

### Accessing Web Dashboard

1. **Find ESP32 IP Address**:
   - Check LCD display: **Menu > Network Info**
   - Or check your router's DHCP client list

2. **Open Browser**:
   - Navigate to `http://[ESP32_IP_ADDRESS]`
   - Example: `http://192.168.1.142`

3. **Login** (if authentication enabled):
   - Default username: `admin`
   - Default password: `kilnin-it` (change immediately!)

### Dashboard Features

- **Real-Time Temperature Graph** - Live updating chart with historical data
- **Firing Progress** - Current segment, time remaining, visual progress bar
- **Power Output** - SSR duty cycle percentage
- **Energy Costs** - Current firing cost and cumulative totals
- **Profile Management** - Create, edit, delete firing profiles
- **System Status** - WiFi signal, uptime, calibration status, errors
- **Remote Control** - Start/stop firings, emergency stop

### API Endpoints (for developers)

```
GET  /api/status              - Current state, temp, progress
GET  /api/profiles            - List all firing profiles
POST /api/profiles            - Create new profile
PUT  /api/profiles/{id}       - Update existing profile
DELETE /api/profiles/{id}     - Delete profile
POST /api/firing/start        - Start firing with profile ID
POST /api/firing/stop         - Stop current firing
GET  /api/costs               - Energy cost tracking data
GET  /api/config              - System configuration
POST /api/config              - Update configuration
```

**WebSocket**: `/ws` for real-time streaming (temp, state, costs @ 1Hz)

---

## 🛡️ Safety Features

### Hardware Safety

- **Solid State Relay (SSR)** - Optical isolation between ESP32 and AC mains
- **Thermocouple Monitoring** - Detects open/short circuit faults
- **Separate Power Supplies** - Logic and kiln power electrically isolated
- **Emergency Stop** - Dual-button activation (0.5s hold prevents accidents)

### Software Safety

- **Watchdog Timer** - Auto-resets if firmware hangs (10s timeout)
- **Temperature Limits** - Hard-coded 1320°C maximum (cannot be overridden)
- **Ramp Rate Limiting** - Prevents thermal shock (600°C/hr max default)
- **Sensor Validation** - Filters noise, detects impossible readings
- **Timeout Protection** - Max firing duration (48 hours default)
- **Fail-Safe Defaults** - SSR always initializes to OFF state

### Safety Checklist Before Every Firing

- [ ] Thermocouple connected and calibrated
- [ ] Kiln chamber clear of flammable materials
- [ ] Adequate ventilation (kiln fumes are toxic)
- [ ] Fire extinguisher accessible (Class C for electrical fires)
- [ ] No water near electrical components
- [ ] Controller display shows correct temperature
- [ ] Emergency stop tested before starting
- [ ] Never leave kiln unattended during firing

**⚠️ DANGER**: Kilns reach extreme temperatures. Improper use can cause:
- **Fire hazard** - Keep 12" clearance from walls/combustibles
- **Burn injury** - Never touch kiln during or immediately after firing
- **Toxic fumes** - Ensure proper ventilation during burnout phases
- **Electric shock** - Only qualified persons should modify wiring

---

## 📈 Energy Cost Tracking

### How It Works

Controller monitors SSR duty cycle and calculates energy consumption:

**Formula**: `Energy (kWh) = (Kiln Wattage × Duty Cycle × Time) / 1000`

**Example**:
- Kiln: 1500W
- Duty Cycle: 60% average
- Firing Time: 6 hours
- Energy: (1500 × 0.60 × 6) / 1000 = **5.4 kWh**
- Cost @ $0.12/kWh: **$0.65**

### Cost Tracking Categories

- **Current Firing** - Real-time cost accumulation
- **Daily Total** - Resets at midnight
- **Billing Cycle** - Resets on configured billing day (1-31)
- **Annual** - Resets January 1
- **Lifetime** - Never resets (stored in NVS)

### Configuration

Set your electricity rate in **Settings > Energy**:
- **Electricity Rate**: $/kWh (check your utility bill)
- **Billing Cycle Day**: Day of month billing resets (1-31)
- **Currency Symbol**: $ € £ etc.

---

## 🧩 PID Auto-Tuning

### What is PID?

**PID** (Proportional-Integral-Derivative) is a control algorithm that adjusts SSR output to maintain target temperature:

- **P (Proportional)**: Reacts to current error (target - actual)
- **I (Integral)**: Eliminates steady-state error over time
- **D (Derivative)**: Dampens oscillation, predicts future error

### Why Auto-Tune?

Every kiln has different thermal mass, insulation, and element characteristics. Auto-tuning calculates optimal PID values for **your specific kiln**.

### Auto-Tune Procedure

1. **Prerequisites**:
   - Kiln must be empty (no ceramics)
   - Thermocouple calibrated
   - Kiln at room temperature (< 50°C)

2. **Start Auto-Tune**:
   - Navigate to **Settings > Auto-Tune > Start**
   - Controller uses "relay method" (on/off cycling)
   - Heats to ~400°C, measures thermal response

3. **Process** (60-90 minutes):
   - SSR cycles on/off at 50% power
   - Controller measures temperature oscillation period and amplitude
   - Calculates Kp, Ki, Kd using Ziegler-Nichols rules

4. **Results**:
   - New PID values displayed and automatically saved
   - Can manually fine-tune if needed

**Note**: Auto-tune values are conservative. You can manually increase Kp for faster response if you notice sluggish behavior.

---

## 🐛 Troubleshooting

### Controller Won't Boot

- **Check Power Supply**: 5V/2A minimum, verify with multimeter
- **Check USB Cable**: Some cables are charge-only (no data)
- **Check Serial Monitor**: Connect at 115200 baud for boot messages
- **Reflash Firmware**: May be corrupted, re-upload via PlatformIO

### No Temperature Reading

- **Check Thermocouple Connection**: Verify all SPI wires (CS, CLK, MISO)
- **Check MAX31855 Module**: Red LED should be on (power indicator)
- **Check Serial Debug**: Look for "Thermocouple Error" or "NaN" readings
- **Thermocouple Polarity**: Yellow = negative, red = positive (K-type)
- **Open Circuit**: Readings > 1400°C indicate disconnected thermocouple

### Temperature Reading is Inaccurate

- **Calibrate Thermocouple**: Use ice-point method (see above)
- **Check Probe Placement**: Should be centered in kiln, not touching elements
- **Thermal Lag**: Thermocouple responds slower than elements (normal)
- **Kiln Door Open**: Ensure kiln is fully sealed during measurement

### SSR Not Switching

- **Check GPIO 25 Connection**: Verify wiring to SSR input terminals
- **Check SSR Input Voltage**: Should see 3.3V when active (multimeter)
- **Check SSR Load**: Must have kiln connected (some SSRs need minimum load)
- **SSR LED Indicator**: Should blink during firing (if equipped)
- **SSR Failed**: Test with known-good SSR (they do burn out)

### WiFi Connection Issues

- **Check SSID/Password**: Verify in **Settings > WiFi**
- **Signal Strength**: ESP32 needs strong 2.4GHz signal (not 5GHz)
- **Router MAC Filtering**: Whitelist ESP32 MAC address
- **Reset WiFi**: Hold both encoders on boot to clear credentials

### Display Not Working

- **Check SPI Wiring**: Verify CS=15, MOSI=23, SCK=18
- **Contrast Adjustment**: Some ST7920 displays have potentiometer
- **Check Backlight**: May be wired separately (verify power to LED pins)
- **Test with Example**: Load U8g2 library examples to isolate issue

### Kiln Overshoots Target Temperature

- **PID Too Aggressive**: Reduce Kp value (try 50% of current)
- **Run Auto-Tune**: Let controller calculate optimal values
- **Reduce Ramp Rate**: Slower heating = less overshoot
- **Thermal Mass**: Overshoot normal for small kilns (< 5°C acceptable)

### Emergency Stop Not Working

- **Test Each Encoder Button**: Verify both switches work independently
- **Check Wiring**: GPIO 34 (left SW), GPIO 36 (right SW)
- **Hold Duration**: Must press both for 0.5 seconds
- **Check Serial Monitor**: Should see "EMERGENCY STOP" message

### Web Interface Not Loading

- **Verify IP Address**: Check LCD **Menu > Network Info**
- **Ping ESP32**: `ping [IP_ADDRESS]` to verify network connectivity
- **SPIFFS Not Uploaded**: Run `pio run --target uploadfs`
- **Browser Cache**: Hard refresh (Ctrl+F5) or try incognito mode
- **Check Firewall**: Some networks block IoT device web servers

---

## 🔧 Advanced Configuration

### Custom PID Values

If auto-tune doesn't work well for your kiln, manually adjust in **Settings > PID**:

- **Kp (Proportional)**: Start at 5.0, increase for faster response, decrease for less overshoot
- **Ki (Integral)**: Start at 0.5, increase to eliminate steady-state error
- **Kd (Derivative)**: Start at 1.0, increase to dampen oscillation

**Tuning Tips**:
- Change one parameter at a time
- Test with empty kiln first
- Document your changes
- Overshoot = reduce Kp or increase Kd
- Oscillation = reduce Kp, increase Kd
- Slow response = increase Kp

### Temperature-Dependent PID (Future Feature)

Different PID values for different temperature ranges:
- Low (< 400°C): More aggressive
- Mid (400-800°C): Standard
- High (> 800°C): Conservative

### SSR Cycle Time

Default: 2 seconds (2000ms)

Adjust in `src/config.h`:
```cpp
#define SSR_CYCLE_MS 2000  // Increase to 4000 for more gradual control
```

**Trade-offs**:
- Shorter cycle = faster response, more SSR switching
- Longer cycle = smoother control, less switching wear

### Data Logging Interval

Default: 10 seconds

Adjust in **Settings > Logging**:
- Faster logging (1s): More data, more storage usage
- Slower logging (60s): Less data, longer history retention

### Partition Scheme (Advanced)

If you need more SPIFFS space for web files, modify `platformio.ini`:

```ini
board_build.partitions = custom_partitions.csv
```

Create `custom_partitions.csv`:
```csv
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000
otadata,  data, ota,     0xe000,  0x2000
app0,     app,  ota_0,   0x10000, 0x140000
app1,     app,  ota_1,   0x150000,0x140000
spiffs,   data, spiffs,  0x290000,0x170000
```

---

## 🤝 Contributing

We welcome contributions! This is an open-source project.

### How to Contribute

1. **Fork the Repository**
2. **Create Feature Branch**: `git checkout -b feature/your-feature`
3. **Make Changes**: Follow coding standards (see CLAUDE.md)
4. **Test on Hardware**: Verify changes don't break safety features
5. **Commit**: `git commit -m "Add your feature"`
6. **Push**: `git push origin feature/your-feature`
7. **Open Pull Request**: Describe changes and testing performed

### Development Guidelines

- **Safety First**: All changes must maintain safety features
- **Code Style**: Follow PlatformIO/Arduino conventions (see CLAUDE.md)
- **Documentation**: Update README and CLAUDE.md for significant changes
- **Testing**: Test on actual hardware before submitting PR
- **Comments**: Document complex logic and safety-critical sections

### Reporting Bugs

Open an issue with:
- **Description**: What went wrong?
- **Steps to Reproduce**: How can we replicate it?
- **Expected Behavior**: What should happen?
- **Hardware**: ESP32 variant, kiln type, SSR model
- **Firmware Version**: Check **Settings > About**
- **Serial Output**: Include relevant debug messages

### Feature Requests

We'd love to hear your ideas! Open an issue with:
- **Use Case**: What problem does this solve?
- **Proposed Solution**: How would it work?
- **Alternatives Considered**: Other approaches?
- **Impact**: Who benefits from this feature?

---

## 📜 License

**MIT License** - See [LICENSE](LICENSE) file for details.

You are free to:
- ✅ Use commercially
- ✅ Modify and distribute
- ✅ Use privately
- ✅ Sublicense

**Disclaimer**: This project involves high-voltage electricity and extreme temperatures. Use at your own risk. The authors are not responsible for injury, property damage, or fires resulting from building or using this controller.

---

## 🙏 Acknowledgments

### Libraries Used

- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) - Espressif
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - me-no-dev
- [ArduinoJson](https://arduinojson.org/) - Benoit Blanchon
- [Adafruit MAX31855 Library](https://github.com/adafruit/Adafruit-MAX31855-library) - Adafruit Industries
- [Arduino PID Library](https://github.com/br3ttb/Arduino-PID-Library) - Brett Beauregard
- [U8g2 Graphics Library](https://github.com/olikraus/u8g2) - Oliver Kraus

### Inspiration

- Open-source kiln controller projects (Apollo, Controleo3)
- Ceramic arts community feedback and testing
- DIY electronics and maker communities

### Support

- **Documentation**: [GitHub Wiki](https://github.com/yourusername/kilnin-it/wiki) (coming soon)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/kilnin-it/discussions)
- **Email**: support@kilnin-it.example.com (update with actual contact)

---

## 🗺️ Roadmap

### Current Status

- ✅ Development environment setup (M1.1 complete)
- 🔄 Hardware acquisition pending (M1.2)
- ⏳ Temperature reading module (M1.3)
- ⏳ Basic PID control (M2.1)
- ⏳ Web interface foundation (M2.2)

### Planned Features

- [ ] **Mobile App** - Native iOS/Android control (Phase 3)
- [ ] **Cloud Logging** - Optional cloud backup of firing data (Phase 3)
- [ ] **Notifications** - Push notifications when firing completes (Phase 3)
- [ ] **Multi-Zone Control** - Control multiple kilns or zones (Phase 4)
- [ ] **Bluetooth Control** - Local control via Bluetooth LE (Phase 4)
- [ ] **Advanced Profiles** - Import Orton cone charts, glaze manufacturer profiles (Phase 3)
- [ ] **OTA Updates** - Over-the-air firmware updates via web interface (Phase 2)

See [TASKS.md](TASKS.md) for detailed development plan.

---

## 📞 Contact

**Project Maintainer**: [Your Name]
**Email**: your.email@example.com
**GitHub**: [@yourusername](https://github.com/yourusername)

**Community**:
- Discussions: [GitHub Discussions](https://github.com/yourusername/kilnin-it/discussions)
- Issues: [GitHub Issues](https://github.com/yourusername/kilnin-it/issues)

---

## ⚠️ Final Safety Warning

**This project involves:**
- High-voltage AC mains electricity (120VAC)
- Extreme temperatures (up to 1320°C / 2408°F)
- Fire hazard if improperly built or operated

**ONLY proceed if you:**
- Understand electrical safety and AC wiring
- Are comfortable working with high-temperature equipment
- Have read and understood all safety sections of this document
- Accept full responsibility for your build

**When in doubt:**
- Consult a licensed electrician for AC wiring
- Test with empty kiln extensively before firing ceramics
- Keep fire extinguisher accessible (Class C electrical)
- Never leave kiln unattended during firing

---

**Happy Firing! 🎨🔥**

*Built with ❤️ by the ceramic maker community*
