/**
 * TFT_eSPI Display Test
 * Simple test to verify ILI9341 display is wired correctly
 *
 * To upload: pio run -e tft_test --target upload
 *
 * Expected results:
 * - Display fills with different colors
 * - Text appears in various sizes
 * - Procedural flame animation test
 */

#include <Arduino.h>
#include <TFT_eSPI.h>

// Create TFT instance (pin config from platformio.ini build_flags)
TFT_eSPI tft = TFT_eSPI();

// Fire effect buffer (64 pixels wide × 32 pixels tall for test)
uint8_t fireBuffer[32][64];
unsigned long lastFireUpdate = 0;
const int FIRE_UPDATE_INTERVAL = 50;  // 20 FPS

void updateFire();
void drawFire(int xPos, int yPos);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== TFT_eSPI Display Test ===");

    // Initialize TFT
    tft.init();
    tft.setRotation(1);  // Landscape mode (320x240)

    Serial.println("Display initialized");

    // Test 1: Fill screen with colors
    Serial.println("Test 1: Color fills...");
    tft.fillScreen(TFT_RED);
    delay(500);
    tft.fillScreen(TFT_GREEN);
    delay(500);
    tft.fillScreen(TFT_BLUE);
    delay(500);
    tft.fillScreen(TFT_BLACK);
    delay(500);

    // Test 2: Text rendering
    Serial.println("Test 2: Text rendering...");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.setTextSize(1);
    tft.setCursor(10, 10);
    tft.println("TFT_eSPI Display Test");

    tft.setTextSize(2);
    tft.setCursor(10, 30);
    tft.println("Resolution: 240x320");

    tft.setTextSize(3);
    tft.setCursor(10, 60);
    tft.setTextColor(TFT_RED);
    tft.println("850.5C");

    tft.setTextSize(2);
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_GREEN);
    tft.println("Kiln Controller");

    delay(2000);

    // Test 3: Graphics primitives
    Serial.println("Test 3: Graphics...");
    tft.fillScreen(TFT_BLACK);

    tft.drawRect(10, 10, 100, 50, TFT_YELLOW);
    tft.fillRect(120, 10, 100, 50, TFT_BLUE);
    tft.drawCircle(60, 120, 40, TFT_GREEN);
    tft.fillCircle(160, 120, 40, TFT_RED);
    tft.drawLine(10, 180, 220, 220, TFT_MAGENTA);

    delay(2000);

    Serial.println("All tests passed! Starting flame animation...");

    // Clear screen for flame test
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("Procedural Flame Test");
}

void loop() {
    // Update and draw procedural flame
    if (millis() - lastFireUpdate >= FIRE_UPDATE_INTERVAL) {
        updateFire();
        lastFireUpdate = millis();
    }

    // Draw flame in center of screen
    drawFire(120, 80);  // Centered (64px wide, 32px tall)

    // Update temperature display simulation
    static unsigned long lastTempUpdate = 0;
    static float simulatedTemp = 20.0;

    if (millis() - lastTempUpdate > 1000) {
        simulatedTemp += 5.0;
        if (simulatedTemp > 1200.0) simulatedTemp = 20.0;

        tft.setTextSize(3);
        tft.setCursor(200, 200);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.printf("%.1fC", simulatedTemp);

        lastTempUpdate = millis();
    }
}

/**
 * Update procedural flame animation
 * Classic demoscene fire effect
 */
void updateFire() {
    // Simulate varying heat based on time (for demo)
    int baseHeat = 150 + (sin(millis() / 1000.0) * 50);

    // STEP 1: Add random heat at bottom row (fuel source)
    for (int x = 0; x < 64; x++) {
        int heat = random(baseHeat - 30, baseHeat);
        fireBuffer[31][x] = constrain(heat, 0, 255);
    }

    // STEP 2: Propagate heat upward with cooling
    for (int y = 0; y < 31; y++) {
        for (int x = 0; x < 64; x++) {
            // Get pixels from row below (with wrapping at edges)
            int left = (x == 0) ? 63 : x - 1;
            int right = (x == 63) ? 0 : x + 1;

            // Average the 3 pixels below this one
            int heat = (fireBuffer[y + 1][left] +
                       fireBuffer[y + 1][x] +
                       fireBuffer[y + 1][right]) / 3;

            // Cool down as heat rises
            int cooling = random(5, 15 + (y * 2));
            heat = (heat > cooling) ? heat - cooling : 0;

            fireBuffer[y][x] = heat;
        }
    }
}

/**
 * Draw procedural flame with color gradient
 * Position: xPos, yPos (top-left corner)
 */
void drawFire(int xPos, int yPos) {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            uint8_t heat = fireBuffer[y][x];
            uint16_t color;

            // Map heat to color gradient (fire colors)
            if (heat > 220) {
                color = TFT_WHITE;           // White-hot core
            } else if (heat > 180) {
                color = 0xFFE0;              // Yellow (R=31, G=63, B=0)
            } else if (heat > 140) {
                color = 0xFD20;              // Orange (R=31, G=42, B=0)
            } else if (heat > 100) {
                color = TFT_RED;             // Red (R=31, G=0, B=0)
            } else if (heat > 60) {
                color = 0x7800;              // Dark red (R=15, G=0, B=0)
            } else {
                continue;  // Black (don't draw)
            }

            // Draw pixel (flip Y so flame rises upward)
            tft.drawPixel(xPos + x, yPos + (31 - y), color);
        }
    }
}
