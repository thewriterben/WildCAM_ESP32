/**
 * @file diagnostics.cpp
 * @brief System diagnostics implementation for WildCAM ESP32
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 */

#include "diagnostics.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <esp_system.h>

// Include system headers
#include "../utils/logger.h"
#include "../core/storage_manager.h"
#include "camera/camera_manager.h"
#include "power/power_management.h"
#include "utils/time_manager.h"

// External storage manager instance
extern StorageManager g_storage;

// Pin definitions
#define PIR_PIN 13
#define SD_CS_PIN 13

// Diagnostics log file
#define DIAGNOSTICS_LOG_FILE "/diagnostics.log"

/**
 * @brief Write diagnostic result to log file
 */
static void logDiagnostic(const char* test_name, bool result, const char* details = nullptr) {
    char log_buffer[256];
    String timestamp = getFormattedTime();
    
    if (details) {
        snprintf(log_buffer, sizeof(log_buffer), "[%s] %s: %s - %s\n", 
                timestamp.c_str(), test_name, result ? "PASS" : "FAIL", details);
    } else {
        snprintf(log_buffer, sizeof(log_buffer), "[%s] %s: %s\n", 
                timestamp.c_str(), test_name, result ? "PASS" : "FAIL");
    }
    
    // Log to serial
    Serial.print(log_buffer);
    
    // Log to SD card if available
    if (g_storage.isReady()) {
        g_storage.saveLog(log_buffer, DIAGNOSTICS_LOG_FILE);
    }
}

/**
 * @brief Test camera initialization
 */
bool testCameraInit() {
    Logger::info("Testing camera initialization...");
    
    CameraManager camera;
    bool initialized = camera.initialize();
    
    if (!initialized) {
        logDiagnostic("Camera Init", false, camera.getLastError().c_str());
        return false;
    }
    
    // Try to capture a frame to verify camera works
    int width = 0, height = 0;
    uint8_t* frame = camera.captureFrame(&width, &height);
    
    if (frame == nullptr || width == 0 || height == 0) {
        logDiagnostic("Camera Init", false, "Failed to capture test frame");
        return false;
    }
    
    // Release the frame
    camera.releaseFrame(frame);
    
    char details[64];
    snprintf(details, sizeof(details), "Resolution: %dx%d", width, height);
    logDiagnostic("Camera Init", true, details);
    
    return true;
}

/**
 * @brief Test SD card read/write operations
 */
bool testSDCardReadWrite() {
    Logger::info("Testing SD card read/write...");
    
    // Try to initialize SD card
    if (!SD_MMC.begin("/sdcard", true)) {
        logDiagnostic("SD Card Init", false, "Failed to mount SD card");
        return false;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        logDiagnostic("SD Card Init", false, "No SD card detected");
        return false;
    }
    
    // Write test file
    const char* test_filename = "/test_diagnostic.txt";
    const char* test_content = "WildCAM diagnostic test";
    
    File file = SD_MMC.open(test_filename, FILE_WRITE);
    if (!file) {
        logDiagnostic("SD Card Write", false, "Failed to create test file");
        return false;
    }
    
    size_t written = file.print(test_content);
    file.close();
    
    if (written != strlen(test_content)) {
        logDiagnostic("SD Card Write", false, "Write size mismatch");
        return false;
    }
    
    // Read test file
    file = SD_MMC.open(test_filename, FILE_READ);
    if (!file) {
        logDiagnostic("SD Card Read", false, "Failed to open test file");
        return false;
    }
    
    char read_buffer[64];
    size_t read_size = file.readBytes(read_buffer, sizeof(read_buffer));
    file.close();
    
    if (read_size != strlen(test_content) || 
        strncmp(read_buffer, test_content, read_size) != 0) {
        logDiagnostic("SD Card Read", false, "Read verification failed");
        return false;
    }
    
    // Delete test file
    SD_MMC.remove(test_filename);
    
    // Get card info
    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    char details[64];
    snprintf(details, sizeof(details), "Size: %llu MB, Type: %d", cardSize, cardType);
    logDiagnostic("SD Card R/W", true, details);
    
    return true;
}

/**
 * @brief Test PIR sensor reading
 */
bool testPIRSensor() {
    Logger::info("Testing PIR sensor...");
    
    pinMode(PIR_PIN, INPUT);
    
    // Read PIR sensor state
    int pirState = digitalRead(PIR_PIN);
    
    // PIR sensor should return a valid digital value (0 or 1)
    // We can't verify motion without actual movement, but we can verify it's readable
    if (pirState != HIGH && pirState != LOW) {
        logDiagnostic("PIR Sensor", false, "Invalid sensor reading");
        return false;
    }
    
    char details[32];
    snprintf(details, sizeof(details), "State: %d", pirState);
    logDiagnostic("PIR Sensor", true, details);
    
    return true;
}

/**
 * @brief Test BME280 sensor (if enabled)
 */
bool testBME280Sensor() {
    Logger::info("Testing BME280 sensor...");
    
    // BME280 is part of EnvironmentalSuite
    // For now, we'll mark this as optional since it may not be present
    // In a production system, you'd check if the sensor is enabled in config
    
    #ifdef ENABLE_BME280
    // Check if BME280 is available
    // This would require actual BME280 library integration
    logDiagnostic("BME280 Sensor", false, "Not implemented yet");
    return false;
    #else
    // Not enabled, skip test
    logDiagnostic("BME280 Sensor", true, "Not enabled (skipped)");
    return true;
    #endif
}

/**
 * @brief Test RTC (if enabled)
 */
bool testRTC() {
    Logger::info("Testing RTC...");
    
    // Try to sync with RTC
    bool rtcAvailable = syncWithRTC();
    
    if (!rtcAvailable) {
        // RTC is optional, so we log but don't fail
        logDiagnostic("RTC", true, "Not available (optional)");
        return true;
    }
    
    // Get current time to verify RTC is working
    time_t now = getCurrentTime();
    if (now == 0) {
        logDiagnostic("RTC", false, "Invalid time reading");
        return false;
    }
    
    char details[64];
    snprintf(details, sizeof(details), "Time: %ld", now);
    logDiagnostic("RTC", true, details);
    
    return true;
}

/**
 * @brief Test GPS (if enabled)
 */
bool testGPS() {
    Logger::info("Testing GPS...");
    
    // GPS is optional for this system
    // In a production system, you'd check GPS manager if available
    
    #ifdef ENABLE_GPS
    // Check GPS manager
    logDiagnostic("GPS", false, "Not implemented yet");
    return false;
    #else
    // Not enabled, skip test
    logDiagnostic("GPS", true, "Not enabled (skipped)");
    return true;
    #endif
}

/**
 * @brief Test power management system
 */
bool testPowerManagement() {
    Logger::info("Testing power management...");
    
    // Initialize power management if not already done
    if (!initializePowerManagement()) {
        logDiagnostic("Power Mgmt Init", false, "Initialization failed");
        return false;
    }
    
    // Read battery voltage
    float batteryVoltage = getBatteryVoltage();
    if (batteryVoltage < 2.5f || batteryVoltage > 5.0f) {
        logDiagnostic("Battery Voltage", false, "Out of range");
        return false;
    }
    
    // Check charging state
    bool charging = isCharging();
    
    // Check solar voltage if available
    float solarVoltage = getSolarVoltage();
    
    char details[128];
    snprintf(details, sizeof(details), "Battery: %.2fV, Solar: %.2fV, Charging: %s", 
            batteryVoltage, solarVoltage, charging ? "Yes" : "No");
    logDiagnostic("Power Management", true, details);
    
    return true;
}

/**
 * @brief Check available RAM and flash memory
 */
bool testMemory() {
    Logger::info("Testing memory availability...");
    
    // Get free heap
    uint32_t freeHeap = ESP.getFreeHeap();
    
    // Get free PSRAM if available
    uint32_t freePsram = 0;
    if (psramFound()) {
        freePsram = ESP.getFreePsram();
    }
    
    // Get flash chip size
    uint32_t flashSize = ESP.getFlashChipSize();
    
    // Check if we have sufficient memory
    if (freeHeap < 50000) {  // Less than 50KB free heap is concerning
        char details[64];
        snprintf(details, sizeof(details), "Low heap: %u bytes", freeHeap);
        logDiagnostic("Memory Check", false, details);
        return false;
    }
    
    char details[128];
    snprintf(details, sizeof(details), "Heap: %u, PSRAM: %u, Flash: %u MB", 
            freeHeap, freePsram, flashSize / (1024 * 1024));
    logDiagnostic("Memory Check", true, details);
    
    return true;
}

/**
 * @brief Run comprehensive system diagnostics
 */
bool runSystemDiagnostics() {
    unsigned long startTime = millis();
    
    Logger::info("====================================");
    Logger::info("Starting System Diagnostics");
    Logger::info("====================================");
    
    // Track critical system status
    bool cameraOK = false;
    bool sdCardOK = false;
    bool pirOK = false;
    bool powerOK = false;
    
    // Track optional system status
    bool bme280OK = true;  // Optional, defaults to OK
    bool rtcOK = true;     // Optional, defaults to OK
    bool gpsOK = true;     // Optional, defaults to OK
    bool memoryOK = false;
    
    // Test 1: Camera initialization (CRITICAL)
    cameraOK = testCameraInit();
    delay(100);
    
    // Test 2: SD card read/write (CRITICAL)
    sdCardOK = testSDCardReadWrite();
    delay(100);
    
    // Test 3: PIR sensor (CRITICAL)
    pirOK = testPIRSensor();
    delay(100);
    
    // Test 4: BME280 sensor (OPTIONAL)
    bme280OK = testBME280Sensor();
    delay(100);
    
    // Test 5: RTC (OPTIONAL)
    rtcOK = testRTC();
    delay(100);
    
    // Test 6: GPS (OPTIONAL)
    gpsOK = testGPS();
    delay(100);
    
    // Test 7: Power management (CRITICAL)
    powerOK = testPowerManagement();
    delay(100);
    
    // Test 8: Memory check (CRITICAL)
    memoryOK = testMemory();
    delay(100);
    
    // Calculate diagnostics duration
    unsigned long duration = millis() - startTime;
    
    // Log summary
    Logger::info("====================================");
    Logger::info("Diagnostics Summary");
    Logger::info("====================================");
    Logger::info("Critical Systems:");
    Logger::info("  Camera:        %s", cameraOK ? "PASS" : "FAIL");
    Logger::info("  SD Card:       %s", sdCardOK ? "PASS" : "FAIL");
    Logger::info("  PIR Sensor:    %s", pirOK ? "PASS" : "FAIL");
    Logger::info("  Power Mgmt:    %s", powerOK ? "PASS" : "FAIL");
    Logger::info("  Memory:        %s", memoryOK ? "PASS" : "FAIL");
    Logger::info("Optional Systems:");
    Logger::info("  BME280:        %s", bme280OK ? "PASS" : "FAIL");
    Logger::info("  RTC:           %s", rtcOK ? "PASS" : "FAIL");
    Logger::info("  GPS:           %s", gpsOK ? "PASS" : "FAIL");
    Logger::info("Duration: %lu ms", duration);
    Logger::info("====================================");
    
    // Write summary to log file
    char summary[512];
    snprintf(summary, sizeof(summary),
             "\n=== DIAGNOSTICS SUMMARY ===\n"
             "Timestamp: %s\n"
             "Critical: Camera=%s SD=%s PIR=%s Power=%s Memory=%s\n"
             "Optional: BME280=%s RTC=%s GPS=%s\n"
             "Duration: %lu ms\n"
             "===========================\n",
             getFormattedTime().c_str(),
             cameraOK ? "OK" : "FAIL",
             sdCardOK ? "OK" : "FAIL",
             pirOK ? "OK" : "FAIL",
             powerOK ? "OK" : "FAIL",
             memoryOK ? "OK" : "FAIL",
             bme280OK ? "OK" : "FAIL",
             rtcOK ? "OK" : "FAIL",
             gpsOK ? "OK" : "FAIL",
             duration);
    
    if (g_storage.isReady()) {
        g_storage.saveLog(summary, DIAGNOSTICS_LOG_FILE);
    }
    
    // Return true only if all CRITICAL systems pass
    bool allCriticalPass = cameraOK && sdCardOK && pirOK && powerOK && memoryOK;
    
    if (allCriticalPass) {
        Logger::info("✓ All critical systems operational");
    } else {
        Logger::error("✗ Critical system failure detected!");
    }
    
    return allCriticalPass;
}
