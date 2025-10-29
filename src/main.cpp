#include <Arduino.h>
#include <WiFi.h>

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
    power.init(BATTERY_ADC_PIN);
    Serial.println("   ✓ Power Manager initialized");
    
    // Check battery status
    float batteryVoltage = power.getBatteryVoltage();
    int batteryPercent = power.getBatteryPercentage();
    Serial.printf("   Battery: %.2fV (%d%%)\n", batteryVoltage, batteryPercent);
    
    if (power.isLowBattery()) {
        Serial.println("   ⚠ WARNING: Battery low!");
        enableWebServer = false;  // Disable web server to save power
    }
    
    // Initialize Camera
    Serial.println("\n2. Initializing Camera...");
    if (camera.init()) {
        Serial.println("   ✓ Camera initialized");
    } else {
        Serial.println("   ✗ Camera initialization failed!");
    }
    
    // Initialize Storage
    Serial.println("\n3. Initializing Storage...");
    if (storage.init()) {
        Serial.println("   ✓ Storage initialized");
        Serial.printf("   Storage: %lu bytes free / %lu bytes used\n", 
                      storage.getFreeSpace(), storage.getUsedSpace());
    } else {
        Serial.println("   ✗ Storage initialization failed!");
    }
    
    // Initialize Motion Detector
    Serial.println("\n4. Initializing Motion Detector...");
    if (motionDetector.init(PIR_SENSOR_PIN, MOTION_COOLDOWN_MS)) {
        Serial.println("   ✓ Motion detector ready");
    } else {
        Serial.println("   ✗ Motion detector initialization failed!");
    }
    
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
    
    Serial.println("\n=================================");
    Serial.println("  System Ready!");
    Serial.println("=================================\n");
    
    currentState = IDLE;
}

void loop() {
    // Check for motion
    if (motionDetector.isMotionDetected()) {
        currentState = MOTION_DETECTED;
        Serial.println("\n*** MOTION DETECTED ***");
        
        // Capture image
        currentState = CAPTURING;
        Serial.println("Capturing image...");
        
        camera_fb_t* fb = camera.captureImage();
        if (fb != nullptr) {
            currentState = PROCESSING;
            
            // Save to SD card
            String filepath = storage.saveImage(fb);
            if (filepath.length() > 0) {
                Serial.printf("Image saved successfully: %s\n", filepath.c_str());
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
        if (power.isLowBattery()) {
            Serial.println("Battery critically low - entering deep sleep");
            power.configureWakeOnMotion(PIR_SENSOR_PIN);
            power.enterDeepSleep(DEEP_SLEEP_DURATION);
        }
        
        lastBatteryCheck = millis();
    }
    
    // Small delay to prevent tight loop
    delay(100);
}