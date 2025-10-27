# ILI9341 TFT Display Wiring Guide

## Pin Connections

### ESP32 to ILI9341 TFT

```
ILI9341 TFT Pin  →  ESP32 GPIO    Description
─────────────────────────────────────────────────
VCC              →  3.3V          Power (or 5V if module has regulator)
GND              →  GND           Ground
CS               →  GPIO 15       Chip Select
RESET            →  GPIO 4        Reset
DC/RS            →  GPIO 2        Data/Command select
SDI (MOSI)       →  GPIO 23       SPI Data Out (ESP32 → TFT)
SCK              →  GPIO 18       SPI Clock
LED              →  3.3V          Backlight (or connect to GPIO for PWM dimming)
SDO (MISO)       →  Not connected TFT doesn't send data back
```

### Important Notes

**Power**:
- Most ILI9341 modules have an onboard 3.3V regulator and can accept 5V
- Check your specific module's datasheet
- If unsure, use 3.3V (safer)

**Shared SPI Bus**:
- The TFT shares the SPI bus with the MAX31855 thermocouple
- Both use the same CLK (GPIO 18)
- Different CS pins keep them separate (TFT=15, MAX31855=5)

**Backlight**:
- Connect LED pin to 3.3V for always-on backlight
- Or connect to a GPIO pin (e.g., GPIO 27) for PWM brightness control
- Some modules have LED pin built-in, check yours

## Testing the Display

### Step 1: Wire it up
Follow the connections above.

### Step 2: Build and upload test
```bash
pio run -e tft_test --target upload
```

### Step 3: Open serial monitor
```bash
pio device monitor -e tft_test
```

### Expected Results

The test runs through several stages:

**Stage 1: Color Fills** (2 seconds)
- Screen fills RED → GREEN → BLUE → BLACK
- Verifies display is responding and colors work

**Stage 2: Text Rendering** (2 seconds)
- Various text sizes displayed
- "TFT_eSPI Display Test"
- Resolution info
- Simulated temperature in red
- "Kiln Controller" in green

**Stage 3: Graphics Primitives** (2 seconds)
- Yellow rectangle (outline)
- Blue filled rectangle
- Green circle (outline)
- Red filled circle
- Magenta diagonal line

**Stage 4: Procedural Flame Animation** (continuous)
- Full-color fire effect in center of screen
- Temperature counter updating (simulated)
- **THIS IS THE COOL PART** 🔥

### Troubleshooting

**Nothing on screen**:
- Check power connections (VCC and GND)
- Verify 3.3V is present at VCC pin
- Try connecting RESET pin to 3.3V instead of GPIO 4
- Check LED (backlight) is connected

**White screen only**:
- CS pin might be wrong
- Try swapping MOSI and MISO
- Check SPI clock (SCK) connection

**Garbled display / wrong colors**:
- DC (Data/Command) pin might be loose or wrong
- Verify GPIO 2 connection is solid
- Try different SPI speed (edit platformio.ini SPI_FREQUENCY)

**Partial display / artifacts**:
- SPI connections might be poor (check breadboard connections)
- Try lower SPI speed: Change `-DSPI_FREQUENCY=40000000` to `20000000`
- Check for loose wires

**Display works but flame doesn't animate**:
- Open serial monitor to check for errors
- Flame should update at 20 FPS (50ms intervals)

## Next Steps

Once the test is working:

1. **Verify flame looks good** - Should be smooth, colorful, organic
2. **Check performance** - Should run smoothly with no lag
3. **Test rotation** - Try different `tft.setRotation()` values (0-3)
   - 0: Portrait
   - 1: Landscape (default in test)
   - 2: Portrait (upside down)
   - 3: Landscape (upside down)

4. **Integrate into main code** - Port the flame code to your main display task

## TFT_eSPI Quick Reference

### Basic Commands

```cpp
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// Initialization
tft.init();
tft.setRotation(1);  // 0-3

// Drawing
tft.fillScreen(TFT_BLACK);
tft.drawPixel(x, y, color);
tft.drawLine(x0, y0, x1, y1, color);
tft.drawRect(x, y, w, h, color);
tft.fillRect(x, y, w, h, color);
tft.drawCircle(x, y, r, color);
tft.fillCircle(x, y, r, color);

// Text
tft.setCursor(x, y);
tft.setTextColor(color);
tft.setTextColor(color, bgcolor);  // With background
tft.setTextSize(1-7);
tft.print("Hello");
tft.println("World");
tft.printf("%.1f", value);

// Common Colors
TFT_BLACK    0x0000
TFT_WHITE    0xFFFF
TFT_RED      0xF800
TFT_GREEN    0x07E0
TFT_BLUE     0x001F
TFT_YELLOW   0xFFE0
TFT_ORANGE   0xFD20
TFT_MAGENTA  0xF81F
TFT_CYAN     0x07FF
```

### Custom 16-bit Colors (RGB565)

```cpp
// Format: RRRRRGGGGGGBBBBB (5-6-5 bits)
uint16_t color = tft.color565(r, g, b);  // r,g,b = 0-255

// Or manually:
uint16_t orange = 0xFD20;  // R=31, G=42, B=0
uint16_t darkRed = 0x7800; // R=15, G=0, B=0
```

## Performance Tips

- Use `tft.fillScreen()` instead of drawing individual black pixels
- Batch drawing operations when possible
- Avoid `tft.print()` in tight loops - use `tft.printf()` or pre-format strings
- For animations, only redraw changed areas (not whole screen)
- Keep SPI frequency at 40MHz for best performance

---

**Ready to see that flame in COLOR?** 🔥

Upload the test and watch it dance!
