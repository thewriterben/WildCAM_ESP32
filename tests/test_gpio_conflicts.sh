#!/bin/bash
# Test script to verify GPIO pin conflict compile-time checks
# This script tests various feature combinations to ensure conflicts are detected

echo "=========================================="
echo "GPIO Pin Conflict Compile-Time Check Test"
echo "=========================================="
echo ""

TEST_DIR="/tmp/gpio_conflict_tests"
mkdir -p "$TEST_DIR"

# Create a minimal test file that includes config.h
create_test_file() {
    local test_name=$1
    local defines=$2
    
    cat > "$TEST_DIR/${test_name}.cpp" << EOF
// Test: ${test_name}
// Defines: ${defines}

${defines}

#include "/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/config.h"

// Minimal main to satisfy compiler
void setup() {}
void loop() {}
EOF
}

# Function to test a configuration
test_config() {
    local test_name=$1
    local defines=$2
    local should_fail=$3
    
    echo "Testing: $test_name"
    echo "  Defines: $defines"
    
    create_test_file "$test_name" "$defines"
    
    # Try to compile (just preprocessor check)
    g++ -E -x c++ -std=c++11 \
        -I/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware \
        -I/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/include \
        "$TEST_DIR/${test_name}.cpp" > /dev/null 2>&1
    
    local result=$?
    
    if [ $should_fail -eq 1 ]; then
        if [ $result -ne 0 ]; then
            echo "  ✅ PASS - Correctly detected conflict"
        else
            echo "  ❌ FAIL - Should have detected conflict but didn't"
            return 1
        fi
    else
        if [ $result -eq 0 ]; then
            echo "  ✅ PASS - No conflict detected (as expected)"
        else
            echo "  ❌ FAIL - Unexpected compilation error"
            return 1
        fi
    fi
    
    echo ""
    return 0
}

echo "Test 1: Camera + LoRa on AI-Thinker (should FAIL)"
test_config "test1_camera_lora_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define LORA_ENABLED true" \
    1

echo "Test 2: Camera only on AI-Thinker (should PASS)"
test_config "test2_camera_only" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define LORA_ENABLED false" \
    0

echo "Test 3: Camera + WiFi on AI-Thinker (should PASS)"
test_config "test3_camera_wifi" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define LORA_ENABLED false
#define WIFI_ENABLED true" \
    0

echo "Test 4: Solar voltage monitoring on GPIO 34 with camera (should FAIL)"
test_config "test4_solar_voltage_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define VOLTAGE_MONITORING_ENABLED true
#define SOLAR_VOLTAGE_PIN 34" \
    1

echo "Test 5: Battery voltage monitoring on GPIO 34 with camera (should FAIL)"
test_config "test5_battery_voltage_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define VOLTAGE_MONITORING_ENABLED true
#define BATTERY_VOLTAGE_PIN 34" \
    1

echo "Test 6: SD card + LED on GPIO 2 (should FAIL)"
test_config "test6_sd_led_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define SD_CARD_ENABLED true
#define CHARGING_LED_PIN 2" \
    1

echo "Test 7: IR LED + LoRa on same pin (should FAIL)"
test_config "test7_ir_lora_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define IR_LED_ENABLED true
#define LORA_ENABLED true
#define IR_LED_PIN 16
#define LORA_CS_PIN 16" \
    1

echo "Test 8: Audio + Camera on AI-Thinker (should FAIL)"
test_config "test8_audio_camera_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define CAMERA_ENABLED true
#define AUDIO_CLASSIFICATION_ENABLED true
#define I2S_SD_PIN 22
#define I2S_WS_PIN 25
#define I2S_SCK_PIN 26" \
    1

echo "Test 9: Servo + LoRa on AI-Thinker (should FAIL)"
test_config "test9_servo_lora_conflict" \
    "#define CAMERA_MODEL_AI_THINKER
#define SERVO_CONTROL_ENABLED true
#define LORA_ENABLED true" \
    1

echo "Test 10: Camera + LoRa on ESP32-S3 (should PASS)"
test_config "test10_s3_no_conflict" \
    "#define CAMERA_MODEL_ESP32_S3_CAM
#define CAMERA_ENABLED true
#define LORA_ENABLED true" \
    0

echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo ""
echo "All compile-time conflict checks tested."
echo "Review output above for any failures."
echo ""
echo "Note: This test validates that the #error directives"
echo "in firmware/config.h are working correctly."
echo ""
echo "For detailed GPIO conflict information, see:"
echo "  docs/GPIO_PIN_CONFLICTS.md"
echo "  docs/HARDWARE_SELECTION_GUIDE.md"
echo ""
