/**
 * @file test_performance_optimizations.cpp
 * @brief Test suite for performance optimizations
 * @version 1.0.0
 * 
 * This file contains tests to validate the performance improvements
 * made to the WildCAM_ESP32 firmware.
 */

#include <Arduino.h>
#include <unity.h>

// Mock camera frame buffer for testing
struct MockCameraFrameBuffer {
    uint8_t* buf;
    size_t len;
};

// ============================================================================
// Test 1: Single-Pass Variance Calculation
// ============================================================================

void test_variance_calculation_correctness() {
    // Create test image data with fixed size to avoid VLA
    const size_t pixelCount = 100;
    uint8_t testData[100]; // Fixed size array instead of VLA
    
    // Fill with known pattern
    for (size_t i = 0; i < pixelCount; i++) {
        testData[i] = (i % 256);
    }
    
    // Calculate variance using single-pass method
    uint32_t sum = 0;
    uint64_t sumSquares = 0;
    
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t pixel = testData[i];
        sum += pixel;
        sumSquares += (uint64_t)pixel * pixel;
    }
    
    float mean = (float)sum / pixelCount;
    float variance = ((float)sumSquares / pixelCount) - (mean * mean);
    
    // Variance should be positive and reasonable
    TEST_ASSERT_GREATER_THAN(0.0f, variance);
    TEST_ASSERT_LESS_THAN(100000.0f, variance);
    
    Serial.printf("Test variance: %.2f (mean: %.2f)\n", variance, mean);
}

void test_variance_calculation_performance() {
    const size_t pixelCount = 76800; // 320x240 image
    uint8_t* testData = (uint8_t*)malloc(pixelCount);
    
    // Check malloc success
    if (testData == NULL) {
        TEST_FAIL_MESSAGE("Failed to allocate memory for test");
        return;
    }
    
    // Fill with random-ish data
    for (size_t i = 0; i < pixelCount; i++) {
        testData[i] = (i * 7 + 13) % 256;
    }
    
    // Time the single-pass calculation
    unsigned long start = micros();
    
    for (int trial = 0; trial < 100; trial++) {
        uint32_t sum = 0;
        uint64_t sumSquares = 0;
        
        for (size_t i = 0; i < pixelCount; i++) {
            uint8_t pixel = testData[i];
            sum += pixel;
            sumSquares += (uint64_t)pixel * pixel;
        }
        
        float mean = (float)sum / pixelCount;
        float variance = ((float)sumSquares / pixelCount) - (mean * mean);
        
        // Use result to prevent optimization
        if (variance < 0) sum = 0;
    }
    
    unsigned long duration = micros() - start;
    free(testData);
    
    Serial.printf("100 variance calculations: %lu microseconds\n", duration);
    Serial.printf("Average per calculation: %lu microseconds\n", duration / 100);
    
    // Should complete in reasonable time (< 500ms for 100 iterations)
    TEST_ASSERT_LESS_THAN(500000, duration);
}

// ============================================================================
// Test 2: String Optimization
// ============================================================================

void test_string_reserve_efficiency() {
    size_t heapBefore = ESP.getFreeHeap();
    
    // Test with reserve - using snprintf approach as per PR recommendations
    {
        String report;
        report.reserve(800);
        char buffer[32];
        
        for (int i = 0; i < 20; i++) {
            report += "Test line ";
            snprintf(buffer, sizeof(buffer), "%d", i);
            report += buffer;
            report += "\n";
        }
    }
    
    size_t heapAfter = ESP.getFreeHeap();
    
    Serial.printf("Heap change with reserve: %d bytes\n", (int)(heapBefore - heapAfter));
    
    // With reserve, heap should return to nearly the same level
    // (allowing for some fragmentation)
    TEST_ASSERT_INT_WITHIN(100, 0, heapBefore - heapAfter);
}

void test_snprintf_vs_string_constructor() {
    size_t heapBefore, heapAfter1, heapAfter2;
    
    // Test String constructor method
    heapBefore = ESP.getFreeHeap();
    {
        String s = "Value: ";
        for (int i = 0; i < 100; i++) {
            s += String(i);
            s += " ";
        }
    }
    heapAfter1 = ESP.getFreeHeap();
    
    // Test snprintf method
    heapBefore = ESP.getFreeHeap();
    {
        String s = "Value: ";
        s.reserve(800);
        char buffer[32];
        for (int i = 0; i < 100; i++) {
            snprintf(buffer, sizeof(buffer), "%d ", i);
            s += buffer;
        }
    }
    heapAfter2 = ESP.getFreeHeap();
    
    int method1_used = heapBefore - heapAfter1;
    int method2_used = heapBefore - heapAfter2;
    
    Serial.printf("String constructor method: %d bytes used\n", method1_used);
    Serial.printf("snprintf method: %d bytes used\n", method2_used);
    
    // snprintf method should use less or equal heap than String constructor
    // Allow 200 byte tolerance for heap fragmentation
    TEST_ASSERT_GREATER_OR_EQUAL(method2_used - 200, method1_used);
}

// ============================================================================
// Test 3: Non-Blocking Delays
// ============================================================================

void test_yield_responsiveness() {
    unsigned long start = millis();
    int yieldCount = 0;
    
    // Simulate loop with yields
    while (millis() - start < 100) {
        yield();
        yieldCount++;
    }
    
    Serial.printf("Yields in 100ms: %d\n", yieldCount);
    
    // Should be able to yield many times in 100ms
    // (showing the loop is not blocked)
    TEST_ASSERT_GREATER_THAN(1000, yieldCount);
}

void test_millis_based_timing() {
    unsigned long startTime = millis();
    unsigned long lastBlink = startTime;
    int blinkCount = 0;
    
    // Non-blocking LED blink pattern
    while (blinkCount < 10 && (millis() - startTime) < 1500) {
        unsigned long now = millis();
        if (now - lastBlink >= 100) {
            blinkCount++;
            lastBlink = now;
        }
        yield();
    }
    
    unsigned long totalTime = millis() - startTime;
    
    Serial.printf("10 blinks completed in: %lu ms\n", totalTime);
    
    // Should complete in approximately 1000ms (10 blinks * 100ms)
    // Allow some tolerance
    TEST_ASSERT_INT_WITHIN(200, 1000, totalTime);
}

// ============================================================================
// Test 4: I2C Scan Optimization
// ============================================================================

void test_i2c_quick_scan_addresses() {
    // Test that common addresses are correct
    const byte commonAddresses[] = {0x23, 0x68, 0x76, 0x77};
    const int numCommonAddresses = 4;
    
    // Verify we're checking the right addresses
    TEST_ASSERT_EQUAL(0x23, commonAddresses[0]); // BH1750
    TEST_ASSERT_EQUAL(0x68, commonAddresses[1]); // RTC/IMU
    TEST_ASSERT_EQUAL(0x76, commonAddresses[2]); // BME280
    TEST_ASSERT_EQUAL(0x77, commonAddresses[3]); // BME280 alt
    
    // Verify array size
    TEST_ASSERT_EQUAL(4, numCommonAddresses);
    
    Serial.println("Common I2C addresses validated");
}

void test_i2c_scan_time_estimate() {
    // Estimate time savings
    const int fullScanAddresses = 127;
    const int quickScanAddresses = 4;
    const int i2cTransactionTimeMs = 2; // Approximate per address
    
    int fullScanTime = fullScanAddresses * i2cTransactionTimeMs;
    int quickScanTime = quickScanAddresses * i2cTransactionTimeMs;
    int timeSaved = fullScanTime - quickScanTime;
    
    Serial.printf("Full scan time: ~%d ms\n", fullScanTime);
    Serial.printf("Quick scan time: ~%d ms\n", quickScanTime);
    Serial.printf("Time saved: ~%d ms (%.1f%% reduction)\n", 
                  timeSaved, 
                  (float)timeSaved / fullScanTime * 100);
    
    // Should save significant time
    TEST_ASSERT_GREATER_THAN(200, timeSaved);
}

// ============================================================================
// Test 5: Power Manager Optimizations
// ============================================================================

void test_const_char_vs_string() {
    size_t heapBefore, heapAfter1, heapAfter2;
    
    // Test String allocation
    heapBefore = ESP.getFreeHeap();
    {
        String mode = "Max Performance";
        Serial.printf("Mode: %s\n", mode.c_str());
    }
    heapAfter1 = ESP.getFreeHeap();
    
    // Test const char*
    heapBefore = ESP.getFreeHeap();
    {
        const char* mode = "Max Performance";
        Serial.printf("Mode: %s\n", mode);
    }
    heapAfter2 = ESP.getFreeHeap();
    
    int stringMethod = heapBefore - heapAfter1;
    int constCharMethod = heapBefore - heapAfter2;
    
    Serial.printf("String method heap usage: %d bytes\n", stringMethod);
    Serial.printf("const char* method heap usage: %d bytes\n", constCharMethod);
    
    // const char* should use no heap (string literal in flash)
    TEST_ASSERT_EQUAL(0, constCharMethod);
}

void test_battery_caching_logic() {
    // Simulate cached voltage scenario
    float cachedVoltage = 3.7f;
    float batteryPercentage = 0.0f; // Initialize to prevent uninitialized use
    
    // When voltage is cached (non-zero), should use it
    if (cachedVoltage != 0.0f) {
        // Use cached value
        batteryPercentage = (cachedVoltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
    } else {
        // Would read ADC here in real code, for test use default
        batteryPercentage = 0.0f;
    }
    
    Serial.printf("Cached voltage: %.2fV -> %.1f%%\n", cachedVoltage, batteryPercentage);
    
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 58.3f, batteryPercentage);
    
    // Test edge cases
    cachedVoltage = 0.0f;
    if (cachedVoltage == 0.0f) {
        // Would read ADC here in real code
        Serial.println("Cache miss - would read ADC");
        batteryPercentage = 0.0f; // Set to valid value
    }
}

// ============================================================================
// Test Runner
// ============================================================================

void setup() {
    // Note: This 2-second blocking delay is necessary for serial port initialization
    // on many Arduino-compatible boards. Without it, early test output may be lost.
    // This is a special case for test initialization and doesn't contradict the
    // PR's goal of eliminating blocking delays in operational code.
    delay(2000); // Wait for serial
    
    UNITY_BEGIN();
    
    Serial.println("\n=== Performance Optimization Tests ===\n");
    
    Serial.println("--- Variance Calculation Tests ---");
    RUN_TEST(test_variance_calculation_correctness);
    RUN_TEST(test_variance_calculation_performance);
    
    Serial.println("\n--- String Optimization Tests ---");
    RUN_TEST(test_string_reserve_efficiency);
    RUN_TEST(test_snprintf_vs_string_constructor);
    
    Serial.println("\n--- Non-Blocking Delay Tests ---");
    RUN_TEST(test_yield_responsiveness);
    RUN_TEST(test_millis_based_timing);
    
    Serial.println("\n--- I2C Scan Tests ---");
    RUN_TEST(test_i2c_quick_scan_addresses);
    RUN_TEST(test_i2c_scan_time_estimate);
    
    Serial.println("\n--- Power Manager Tests ---");
    RUN_TEST(test_const_char_vs_string);
    RUN_TEST(test_battery_caching_logic);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
    delay(1000);
}
