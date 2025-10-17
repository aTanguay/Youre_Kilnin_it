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
// Note: SPI.h not needed - using software SPI for both MAX31855 and LCD
#include <Adafruit_MAX31855.h>
#include <U8g2lib.h>

// ============================================================================
// HARDWARE OBJECTS
// ============================================================================

// Thermocouple using SOFTWARE SPI
// Constructor: Adafruit_MAX31855(SCLK, CS, MISO)
// We use software SPI to match the LCD's software SPI implementation
// Pins: CLK=18, CS=5, MISO=19
Adafruit_MAX31855 thermocouple(THERMOCOUPLE_CLK, THERMOCOUPLE_CS, THERMOCOUPLE_MISO);

// LCD Display using SOFTWARE SPI (ST7920)
// U8g2 constructor order: (rotation, SCK, MOSI, CS)
// Pins: SCK=18 (shared clock), MOSI=23, CS=15
U8G2_ST7920_128X64_F_SW_SPI display(U8G2_R0, LCD_SCK, LCD_MOSI, LCD_CS);

// ============================================================================
// SYSTEM STATE
// ============================================================================

enum SystemMode {
    MODE_IDLE,      // System idle, not heating
    MODE_MANUAL     // Manual heating mode (simple on/off)
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
 * Simple on/off control based on temperature
 * Implements basic hysteresis to prevent rapid cycling
 */
void updateSSRControl() {
    const float HYSTERESIS = 2.0;  // °C of hysteresis

    // SAFETY: Don't heat if sensor error or in idle mode
    if (state.sensorError || state.mode == MODE_IDLE) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        return;
    }

    // SAFETY: Don't heat if target exceeds hard limit
    if (state.targetTemp > MAX_TEMP_LIMIT) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        DEBUG_PRINTLN("[SAFETY] Target exceeds MAX_TEMP_LIMIT - heating disabled");
        return;
    }

    // SAFETY: Don't heat if current temperature exceeds hard limit
    if (state.currentTemp >= MAX_TEMP_LIMIT) {
        digitalWrite(SSR_PIN, LOW);
        state.heating = false;
        DEBUG_PRINTLN("[SAFETY] Current temp at/above MAX_TEMP_LIMIT - heating disabled");
        return;
    }

    // Simple on/off control with hysteresis
    if (state.currentTemp < (state.targetTemp - HYSTERESIS)) {
        // Too cold, turn on heater
        if (!state.heating) {
            state.heating = true;
            state.heatingStartTime = millis();
            digitalWrite(SSR_PIN, HIGH);
            DEBUG_PRINTLN("[SSR] Heating ON");
        }
    } else if (state.currentTemp > state.targetTemp) {
        // At or above target, turn off heater
        if (state.heating) {
            state.heating = false;
            digitalWrite(SSR_PIN, LOW);
            DEBUG_PRINTLN("[SSR] Heating OFF");
        }
    }
    // If temperature is between (target - hysteresis) and target, maintain current state
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

/**
 * Update LCD with current system state
 */
void updateDisplay() {
    display.clearBuffer();

    // Header - Show current mode
    display.setFont(u8g2_font_ncenB08_tr);
    if (state.mode == MODE_IDLE) {
        display.drawStr(0, 10, "IDLE");
    } else {
        display.drawStr(0, 10, "MANUAL");
    }

    // Heating indicator
    if (state.heating) {
        display.drawStr(90, 10, "HEATING");
    }

    display.drawLine(0, 12, 127, 12);

    // Current Temperature (large)
    display.setFont(u8g2_font_ncenB14_tr);
    char tempStr[20];
    if (state.sensorError) {
        snprintf(tempStr, sizeof(tempStr), "ERROR!");
    } else {
        snprintf(tempStr, sizeof(tempStr), "%.1f", state.currentTemp);
    }
    display.drawStr(5, 32, tempStr);

    // Degree symbol and C
    display.setFont(u8g2_font_ncenB08_tr);
    if (!state.sensorError) {
        display.drawCircle(85, 25, 3);
        display.drawStr(90, 32, "C");
    }

    // Target Temperature
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(5, 45, "Target:");
    snprintf(tempStr, sizeof(tempStr), "%.0f", state.targetTemp);
    display.drawStr(45, 45, tempStr);
    display.drawCircle(85, 39, 2);
    display.drawStr(88, 45, "C");

    // Instructions
    display.drawLine(0, 48, 127, 48);
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(0, 58, "L:Mode R:Setpoint");

    display.sendBuffer();
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

/**
 * Handle left encoder (mode selection)
 */
void handleLeftEncoder() {
    int clk = digitalRead(ENCODER_LEFT_CLK_PIN);
    int dt = digitalRead(ENCODER_LEFT_DT_PIN);
    int sw = digitalRead(ENCODER_LEFT_SW_PIN);

    // Rotation detection
    if (clk != leftEncoder.lastCLK) {
        // Encoder turned - toggle mode
        if (state.mode == MODE_IDLE) {
            state.mode = MODE_MANUAL;
            DEBUG_PRINTLN("[MODE] Switched to MANUAL");
            playTone(1500, 50);
        } else {
            state.mode = MODE_IDLE;
            DEBUG_PRINTLN("[MODE] Switched to IDLE");
            playTone(1000, 50);
        }
        leftEncoder.lastCLK = clk;
    }

    // Button press detection (for future use)
    if (sw != leftEncoder.lastSW && sw == LOW) {
        unsigned long now = millis();
        if (now - leftEncoder.lastButtonPress > DEBOUNCE_MS) {
            DEBUG_PRINTLN("[LEFT] Button pressed");
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

    // NOTE: Both MAX31855 and LCD are using SOFTWARE SPI (bit-banging)
    // Hardware SPI.begin() is not needed since each device controls its own pins
    // If we later switch to hardware SPI, uncomment and configure:
    // SPI.begin(LCD_SCK_PIN, THERMOCOUPLE_MISO_PIN, LCD_MOSI_PIN, -1);
    Serial.println("[INFO] Using software SPI for all devices");

    // Initialize thermocouple (software SPI)
    delay(500);  // Give MAX31855 time to stabilize
    Serial.println("[OK] MAX31855 thermocouple initialized (software SPI)");

    // Initialize LCD display
    display.begin();
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB10_tr);
    display.drawStr(10, 30, "Kiln Controller");
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(20, 45, "Initializing...");
    display.sendBuffer();
    Serial.println("[OK] LCD display initialized");

    delay(2000);  // Show splash screen

    // Startup beep sequence
    playTone(1000, 100);
    delay(50);
    playTone(1500, 100);

    Serial.println();
    Serial.println("========================================");
    Serial.println("System Ready");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Controls:");
    Serial.println("  Left Encoder:  Toggle IDLE/MANUAL mode");
    Serial.println("  Right Encoder: Adjust target temperature");
    Serial.println("  Both Buttons:  Emergency stop (hold 0.5s)");
    Serial.println();

    state.lastTempRead = millis();
    state.lastDisplayUpdate = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    unsigned long now = millis();

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
