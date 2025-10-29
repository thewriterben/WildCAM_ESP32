#include <Arduino.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

// Include all modular components
#include "config.h"
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
    
    // Initialize watchdog timer for crash recovery
    Serial.println("Initializing watchdog timer...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    Serial.println("   ✓ Watchdog timer initialized\n");
    
    // Initialize Power Manager first
    Serial.println("1. Initializing Power Manager...");
    if (!power.init(BATTERY_ADC_PIN)) {
        Serial.println("   ✗ Power Manager initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("   ✓ Power Manager initialized");
    
    // Check battery status
    float batteryVoltage = power.getBatteryVoltage();
    int batteryPercent = power.getBatteryPercentage();
    Serial.printf("   Battery: %.2fV (%d%%)\n", batteryVoltage, batteryPercent);
    
    // Check for critical battery - enter deep sleep immediately
    if (batteryVoltage < BATTERY_CRITICAL_THRESHOLD) {
        Serial.println("   ⚠ CRITICAL: Battery critically low!");
        Serial.println("   Entering deep sleep to protect battery...");
        power.configureWakeOnMotion(PIR_SENSOR_PIN);
        power.enterDeepSleep(DEEP_SLEEP_DURATION);
    }
    
    if (power.isLowBattery()) {
        Serial.println("   ⚠ WARNING: Battery low!");
        enableWebServer = false;  // Disable web server to save power
    }
    
    // Initialize Camera
    Serial.println("\n2. Initializing Camera...");
    if (!camera.init()) {
        Serial.println("   ✗ Camera initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("   ✓ Camera initialized");
    
    // Initialize Storage
    Serial.println("\n3. Initializing Storage...");
    if (!storage.init()) {
        Serial.println("   ✗ Storage initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("   ✓ Storage initialized");
    Serial.printf("   Storage: %lu bytes free / %lu bytes used\n", 
                  storage.getFreeSpace(), storage.getUsedSpace());
    
    // Initialize Motion Detector
    Serial.println("\n4. Initializing Motion Detector...");
    if (!motionDetector.init(PIR_SENSOR_PIN, MOTION_COOLDOWN_MS)) {
        Serial.println("   ✗ Motion detector initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("   ✓ Motion detector ready");
    
    // Initialize Web Server (if enabled)
    if (enableWebServer) {
        Serial.println("\n5. Initializing Web Server...");
        
        // Connect to WiFi
        Serial.print("   Connecting to WiFi");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println(" connected!");
            Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
            
            // Initialize web server with manager references
            if (webServer.init(&storage, &camera, &power)) {
                webServer.begin();
                Serial.println("   ✓ Web server started");
            } else {
                Serial.println("   ✗ Web server initialization failed");
            }
        } else {
            Serial.println(" failed!");
            Serial.println("   ✗ WiFi connection failed");
        }
    } else {
        Serial.println("\n5. Web Server disabled (low power mode)");
    }
    
    // Configure wake-up sources for deep sleep
    Serial.println("\nConfiguring wake-up sources...");
    power.configureWakeOnMotion(PIR_SENSOR_PIN);
    power.configureWakeOnTimer(DEEP_SLEEP_DURATION);
    Serial.println("   ✓ Wake-up sources configured");
    
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
        Serial.println("\n*** MOTION DETECTED ***");
        
        // Update last motion time
        lastMotionTime = millis();
        
        // Stabilization delay to reduce motion blur
        Serial.println("Stabilizing camera...");
        delay(IMAGE_CAPTURE_DELAY_MS);
        
        // Capture image
        currentState = CAPTURING;
        Serial.println("Capturing image...");
        
        camera_fb_t* fb = camera.captureImage();
        if (fb != nullptr) {
            currentState = SAVING;
            
            // Save image to SD card
            String filepath = storage.saveImage(fb);
            if (filepath.length() > 0) {
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
                    Serial.println("Metadata saved successfully");
                } else {
                    Serial.println("Warning: Failed to save metadata");
                }
                
                // Increment image counter
                imageCount++;
                Serial.printf("Total images captured: %lu\n", imageCount);
            } else {
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
            Serial.println("ERROR: Failed to capture image from camera");
            // Log error to SD card
            File errorLog = SD_MMC.open("/error.log", FILE_APPEND);
            if (errorLog) {
                errorLog.printf("[%lu] Failed to capture image\n", millis());
                errorLog.close();
            }
        }
        
        currentState = IDLE;
        Serial.println("Returning to idle state\n");
    }
    
    // Check battery status periodically
    if (millis() - lastBatteryCheck > 60000) {  // Every 60 seconds
        float voltage = power.getBatteryVoltage();
        int percent = power.getBatteryPercentage();
        Serial.printf("Battery check: %.2fV (%d%%)\n", voltage, percent);
        
        // Enter deep sleep if battery is critically low
        if (voltage < BATTERY_CRITICAL_THRESHOLD) {
            Serial.println("Battery critically low - entering deep sleep");
            currentState = SLEEPING;
            power.configureWakeOnMotion(PIR_SENSOR_PIN);
            power.enterDeepSleep(DEEP_SLEEP_DURATION);
        }
        
        lastBatteryCheck = millis();
    }
    
    // Check for inactivity - enter deep sleep if no motion for DEEP_SLEEP_DURATION
    if (millis() - lastMotionTime > (DEEP_SLEEP_DURATION * 1000)) {
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