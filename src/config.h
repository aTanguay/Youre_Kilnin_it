#ifndef CONFIG_H
#define CONFIG_H

// ESP32 Kiln Controller Configuration
// Pin definitions, constants, and feature flags

// ============================================================================
// GPIO PIN DEFINITIONS
// ============================================================================

// SPI pins for MAX31855 (Thermocouple)
#define MAX31855_CS_PIN     5
#define MAX31855_CLK_PIN    18
#define MAX31855_MISO_PIN   19

// SSR Control
#define SSR_PIN             25

// I2C pins for OLED Display (SSD1306)
#define OLED_SDA_PIN        21
#define OLED_SCL_PIN        22

// Rotary Encoder
#define ENCODER_CLK_PIN     32
#define ENCODER_DT_PIN      33
#define ENCODER_SW_PIN      34  // Input-only pin

// Additional Buttons
#define BUTTON_SECONDARY    35  // Input-only pin

// Emergency Stop (NC switch)
#define EMERGENCY_STOP_PIN  36  // Input-only pin (VP)

// Piezo Buzzer
#define BUZZER_PIN          26

// Status LEDs
#define LED_POWER_PIN       27
#define LED_WIFI_PIN        14
#define LED_ERROR_PIN       12

// ============================================================================
// SYSTEM CONSTANTS
// ============================================================================

// Temperature limits
#define MAX_TEMP_LIMIT      1320.0  // Maximum temperature (°C) - HARD CODED
#define MIN_VALID_TEMP      -50.0   // Minimum valid temperature reading
#define MAX_VALID_TEMP      1400.0  // Maximum valid temperature reading
#define MAX_RAMP_RATE       600.0   // Maximum ramp rate (°C/hour)
#define TEMP_ERROR_VALUE    -999.0  // Error indicator value

// PID defaults
#define DEFAULT_KP          5.0     // Proportional gain
#define DEFAULT_KI          0.5     // Integral gain
#define DEFAULT_KD          1.0     // Derivative gain
#define PID_SAMPLE_TIME     1000    // PID sample time (ms)

// SSR control
#define SSR_CYCLE_TIME_MS   2000    // SSR cycle time (2 seconds)

// Safety timing
#define MAX_FIRING_DURATION (48UL * 60UL * 60UL * 1000UL)  // 48 hours in ms
#define SAFETY_CHECK_INTERVAL_MS    500   // Safety check every 500ms
#define WATCHDOG_TIMEOUT_SEC        10    // Watchdog timeout

// Task timing
#define TEMP_READ_INTERVAL_MS       100   // Temperature reading every 100ms
#define PID_UPDATE_INTERVAL_MS      1000  // PID update every 1 second
#define DISPLAY_UPDATE_INTERVAL_MS  250   // Display update every 250ms
#define INPUT_CHECK_INTERVAL_MS     50    // Input check every 50ms
#define ENERGY_UPDATE_INTERVAL_MS   1000  // Energy tracking every 1 second

// Display
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_ADDRESS        0x3C

// Serial
#define SERIAL_BAUD_RATE    115200

// WiFi
#define WIFI_AP_SSID_PREFIX "KilnController"
#define WIFI_AP_PASSWORD    "kiln1234"
#define WEB_SERVER_PORT     80

// Energy defaults
#define DEFAULT_KILN_WATTAGE        1800   // Watts
#define DEFAULT_ELECTRICITY_RATE    0.12   // $/kWh
#define DEFAULT_CURRENCY_SYMBOL     "$"

// ============================================================================
// FEATURE FLAGS
// ============================================================================

#define ENABLE_WIFI         true
#define ENABLE_WEB_SERVER   true
#define ENABLE_COST_TRACKING true
#define ENABLE_DATA_LOGGING true
#define ENABLE_DEBUG_OUTPUT true

// ============================================================================
// MACROS
// ============================================================================

#if ENABLE_DEBUG_OUTPUT
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
