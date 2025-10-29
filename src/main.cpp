#include <Arduino.h>

// Include all modular components
#include "config.h"
#include "MotionDetector.h"
#include "CameraManager.h"
#include "StorageManager.h"
#include "PowerManager.h"
#include "WebServer.h"

// Global instances
MotionDetector motionDetector(PIR_SENSOR_PIN, MOTION_SENSITIVITY, MOTION_COOLDOWN_MS);
CameraManager camera;
StorageManager storage;
PowerManager power(BATTERY_ADC_PIN);
WebServer webServer(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);

// System state
enum SystemState {
    IDLE,
    MOTION_DETECTED,
    CAPTURING,
    PROCESSING,
    SLEEPING
};

SystemState currentState = IDLE;
bool enableWebServer = true;  // Set to false for low-power operation

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=================================");
    Serial.println("  WildCAM ESP32 Starting Up");
    Serial.println("=================================\n");
    
    // Initialize Power Manager first
    Serial.println("1. Initializing Power Manager...");
    if (power.begin()) {
        Serial.println("   ✓ Power Manager initialized");
        
        // Check battery status
        float batteryVoltage = power.getBatteryVoltage();
        int batteryPercent = power.getBatteryPercentage();
        Serial.printf("   Battery: %.2fV (%d%%)\n", batteryVoltage, batteryPercent);
        
        if (power.isBatteryLow()) {
            Serial.println("   ⚠ WARNING: Battery low!");
            enableWebServer = false;  // Disable web server to save power
        }
    }
    
    // Initialize Camera
    Serial.println("\n2. Initializing Camera...");
    if (camera.begin()) {
        Serial.println("   ✓ Camera initialized");
    } else {
        Serial.println("   ✗ Camera initialization failed!");
    }
    
    // Initialize Storage
    Serial.println("\n3. Initializing Storage...");
    if (storage.begin()) {
        Serial.println("   ✓ Storage initialized");
        Serial.printf("   Storage: %llu MB free / %llu MB total\n", 
                      storage.getAvailableSpace(), storage.getTotalSpace());
        Serial.printf("   Images stored: %d\n", storage.getImageCount());
    } else {
        Serial.println("   ✗ Storage initialization failed!");
    }
    
    // Initialize Motion Detector
    Serial.println("\n4. Initializing Motion Detector...");
    if (motionDetector.begin()) {
        Serial.println("   ✓ Motion detector ready");
    }
    
    // Initialize Web Server (if enabled)
    if (enableWebServer) {
        Serial.println("\n5. Initializing Web Server...");
        if (webServer.begin()) {
            Serial.println("   ✓ Web server started");
            Serial.printf("   Access at: http://%s\n", webServer.getIPAddress().c_str());
        } else {
            Serial.println("   ✗ Web server failed (continuing without WiFi)");
        }
    } else {
        Serial.println("\n5. Web Server disabled (low power mode)");
    }
    
    Serial.println("\n=================================");
    Serial.println("  System Ready!");
    Serial.println("=================================\n");
    
    currentState = IDLE;
}

void loop() {
    // Check for motion
    if (motionDetector.detectMotion()) {
        currentState = MOTION_DETECTED;
        Serial.println("\n*** MOTION DETECTED ***");
        
        // Capture image
        currentState = CAPTURING;
        Serial.println("Capturing image...");
        
        camera_fb_t* fb = camera.captureImage();
        if (fb != nullptr) {
            currentState = PROCESSING;
            
            // Save to SD card
            String filepath;
            if (storage.saveImage(fb, filepath)) {
                Serial.printf("Image saved successfully: %s\n", filepath.c_str());
                Serial.printf("Total images: %d\n", storage.getImageCount());
            } else {
                Serial.println("Failed to save image");
            }
            
            // Release frame buffer
            camera.releaseFrameBuffer(fb);
        } else {
            Serial.println("Failed to capture image");
        }
        
        currentState = IDLE;
        Serial.println("Returning to idle state\n");
    }
    
    // Check battery status periodically
    static unsigned long lastBatteryCheck = 0;
    if (millis() - lastBatteryCheck > 60000) {  // Every 60 seconds
        float voltage = power.getBatteryVoltage();
        int percent = power.getBatteryPercentage();
        Serial.printf("Battery check: %.2fV (%d%%)\n", voltage, percent);
        
        // Enter deep sleep if battery is critically low, regardless of web server status
        if (power.isBatteryLow()) {
            Serial.println("Battery critically low - entering deep sleep");
            power.configureWakeup(PIR_SENSOR_PIN);
            power.enterDeepSleep(DEEP_SLEEP_DURATION);
        }
        
        lastBatteryCheck = millis();
    }
    
    // Small delay to prevent tight loop
    delay(100);
}