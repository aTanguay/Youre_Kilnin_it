# Hardware Test Firmware

Quick breadboard verification firmware to test each component individually.

## Build and Upload

**Upload hardware test firmware:**
```bash
pio run -e hardware_test --target upload
pio run -e hardware_test --target monitor
```

**Upload main firmware (default):**
```bash
pio run --target upload
pio run --target monitor
```

## Using the Test Firmware

1. **Open Serial Monitor** (115200 baud)
2. **Send commands** to run individual tests:
   - `0` - Run all tests in sequence
   - `1` - Built-in LED blink test
   - `2` - Status LEDs (Power, WiFi, Error)
   - `3` - Buzzer test tones
   - `4` - Left rotary encoder
   - `5` - Right rotary encoder
   - `6` - Emergency stop (dual button press)
   - `7` - SSR output test ⚠️ **DO NOT CONNECT KILN**
   - `8` - Thermocouple (MAX31855)
   - `9` - LCD Display (ST7920)

## Test Sequence Recommendations

### Phase 1: Basic GPIO (No external components)
1. Test 1: Built-in LED - Verifies ESP32 is running
2. Test 7: SSR output - Measure GPIO 25 with multimeter (should see 3.3V pulses)

### Phase 2: Simple Components
3. Test 2: Status LEDs - Add LEDs + resistors
4. Test 3: Buzzer - Add piezo buzzer

### Phase 3: Encoders
5. Test 4: Left encoder - Wire up first encoder
6. Test 5: Right encoder - Wire up second encoder
7. Test 6: Emergency stop - Test dual button press

### Phase 4: SPI Devices (Add libraries first)
8. Test 8: Thermocouple - Wire MAX31855, uncomment library
9. Test 9: LCD Display - Wire ST7920, uncomment library

## Troubleshooting

### Built-in LED doesn't blink
- Check USB connection
- Check ESP32 power LED is on
- Try pressing RESET button on ESP32

### Status LEDs don't light
- Check LED polarity (long leg = anode = +)
- Check resistor value (220Ω recommended)
- Verify GPIO connections (27, 14, 12)
- Test LED with 3.3V directly

### No buzzer sound
- Check buzzer polarity (+ to GPIO 26)
- Some buzzers are very quiet
- Try different frequencies in code

### Encoders don't respond
- **Left encoder**: Check CLK(32), DT(33), SW(34)
- **Right encoder**: Check CLK(35), DT(39), SW(36)
  - Note: GPIO 34-39 are INPUT_ONLY (no internal pull-ups)
  - 5V encoders need external pull-up resistors on these pins
- Check 5V and GND connections
- Verify encoder is 5V type (3.3V won't work reliably)

### SSR test shows no voltage
- Measure GPIO 25 with multimeter
- Should see 3.3V when ON, 0V when OFF
- If no voltage, check GPIO 25 connection

### Thermocouple shows ERROR
- Uncomment library in platformio.ini first
- Check SPI connections: CS(5), CLK(18), MISO(19)
- MAX31855 needs 3.3V power (not 5V!)
- Check thermocouple is plugged into MAX31855
- Yellow wire = negative, Red wire = positive

### LCD shows nothing
- Uncomment library in platformio.ini first
- Check SPI connections: CS(15), MOSI(23), SCK(18)
- ST7920 needs 5V power
- Adjust contrast potentiometer on back of LCD
- Check PSB pin is connected to GND (selects serial mode)

## Safety Notes

⚠️ **CRITICAL**: Do NOT connect SSR to kiln during testing!

- Test 7 only verifies GPIO output
- SSR control is limited to brief pulses
- Use multimeter or oscilloscope to verify signal
- Kiln connection testing requires full safety firmware

## Next Steps

After all hardware tests pass:
1. Switch back to main firmware: `pio run -e esp32dev --target upload`
2. Begin implementing temperature reading module
3. Perform thermocouple ice-point calibration (see NOTES.md)
4. Implement PID control

## Expected Results

**All tests passing means:**
- ✅ ESP32 is working
- ✅ GPIO pins are correctly wired
- ✅ Encoders respond to rotation and button press
- ✅ Emergency stop dual-button works
- ✅ SSR control signal is working
- ✅ SPI bus is functional
- ✅ Ready for firmware development

Good luck with your breadboard build! 🔧
