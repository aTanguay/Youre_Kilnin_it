/**
 * ESP32 Kiln Controller - Hardware Test Firmware
 *
 * Comprehensive breadboard wiring test for all components.
 * Tests each component individually to verify connections.
 *
 * SAFETY: SSR control is limited to brief test pulses only.
 * Do NOT connect to actual kiln during testing.
 *
 * Serial Monitor Instructions:
 * - Set baud rate to 115200
 * - Send '1'-'8' to run individual tests
 * - Send '0' to run all tests in sequence
 */

#include <Arduino.h>
#include "config.h"
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <U8g2lib.h>

// ============================================================================
// TEST CONFIGURATION
// ============================================================================

#define BUILTIN_LED 2
#define TEST_DELAY 2000  // 2 seconds between tests

// Set to 1 to auto-run tests on startup, 0 to require manual menu selection
#define AUTO_RUN_ON_STARTUP 1

// Track if auto-run has completed
bool autoRunComplete = false;

// Global LCD object (initialized in setup)
U8G2_ST7920_128X64_F_SW_SPI* display = nullptr;

// Buzzer LEDC configuration (ESP32-native)
#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8  // 8-bit resolution

// ============================================================================
// BUZZER FUNCTIONS (ESP32 LEDC)
// ============================================================================

/**
 * Play a tone on the buzzer using ESP32 LEDC peripheral
 * More reliable than Arduino tone() on ESP32
 */
void playTone(uint32_t frequency, uint32_t duration) {
    if (frequency > 0) {
        ledcWriteTone(BUZZER_CHANNEL, frequency);
        delay(duration);
        ledcWriteTone(BUZZER_CHANNEL, 0);  // Stop tone
    }
}

// ============================================================================
// LCD HELPER FUNCTIONS
// ============================================================================

/**
 * Clear LCD and show test header
 */
void lcd_showTestHeader(const char* testName, int testNum) {
    if (!display) return;

    display->clearBuffer();
    display->setFont(u8g2_font_ncenB08_tr);

    // Header
    char header[32];
    snprintf(header, sizeof(header), "TEST %d", testNum);
    display->drawStr(0, 10, header);

    // Test name
    display->setFont(u8g2_font_6x10_tr);
    display->drawStr(0, 22, testName);
    display->drawLine(0, 24, 127, 24);

    display->sendBuffer();
}

/**
 * Add a line to the LCD (max 4 lines after header)
 */
void lcd_println(const char* text, int lineNum) {
    if (!display) return;

    display->setFont(u8g2_font_6x10_tr);
    int yPos = 26 + (lineNum * 12);

    if (yPos <= 64) {
        display->drawStr(0, yPos, text);
        display->sendBuffer();
    }
}

/**
 * Show status message (PASS/FAIL/etc)
 */
void lcd_showStatus(const char* status, bool isPass) {
    if (!display) return;

    display->setFont(u8g2_font_ncenB08_tr);

    // Clear bottom area
    display->setDrawColor(0);
    display->drawBox(0, 50, 128, 14);
    display->setDrawColor(1);

    // Draw status
    int x = (128 - (strlen(status) * 8)) / 2;  // Center text
    display->drawStr(x, 62, status);
    display->sendBuffer();
}

/**
 * Update display with current output
 */
void lcd_update() {
    if (!display) return;
    display->sendBuffer();
}

// ============================================================================
// HARDWARE TEST FUNCTIONS
// ============================================================================

/**
 * Test 1: Built-in LED and Serial Communication
 */
void test_builtin_led() {
    Serial.println("\n[TEST 1] Built-in LED & Serial");
    Serial.println("----------------------------------------");

    lcd_showTestHeader("Built-in LED", 1);
    delay(500);

    for (int i = 0; i < 3; i++) {
        digitalWrite(BUILTIN_LED, HIGH);
        Serial.println("  LED: ON");
        char msg[20];
        snprintf(msg, sizeof(msg), "Blink %d: ON", i + 1);
        lcd_println(msg, i + 1);
        delay(500);

        digitalWrite(BUILTIN_LED, LOW);
        Serial.println("  LED: OFF");
        delay(500);
    }

    Serial.println("  RESULT: If LED blinked 3 times, PASS");
    Serial.println("  If no blink, check ESP32 power/USB");

    lcd_showStatus("PASS", true);
    delay(2000);
}

/**
 * Test 2: Status LEDs
 */
void test_status_leds() {
    Serial.println("\n[TEST 2] Status LEDs");
    Serial.println("----------------------------------------");

    lcd_showTestHeader("Status LEDs", 2);
    delay(500);

    Serial.println("  Testing Power LED (GPIO 27)...");
    lcd_println("Power LED...", 1);
    digitalWrite(LED_POWER_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_POWER_PIN, LOW);
    delay(500);

    Serial.println("  Testing WiFi LED (GPIO 14)...");
    lcd_println("WiFi LED...", 2);
    digitalWrite(LED_WIFI_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_WIFI_PIN, LOW);
    delay(500);

    Serial.println("  Testing Error LED (GPIO 12)...");
    lcd_println("Error LED...", 3);
    digitalWrite(LED_ERROR_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_ERROR_PIN, LOW);

    Serial.println("  RESULT: Check each LED lit in sequence");
    Serial.println("  If no light: Check LED polarity, resistor, GPIO connections");

    lcd_showStatus("PASS", true);
    delay(2000);
}

/**
 * Test 3: Buzzer
 */
void test_buzzer() {
    Serial.println("\n[TEST 3] Piezo Buzzer");
    Serial.println("----------------------------------------");

    lcd_showTestHeader("Piezo Buzzer", 3);
    delay(500);

    Serial.println("  Playing test tones...");
    lcd_println("Playing tones...", 1);

    // Short beep
    lcd_println("Low beep", 2);
    playTone(1000, 200);
    delay(200);

    // Medium beep
    lcd_println("Mid beep", 3);
    playTone(1500, 500);
    delay(200);

    // High beep
    lcd_println("High beep", 4);
    playTone(2000, 200);
    delay(200);

    Serial.println("  RESULT: Should hear 3 beeps (low, mid, high)");
    Serial.println("  If no sound: Check buzzer polarity, GPIO 26 connection");

    lcd_showStatus("PASS", true);
    delay(2000);
}

/**
 * Test 4: Left Rotary Encoder
 */
void test_left_encoder() {
    Serial.println("\n[TEST 4] Left Rotary Encoder");
    Serial.println("----------------------------------------");
    Serial.println("  Rotate LEFT encoder (both directions)");
    Serial.println("  Press LEFT encoder button");
    Serial.println("  Monitoring for 10 seconds...");

    lcd_showTestHeader("Left Encoder", 4);
    lcd_println("Rotate & Press", 1);
    lcd_println("10 seconds...", 2);

    int lastCLK = digitalRead(ENCODER_LEFT_CLK_PIN);
    int lastSW = digitalRead(ENCODER_LEFT_SW_PIN);
    int cwCount = 0;
    int ccwCount = 0;
    int pressCount = 0;

    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {
        // Read CLK state
        int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
        int dt = digitalRead(ENCODER_LEFT_DT_PIN);
        int sw = digitalRead(ENCODER_LEFT_SW_PIN);

        // Detect rotation
        if (clk != lastCLK) {
            if (dt != clk) {
                Serial.println("  LEFT Encoder: Clockwise");
                cwCount++;
            } else {
                Serial.println("  LEFT Encoder: Counter-clockwise");
                ccwCount++;
            }
            lastCLK = clk;

            // Update LCD with counts
            char msg[20];
            snprintf(msg, sizeof(msg), "CW:%d CCW:%d", cwCount, ccwCount);
            lcd_println(msg, 3);
        }

        // Detect button press
        if (sw != lastSW && sw == LOW) {
            Serial.println("  LEFT Encoder: BUTTON PRESSED");
            pressCount++;

            char msg[20];
            snprintf(msg, sizeof(msg), "Button: %d", pressCount);
            lcd_println(msg, 4);
        }
        lastSW = sw;

        delay(10);
    }

    Serial.println("  RESULT: Check if rotations and button press detected");
    Serial.println("  If no response: Check CLK(32), DT(33), SW(34), 5V, GND");

    if (cwCount > 0 || ccwCount > 0 || pressCount > 0) {
        lcd_showStatus("PASS", true);
    } else {
        lcd_showStatus("NO INPUT", false);
    }
    delay(2000);
}

/**
 * Test 5: Right Rotary Encoder
 */
void test_right_encoder() {
    Serial.println("\n[TEST 5] Right Rotary Encoder");
    Serial.println("----------------------------------------");
    Serial.println("  Rotate RIGHT encoder (both directions)");
    Serial.println("  Press RIGHT encoder button");
    Serial.println("  Monitoring for 10 seconds...");

    lcd_showTestHeader("Right Encoder", 5);
    lcd_println("Rotate & Press", 1);
    lcd_println("10 seconds...", 2);

    int lastCLK = digitalRead(ENCODER_RIGHT_CLK_PIN);
    int lastSW = digitalRead(ENCODER_RIGHT_SW_PIN);
    int cwCount = 0;
    int ccwCount = 0;
    int pressCount = 0;

    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {
        // Read CLK state
        int clk = digitalRead(ENCODER_RIGHT_CLK_PIN);
        int dt = digitalRead(ENCODER_RIGHT_DT_PIN);
        int sw = digitalRead(ENCODER_RIGHT_SW_PIN);

        // Detect rotation
        if (clk != lastCLK) {
            if (dt != clk) {
                Serial.println("  RIGHT Encoder: Clockwise");
                cwCount++;
            } else {
                Serial.println("  RIGHT Encoder: Counter-clockwise");
                ccwCount++;
            }
            lastCLK = clk;

            // Update LCD with counts
            char msg[20];
            snprintf(msg, sizeof(msg), "CW:%d CCW:%d", cwCount, ccwCount);
            lcd_println(msg, 3);
        }

        // Detect button press
        if (sw != lastSW && sw == LOW) {
            Serial.println("  RIGHT Encoder: BUTTON PRESSED");
            pressCount++;

            char msg[20];
            snprintf(msg, sizeof(msg), "Button: %d", pressCount);
            lcd_println(msg, 4);
        }
        lastSW = sw;

        delay(10);
    }

    Serial.println("  RESULT: Check if rotations and button press detected");
    Serial.println("  If no response: Check CLK(35), DT(39), SW(36), 5V, GND");

    if (cwCount > 0 || ccwCount > 0 || pressCount > 0) {
        lcd_showStatus("PASS", true);
    } else {
        lcd_showStatus("NO INPUT", false);
    }
    delay(2000);
}

/**
 * Test 6: Emergency Stop (Dual Button)
 */
void test_emergency_stop() {
    Serial.println("\n[TEST 6] Emergency Stop (Dual Button)");
    Serial.println("----------------------------------------");
    Serial.println("  Press BOTH encoder buttons simultaneously");
    Serial.println("  Hold for 0.5 seconds");
    Serial.println("  Monitoring for 15 seconds...");

    lcd_showTestHeader("Emergency Stop", 6);
    lcd_println("Press BOTH btns", 1);
    lcd_println("Hold 0.5s", 2);
    lcd_println("15 seconds...", 3);

    unsigned long startTime = millis();
    unsigned long bothPressedStart = 0;
    bool bothPressed = false;
    bool emergencyTriggered = false;

    while (millis() - startTime < 15000 && !emergencyTriggered) {
        int leftSW = digitalRead(ENCODER_LEFT_SW_PIN);
        int rightSW = digitalRead(ENCODER_RIGHT_SW_PIN);

        // Check if both buttons are pressed
        if (leftSW == LOW && rightSW == LOW) {
            if (!bothPressed) {
                bothPressed = true;
                bothPressedStart = millis();
                Serial.println("  Both buttons pressed - hold for 0.5s...");
                lcd_println("HOLDING...", 4);
            }

            // Check if held for 500ms
            if (millis() - bothPressedStart >= 500) {
                Serial.println("  *** EMERGENCY STOP TRIGGERED ***");
                lcd_showTestHeader("Emergency Stop", 6);
                lcd_println("*** TRIGGERED ***", 2);
                playTone(2000, 1000);  // Loud alarm
                emergencyTriggered = true;
            }
        } else {
            if (bothPressed) {
                Serial.println("  Buttons released (hold longer next time)");
                lcd_println("Released early", 4);
                delay(1000);
                lcd_showTestHeader("Emergency Stop", 6);
                lcd_println("Press BOTH btns", 1);
                lcd_println("Hold 0.5s", 2);
            }
            bothPressed = false;
        }

        delay(10);
    }

    if (!emergencyTriggered) {
        Serial.println("  RESULT: Emergency stop NOT triggered");
        Serial.println("  Try pressing both buttons firmly and holding");
        lcd_showStatus("NO TRIGGER", false);
    } else {
        Serial.println("  RESULT: PASS - Emergency stop works!");
        lcd_showStatus("PASS", true);
    }
    delay(2000);
}

/**
 * Test 7: SSR Control (WARNING: Brief test only!)
 */
void test_ssr() {
    Serial.println("\n[TEST 7] SSR Control Output");
    Serial.println("----------------------------------------");
    Serial.println("  WARNING: DO NOT CONNECT TO KILN!");
    Serial.println("  Testing GPIO 25 output only");
    Serial.println("  Use multimeter or LED to verify");

    lcd_showTestHeader("SSR Control", 7);
    lcd_println("WARNING:", 1);
    lcd_println("NO KILN!", 2);
    delay(2000);

    Serial.println("\n  SSR will pulse 3 times (0.5s on, 0.5s off)");
    lcd_showTestHeader("SSR Control", 7);
    lcd_println("Pulsing 3x...", 1);

    for (int i = 0; i < 3; i++) {
        Serial.print("  Pulse ");
        Serial.print(i + 1);
        Serial.println(": ON");

        char msg[20];
        snprintf(msg, sizeof(msg), "Pulse %d: ON", i + 1);
        lcd_println(msg, i + 2);

        digitalWrite(SSR_PIN, HIGH);
        delay(500);

        Serial.println("         OFF");
        digitalWrite(SSR_PIN, LOW);
        delay(500);
    }

    Serial.println("  RESULT: Measure GPIO 25 with multimeter");
    Serial.println("  Should see 3.3V pulses");
    Serial.println("  SSR LED should blink (if SSR connected)");

    lcd_showStatus("PASS", true);
    delay(2000);
}

/**
 * Test 8: MAX31855 Thermocouple
 */
void test_thermocouple() {
    Serial.println("\n[TEST 8] MAX31855 Thermocouple");
    Serial.println("----------------------------------------");

    lcd_showTestHeader("Thermocouple", 8);
    lcd_println("Initializing...", 1);
    delay(500);

    Serial.println("  Initializing SPI and MAX31855...");

    // Initialize SPI
    SPI.begin();

    // Create thermocouple object (software SPI for MAX31855)
    Adafruit_MAX31855 thermocouple(THERMOCOUPLE_CLK, THERMOCOUPLE_CS, THERMOCOUPLE_MISO);

    delay(500);

    Serial.println("  Reading temperature for 20 seconds...");
    Serial.println("  (Try touching probe to see temp change)");
    lcd_showTestHeader("Thermocouple", 8);
    lcd_println("Reading 20s...", 1);
    lcd_println("Touch probe!", 2);

    int errorCount = 0;
    double totalTemp = 0;
    int goodSamples = 0;
    double minTemp = 9999;
    double maxTemp = -9999;
    double lastTemp = 0;

    unsigned long startTime = millis();
    int sampleNum = 0;

    while (millis() - startTime < 20000) {  // 20 seconds
        double temp = thermocouple.readCelsius();
        sampleNum++;

        Serial.print("  Sample ");
        Serial.print(sampleNum);
        Serial.print(" (");
        Serial.print((20000 - (millis() - startTime)) / 1000);
        Serial.print("s remaining): ");

        if (isnan(temp)) {
            Serial.println("ERROR - Check wiring!");
            errorCount++;

            lcd_showTestHeader("Thermocouple", 8);
            lcd_println("ERROR!", 1);
            char timeMsg[20];
            snprintf(timeMsg, sizeof(timeMsg), "%ds left", (20000 - (millis() - startTime)) / 1000);
            lcd_println(timeMsg, 2);
        } else {
            Serial.print(temp);
            Serial.println(" °C");
            totalTemp += temp;
            goodSamples++;

            if (temp < minTemp) minTemp = temp;
            if (temp > maxTemp) maxTemp = temp;
            lastTemp = temp;

            // Update LCD with current reading
            lcd_showTestHeader("Thermocouple", 8);
            char tempMsg[20];
            snprintf(tempMsg, sizeof(tempMsg), "%.1f C", temp);
            lcd_println(tempMsg, 1);

            char rangeMsg[20];
            snprintf(rangeMsg, sizeof(rangeMsg), "%.1f-%.1f C", minTemp, maxTemp);
            lcd_println(rangeMsg, 2);

            char timeMsg[20];
            snprintf(timeMsg, sizeof(timeMsg), "%ds / %d smpl", (millis() - startTime) / 1000, sampleNum);
            lcd_println(timeMsg, 3);
        }

        delay(500);  // Sample every 500ms
    }

    // Quality assessment
    Serial.println();
    lcd_showTestHeader("Thermocouple", 8);

    if (errorCount == 0 && goodSamples > 0) {
        double avgTemp = totalTemp / goodSamples;
        Serial.print("  Samples: ");
        Serial.println(goodSamples);
        Serial.print("  Average: ");
        Serial.print(avgTemp);
        Serial.println(" °C");
        Serial.print("  Range: ");
        Serial.print(minTemp);
        Serial.print(" - ");
        Serial.print(maxTemp);
        Serial.println(" °C");

        char avgMsg[20];
        snprintf(avgMsg, sizeof(avgMsg), "Avg: %.1fC", avgTemp);
        lcd_println(avgMsg, 1);

        char rangeMsg[20];
        snprintf(rangeMsg, sizeof(rangeMsg), "Range: %.1fC", maxTemp - minTemp);
        lcd_println(rangeMsg, 2);

        if (avgTemp >= 15.0 && avgTemp <= 35.0) {
            Serial.println("  RESULT: PASS - Reading room temperature");
            lcd_println("Temp OK", 3);
            lcd_showStatus("PASS", true);
        } else if (avgTemp > 35.0 && avgTemp < 100.0) {
            Serial.println("  RESULT: PASS - Detected heat (touched probe?)");
            lcd_println("Heat detected", 3);
            lcd_showStatus("PASS", true);
        } else {
            Serial.println("  WARNING: Temperature seems unusual");
            Serial.println("  (Expected 15-100°C for testing)");
            lcd_println("Unusual temp", 3);
            lcd_showStatus("WARNING", false);
        }
    } else if (goodSamples > 0) {
        Serial.print("  RESULT: PARTIAL - ");
        Serial.print(errorCount);
        Serial.println(" errors detected");
        Serial.print("  Got ");
        Serial.print(goodSamples);
        Serial.println(" good readings");

        char errMsg[20];
        snprintf(errMsg, sizeof(errMsg), "%d errors", errorCount);
        lcd_println(errMsg, 1);
        char goodMsg[20];
        snprintf(goodMsg, sizeof(goodMsg), "%d good", goodSamples);
        lcd_println(goodMsg, 2);
        lcd_showStatus("PARTIAL", false);
    } else {
        Serial.println("  RESULT: FAIL - No valid readings");
        Serial.println("  Check: CS(5), CLK(18), MISO(19), 3.3V, GND");
        Serial.println("  Check: Thermocouple polarity (Yellow-, Red+)");

        lcd_println("No readings!", 1);
        lcd_println("Check wiring", 2);
        lcd_showStatus("FAIL", false);
    }

    delay(3000);
}

/**
 * Test 9: ST7920 LCD Display
 */
void test_display() {
    Serial.println("\n[TEST 9] ST7920 LCD Display");
    Serial.println("----------------------------------------");

    Serial.println("  Initializing display...");

    // Initialize display using software SPI
    U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_SCK, LCD_MOSI, LCD_CS);
    u8g2.begin();

    Serial.println("  Drawing test pattern...");

    // Test 1: Text display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(5, 15, "Kiln Controller");
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(15, 30, "Hardware Test");
    u8g2.drawStr(20, 45, "Display OK!");
    u8g2.sendBuffer();

    Serial.println("  Test 1: Text displayed");
    delay(2000);

    // Test 2: Graphics
    u8g2.clearBuffer();
    u8g2.drawFrame(0, 0, 128, 64);  // Border
    u8g2.drawBox(10, 10, 20, 20);   // Filled box
    u8g2.drawCircle(64, 32, 15);    // Circle
    u8g2.drawLine(90, 10, 110, 50); // Line
    u8g2.sendBuffer();

    Serial.println("  Test 2: Graphics displayed");
    delay(2000);

    // Test 3: Temperature-like display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(10, 25, "TEMP:");
    u8g2.drawStr(30, 50, "23.5");
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(90, 50, "C");
    u8g2.drawCircle(85, 43, 3);  // Degree symbol
    u8g2.sendBuffer();

    Serial.println("  Test 3: Temperature display");
    delay(2000);

    // Final message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(15, 30, "TEST COMPLETE");
    u8g2.sendBuffer();

    Serial.println();
    Serial.println("  RESULT: Check LCD for 4 screens:");
    Serial.println("    1. Text: 'Kiln Controller...'");
    Serial.println("    2. Graphics: box, circle, line");
    Serial.println("    3. Temperature: '23.5°C'");
    Serial.println("    4. 'TEST COMPLETE'");
    Serial.println();
    Serial.println("  If blank: Check CS(15), MOSI(23), SCK(18), 5V, GND");
    Serial.println("  If garbled: Adjust contrast pot on back of LCD");
}

/**
 * Run all tests in sequence (interactive - requires user input)
 */
void run_all_tests() {
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("  RUNNING ALL HARDWARE TESTS");
    Serial.println("========================================");

    test_builtin_led();
    delay(TEST_DELAY);

    test_status_leds();
    delay(TEST_DELAY);

    test_buzzer();
    delay(TEST_DELAY);

    test_left_encoder();
    delay(TEST_DELAY);

    test_right_encoder();
    delay(TEST_DELAY);

    test_emergency_stop();
    delay(TEST_DELAY);

    test_ssr();
    delay(TEST_DELAY);

    test_thermocouple();
    delay(TEST_DELAY);

    test_display();

    Serial.println("\n========================================");
    Serial.println("  ALL TESTS COMPLETE");
    Serial.println("========================================");
}

/**
 * Run automated tests only (no user interaction required)
 */
void run_auto_tests() {
    Serial.println("\n");
    Serial.println("========================================");
    Serial.println("  AUTO-RUNNING HARDWARE TESTS");
    Serial.println("========================================");
    Serial.println();

    test_builtin_led();
    delay(TEST_DELAY);

    test_status_leds();
    delay(TEST_DELAY);

    test_buzzer();
    delay(TEST_DELAY);

    test_left_encoder();
    delay(TEST_DELAY);

    test_right_encoder();
    delay(TEST_DELAY);

    test_emergency_stop();
    delay(TEST_DELAY);

    test_ssr();
    delay(TEST_DELAY);

    test_thermocouple();
    delay(TEST_DELAY);

    test_display();

    Serial.println("\n========================================");
    Serial.println("  ALL TESTS COMPLETE");
    Serial.println("========================================");

    // Show completion on LCD
    display->clearBuffer();
    display->setFont(u8g2_font_ncenB10_tr);
    display->drawStr(5, 20, "TESTS COMPLETE");
    display->setFont(u8g2_font_6x10_tr);
    display->drawStr(5, 35, "Standalone mode:");
    display->drawStr(5, 47, "Unplug USB and");
    display->drawStr(5, 59, "power with 5V");
    display->sendBuffer();
    delay(3000);
}

/**
 * Print test menu
 */
void print_menu() {
    Serial.println("\n========================================");
    Serial.println("  ESP32 KILN CONTROLLER");
    Serial.println("  Hardware Test Menu");
    Serial.println("========================================");
    Serial.println();
    Serial.println("  0 - Run all tests");
    Serial.println("  1 - Built-in LED");
    Serial.println("  2 - Status LEDs");
    Serial.println("  3 - Buzzer");
    Serial.println("  4 - Left Rotary Encoder");
    Serial.println("  5 - Right Rotary Encoder");
    Serial.println("  6 - Emergency Stop");
    Serial.println("  7 - SSR Output (WARNING: No kiln!)");
    Serial.println("  8 - Thermocouple (MAX31855)");
    Serial.println("  9 - LCD Display (ST7920)");
    Serial.println();
    Serial.println("  Send number (0-9) to run test");
    Serial.println("========================================");

    // Show menu on LCD
    display->clearBuffer();
    display->setFont(u8g2_font_ncenB08_tr);
    display->drawStr(15, 10, "TEST MENU");
    display->drawLine(0, 12, 127, 12);
    display->setFont(u8g2_font_6x10_tr);
    display->drawStr(0, 24, "USB: Send 0-9");
    display->drawStr(0, 36, "0:All tests");
    display->drawStr(0, 48, "1-7:Components");
    display->drawStr(0, 60, "8:TC  9:LCD");
    display->sendBuffer();
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);  // Give time to open serial monitor

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  ESP32 Kiln Controller");
    Serial.println("  Hardware Test Firmware");
    Serial.println("========================================");
    Serial.println();

    // Initialize all GPIO pins
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);

    // SSR (SAFETY: Default OFF)
    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);
    Serial.println("[SAFETY] SSR pin initialized to OFF");

    // Status LEDs
    pinMode(LED_POWER_PIN, OUTPUT);
    pinMode(LED_WIFI_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
    digitalWrite(LED_POWER_PIN, LOW);
    digitalWrite(LED_WIFI_PIN, LOW);
    digitalWrite(LED_ERROR_PIN, LOW);

    // Initialize buzzer using ESP32 LEDC peripheral
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    ledcSetup(BUZZER_CHANNEL, 2000, BUZZER_RESOLUTION);  // 2kHz base frequency, 8-bit resolution
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWriteTone(BUZZER_CHANNEL, 0);  // Ensure buzzer starts silent

    // Initialize encoder pins - all use INPUT mode
    // 5V encoder modules have their own external pull-up resistors
    // CRITICAL: GPIOs 34-39 are INPUT-ONLY and have NO internal pull-up capability
    pinMode(ENCODER_LEFT_CLK_PIN, INPUT);   // GPIO 32 - has internal pull-up but using external
    pinMode(ENCODER_LEFT_DT_PIN, INPUT);    // GPIO 33 - has internal pull-up but using external
    pinMode(ENCODER_LEFT_SW_PIN, INPUT);    // GPIO 34 - INPUT-ONLY (no internal pull-up)
    pinMode(ENCODER_RIGHT_CLK_PIN, INPUT);  // GPIO 35 - INPUT-ONLY (no internal pull-up)
    pinMode(ENCODER_RIGHT_DT_PIN, INPUT);   // GPIO 39 - INPUT-ONLY (no internal pull-up)
    pinMode(ENCODER_RIGHT_SW_PIN, INPUT);   // GPIO 36 - INPUT-ONLY (no internal pull-up)

    Serial.println("[OK] All GPIO pins initialized");
    Serial.println();

    // Initialize LCD display
    Serial.println("[INIT] Initializing LCD display...");
    display = new U8G2_ST7920_128X64_F_SW_SPI(U8G2_R0, LCD_SCK, LCD_MOSI, LCD_CS);
    display->begin();

    // Show startup screen
    display->clearBuffer();
    display->setFont(u8g2_font_ncenB10_tr);
    display->drawStr(10, 20, "KILN TEST");
    display->setFont(u8g2_font_6x10_tr);
    display->drawStr(15, 40, "Initializing...");
    display->sendBuffer();

    Serial.println("[OK] LCD display ready");
    Serial.println();

#if AUTO_RUN_ON_STARTUP
    // Auto-run tests on startup
    Serial.println("INFO: AUTO_RUN_ON_STARTUP is enabled");
    Serial.println("      Tests will run automatically in 3 seconds...");
    Serial.println("      (Send any character to skip to menu)");
    Serial.println();

    // Show on LCD too
    display->clearBuffer();
    display->setFont(u8g2_font_ncenB10_tr);
    display->drawStr(10, 20, "AUTO-RUN");
    display->setFont(u8g2_font_6x10_tr);
    display->drawStr(5, 35, "Starting in 3s");
    display->drawStr(5, 50, "USB: Send char");
    display->drawStr(5, 62, "to cancel");
    display->sendBuffer();

    // Visual indicator that new firmware is loaded - rapid blink
    for (int i = 0; i < 6; i++) {
        digitalWrite(BUILTIN_LED, HIGH);
        delay(100);
        digitalWrite(BUILTIN_LED, LOW);
        delay(100);
    }

    // Give user a chance to interrupt
    unsigned long startWait = millis();
    bool skipAuto = false;
    while (millis() - startWait < 1800) {  // 1.8s remaining = 3s total with blinks
        if (Serial.available() > 0) {
            // Clear buffer
            while (Serial.available() > 0) Serial.read();
            skipAuto = true;
            Serial.println("Auto-run cancelled by user");
            break;
        }
        delay(100);
    }

    if (!skipAuto) {
        run_auto_tests();
        autoRunComplete = true;
    }
#endif

    // Print menu
    print_menu();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Check for serial input
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Clear any extra characters
        while (Serial.available() > 0) {
            Serial.read();
        }

        // Run requested test
        switch (cmd) {
            case '0':
                run_all_tests();
                print_menu();
                break;
            case '1':
                test_builtin_led();
                print_menu();
                break;
            case '2':
                test_status_leds();
                print_menu();
                break;
            case '3':
                test_buzzer();
                print_menu();
                break;
            case '4':
                test_left_encoder();
                print_menu();
                break;
            case '5':
                test_right_encoder();
                print_menu();
                break;
            case '6':
                test_emergency_stop();
                print_menu();
                break;
            case '7':
                test_ssr();
                print_menu();
                break;
            case '8':
                test_thermocouple();
                print_menu();
                break;
            case '9':
                test_display();
                print_menu();
                break;
            default:
                Serial.println("Invalid option. Send 0-9.");
                print_menu();
                break;
        }
    }

    // Blink built-in LED slowly to show system is running
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink >= 2000) {
        lastBlink = millis();
        digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    }

    delay(10);
}
