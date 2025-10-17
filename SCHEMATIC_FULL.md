# Kilnin' It - Hardware Wiring Schematic

This document provides detailed wiring diagrams for connecting all components to the ESP32-WROOM-32 microcontroller.

---

## Component Wiring Overview

### MAX31855 Thermocouple Module → ESP32
```
MAX31855          ESP32
--------          -----
VCC       →       3.3V
GND       →       GND
SCK       →       GPIO 18 (shared SPI clock)
CS        →       GPIO 5
DO (MISO) →       GPIO 19
```

### ST7920 LCD Display → ESP32
```
ST7920            ESP32
------            -----
VCC       →       5V
GND       →       GND
SCK       →       GPIO 18 (shared SPI clock with MAX31855)
SID (MOSI)→       GPIO 23
CS        →       GPIO 15
RST       →       3.3V (or GPIO if you want software reset)
PSB       →       GND (selects serial mode)
BLA       →       5V (backlight anode, through 100Ω resistor optional)
BLK       →       GND (backlight cathode)
```

### Left Rotary Encoder → ESP32
```
Encoder           ESP32
-------           -----
+         →       5V
GND       →       GND
CLK       →       GPIO 32
DT        →       GPIO 33
SW        →       GPIO 34 (input-only pin, has internal pull-down)
```

### Right Rotary Encoder → ESP32
```
Encoder           ESP32
-------           -----
+         →       5V
GND       →       GND
CLK       →       GPIO 35 (input-only)
DT        →       GPIO 39 (input-only)
SW        →       GPIO 36 (input-only)
```

### SSR Control → ESP32
```
SSR Input         ESP32
---------         -----
+ (control)→      GPIO 25
- (control)→      GND
```
**SSR Output Side** (120VAC - isolated):
```
Wall Outlet Hot → SSR Input Terminal
SSR Output Terminal → Kiln Hot Wire
Kiln Neutral → Wall Outlet Neutral (direct)
Kiln Ground → Wall Outlet Ground (direct)
```

### Piezo Buzzer → ESP32
```
Buzzer            ESP32
------            -----
+         →       GPIO 26
-         →       GND
```

### Status LEDs → ESP32 (with current-limiting resistors)
```
Power LED         ESP32
---------         -----
Anode (+) →       GPIO 27 → 220Ω resistor → LED → GND
Cathode (-)→      GND

WiFi LED          ESP32
--------          -----
Anode (+) →       GPIO 14 → 220Ω resistor → LED → GND
Cathode (-)→      GND

Error LED         ESP32
---------         -----
Anode (+) →       GPIO 12 → 220Ω resistor → LED → GND
Cathode (-)→      GND
```

### Power Supply
```
5V/2A Supply      ESP32
------------      -----
5V+       →       VIN (or 5V pin)
GND       →       GND
```

---

## ASCII Schematic (Simplified)

```
                    ┌─────────────────────────────┐
                    │       ESP32-WROOM-32        │
                    │                             │
    MAX31855        │                             │        ST7920 LCD
  ┌──────────┐      │                             │      ┌──────────┐
  │ VCC→3.3V ├──────┤ 3.3V                    5V  ├──────┤ VCC      │
  │ GND      ├──────┤ GND                    GND  ├──────┤ GND      │
  │ SCK      ├──────┤ GPIO 18 (SCK) ──────────────┼──────┤ SCK      │
  │ CS       ├──────┤ GPIO 5                      │      │          │
  │ DO(MISO) ├──────┤ GPIO 19          GPIO 23────┼──────┤ SID      │
  │          │      │                   GPIO 15───┼──────┤ CS       │
  └──────────┘      │                             │      └──────────┘
                    │                             │
  Left Encoder      │                             │     Right Encoder
  ┌──────────┐      │                             │      ┌──────────┐
  │ +  → 5V  ├──────┤ 5V                      5V  ├──────┤ +        │
  │ GND      ├──────┤ GND                    GND  ├──────┤ GND      │
  │ CLK      ├──────┤ GPIO 32          GPIO 35────┼──────┤ CLK      │
  │ DT       ├──────┤ GPIO 33          GPIO 39────┼──────┤ DT       │
  │ SW       ├──────┤ GPIO 34 (IN)     GPIO 36────┼──────┤ SW       │
  └──────────┘      │                             │      └──────────┘
                    │                             │
    SSR Control     │                             │        Buzzer
  ┌──────────┐      │                             │      ┌──────────┐
  │ +        ├──────┤ GPIO 25                     │      │ +        │
  │ -        ├──────┤ GND              GPIO 26────┼──────┤          │
  └────┬─────┘      │                             │      │ -  → GND │
       │            │                             │      └──────────┘
       │ (Isolated) │                             │
       ↓            │                             │      LEDs (3x)
   [AC MAINS]       │   GPIO 27 ─────────────┬───[220Ω]→ Power LED → GND
   Hot→SSR→Kiln     │   GPIO 14 ─────────────┬───[220Ω]→ WiFi LED  → GND
                    │   GPIO 12 ─────────────┬───[220Ω]→ Error LED → GND
                    │                             │
   5V Power Supply  │                             │
  ┌──────────┐      │                             │
  │ 5V+      ├──────┤ VIN                         │
  │ GND      ├──────┤ GND                         │
  └──────────┘      └─────────────────────────────┘
```

---

## Detailed Connection Tables

### ESP32 GPIO Pin Assignment Summary

| GPIO | Function | Component | Direction | Notes |
|------|----------|-----------|-----------|-------|
| 5 | SPI CS | MAX31855 Thermocouple | Output | Chip select for temp sensor |
| 12 | Digital Out | Error LED | Output | Status indicator (avoid boot issues) |
| 14 | Digital Out | WiFi LED | Output | Status indicator |
| 15 | SPI CS | ST7920 LCD | Output | Chip select for display |
| 18 | SPI SCK | MAX31855 + LCD | Output | Shared clock for SPI bus |
| 19 | SPI MISO | MAX31855 | Input | Data from thermocouple |
| 23 | SPI MOSI | ST7920 LCD | Output | Data to display |
| 25 | PWM/Digital | SSR Control | Output | Kiln element control |
| 26 | Digital Out | Piezo Buzzer | Output | Audio feedback |
| 27 | Digital Out | Power LED | Output | Status indicator |
| 32 | Digital In | Left Encoder CLK | Input | Navigation/time control |
| 33 | Digital In | Left Encoder DT | Input | Navigation/time control |
| 34 | Digital In | Left Encoder SW | Input Only | Button (no pull-up) |
| 35 | Digital In | Right Encoder CLK | Input Only | Value/temp control |
| 36 | Digital In | Right Encoder SW | Input Only | Button (no pull-up) |
| 39 | Digital In | Right Encoder DT | Input Only | Value/temp control |

**Reserved/Unused GPIOs**:
- GPIO 0, 2: Boot strapping pins (avoid)
- GPIO 13: Available for future expansion
- GPIO 1, 3: UART TX/RX (USB serial, avoid)
- GPIO 6-11: Connected to flash (do not use)

---

## SPI Bus Configuration

The ESP32 uses **VSPI** (SPI bus 3) by default for Arduino SPI library:

| SPI Signal | GPIO | Devices |
|------------|------|---------|
| MOSI | 23 | ST7920 LCD (SID) |
| MISO | 19 | MAX31855 (DO) |
| SCK | 18 | MAX31855 + ST7920 (shared) |
| CS (MAX31855) | 5 | Thermocouple select |
| CS (LCD) | 15 | Display select |

**How Shared SPI Works**:
- Both devices share the same clock (SCK) and data lines
- Separate CS (Chip Select) pins allow independent communication
- Only one device can be active at a time (CS LOW = active)
- SPI library handles timing automatically

---

## Important Hardware Notes

### 1. Input-Only GPIOs (34-39)

GPIOs 34, 35, 36, 39 are **input-only** and have no internal pull-up/pull-down resistors:

**If your rotary encoder modules don't have built-in pull-ups**, add external resistors:
```
Right Encoder CLK (GPIO 35) ──┬──── 10kΩ resistor ──── 3.3V
                              └──── to encoder CLK pin

Right Encoder DT (GPIO 39)  ──┬──── 10kΩ resistor ──── 3.3V
                              └──── to encoder DT pin

Right Encoder SW (GPIO 36)  ──┬──── 10kΩ resistor ──── 3.3V
                              └──── to encoder SW pin

Left Encoder SW (GPIO 34)   ──┬──── 10kΩ resistor ──── 3.3V
                              └──── to encoder SW pin
```

Most 5V rotary encoder modules include pull-ups, so this is usually not needed.

### 2. 5V Tolerance

ESP32 GPIO inputs are **5V tolerant** when configured as inputs. This means:
- ✅ Safe to connect 5V encoder outputs directly to ESP32 inputs
- ✅ No level shifters required for encoders
- ❌ Never apply 5V to GPIO configured as output
- ❌ Never apply 5V to 3.3V power pin

### 3. LED Current Limiting

Always use current-limiting resistors for LEDs:

**Standard 5mm LEDs** (20mA max, 2V forward voltage):
```
Resistor = (Vsource - Vled) / Iled
         = (3.3V - 2.0V) / 0.015A
         = 86Ω → use 100Ω or 220Ω (standard values)
```

**High-brightness LEDs** (reduce current to 5-10mA):
```
Use 330Ω to 470Ω resistors
```

### 4. SSR Isolation

**CRITICAL SAFETY**: The SSR creates complete electrical isolation:

**Low-Voltage Side** (DC control):
- ESP32 GPIO 25 (+) → SSR control input (+)
- ESP32 GND (-) → SSR control input (-)
- Voltage: 3.3V logic level
- Current: ~10-20mA

**High-Voltage Side** (AC switched):
- Wall outlet hot → SSR input terminal
- SSR output terminal → Kiln hot wire
- Voltage: 120VAC (or 240VAC depending on region)
- Current: Up to SSR rating (e.g., 25A)

**⚠️ NEVER connect DC ground to AC ground/neutral/earth!**

### 5. Power Supply Considerations

**Option A: USB Power** (easiest for prototype)
```
USB 5V/2A adapter → ESP32 USB port
- Powers ESP32 and all 3.3V/5V components
- Built-in voltage regulator handles 3.3V conversion
```

**Option B: External 5V Supply**
```
5V/2A DC adapter → ESP32 VIN pin + GND
- Use barrel jack or screw terminals
- VIN goes through onboard regulator to 3.3V
```

**Option C: External 3.3V Supply** (not recommended)
```
Regulated 3.3V supply → ESP32 3.3V pin + GND
- Bypasses onboard regulator
- Must be very clean/stable (use only high-quality supply)
- ST7920 LCD still needs 5V separately
```

**Recommended**: Use Option A (USB) for prototyping, Option B for final build.

### 6. Decoupling Capacitors (Recommended)

Add ceramic capacitors near each IC to reduce noise:

```
0.1µF (100nF) capacitor between VCC and GND, placed as close as possible to:
- MAX31855 module
- ST7920 LCD
- ESP32 3.3V pin
- ESP32 5V pin

Optional: 10µF electrolytic capacitor on main power input
```

This improves SPI reliability and reduces temperature reading noise.

### 7. Breadboard Prototype Tips

**Wire Management**:
- Use different colors for power rails (red = 5V, orange = 3.3V, black = GND)
- Keep SPI wires short (< 6 inches) to reduce noise
- Route encoder wires away from SPI bus (minimize crosstalk)
- Use solid-core 22 AWG wire for breadboard connections

**Breadboard Power Rails**:
```
Top Rail:    5V  (from ESP32 5V pin or external supply)
             GND (common ground for all components)

Bottom Rail: 3.3V (from ESP32 3.3V pin)
             GND (connected to top GND rail)
```

**Component Placement**:
- ESP32 in center of breadboard
- MAX31855 and LCD on opposite sides (reduce SPI contention)
- Encoders at edges for easy access
- SSR off-breadboard (keep high-current paths separate)

---

## Testing Sequence

Once wired, test components **one at a time** in this order:

### 1. Power Supply Test
```
Test: Plug in ESP32, measure voltages with multimeter
Expected:
- VIN pin: 5.0V (±0.25V)
- 3.3V pin: 3.3V (±0.1V)
- GND: 0V (reference)
Pass: All voltages within spec
Fail: Check power supply, USB cable, or onboard regulator
```

### 2. LED Blink Test
```
Test: Upload blink sketch (already in main.cpp)
Expected: Built-in LED (GPIO 2) blinks 1Hz
Pass: LED blinks steadily
Fail: Check ESP32 connection, try re-uploading firmware
```

### 3. MAX31855 Temperature Test
```
Test: Read thermocouple via SPI, print to serial monitor
Code: See src/temperature/thermocouple.cpp (to be written)
Expected: Room temperature (~20-25°C)
Pass: Reasonable temperature displayed
Fail: Check SPI wiring (CS=5, CLK=18, MISO=19)
```

### 4. LCD Display Test
```
Test: Initialize U8g2, draw text/shapes
Code: See src/ui/display.cpp (to be written)
Expected: Text appears on LCD screen
Pass: Display shows test pattern
Fail: Check SPI wiring (CS=15, MOSI=23, SCK=18), check PSB=GND
```

### 5. Rotary Encoder Test
```
Test: Read encoder rotation and button presses, print to serial
Code: See src/ui/input_handler.cpp (to be written)
Expected: Serial output shows rotation direction and clicks
Pass: All encoder movements detected
Fail: Check GPIO connections, verify pull-ups if needed
```

### 6. Status LED Test
```
Test: Cycle through each LED (power, WiFi, error)
Expected: Each LED turns on/off individually
Pass: All LEDs work
Fail: Check GPIO connections (27, 14, 12), verify resistor values
```

### 7. Buzzer Test
```
Test: Play tone (1kHz, 100ms)
Expected: Audible beep
Pass: Buzzer sounds
Fail: Check GPIO 26 connection, verify buzzer polarity
```

### 8. SSR Control Test (NO AC POWER!)
```
Test: Toggle GPIO 25, measure voltage at SSR input with multimeter
Expected:
- GPIO 25 HIGH: ~3.3V at SSR input
- GPIO 25 LOW: ~0V at SSR input
Pass: SSR input voltage toggles
Fail: Check GPIO 25 connection, check SSR input polarity
```

### 9. Emergency Stop Test
```
Test: Press both encoder switches simultaneously for 0.5s
Expected: SSR turns off, system enters safe state, alarm sounds
Pass: Emergency stop triggers correctly
Fail: Check encoder switch connections (GPIO 34, 36)
```

---

## Common Wiring Mistakes

### Problem: MAX31855 returns NaN or "thermocouple error"
**Causes**:
- Thermocouple not connected to MAX31855 module
- Incorrect SPI wiring (especially MISO on GPIO 19)
- CS pin not defined or wrong GPIO
- SPI bus conflict with LCD

**Solutions**:
- Verify thermocouple plugged into MAX31855 screw terminals
- Double-check MISO wire (GPIO 19)
- Ensure CS pin is GPIO 5 in code
- Test MAX31855 alone (disconnect LCD temporarily)

### Problem: LCD shows nothing (blank screen)
**Causes**:
- PSB pin not grounded (LCD in parallel mode instead of serial)
- Incorrect SPI wiring (especially MOSI on GPIO 23)
- CS pin wrong or not defined
- Contrast too low (some displays have potentiometer)
- Backlight not powered

**Solutions**:
- **Always connect PSB to GND** (enables serial SPI mode)
- Verify MOSI wire (GPIO 23)
- Ensure CS pin is GPIO 15 in code
- Adjust contrast potentiometer if present
- Check BLA (5V) and BLK (GND) connections

### Problem: Rotary encoder doesn't respond
**Causes**:
- Encoder not powered (missing 5V or GND)
- CLK/DT pins swapped
- Input-only GPIOs missing pull-ups
- Encoder module defective

**Solutions**:
- Verify 5V and GND connected to encoder
- Swap CLK and DT wires (rotation will reverse direction)
- Add 10kΩ pull-up resistors to 3.3V for GPIOs 34-39
- Test encoder with multimeter (pins should change state on rotation)

### Problem: ESP32 won't boot or constantly resets
**Causes**:
- GPIO 0, 2, 12, or 15 pulled LOW during boot (boot strapping conflict)
- Insufficient power supply (voltage drop under load)
- Short circuit somewhere in wiring

**Solutions**:
- Disconnect GPIO 12 (Error LED) and 15 (LCD CS) during boot, reconnect after
- Use higher-amperage power supply (2A minimum)
- Check for shorts with multimeter (power off first!)

### Problem: SSR doesn't switch (kiln doesn't heat)
**Causes**:
- SSR not powered on output side (no 120VAC)
- SSR control input polarity reversed
- SSR requires higher control voltage (some need 5V minimum)
- SSR failed/defective

**Solutions**:
- Verify 120VAC connected to SSR output terminals (**USE CAUTION**)
- Swap SSR control input wires (+ and -)
- Use level shifter or transistor to boost GPIO 25 to 5V
- Test SSR with known-good control voltage source

---

## Next Steps After Wiring

Once all components test successfully:

1. **Enclosure Design**: Plan component layout in project box
2. **Solder Prototype** (optional): Transfer breadboard to perfboard for reliability
3. **Software Development**: Implement temperature reading, PID, display modules
4. **Calibration**: Run ice-point calibration for thermocouple
5. **PID Auto-Tune**: Calculate optimal control parameters for your kiln
6. **Safety Testing**: Verify all safety features (e-stop, limits, watchdog)
7. **Empty Kiln Test**: Run firing profile with empty kiln (no ceramics)
8. **Load Testing**: Fire test pieces (low-value items first!)

---

## Safety Reminders

**Before connecting AC power to SSR**:
- [ ] All low-voltage wiring tested and working
- [ ] SSR mounted with heat sink
- [ ] AC wiring in strain relief, proper gauge (14 AWG min)
- [ ] ESP32 and AC sections physically separated
- [ ] Emergency stop tested and functional
- [ ] Fire extinguisher accessible (Class C electrical)
- [ ] Never work on AC wiring with power connected

**⚠️ HIGH VOLTAGE WARNING**: 120VAC can cause severe injury or death. Only qualified individuals should work with mains voltage. When in doubt, consult a licensed electrician.

---

## Additional Resources

**For Visual Schematics**:
- [Fritzing](https://fritzing.org/) - Free breadboard/schematic design tool
- [EasyEDA](https://easyeda.com/) - Free online PCB design tool
- [KiCad](https://www.kicad.org/) - Professional open-source EDA suite

**ESP32 Pinout References**:
- [ESP32 Pinout Diagram](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ESP32 SPI Configuration](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html)

**Component Datasheets**:
- [MAX31855 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX31855.pdf)
- [ST7920 Controller Datasheet](https://www.crystalfontz.com/controllers/Sitronix/ST7920/)
- [ESP32-WROOM-32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)

---

**Good luck with your build! 🔧⚡**
