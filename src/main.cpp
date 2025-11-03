/**
 * ESP32 Kiln Controller - Main Firmware
 * Milestone 1.7: Integration Testing
 *
 * An affordable, ESP32-based PID kiln controller for small 120VAC ceramic kilns.
 *
 * This is a safety-critical system. All code must prioritize fail-safe operation.
 *
 * Features in this version:
 * - Live temperature reading from MAX31855
 * - LCD display with current and target temperature
 * - Dual encoder input: Left = mode selection, Right = setpoint adjustment
 * - Manual SSR control (simple on/off based on setpoint)
 * - Status LED indicating heating state
 *
 * Author: Kiln Controller Project
 * License: MIT (or Apache 2.0)
 */

#include <Arduino.h>
#include "config.h"
#include <SPI.h>
#include <Adafruit_MAX31855.h>
#include <TFT_eSPI.h>
#include <PID_v1.h>

// ============================================================================
// HARDWARE OBJECTS
// ============================================================================

// Thermocouple using HARDWARE SPI (shares bus with TFT)
// Constructor: Adafruit_MAX31855(CS pin) - uses hardware SPI
// Pins: Hardware SPI uses CLK=18, MISO=19 automatically, CS=5
Adafruit_MAX31855 thermocouple(THERMOCOUPLE_CS);

// TFT Display (ILI9341)
// Uses hardware SPI configured via platformio.ini build flags
// Pins: CS=15, DC=2, RST=4, MOSI=23, SCK=18 (shared with MAX31855)
TFT_eSPI tft = TFT_eSPI();

// ============================================================================
// PID CONTROLLER
// ============================================================================

// PID variables (must be double for PID library)
double pidInput = 0;      // Current temperature
double pidOutput = 0;     // PID output (0-100%)
double pidSetpoint = 100; // Target temperature

// PID tuning parameters (conservative starting values)
double Kp = 5.0;   // Proportional gain
double Ki = 0.5;   // Integral gain
double Kd = 1.0;   // Derivative gain

// Create PID controller instance
// Parameters: &Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT
// DIRECT means: increase output when below setpoint (heating mode)
PID kilnPID(&pidInput, &pidOutput, &pidSetpoint, Kp, Ki, Kd, DIRECT);

// SSR control variables for time-proportional control
unsigned long ssrWindowSize = SSR_CYCLE_TIME_MS;  // 2 seconds from config.h
unsigned long ssrWindowStartTime = 0;

// ============================================================================
// SYSTEM STATE
// ============================================================================

enum SystemMode {
    MODE_MAIN_MENU,     // Top-level menu
    MODE_MANUAL,        // Manual heating mode (simple on/off)
    MODE_PROFILE,       // Profile-based firing (future)
    MODE_SETTINGS,      // Settings menu (future)
    MODE_TEST,          // Hardware test mode
    MODE_IDLE           // System idle, not heating
};

struct SystemState {
    SystemMode mode;
    float currentTemp;
    float targetTemp;
    bool heating;
    bool sensorError;
    unsigned long lastTempRead;
    unsigned long lastDisplayUpdate;
    unsigned long heatingStartTime;
};

SystemState state = {
    .mode = MODE_IDLE,
    .currentTemp = 0.0,
    .targetTemp = 100.0,  // Default target: 100°C
    .heating = false,
    .sensorError = false,
    .lastTempRead = 0,
    .lastDisplayUpdate = 0,
    .heatingStartTime = 0
};

// ============================================================================
// ENCODER STATE
// ============================================================================

struct EncoderState {
    int lastCLK;
    int lastSW;
    unsigned long lastButtonPress;
};

EncoderState leftEncoder = {HIGH, HIGH, 0};
EncoderState rightEncoder = {HIGH, HIGH, 0};

// Debounce time
#define DEBOUNCE_MS 50

// Buzzer LEDC configuration (ESP32-native)
#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8  // 8-bit resolution

// ============================================================================
// BUZZER FUNCTIONS (ESP32 LEDC)
// ============================================================================

/**
 * Play a tone on the buzzer using ESP32 LEDC peripheral
 * This is more reliable than Arduino tone() on ESP32
 *
 * @param frequency Frequency in Hz (e.g., 1000 for 1kHz)
 * @param duration Duration in milliseconds
 */
void playTone(uint32_t frequency, uint32_t duration) {
    if (frequency > 0) {
        ledcWriteTone(BUZZER_CHANNEL, frequency);
        delay(duration);
        ledcWriteTone(BUZZER_CHANNEL, 0);  // Stop tone
    }
}

// ============================================================================
// TEMPERATURE FUNCTIONS
// ============================================================================

/**
 * Read temperature from thermocouple with validation
 * Returns true if reading is valid, false on error
 */
bool readTemperature() {
    double temp = thermocouple.readCelsius();

    // Check for sensor errors
    if (isnan(temp)) {
        state.sensorError = true;
        DEBUG_PRINTLN("[ERROR] Thermocouple returned NaN");
        return false;
    }

    // Validate temperature range
    if (temp < MIN_VALID_TEMP || temp > MAX_VALID_TEMP) {
        state.sensorError = true;
        DEBUG_PRINT("[ERROR] Temperature out of range: ");
        DEBUG_PRINTLN(temp);
        return false;
    }

    // Valid reading
    state.sensorError = false;
    state.currentTemp = temp;
    return true;
}

// ============================================================================
// SSR CONTROL FUNCTIONS
// ============================================================================

/**
 * PID-based time-proportional SSR control
 * Uses a time window to simulate analog output with digital SSR
 */
void updateSSRControl() {
    // SAFETY: Don't heat if sensor error
    if (state.sensorError) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        kilnPID.SetMode(MANUAL);
        pidOutput = 0;
        return;
    }

    // SAFETY: Don't heat if target exceeds hard limit
    if (state.targetTemp > MAX_TEMP_LIMIT) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        kilnPID.SetMode(MANUAL);
        pidOutput = 0;
        DEBUG_PRINTLN("[SAFETY] Target exceeds MAX_TEMP_LIMIT - heating disabled");
        return;
    }

    // SAFETY: Don't heat if current temperature exceeds hard limit
    if (state.currentTemp >= MAX_TEMP_LIMIT) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        kilnPID.SetMode(MANUAL);
        pidOutput = 0;
        DEBUG_PRINTLN("[SAFETY] Current temp at/above MAX_TEMP_LIMIT - heating disabled");
        return;
    }

    // Update PID input and setpoint
    pidInput = state.currentTemp;
    pidSetpoint = state.targetTemp;

    // Enable PID if not already enabled
    if (kilnPID.GetMode() != AUTOMATIC) {
        kilnPID.SetMode(AUTOMATIC);
        ssrWindowStartTime = millis();
        DEBUG_PRINTLN("[PID] PID controller enabled");
    }

    // Compute PID output
    kilnPID.Compute();

    // Time-proportional SSR control
    // pidOutput is 0-100, representing percentage of time SSR should be ON
    unsigned long now = millis();

    // Check if we need to start a new SSR window
    if (now - ssrWindowStartTime >= ssrWindowSize) {
        ssrWindowStartTime = now;
    }

    // Calculate SSR on-time for this window
    unsigned long onTime = (ssrWindowSize * pidOutput) / 100;

    // Determine if SSR should be on or off in current window
    if ((now - ssrWindowStartTime) < onTime) {
        digitalWrite(SSR_PIN, HIGH);
        state.heating = true;
    } else {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
    }
}

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void displayTestMenu();
void initTestState();

// ============================================================================
// MAIN MENU SYSTEM
// ============================================================================

enum MainMenuItem {
    MAIN_MENU_MANUAL,
    MAIN_MENU_PROFILES,
    MAIN_MENU_SETTINGS,
    MAIN_MENU_HARDWARE_TEST,
    MAIN_MENU_ABOUT
};

struct MainMenuState {
    int selection;
};

MainMenuState mainMenu = {0};

const char* mainMenuItems[] = {
    "Manual Control",
    "Firing Profiles",
    "Settings",
    "Hardware Test",
    "About"
};
const int numMainMenuItems = 5;

/**
 * Display main menu
 * Landscape mode: 320x240
 */
void displayMainMenu() {
    tft.fillScreen(TFT_BLACK);

    // Header
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(30, 10);
    tft.println("Kiln Controller");
    tft.drawLine(0, 45, 320, 45, TFT_WHITE);

    tft.setTextSize(2);

    // Menu items
    for (int i = 0; i < numMainMenuItems; i++) {
        int yPos = 65 + (i * 30);

        if (i == mainMenu.selection) {
            // Highlight selected item
            tft.fillRect(0, yPos - 2, 320, 26, TFT_DARKGREEN);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        tft.setCursor(20, yPos);
        tft.print(mainMenuItems[i]);
    }

    // Instructions
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 220);
    tft.print("Turn: Navigate    Press: Select");
}

/**
 * Handle main menu input
 */
void handleMainMenuInput() {
    int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
    static int lastCLK = HIGH;

    // Navigate menu - only on FALLING edge
    if (clk != lastCLK && clk == LOW) {
        int dt = digitalRead(ENCODER_LEFT_DT_PIN);
        if (dt == LOW) {
            // Clockwise - down
            mainMenu.selection++;
            if (mainMenu.selection >= numMainMenuItems) {
                mainMenu.selection = 0;
            }
        } else {
            // Counter-clockwise - up
            mainMenu.selection--;
            if (mainMenu.selection < 0) {
                mainMenu.selection = numMainMenuItems - 1;
            }
        }
        displayMainMenu();
        playTone(1200, 20);
    }
    lastCLK = clk;

    // Select menu item
    int sw = digitalRead(ENCODER_LEFT_SW_PIN);
    static int lastSW = HIGH;
    static unsigned long lastPress = 0;

    if (sw != lastSW && sw == LOW) {
        if (millis() - lastPress > 200) {
            lastPress = millis();
            playTone(2000, 50);

            // Execute selected menu item
            switch (mainMenu.selection) {
                case MAIN_MENU_MANUAL:
                    state.mode = MODE_MANUAL;
                    state.targetTemp = 100.0;
                    state.heating = false;
                    break;
                case MAIN_MENU_PROFILES:
                    // TODO: Implement profiles
                    playTone(500, 100); // Error beep
                    break;
                case MAIN_MENU_SETTINGS:
                    // TODO: Implement settings
                    playTone(500, 100); // Error beep
                    break;
                case MAIN_MENU_HARDWARE_TEST:
                    state.mode = MODE_TEST;
                    initTestState();
                    displayTestMenu();
                    break;
                case MAIN_MENU_ABOUT:
                    // TODO: Show about screen
                    playTone(500, 100); // Error beep
                    break;
            }
        }
    }
    lastSW = sw;
}

// ============================================================================
// HARDWARE TEST MODE
// ============================================================================

enum TestMenuItem {
    TEST_MENU_MAIN,
    TEST_ALL,
    TEST_LEDS,
    TEST_BUZZER,
    TEST_LEFT_ENCODER,
    TEST_RIGHT_ENCODER,
    TEST_EMERGENCY_STOP,
    TEST_SSR,
    TEST_THERMOCOUPLE,
    TEST_DISPLAY,
    TEST_EXIT
};

struct TestState {
    TestMenuItem currentTest;
    int menuSelection;
    bool testRunning;
    unsigned long testStartTime;
    int testProgress;
    char testMessage[100];
    bool testPassed;
};

TestState testState;

void initTestState() {
    testState.currentTest = TEST_MENU_MAIN;
    testState.menuSelection = 0;
    testState.testRunning = false;
    testState.testStartTime = 0;
    testState.testProgress = 0;
    strcpy(testState.testMessage, "");
    testState.testPassed = false;
}

// Test menu items
const char* testMenuItems[] = {
    "Run All Tests",
    "1. Status LEDs",
    "2. Buzzer",
    "3. Left Encoder",
    "4. Right Encoder",
    "5. Emergency Stop",
    "6. SSR Output",
    "7. Thermocouple",
    "8. TFT Display",
    "Exit Test Mode"
};
const int numTestMenuItems = 10;

/**
 * Display the hardware test menu
 * Landscape mode: 320x240
 */
void displayTestMenu() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("HARDWARE TEST");
    tft.drawLine(0, 35, 320, 35, TFT_WHITE);

    tft.setTextSize(1);

    // Show menu items (8 at a time in landscape)
    int startIdx = max(0, testState.menuSelection - 3);
    int endIdx = min(numTestMenuItems, startIdx + 8);

    for (int i = startIdx; i < endIdx; i++) {
        int yPos = 45 + (i - startIdx) * 20;

        if (i == testState.menuSelection) {
            // Highlight selected item
            tft.fillRect(0, yPos - 2, 320, 18, TFT_DARKGREEN);
            tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        tft.setCursor(10, yPos);
        tft.print(testMenuItems[i]);
    }

    // Instructions at bottom
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 215);
    tft.print("Turn: Navigate    Press: Select");
}

/**
 * Show test running screen
 * Landscape mode: 320x240
 */
void displayTestRunning(const char* testName, const char* message, bool inProgress = true) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println(testName);
    tft.drawLine(0, 35, 320, 35, TFT_WHITE);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 50);

    // Word wrap the message
    int lineY = 50;
    char* msg = strdup(message);
    char* line = strtok(msg, "\n");
    while (line != NULL && lineY < 200) {
        tft.setCursor(10, lineY);
        tft.println(line);
        lineY += 15;
        line = strtok(NULL, "\n");
    }
    free(msg);

    if (inProgress) {
        // Show progress indicator
        int dotCount = (millis() / 500) % 4;
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setCursor(10, 220);
        tft.print("Testing");
        for (int i = 0; i < dotCount; i++) {
            tft.print(".");
        }
    }
}

/**
 * Show test results
 * Landscape mode: 320x240
 */
void displayTestResult(const char* testName, bool passed, const char* details = "") {
    tft.fillScreen(passed ? TFT_DARKGREEN : TFT_MAROON);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, passed ? TFT_DARKGREEN : TFT_MAROON);
    tft.setCursor(10, 20);
    tft.println(testName);

    tft.setTextSize(5);
    tft.setCursor(100, 80);
    if (passed) {
        tft.setTextColor(TFT_GREEN, TFT_DARKGREEN);
        tft.println("PASS");
    } else {
        tft.setTextColor(TFT_RED, TFT_MAROON);
        tft.println("FAIL");
    }

    if (strlen(details) > 0) {
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, passed ? TFT_DARKGREEN : TFT_MAROON);
        tft.setCursor(10, 160);
        tft.println(details);
    }

    tft.setTextSize(1);
    tft.setCursor(10, 220);
    tft.print("Press any button...");
}

/**
 * Wait for any button press (with debouncing)
 */
void waitForButtonPress() {
    Serial.println("[WAIT] Waiting for button press...");

    // First check what state the buttons are in
    int leftInitial = digitalRead(ENCODER_LEFT_SW_PIN);
    int rightInitial = digitalRead(ENCODER_RIGHT_SW_PIN);
    Serial.printf("[DEBUG] Initial button states - Left: %s, Right: %s\n",
                  leftInitial == HIGH ? "HIGH" : "LOW",
                  rightInitial == HIGH ? "HIGH" : "LOW");

    // Clear any pending button presses
    delay(500);

    unsigned long startTime = millis();
    int checkCount = 0;

    while (true) {
        int leftSW = digitalRead(ENCODER_LEFT_SW_PIN);
        int rightSW = digitalRead(ENCODER_RIGHT_SW_PIN);

        // Debug output every second
        if (checkCount % 100 == 0) {
            Serial.printf("[DEBUG] Buttons - Left: %s, Right: %s (%.1fs waiting)\n",
                          leftSW == HIGH ? "HIGH" : "LOW",
                          rightSW == HIGH ? "HIGH" : "LOW",
                          (millis() - startTime) / 1000.0);
        }
        checkCount++;

        // Try detecting button press (LOW = pressed for most encoders)
        if (leftSW == LOW) {
            Serial.println("[BUTTON] Left button detected as LOW (pressed)!");
            playTone(1500, 50);
            // Wait for release
            while (digitalRead(ENCODER_LEFT_SW_PIN) == LOW) {
                delay(10);
            }
            delay(200);  // Debounce
            return;
        }

        if (rightSW == LOW) {
            Serial.println("[BUTTON] Right button detected as LOW (pressed)!");
            playTone(1500, 50);
            // Wait for release
            while (digitalRead(ENCODER_RIGHT_SW_PIN) == LOW) {
                delay(10);
            }
            delay(200);  // Debounce
            return;
        }

        delay(10);
    }
}

/**
 * Run LED test
 */
void runLEDTest() {
    displayTestRunning("Status LEDs", "Testing Power,\nWiFi, and Error\nLEDs...");

    // Test each LED
    digitalWrite(LED_POWER_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_POWER_PIN, LOW);

    digitalWrite(LED_WIFI_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_WIFI_PIN, LOW);

    digitalWrite(LED_ERROR_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_ERROR_PIN, LOW);

    displayTestResult("Status LEDs", true, "All LEDs blinked");
    waitForButtonPress();
}

/**
 * Run buzzer test
 */
void runBuzzerTest() {
    displayTestRunning("Buzzer Test", "Playing test\ntones...");

    playTone(1000, 200);
    delay(300);
    playTone(1500, 500);
    delay(300);
    playTone(2000, 200);

    displayTestResult("Buzzer", true, "3 tones played");
    waitForButtonPress();
}

/**
 * Run left encoder test
 */
void runLeftEncoderTest() {
    displayTestRunning("Left Encoder", "Rotate left encoder\nPress to continue\n\n10 seconds...");

    int lastCLK = digitalRead(ENCODER_LEFT_CLK_PIN);
    int cwCount = 0, ccwCount = 0, pressCount = 0;
    unsigned long startTime = millis();
    unsigned long lastRotation = 0;

    while (millis() - startTime < 10000) {
        int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
        int dt = digitalRead(ENCODER_LEFT_DT_PIN);
        int sw = digitalRead(ENCODER_LEFT_SW_PIN);

        // Only detect on FALLING edge of CLK with debounce delay
        if (clk == LOW && lastCLK == HIGH) {
            // Debounce: ignore if less than 5ms since last rotation
            if (millis() - lastRotation > 5) {
                delay(1);  // Small delay to let DT settle
                dt = digitalRead(ENCODER_LEFT_DT_PIN);  // Re-read DT after settle

                if (dt == LOW) {
                    cwCount++;
                    Serial.printf("[LEFT ENC] Clockwise - CW:%d CCW:%d\n", cwCount, ccwCount);
                } else {
                    ccwCount++;
                    Serial.printf("[LEFT ENC] Counter-clockwise - CW:%d CCW:%d\n", cwCount, ccwCount);
                }

                lastRotation = millis();

                char msg[100];
                snprintf(msg, sizeof(msg), "Rotate left encoder\nCW: %d  CCW: %d\nPress: %d\n\n%d sec left",
                         cwCount, ccwCount, pressCount, (int)((10000 - (millis() - startTime)) / 1000));
                displayTestRunning("Left Encoder", msg);
            }
        }
        lastCLK = clk;

        static int lastSW = HIGH;
        if (sw != lastSW && sw == LOW) {
            pressCount++;
            Serial.printf("[LEFT ENC] Button pressed - Count:%d\n", pressCount);
            char msg[100];
            snprintf(msg, sizeof(msg), "Rotate left encoder\nCW: %d  CCW: %d\nPress: %d\n\n%d sec left",
                     cwCount, ccwCount, pressCount, (int)((10000 - (millis() - startTime)) / 1000));
            displayTestRunning("Left Encoder", msg);
        }
        lastSW = sw;

        delay(1);  // Small delay for stability
    }

    bool passed = (cwCount > 0 || ccwCount > 0 || pressCount > 0);
    char details[100];
    snprintf(details, sizeof(details), "CW:%d CCW:%d Press:%d", cwCount, ccwCount, pressCount);
    displayTestResult("Left Encoder", passed, details);
    waitForButtonPress();
}

/**
 * Run right encoder test
 */
void runRightEncoderTest() {
    displayTestRunning("Right Encoder", "Rotate right encoder\nPress to continue\n\n10 seconds...");

    int lastCLK = digitalRead(ENCODER_RIGHT_CLK_PIN);
    int cwCount = 0, ccwCount = 0, pressCount = 0;
    unsigned long startTime = millis();
    unsigned long lastRotation = 0;

    while (millis() - startTime < 10000) {
        int clk = digitalRead(ENCODER_RIGHT_CLK_PIN);
        int dt = digitalRead(ENCODER_RIGHT_DT_PIN);
        int sw = digitalRead(ENCODER_RIGHT_SW_PIN);

        // Only detect on FALLING edge of CLK with debounce delay
        if (clk == LOW && lastCLK == HIGH) {
            // Debounce: ignore if less than 5ms since last rotation
            if (millis() - lastRotation > 5) {
                delay(1);  // Small delay to let DT settle
                dt = digitalRead(ENCODER_RIGHT_DT_PIN);  // Re-read DT after settle

                if (dt == LOW) {
                    cwCount++;
                    Serial.printf("[RIGHT ENC] Clockwise - CW:%d CCW:%d\n", cwCount, ccwCount);
                } else {
                    ccwCount++;
                    Serial.printf("[RIGHT ENC] Counter-clockwise - CW:%d CCW:%d\n", cwCount, ccwCount);
                }

                lastRotation = millis();

                char msg[100];
                snprintf(msg, sizeof(msg), "Rotate right encoder\nCW: %d  CCW: %d\nPress: %d\n\n%d sec left",
                         cwCount, ccwCount, pressCount, (int)((10000 - (millis() - startTime)) / 1000));
                displayTestRunning("Right Encoder", msg);
            }
        }
        lastCLK = clk;

        static int lastSW = HIGH;
        if (sw != lastSW && sw == LOW) {
            pressCount++;
            Serial.printf("[RIGHT ENC] Button pressed - Count:%d\n", pressCount);
            char msg[100];
            snprintf(msg, sizeof(msg), "Rotate right encoder\nCW: %d  CCW: %d\nPress: %d\n\n%d sec left",
                     cwCount, ccwCount, pressCount, (int)((10000 - (millis() - startTime)) / 1000));
            displayTestRunning("Right Encoder", msg);
        }
        lastSW = sw;

        delay(1);  // Small delay for stability
    }

    bool passed = (cwCount > 0 || ccwCount > 0 || pressCount > 0);
    char details[100];
    snprintf(details, sizeof(details), "CW:%d CCW:%d Press:%d", cwCount, ccwCount, pressCount);
    displayTestResult("Right Encoder", passed, details);
    waitForButtonPress();
}

/**
 * Run emergency stop test
 */
void runEmergencyStopTest() {
    displayTestRunning("Emergency Stop", "Press BOTH encoder\nbuttons and hold\n0.5 seconds\n\n15 seconds...");

    unsigned long startTime = millis();
    unsigned long bothPressedStart = 0;
    bool triggered = false;

    while (millis() - startTime < 15000 && !triggered) {
        int leftSW = digitalRead(ENCODER_LEFT_SW_PIN);
        int rightSW = digitalRead(ENCODER_RIGHT_SW_PIN);

        if (leftSW == LOW && rightSW == LOW) {
            if (bothPressedStart == 0) {
                bothPressedStart = millis();
                displayTestRunning("Emergency Stop", "HOLDING...\nKeep holding!");
            }

            if (millis() - bothPressedStart >= 500) {
                triggered = true;
                playTone(2000, 1000);
            }
        } else {
            bothPressedStart = 0;
            char msg[100];
            snprintf(msg, sizeof(msg), "Press BOTH encoder\nbuttons and hold\n0.5 seconds\n\n%d sec left",
                     (int)((15000 - (millis() - startTime)) / 1000));
            displayTestRunning("Emergency Stop", msg);
        }

        delay(10);
    }

    displayTestResult("Emergency Stop", triggered, triggered ? "E-stop activated!" : "Not triggered");
    waitForButtonPress();
}

/**
 * Run SSR test
 */
void runSSRTest() {
    displayTestRunning("SSR Output", "WARNING:\nDO NOT connect\nto kiln!\n\nPulsing 3 times...");
    delay(2000);

    for (int i = 0; i < 3; i++) {
        digitalWrite(SSR_PIN, HIGH);
        char msg[100];
        snprintf(msg, sizeof(msg), "Pulse %d: ON\n\nCheck GPIO 25\nwith multimeter", i + 1);
        displayTestRunning("SSR Output", msg, false);
        delay(500);

        digitalWrite(SSR_PIN, LOW);
        delay(500);
    }

    displayTestResult("SSR Output", true, "3 pulses sent");
    waitForButtonPress();
}

/**
 * Run thermocouple test
 */
void runThermocoupleTest() {
    Serial.println("\n========================================");
    Serial.println("[TEST] Starting Thermocouple Test");
    Serial.println("========================================");

    int goodSamples = 0;
    int errorSamples = 0;
    double minTemp = 9999, maxTemp = -9999;
    double totalTemp = 0;
    double currentTemp = 0;

    unsigned long lastRead = 0;
    bool rightPressed = false;
    unsigned long rightPressTime = 0;
    bool exitTest = false;

    Serial.println("[INFO] Reading thermocouple every 500ms...");
    Serial.println("[INFO] To exit: Press RIGHT button, then LEFT button within 3 seconds");
    Serial.println();
    Serial.println("*** IMPORTANT: If temperature goes DOWN when heated, swap thermocouple wires! ***");
    Serial.println("    K-Type: YELLOW wire = NEGATIVE (-), RED wire = POSITIVE (+)");
    Serial.println();

    while (!exitTest) {
        // Read temperature every 500ms
        if (millis() - lastRead >= 500) {
            lastRead = millis();
            double tempC = thermocouple.readCelsius();

            Serial.printf("[DEBUG] Raw reading: %.1f°C, isnan=%d, valid range: %.1f to %.1f\n",
                          tempC, isnan(tempC), MIN_VALID_TEMP, MAX_VALID_TEMP);

            if (!isnan(tempC) && tempC > MIN_VALID_TEMP && tempC < MAX_VALID_TEMP) {
                // Convert to Fahrenheit
                currentTemp = (tempC * 9.0 / 5.0) + 32.0;

                goodSamples++;
                totalTemp += currentTemp;
                if (currentTemp < minTemp) minTemp = currentTemp;
                if (currentTemp > maxTemp) maxTemp = currentTemp;

                // Print to serial
                Serial.printf("[TEMP] %.1f°F (%.1f°C) | Min: %.1f°F, Max: %.1f°F, Avg: %.1f°F | Good: %d Errors: %d\n",
                              currentTemp, tempC, minTemp, maxTemp, totalTemp / goodSamples,
                              goodSamples, errorSamples);
            } else {
                errorSamples++;
                currentTemp = -999;  // Error indicator
                Serial.printf("[ERROR] Bad reading! (NaN or out of range) - Errors: %d\n", errorSamples);
            }

            Serial.println("[DEBUG] About to update TFT display...");

            // Update display ONLY when we have new temperature data
            // Landscape mode: 320x240
            Serial.println("[DEBUG] Clearing TFT screen...");
            tft.fillScreen(TFT_BLACK);

            Serial.println("[DEBUG] Drawing header...");
            tft.setTextSize(2);
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.setCursor(10, 10);
            tft.println("Thermocouple Test");
            tft.drawLine(0, 35, 320, 35, TFT_WHITE);

            // Current reading - LARGE (left side)
            Serial.printf("[DEBUG] Drawing temperature: %.1f°F\n", currentTemp);
            tft.setTextSize(4);
            if (currentTemp == -999) {
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.setCursor(10, 60);
                tft.print("ERROR");
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(10, 60);
                char tempStr[20];
                snprintf(tempStr, sizeof(tempStr), "%.1f", currentTemp);
                tft.print(tempStr);
                tft.setTextSize(2);
                tft.print(" F");
            }
            Serial.println("[DEBUG] Temperature drawn.");

            // Statistics (right side)
            tft.setTextSize(1);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setCursor(10, 120);
            tft.print("Min: ");
            if (minTemp < 9999) {
                tft.printf("%.1f F", minTemp);
            } else {
                tft.print("---");
            }

            tft.setCursor(120, 120);
            tft.print("Max: ");
            if (maxTemp > -9999) {
                tft.printf("%.1f F", maxTemp);
            } else {
                tft.print("---");
            }

            if (goodSamples > 0) {
                double avgTemp = totalTemp / goodSamples;
                tft.setCursor(10, 140);
                tft.print("Avg: ");
                tft.printf("%.1f F", avgTemp);
            }

            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setCursor(10, 160);
            tft.printf("Good: %d  Errors: %d", goodSamples, errorSamples);

            // Exit instructions
            tft.drawLine(0, 190, 320, 190, TFT_DARKGREY);
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);
            tft.setCursor(10, 200);
            if (rightPressed) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.print("To exit: Now press LEFT!");
            } else {
                tft.setTextColor(TFT_ORANGE, TFT_BLACK);
                tft.print("To exit: 1. Press RIGHT  2. Press LEFT");
            }

            Serial.println("[DEBUG] TFT display update complete!");
            Serial.println();
        }  // End of temperature reading and display update block

        // Check for exit sequence
        int rightSW = digitalRead(ENCODER_RIGHT_SW_PIN);
        int leftSW = digitalRead(ENCODER_LEFT_SW_PIN);

        // Step 1: Press right button
        if (!rightPressed && rightSW == LOW) {
            rightPressed = true;
            rightPressTime = millis();
            playTone(1500, 50);
        }

        // Step 2: Press left button within 3 seconds
        if (rightPressed) {
            if (leftSW == LOW) {
                exitTest = true;
                playTone(2000, 100);
            }

            // Timeout after 3 seconds
            if (millis() - rightPressTime > 3000) {
                rightPressed = false;
            }
        }

        delay(50);
    }

    // Show final results
    bool passed = (goodSamples > 5 && errorSamples == 0);
    char details[100];
    if (goodSamples > 0) {
        double avgTemp = totalTemp / goodSamples;
        snprintf(details, sizeof(details), "Avg: %.1fC\nRange: %.1f-%.1fC\nGood: %d Errors: %d",
                 avgTemp, minTemp, maxTemp, goodSamples, errorSamples);
    } else {
        snprintf(details, sizeof(details), "No valid readings!");
    }

    displayTestResult("Thermocouple", passed, details);
    waitForButtonPress();
}

/**
 * Run display test
 * Landscape mode: 320x240
 */
void runDisplayTest() {
    // Colors
    tft.fillScreen(TFT_RED);
    delay(500);
    tft.fillScreen(TFT_GREEN);
    delay(500);
    tft.fillScreen(TFT_BLUE);
    delay(500);

    // Graphics - adjusted for landscape 320x240
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(20, 20, 280, 200, TFT_WHITE);
    tft.fillCircle(160, 120, 50, TFT_YELLOW);
    tft.drawLine(20, 20, 300, 220, TFT_RED);
    tft.drawLine(300, 20, 20, 220, TFT_GREEN);
    delay(2000);

    displayTestResult("TFT Display", true, "Colors & graphics\ndisplayed");
    waitForButtonPress();
}

/**
 * Handle test mode navigation
 */
void handleTestModeInput() {
    int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
    static int lastCLK = HIGH;

    // Navigate menu - only on FALLING edge to prevent double-counting
    if (clk != lastCLK && clk == LOW && !testState.testRunning) {
        int dt = digitalRead(ENCODER_LEFT_DT_PIN);
        if (dt == LOW) {
            // Clockwise - down
            testState.menuSelection++;
            if (testState.menuSelection >= numTestMenuItems) {
                testState.menuSelection = 0;
            }
        } else {
            // Counter-clockwise - up
            testState.menuSelection--;
            if (testState.menuSelection < 0) {
                testState.menuSelection = numTestMenuItems - 1;
            }
        }
        displayTestMenu();
        playTone(1200, 20);
    }
    lastCLK = clk;

    // Select menu item
    int sw = digitalRead(ENCODER_LEFT_SW_PIN);
    static int lastSW = HIGH;
    static unsigned long lastPress = 0;

    if (sw != lastSW && sw == LOW && !testState.testRunning) {
        if (millis() - lastPress > 200) {
            lastPress = millis();
            playTone(2000, 50);

            // Execute selected test
            switch (testState.menuSelection) {
                case 0: // Run all tests
                    runLEDTest();
                    runBuzzerTest();
                    runLeftEncoderTest();
                    runRightEncoderTest();
                    runEmergencyStopTest();
                    runSSRTest();
                    runThermocoupleTest();
                    runDisplayTest();
                    displayTestMenu();
                    break;
                case 1: runLEDTest(); displayTestMenu(); break;
                case 2: runBuzzerTest(); displayTestMenu(); break;
                case 3: runLeftEncoderTest(); displayTestMenu(); break;
                case 4: runRightEncoderTest(); displayTestMenu(); break;
                case 5: runEmergencyStopTest(); displayTestMenu(); break;
                case 6: runSSRTest(); displayTestMenu(); break;
                case 7: runThermocoupleTest(); displayTestMenu(); break;
                case 8: runDisplayTest(); displayTestMenu(); break;
                case 9: // Exit
                    state.mode = MODE_MAIN_MENU;
                    displayMainMenu();
                    break;
            }
        }
    }
    lastSW = sw;
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

/**
 * Update TFT display with current system state
 * Landscape mode: 320x240
 */
void updateDisplay() {
    // Clear screen
    tft.fillScreen(TFT_BLACK);

    // Header - Show current mode
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("MANUAL CONTROL");

    // Heating indicator
    if (state.heating) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(230, 10);
        tft.print("HEAT");
    }

    tft.drawLine(0, 35, 320, 35, TFT_WHITE);

    // Current Temperature (large)
    tft.setTextSize(6);
    char tempStr[20];
    if (state.sensorError) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(20, 70);
        tft.setTextSize(3);
        tft.print("SENSOR ERROR!");
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(20, 70);
        snprintf(tempStr, sizeof(tempStr), "%.1f", state.currentTemp);
        tft.print(tempStr);

        // Degree symbol and C
        tft.setTextSize(3);
        tft.drawCircle(280, 80, 8, TFT_WHITE);
        tft.setCursor(275, 100);
        tft.print("C");
    }

    // Target Temperature
    tft.drawLine(0, 150, 320, 150, TFT_DARKGREY);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 165);
    tft.print("Target: ");
    snprintf(tempStr, sizeof(tempStr), "%.0f", state.targetTemp);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(tempStr);
    tft.drawCircle(210, 172, 5, TFT_WHITE);
    tft.print("C");

    // PID Output
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 190);
    tft.printf("PID Output: %.1f%%", pidOutput);

    // Instructions
    tft.drawLine(0, 200, 320, 200, TFT_DARKGREY);
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setCursor(10, 210);
    tft.print("L Press: Menu    R Turn: Setpoint");
    tft.setCursor(10, 225);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.print("Both Hold: Emergency Stop");
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * Handle left encoder (back to menu / mode selection)
 */
void handleLeftEncoder() {
    int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
    int dt = digitalRead(ENCODER_LEFT_DT_PIN);
    int sw = digitalRead(ENCODER_LEFT_SW_PIN);

    // Button press detection - back to main menu
    if (sw != leftEncoder.lastSW && sw == LOW) {
        unsigned long now = millis();
        if (now - leftEncoder.lastButtonPress > DEBOUNCE_MS) {
            DEBUG_PRINTLN("[LEFT] Button pressed - returning to main menu");

            // Safety: Turn off heating when returning to menu
            digitalWrite(SSR_PIN, LOW);
            state.heating = false;

            state.mode = MODE_MAIN_MENU;
            displayMainMenu();
            playTone(2000, 30);
            leftEncoder.lastButtonPress = now;
        }
    }
    leftEncoder.lastSW = sw;
}

/**
 * Handle right encoder (setpoint adjustment)
 */
void handleRightEncoder() {
    int clk = digitalRead(ENCODER_RIGHT_CLK_PIN);
    int dt = digitalRead(ENCODER_RIGHT_DT_PIN);
    int sw = digitalRead(ENCODER_RIGHT_SW_PIN);

    // Rotation detection
    if (clk != rightEncoder.lastCLK) {
        if (dt != clk) {
            // Clockwise - increase setpoint
            state.targetTemp += 5.0;
            // SAFETY: Enforce hard temperature limit from config.h
            if (state.targetTemp > MAX_TEMP_LIMIT) {
                state.targetTemp = MAX_TEMP_LIMIT;
            }
            DEBUG_PRINT("[SETPOINT] Increased to: ");
            DEBUG_PRINTLN(state.targetTemp);
        } else {
            // Counter-clockwise - decrease setpoint
            state.targetTemp -= 5.0;
            if (state.targetTemp < 0.0) state.targetTemp = 0.0;
            DEBUG_PRINT("[SETPOINT] Decreased to: ");
            DEBUG_PRINTLN(state.targetTemp);
        }
        playTone(1200, 20);  // Quick beep
        rightEncoder.lastCLK = clk;
    }

    // Button press detection (for future use)
    if (sw != rightEncoder.lastSW && sw == LOW) {
        unsigned long now = millis();
        if (now - rightEncoder.lastButtonPress > DEBOUNCE_MS) {
            DEBUG_PRINTLN("[RIGHT] Button pressed");
            playTone(2000, 30);
            rightEncoder.lastButtonPress = now;
        }
    }
    rightEncoder.lastSW = sw;
}

// ============================================================================
// EMERGENCY STOP
// ============================================================================

/**
 * Check for emergency stop (both encoder buttons pressed simultaneously)
 */
void checkEmergencyStop() {
    static unsigned long bothPressedStart = 0;
    static bool wasTriggered = false;

    int leftSW = digitalRead(ENCODER_LEFT_SW_PIN);
    int rightSW = digitalRead(ENCODER_RIGHT_SW_PIN);

    // Both buttons pressed
    if (leftSW == LOW && rightSW == LOW) {
        if (bothPressedStart == 0) {
            bothPressedStart = millis();
        }

        // Check if held for required time
        if (millis() - bothPressedStart >= EMERGENCY_STOP_HOLD_TIME_MS && !wasTriggered) {
            // EMERGENCY STOP!
            digitalWrite(SSR_PIN, LOW);
            state.mode = MODE_IDLE;
            state.heating = false;
            wasTriggered = true;

            DEBUG_PRINTLN("*** EMERGENCY STOP ACTIVATED ***");
            playTone(2000, 1000);  // Long alarm
        }
    } else {
        // Reset
        bothPressedStart = 0;
        wasTriggered = false;
    }
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("ESP32 Kiln Controller v1.0");
    Serial.println("Milestone 1.7: Integration Testing");
    Serial.println("========================================");
    Serial.println();

    // Initialize SSR pin (SAFETY: Default to OFF)
    pinMode(SSR_PIN, OUTPUT);
    digitalWrite(SSR_PIN, LOW);
    Serial.println("[SAFETY] SSR initialized to OFF state");

    // Initialize status LEDs
    pinMode(LED_POWER_PIN, OUTPUT);
    pinMode(LED_WIFI_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
    digitalWrite(LED_POWER_PIN, HIGH);  // Power LED on
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

    // Initialize encoder state
    leftEncoder.lastCLK = digitalRead(ENCODER_LEFT_CLK_PIN);
    rightEncoder.lastCLK = digitalRead(ENCODER_RIGHT_CLK_PIN);

    Serial.println("[OK] GPIO pins initialized");

    // Initialize test state
    initTestState();

    // Initialize PID controller
    kilnPID.SetOutputLimits(0, 100);  // Output is 0-100%
    kilnPID.SetSampleTime(PID_SAMPLE_TIME);  // 1000ms from config.h
    kilnPID.SetMode(MANUAL);  // Start in manual mode (off)
    pidOutput = 0;
    ssrWindowStartTime = millis();
    Serial.println("[OK] PID controller initialized (Kp=5.0, Ki=0.5, Kd=1.0)");

    // Initialize SPI for shared bus (MAX31855 thermocouple uses software SPI)
    SPI.begin();
    Serial.println("[INFO] SPI bus initialized");

    // Initialize thermocouple (software SPI)
    delay(500);  // Give MAX31855 time to stabilize
    Serial.println("[OK] MAX31855 thermocouple initialized (software SPI)");

    // Initialize TFT display
    tft.init();
    tft.setRotation(1);  // Landscape mode (320x240)
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(60, 60);
    tft.println("Kiln Controller");
    tft.setTextSize(1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 140);
    tft.println("Initializing...");

    Serial.println("[OK] TFT display initialized");

    delay(2000);  // Show splash screen

    // Start with main menu
    Serial.println();
    Serial.println("========================================");
    Serial.println("MAIN MENU");
    Serial.println("========================================");
    Serial.println();

    state.mode = MODE_MAIN_MENU;
    playTone(1500, 100);

    Serial.println("[MODE] Starting at MAIN MENU");
    Serial.println();
    Serial.println("Use LEFT encoder to navigate menu");
    Serial.println("Press LEFT encoder button to select");
    Serial.println();

    displayMainMenu();

    state.lastTempRead = millis();
    state.lastDisplayUpdate = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long now = millis();

    // Handle main menu
    if (state.mode == MODE_MAIN_MENU) {
        handleMainMenuInput();
        delay(10);
        return;
    }

    // Handle test mode
    if (state.mode == MODE_TEST) {
        handleTestModeInput();
        delay(10);
        return;
    }

    // Handle manual control mode

    // Read temperature (every 100ms)
    if (now - state.lastTempRead >= 100) {
        state.lastTempRead = now;

        if (readTemperature()) {
            // Update error LED based on sensor state
            digitalWrite(LED_ERROR_PIN, LOW);
        } else {
            digitalWrite(LED_ERROR_PIN, HIGH);
        }

        // Update SSR control
        updateSSRControl();

        // Update heating LED
        digitalWrite(LED_WIFI_PIN, state.heating ? HIGH : LOW);
    }

    // Update display (every 250ms)
    if (now - state.lastDisplayUpdate >= 250) {
        state.lastDisplayUpdate = now;
        updateDisplay();
    }

    // Handle user input (every loop iteration with debouncing)
    handleLeftEncoder();
    handleRightEncoder();
    checkEmergencyStop();

    // Print status to serial (every 2 seconds)
    static unsigned long lastSerialPrint = 0;
    if (now - lastSerialPrint >= 2000) {
        lastSerialPrint = now;

        Serial.print("[STATUS] Mode: ");
        Serial.print(state.mode == MODE_IDLE ? "IDLE" : "MANUAL");
        Serial.print(" | Temp: ");
        Serial.print(state.currentTemp);
        Serial.print("°C | Target: ");
        Serial.print(state.targetTemp);
        Serial.print("°C | Heating: ");
        Serial.println(state.heating ? "YES" : "NO");
    }

    delay(10);  // Small delay to prevent watchdog issues
}
