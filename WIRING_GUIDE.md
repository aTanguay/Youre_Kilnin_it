# ESP32 Kiln Controller - Breadboard Wiring Guide

**Target**: Milestone 1 Hardware Proof of Concept
**Last Updated**: 2025-10-13

---

## ⚠️ SAFETY FIRST

**CRITICAL WARNINGS**:
- **NO AC POWER during breadboard testing** - Use 5V DC only for initial testing
- **SSR testing**: Use LED or low-voltage load initially, NOT the kiln
- **Thermocouple**: K-type thermocouples generate tiny voltages - handle with care
- **Double-check all connections** before applying power
- **Polarity matters** - Reversing power can damage components

---

## Required Components Checklist

### Core Components
- [ ] ESP32-WROOM-32 Development Board
- [ ] MAX31855 Thermocouple Amplifier Breakout Board
- [ ] K-Type Thermocouple (high-temp rated)
- [ ] 12864 LCD Display Module (128x64, ST7920 controller, blue backlight)
- [ ] Two 5V Rotary Encoders (20 pulses/revolution, with push button)
- [ ] Solid State Relay (40A, zero-cross) + heat sink
- [ ] Piezo Buzzer (active or passive)
- [ ] LEDs: 3x standard (red, blue, green) + 220Ω resistors

### Breadboard & Wiring
- [ ] Large breadboard (830 tie-points minimum) or 2x medium breadboards
- [ ] Jumper wires (male-to-male, male-to-female assortment)
- [ ] USB cable (for ESP32 - Micro-USB or USB-C depending on board)
- [ ] 5V 2A power supply (for powering ESP32 and peripherals)

### Testing Equipment
- [ ] Multimeter (for continuity and voltage checking)
- [ ] Test LED (for SSR output verification)
- [ ] 220Ω resistor (for test LED)

---

## Pin Assignment Quick Reference

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| **MAX31855 Thermocouple** | | |
| CS (Chip Select) | GPIO 5 | SPI device select |
| CLK (Clock) | GPIO 18 | Shared SPI clock |
| MISO (Data Out) | GPIO 19 | SPI data from sensor |
| VCC | 3.3V | MAX31855 uses 3.3V |
| GND | GND | Common ground |
| **SSR Control** | | |
| Control Signal | GPIO 25 | Digital output |
| **LCD Display (ST7920)** | | |
| CS (Chip Select) | GPIO 15 | SPI device select |
| MOSI (Data) | GPIO 23 | SPI data to display |
| SCK (Clock) | GPIO 18 | Shared SPI clock |
| VCC | 5V | ST7920 uses 5V |
| GND | GND | Common ground |
| **Left Rotary Encoder** | | |
| CLK (Pin A) | GPIO 32 | Rotation detection |
| DT (Pin B) | GPIO 33 | Rotation detection |
| SW (Button) | GPIO 34 | Push button (input-only pin) |
| + (VCC) | 5V | Encoder power |
| GND | GND | Common ground |
| **Right Rotary Encoder** | | |
| CLK (Pin A) | GPIO 35 | Rotation detection (input-only pin) |
| DT (Pin B) | GPIO 39 | Rotation detection (input-only pin, VN) |
| SW (Button) | GPIO 36 | Push button (input-only pin, VP) |
| + (VCC) | 5V | Encoder power |
| GND | GND | Common ground |
| **Piezo Buzzer** | | |
| Signal | GPIO 26 | PWM output |
| GND | GND | Common ground |
| **Status LEDs** | | |
| Power LED | GPIO 27 | + 220Ω resistor to GND |
| WiFi LED | GPIO 14 | + 220Ω resistor to GND |
| Error LED | GPIO 12 | + 220Ω resistor to GND |

---

## Step-by-Step Wiring Instructions

### Step 0: Pre-Wiring Checks

1. **Inspect ESP32 Board**
   - Check for physical damage
   - Verify all GPIO pins are accessible
   - Note whether it's Micro-USB or USB-C

2. **Test ESP32 with USB Power**
   - Connect ESP32 to computer via USB
   - Built-in LED should blink or power LED should illuminate
   - If no signs of life, check USB cable and port

3. **Organize Your Workspace**
   - Clear, well-lit work area
   - Components sorted and labeled
   - Multimeter within reach
   - Documentation (this guide) visible

---

### Step 1: Power Rails Setup

**Goal**: Establish stable 3.3V and 5V power rails on breadboard

1. **Insert ESP32 into breadboard**
   - Mount ESP32 across the center gap of breadboard
   - Ensure all pins are accessible on both sides

2. **Connect Power Rails**
   ```
   ESP32 3.3V → Breadboard + rail (red)
   ESP32 5V   → Breadboard + rail (red) - separate rail if possible
   ESP32 GND  → Breadboard - rail (blue/black)
   ```

3. **Verify Power Rails**
   - Use multimeter to check voltages
   - 3.3V rail: Should read 3.2-3.4V
   - 5V rail: Should read 4.8-5.2V

**⚠️ STOP**: Do not proceed until power rails are verified!

---

### Step 2: MAX31855 Thermocouple Amplifier

**Goal**: Wire thermocouple interface for temperature sensing

**MAX31855 Connections**:
```
MAX31855          ESP32
--------          -----
VCC        →      3.3V (NOT 5V!)
GND        →      GND
CS         →      GPIO 5
CLK        →      GPIO 18
DO (MISO)  →      GPIO 19
```

**Thermocouple Connections**:
```
K-Type Thermocouple
-------------------
+ (usually yellow)  →  MAX31855 T+
- (usually red)     →  MAX31855 T-
```

**Notes**:
- MAX31855 is 3.3V - do NOT connect to 5V or it will be damaged
- Thermocouple polarity matters for accurate readings
- Keep thermocouple wires away from high-current traces (EMI reduction)

**Testing**:
- Upload firmware (Step 7) and check serial output
- Room temperature should read ~20-25°C
- Touch thermocouple tip - temperature should rise

---

### Step 3: SSR Control Circuit

**Goal**: Wire solid-state relay control (initially with test LED)

**FOR INITIAL TESTING - Use LED instead of SSR**:
```
ESP32 GPIO 25  →  220Ω Resistor  →  LED Anode (+)
LED Cathode (-) →  GND
```

**Test Code Behavior**:
- LED should blink when firmware runs PID control
- Verify you can control LED before connecting actual SSR

**LATER - When ready for SSR**:
```
ESP32 GPIO 25  →  SSR Control Input (+)
GND            →  SSR Control Input (-)
```

**SSR Wiring Notes**:
- SSR control side is LOW VOLTAGE (3-32V DC)
- SSR load side is HIGH VOLTAGE (120VAC) - NOT for breadboard!
- For breadboard testing, just verify control signal with LED

**⚠️ WARNING**: Do NOT connect AC mains to SSR during breadboard testing!

---

### Step 4: ST7920 LCD Display

**Goal**: Wire 128x64 graphic LCD for user interface

**ST7920 LCD Connections (SPI Mode)**:
```
LCD Pin      ESP32
-------      -----
VCC (5V)  →  5V
GND       →  GND
CS (RS)   →  GPIO 15
MOSI (R/W)→  GPIO 23
SCK (E)   →  GPIO 18 (shared with MAX31855 SCK)
PSB       →  GND (sets SPI mode - CRITICAL!)
RST       →  3.3V (or leave disconnected)
BLA (+)   →  5V (backlight anode)
BLK (-)   →  GND (backlight cathode)
```

**CRITICAL**:
- **PSB pin MUST be connected to GND** to enable SPI mode
- ST7920 uses 5V power (unlike MAX31855 which uses 3.3V)
- SCK is shared between LCD and MAX31855 (GPIO 18)
- Different CS pins allow independent control (GPIO 15 for LCD, GPIO 5 for MAX31855)

**Display Notes**:
- Backlight may be very bright - you can add resistor to dim if needed
- Some displays have built-in backlight current limiting
- If display shows random pixels, check PSB is grounded

**Testing**:
- Upload firmware with U8g2 library configured for ST7920
- Display should show "Hello World" or temperature reading
- Backlight should illuminate (blue)

---

### Step 5: Left Rotary Encoder (Navigation/Time)

**Goal**: Wire left encoder for menu navigation and time axis control

**Left Encoder Connections**:
```
Encoder Pin    ESP32
-----------    -----
CLK (Pin A) →  GPIO 32
DT (Pin B)  →  GPIO 33
SW (Button) →  GPIO 34 (input-only pin)
+ (VCC)     →  5V
GND         →  GND
```

**Encoder Notes**:
- 5V encoder output is safe for ESP32 3.3V GPIO (high-impedance inputs)
- CLK and DT pins detect rotation direction
- SW pin is the push button (active LOW when pressed)
- GPIO 34 is input-only (no internal pull-up) - encoder has external pull-up

**Testing**:
- Rotate encoder - serial output should show direction (CW/CCW)
- Press button - serial output should show "Left Button Pressed"
- 20 pulses per full rotation (detents)

---

### Step 6: Right Rotary Encoder (Values/Temperature)

**Goal**: Wire right encoder for value adjustment and temperature axis control

**Right Encoder Connections**:
```
Encoder Pin    ESP32
-----------    -----
CLK (Pin A) →  GPIO 35 (input-only pin)
DT (Pin B)  →  GPIO 39 (input-only pin, VN)
SW (Button) →  GPIO 36 (input-only pin, VP)
+ (VCC)     →  5V
GND         →  GND
```

**IMPORTANT**:
- GPIOs 35, 36, 39 are ALL input-only pins
- These pins have no internal pull-up/pull-down resistors
- Encoder must have external pull-up resistors (5V encoders typically do)

**Emergency Stop Testing**:
- Press BOTH encoder buttons simultaneously
- Hold for 0.5 seconds
- System should enter emergency stop state
- SSR should turn OFF immediately
- Display/serial should show "EMERGENCY STOP"

---

### Step 7: Piezo Buzzer and Status LEDs

**Goal**: Wire audio feedback and visual status indicators

**Piezo Buzzer**:
```
Buzzer Pin    ESP32
----------    -----
Signal (+) →  GPIO 26
GND (-)    →  GND
```

**Note**: If using passive buzzer, PWM will generate tones. Active buzzer just beeps.

**Status LEDs** (each with 220Ω current-limiting resistor):
```
LED          ESP32       Resistor   Notes
---          -----       --------   -----
Power LED →  GPIO 27  →  220Ω  →  GND  (Green - system powered)
WiFi LED  →  GPIO 14  →  220Ω  →  GND  (Blue - WiFi status)
Error LED →  GPIO 12  →  220Ω  →  GND  (Red - error conditions)
```

**LED Orientation**:
- Long leg (anode +) goes to GPIO via resistor
- Short leg (cathode -) goes to GND

**Testing**:
- Power LED should be solid when system running
- WiFi LED will blink during connection attempts
- Error LED should only light during error conditions
- Buzzer should beep on startup

---

## Step 8: Final Checks Before Power-On

### Visual Inspection Checklist
- [ ] No loose wires or components
- [ ] No short circuits (wires touching where they shouldn't)
- [ ] All polarized components correctly oriented (LEDs, buzzer)
- [ ] Power rails correctly connected (3.3V, 5V, GND)
- [ ] SPI connections correct (shared SCK, separate CS pins)

### Multimeter Continuity Checks
- [ ] 3.3V rail to ESP32 3.3V pin
- [ ] 5V rail to ESP32 5V pin
- [ ] All GND connections to ESP32 GND
- [ ] No continuity between 3.3V and 5V rails (separate!)
- [ ] No continuity between 3.3V/5V and GND (would indicate short circuit)

### GPIO Assignments Review
- [ ] MAX31855: CS=5, CLK=18, MISO=19
- [ ] LCD: CS=15, MOSI=23, SCK=18
- [ ] SSR: GPIO 25 (test LED for now)
- [ ] Left Encoder: CLK=32, DT=33, SW=34
- [ ] Right Encoder: CLK=35, DT=39, SW=36
- [ ] Buzzer: GPIO 26
- [ ] LEDs: Power=27, WiFi=14, Error=12

---

## Step 9: Upload Firmware and Test

### Upload Process

1. **Connect ESP32 via USB**
   ```bash
   # Verify ESP32 is detected
   ls /dev/tty.* | grep usb  # macOS
   ```

2. **Build and Upload Firmware**
   ```bash
   cd /Users/atanguay/Desktop/Kilnin_It_2
   pio run --target upload
   ```

3. **Monitor Serial Output**
   ```bash
   pio device monitor
   ```

### Expected Behavior on First Boot

**Serial Output Should Show**:
```
ESP32 Kiln Controller
Initializing...
- SSR: OFF (safe state)
- Thermocouple: OK
- Temperature: 22.5°C
- Display: OK
- Encoders: Ready
- System: IDLE
```

**Visual Indicators**:
- Power LED: Solid green
- WiFi LED: Blinking (attempting to connect)
- Error LED: Off
- Display: Shows temperature
- Buzzer: Single startup beep

**Interactive Tests**:
1. **Thermocouple Test**
   - Serial shows room temperature (~20-25°C)
   - Touch thermocouple - temperature increases
   - Release - temperature decreases back to room temp

2. **Encoder Tests**
   - Rotate left encoder → Serial shows "Left: CW" or "Left: CCW"
   - Rotate right encoder → Serial shows "Right: CW" or "Right: CCW"
   - Press left button → Serial shows "Left Button"
   - Press right button → Serial shows "Right Button"

3. **Emergency Stop Test**
   - Press BOTH encoder buttons
   - Hold for 0.5 seconds
   - Serial shows "EMERGENCY STOP TRIGGERED"
   - Test LED (SSR output) turns OFF
   - Display shows "E-STOP" message

4. **Display Test**
   - LCD should show current temperature (large font)
   - Backlight should be on (blue)
   - No random pixels or artifacts

---

## Troubleshooting Common Issues

### ESP32 Won't Program
**Symptoms**: Upload fails, "Failed to connect to ESP32"
**Solutions**:
- Press and hold BOOT button during upload
- Check USB cable (some are power-only, need data cable)
- Try different USB port
- Install CP2102 or CH340 drivers

### Thermocouple Reads Incorrect Temperature
**Symptoms**: Shows 0.0°C, -999.0°C, or wildly incorrect values
**Solutions**:
- Check MAX31855 is connected to 3.3V (NOT 5V)
- Verify thermocouple polarity (+ to T+, - to T-)
- Check SPI wiring (CS=5, CLK=18, MISO=19)
- Ensure thermocouple is not shorted to metal surfaces

### LCD Display Blank or Shows Random Pixels
**Symptoms**: No text, garbage characters, or no backlight
**Solutions**:
- Verify PSB pin is connected to GND (enables SPI mode)
- Check LCD is connected to 5V (NOT 3.3V)
- Verify SPI connections (CS=15, MOSI=23, SCK=18)
- Adjust contrast potentiometer (if equipped)
- Check backlight connections (BLA to 5V, BLK to GND)

### Rotary Encoders Not Responding
**Symptoms**: No output when rotating or pressing buttons
**Solutions**:
- Verify 5V power to encoders
- Check GPIO assignments match code
- For right encoder: GPIOs 35, 36, 39 are input-only
- Encoders should have built-in pull-up resistors

### Emergency Stop Not Working
**Symptoms**: Pressing both buttons doesn't trigger emergency stop
**Solutions**:
- Verify both encoder SW pins connected (GPIO 34 and GPIO 36)
- Check that code monitors both pins simultaneously
- Test each button individually first
- Ensure 0.5 second hold timer is implemented

### SSR/Test LED Not Responding
**Symptoms**: LED doesn't blink or respond to control
**Solutions**:
- Verify GPIO 25 connection
- Check LED polarity (long leg to GPIO, short leg to GND)
- Test GPIO 25 with multimeter (should see voltage changes)
- Verify 220Ω resistor is present (LED protection)

---

## Next Steps After Successful Wiring

### Milestone 1 Completion Tasks

- [ ] **M1.3**: Basic thermocouple reading functional
- [ ] **M1.4**: SSR control working (test LED responding)
- [ ] **M1.5**: LCD displaying temperature
- [ ] **M1.6**: Both encoders responding (rotation and buttons)
- [ ] **M1.7**: Integration test - full system running for 1+ hour

### Calibration (Required Before Further Work)

**CRITICAL**: Perform thermocouple ice-point calibration before moving to Milestone 2
- See PLANNING.md "Thermocouple Calibration Procedure" section
- This ensures accurate temperature readings for PID tuning

### Moving to Milestone 2

Once breadboard prototype is stable and calibrated:
- [ ] Implement PID controller (M2.1)
- [ ] Add safety monitoring (M2.2-M2.5)
- [ ] Develop menu system (M2.6)
- [ ] Implement audio feedback (M2.7)

---

## Safety Reminders for SSR/Kiln Testing

**When you progress to actual kiln testing (NOT during breadboard phase)**:

1. **SSR Installation**
   - Mount SSR on adequate heat sink
   - Install in properly rated enclosure
   - Use correct wire gauge for amperage

2. **AC Wiring Safety**
   - TURN OFF breaker before wiring
   - Use proper AC-rated terminals and connectors
   - Physical separation between AC and DC sections
   - Ground all metal enclosures

3. **Initial Kiln Testing**
   - Start with low-power resistive load (NOT full kiln)
   - Test emergency stop functionality thoroughly
   - Verify all safety limits trigger correctly
   - Never leave unattended during initial tests

---

## Questions or Issues?

**Documentation References**:
- `CLAUDE.md` - Technical specifications and code examples
- `PLANNING.md` - Overall project architecture and decisions
- `TASKS.md` - Detailed task breakdown and status

**If you encounter issues**:
1. Check this wiring guide's troubleshooting section
2. Verify against pin assignment quick reference
3. Use multimeter to check continuity and voltages
4. Review serial monitor output for error messages

---

**Happy Building! 🔨**

Remember: Safety first, test incrementally, and document any issues you encounter.
