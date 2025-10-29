/**
 * @file main.cpp
 * @brief WildCAM ESP32 Wildlife Camera Main Application
 * @version 1.0.0
 * 
 * This is the main application that integrates all modules into a functional
 * wildlife camera system.
 * 
 * FEATURES IMPLEMENTED:
 * 
 * 1. Global Objects:
 *    - All manager class instances created (PowerManager, StorageManager, 
 *      CameraManager, MotionDetector, WebServer)
 *    - State variables for tracking system operation (lastMotionTime, imageCount)
 * 
 * 2. setup() function:
 *    - Serial communication initialized at 115200 baud
 *    - Welcome banner with firmware version displayed
 *    - All managers initialized in proper order:
 *      a. PowerManager
 *      b. CameraManager  
 *      c. StorageManager
 *      d. MotionDetector
 *      e. WebServer (if WiFi enabled and battery sufficient)
 *    - Battery level checked; deep sleep entered if critical
 *    - Wake-up sources configured (motion sensor and timer)
 *    - Error handling: system halts on initialization failure
 *    - Watchdog timer initialized for crash recovery
 * 
 * 3. loop() function:
 *    - Watchdog timer fed to prevent resets
 *    - Motion detection monitoring
 *    - When motion detected:
 *      a. Motion detection message printed
 *      b. Stabilization delay applied (IMAGE_CAPTURE_DELAY_MS)
 *      c. Image captured using CameraManager
 *      d. If capture successful:
 *         i. Image saved using StorageManager
 *         ii. Metadata JSON created (timestamp, battery, image size, count)
 *         iii. Metadata saved to SD card
 *         iv. Frame buffer released
 *         v. Image counter incremented
 *      e. If capture failed, error logged to SD card
 *    - Battery level checked periodically (every 60 seconds)
 *    - Deep sleep entered if no motion for DEEP_SLEEP_DURATION
 *    - CPU spin prevention with 100ms delay
 * 
 * 4. Error Handling:
 *    - Initialization failures halt the system with error message
 *    - Watchdog timer implemented for crash recovery (30 second timeout)
 *    - All errors logged to /error.log on SD card
 * 
 * 5. State Management:
 *    - System states: IDLE, MOTION_DETECTED, CAPTURING, SAVING, SLEEPING
 *    - State machine tracks current operation mode
 *    - State transitions logged to Serial
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <ArduinoJson.h>
#include <SD_MMC.h>

// Include all modular components
#include "config.h"
#include "Logger.h"
#include "MotionDetector.h"
#include "CameraManager.h"
#include "StorageManager.h"
#include "PowerManager.h"
#include "WebServer.h"

// Global instances
MotionDetector motionDetector;
CameraManager camera;
StorageManager storage;
PowerManager power;
WebServer webServer(WEB_SERVER_PORT);

// System state
enum SystemState {
    IDLE,
    MOTION_DETECTED,
    CAPTURING,
    SAVING,
    SLEEPING
};

SystemState currentState = IDLE;
bool enableWebServer = true;  // Set to false for low-power operation

// State tracking variables
unsigned long lastMotionTime = 0;
unsigned long imageCount = 0;
unsigned long lastBatteryCheck = 0;

// Watchdog timeout (30 seconds)
#define WDT_TIMEOUT 30

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=================================");
    Serial.println("  WildCAM ESP32 Starting Up");
    Serial.print("  Firmware Version: ");
    Serial.println(FIRMWARE_VERSION);
    Serial.println("=================================\n");
    
    // Initialize logging system
    #if LOGGING_ENABLED
    Serial.println("Initializing logging system...");
    Logger::init((LogLevel)DEFAULT_LOG_LEVEL, LOG_TO_SERIAL, LOG_TO_SD, LOG_FILE_PATH);
    LOG_INFO("WildCAM ESP32 Starting Up - Version %s", FIRMWARE_VERSION);
    #endif
    
    // Initialize watchdog timer for crash recovery
    LOG_INFO("Initializing watchdog timer...");
    Serial.println("Initializing watchdog timer...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    LOG_INFO("Watchdog timer initialized");
    Serial.println("   ✓ Watchdog timer initialized\n");
    
    // Initialize Power Manager first
    LOG_INFO("Initializing Power Manager...");
    Serial.println("1. Initializing Power Manager...");
    if (!power.init(BATTERY_ADC_PIN)) {
        LOG_ERROR("Power Manager initialization failed!");
        Serial.println("   ✗ Power Manager initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    LOG_INFO("Power Manager initialized");
    Serial.println("   ✓ Power Manager initialized");
    
    // Check battery status
    float batteryVoltage = power.getBatteryVoltage();
    int batteryPercent = power.getBatteryPercentage();
    LOG_INFO("Battery status: %.2fV (%d%%)", batteryVoltage, batteryPercent);
    Serial.printf("   Battery: %.2fV (%d%%)\n", batteryVoltage, batteryPercent);
    
    // Check for critical battery - enter deep sleep immediately
    if (batteryVoltage < BATTERY_CRITICAL_THRESHOLD) {
        LOG_WARN("Battery critically low! Entering deep sleep to protect battery");
        Serial.println("   ⚠ CRITICAL: Battery critically low!");
        Serial.println("   Entering deep sleep to protect battery...");
        power.configureWakeOnMotion(PIR_SENSOR_PIN);
        power.enterDeepSleep(DEEP_SLEEP_DURATION);
    }
    
    if (power.isLowBattery()) {
        LOG_WARN("Battery low! Disabling web server to save power");
        Serial.println("   ⚠ WARNING: Battery low!");
        enableWebServer = false;  // Disable web server to save power
    }
    
    // Initialize Camera
    LOG_INFO("Initializing Camera...");
    Serial.println("\n2. Initializing Camera...");
    if (!camera.init()) {
        LOG_ERROR("Camera initialization failed!");
        Serial.println("   ✗ Camera initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    LOG_INFO("Camera initialized");
    Serial.println("   ✓ Camera initialized");
    
    // Initialize Storage
    LOG_INFO("Initializing Storage...");
    Serial.println("\n3. Initializing Storage...");
    if (!storage.init()) {
        LOG_ERROR("Storage initialization failed!");
        Serial.println("   ✗ Storage initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    LOG_INFO("Storage initialized - Free: %lu bytes, Used: %lu bytes", 
             storage.getFreeSpace(), storage.getUsedSpace());
    Serial.println("   ✓ Storage initialized");
    Serial.printf("   Storage: %lu bytes free / %lu bytes used\n", 
                  storage.getFreeSpace(), storage.getUsedSpace());
    
    // Initialize Motion Detector
    LOG_INFO("Initializing Motion Detector...");
    Serial.println("\n4. Initializing Motion Detector...");
    if (!motionDetector.init(PIR_SENSOR_PIN, MOTION_COOLDOWN_MS)) {
        LOG_ERROR("Motion detector initialization failed!");
        Serial.println("   ✗ Motion detector initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    LOG_INFO("Motion detector ready");
    Serial.println("   ✓ Motion detector ready");
    
    // Initialize Web Server (if enabled)
    if (enableWebServer) {
        LOG_INFO("Initializing Web Server...");
        Serial.println("\n5. Initializing Web Server...");
        
        // Connect to WiFi
        Serial.print("   Connecting to WiFi");
        LOG_DEBUG("Connecting to WiFi SSID: %s", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_INFO("WiFi connected - IP: %s", WiFi.localIP().toString().c_str());
            Serial.println(" connected!");
            Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
            
            // Initialize web server with manager references
            if (webServer.init(&storage, &camera, &power)) {
                webServer.begin();
                LOG_INFO("Web server started");
                Serial.println("   ✓ Web server started");
            } else {
                LOG_ERROR("Web server initialization failed");
                Serial.println("   ✗ Web server initialization failed");
            }
        } else {
            LOG_WARN("WiFi connection failed after %d attempts", attempts);
            Serial.println(" failed!");
            Serial.println("   ✗ WiFi connection failed");
        }
    } else {
        LOG_INFO("Web Server disabled (low power mode)");
        Serial.println("\n5. Web Server disabled (low power mode)");
    }
    
    // Configure wake-up sources for deep sleep
    LOG_INFO("Configuring wake-up sources...");
    Serial.println("\nConfiguring wake-up sources...");
    power.configureWakeOnMotion(PIR_SENSOR_PIN);
    power.configureWakeOnTimer(DEEP_SLEEP_DURATION);
    LOG_INFO("Wake-up sources configured");
    Serial.println("   ✓ Wake-up sources configured");
    
    LOG_INFO("System Ready!");
    Serial.println("\n=================================");
    Serial.println("  System Ready!");
    Serial.println("=================================\n");
    
    // Initialize last motion time
    lastMotionTime = millis();
    lastBatteryCheck = millis();
    
    currentState = IDLE;
}

void loop() {
    // Feed watchdog timer
    esp_task_wdt_reset();
    
    // Check for motion
    if (motionDetector.isMotionDetected()) {
        currentState = MOTION_DETECTED;
        LOG_INFO("Motion detected");
        Serial.println("\n*** MOTION DETECTED ***");
        
        // Update last motion time
        lastMotionTime = millis();
        
        // Stabilization delay to reduce motion blur
        LOG_DEBUG("Stabilizing camera for %d ms", IMAGE_CAPTURE_DELAY_MS);
        Serial.println("Stabilizing camera...");
        delay(IMAGE_CAPTURE_DELAY_MS);
        
        // Capture image
        currentState = CAPTURING;
        LOG_INFO("Capturing image...");
        Serial.println("Capturing image...");
        
        camera_fb_t* fb = camera.captureImage();
        if (fb != nullptr) {
            currentState = SAVING;
            
            // Save image to SD card
            String filepath = storage.saveImage(fb);
            if (filepath.length() > 0) {
                LOG_INFO("Image saved: %s (size: %d bytes)", filepath.c_str(), fb->len);
                Serial.printf("Image saved successfully: %s\n", filepath.c_str());
                
                // Create and save metadata JSON
                StaticJsonDocument<256> metadata;
                metadata["timestamp"] = millis();
                metadata["image_path"] = filepath;
                metadata["battery_voltage"] = power.getBatteryVoltage();
                metadata["battery_percent"] = power.getBatteryPercentage();
                metadata["image_size"] = fb->len;
                metadata["image_count"] = imageCount + 1;
                
                // Save metadata
                if (storage.saveMetadata(filepath, metadata)) {
                    LOG_DEBUG("Metadata saved for %s", filepath.c_str());
                    Serial.println("Metadata saved successfully");
                } else {
                    LOG_WARN("Failed to save metadata for %s", filepath.c_str());
                    Serial.println("Warning: Failed to save metadata");
                }
                
                // Increment image counter
                imageCount++;
                LOG_INFO("Total images captured: %lu", imageCount);
                Serial.printf("Total images captured: %lu\n", imageCount);
            } else {
                LOG_ERROR("Failed to save image to SD card");
                Serial.println("ERROR: Failed to save image to SD card");
                // Log error to SD card
                File errorLog = SD_MMC.open("/error.log", FILE_APPEND);
                if (errorLog) {
                    errorLog.printf("[%lu] Failed to save image\n", millis());
                    errorLog.close();
                }
            }
            
            // Release frame buffer
            camera.releaseFrameBuffer(fb);
        } else {
            LOG_ERROR("Failed to capture image from camera");
            Serial.println("ERROR: Failed to capture image from camera");
            // Log error to SD card
            File errorLog = SD_MMC.open("/error.log", FILE_APPEND);
            if (errorLog) {
                errorLog.printf("[%lu] Failed to capture image\n", millis());
                errorLog.close();
            }
        }
        
        currentState = IDLE;
        LOG_DEBUG("Returning to idle state");
        Serial.println("Returning to idle state\n");
    }
    
    // Check battery status periodically
    if (millis() - lastBatteryCheck > 60000) {  // Every 60 seconds
        float voltage = power.getBatteryVoltage();
        int percent = power.getBatteryPercentage();
        LOG_INFO("Battery check: %.2fV (%d%%)", voltage, percent);
        Serial.printf("Battery check: %.2fV (%d%%)\n", voltage, percent);
        
        // Enter deep sleep if battery is critically low
        if (voltage < BATTERY_CRITICAL_THRESHOLD) {
            LOG_WARN("Battery critically low - entering deep sleep");
            Serial.println("Battery critically low - entering deep sleep");
            currentState = SLEEPING;
            power.configureWakeOnMotion(PIR_SENSOR_PIN);
            power.enterDeepSleep(DEEP_SLEEP_DURATION);
        }
        
        lastBatteryCheck = millis();
    }
    
    // Check for inactivity - enter deep sleep if no motion for DEEP_SLEEP_DURATION
    if (millis() - lastMotionTime > (DEEP_SLEEP_DURATION * 1000)) {
        LOG_INFO("No motion detected for %d seconds - entering deep sleep", DEEP_SLEEP_DURATION);
        Serial.println("\nNo motion detected for extended period");
        Serial.println("Entering deep sleep mode...");
        Serial.printf("Will wake on motion or after %d seconds\n", DEEP_SLEEP_DURATION);
        
        currentState = SLEEPING;
        power.configureWakeOnMotion(PIR_SENSOR_PIN);
        power.enterDeepSleep(DEEP_SLEEP_DURATION);
    }
    
    // Small delay to prevent CPU spinning
    delay(100);
}