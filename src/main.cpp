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
#include "TimeManager.h"
#include "MotionDetector.h"
#include "CameraManager.h"
#include "StorageManager.h"
#include "PowerManager.h"
#include "WebServer.h"
#include "MeshManager.h"
#include "SensorManager.h"

// Global instances
MotionDetector motionDetector;
CameraManager camera;
StorageManager storage;
PowerManager power;
WebServer webServer(WEB_SERVER_PORT);
SensorManager sensors;

#if TIME_MANAGEMENT_ENABLED
TimeManager timeManager;
#endif

/**
 * @brief System state enumeration
 * 
 * Tracks the current operational state of the wildlife camera system.
 * 
 * State Transitions:
 * - IDLE → MOTION_DETECTED (motion sensor triggered)
 * - MOTION_DETECTED → CAPTURING (after stabilization delay)
 * - CAPTURING → SAVING (image captured successfully)
 * - CAPTURING → IDLE (capture failed)
 * - SAVING → IDLE (image and metadata saved)
 * - Any state → SLEEPING (inactivity timeout or low battery)
 * 
 * @note SLEEPING state is terminal - system enters deep sleep and resets on wake
 */
enum SystemState {
    IDLE,            ///< Waiting for motion event
    MOTION_DETECTED, ///< Motion detected, camera stabilizing
    CAPTURING,       ///< Capturing image from camera
    SAVING,          ///< Saving image and metadata to SD card
    SLEEPING         ///< Deep sleep mode (power saving)
};

SystemState currentState = IDLE;
bool enableWebServer = true;  // Set to false for low-power operation

// State tracking variables
unsigned long lastMotionTime = 0;
unsigned long imageCount = 0;
unsigned long lastBatteryCheck = 0;
unsigned long lastSensorRead = 0;

// Watchdog timeout (30 seconds)
#define WDT_TIMEOUT 30

/**
 * @brief Initialize all system components and configure wake-up sources
 * 
 * This function initializes the WildCAM system in the following order:
 * 1. Logging system
 * 2. Watchdog timer
 * 3. Power manager
 * 4. Camera
 * 5. Storage
 * 6. Motion detector
 * 7. Web server (if enabled and battery sufficient)
 * 8. Wake-up sources configuration
 * 
 * If any critical component fails to initialize, the system halts with an
 * error message and enters an infinite loop. This prevents operation with
 * missing or malfunctioning components.
 * 
 * Battery level is checked at startup. If critically low, the system
 * immediately enters deep sleep to protect the battery.
 * 
 * @note This function is called once at system startup/reset
 * @note System will halt (infinite loop) if any component fails to initialize
 */
void setup() {
    Serial.begin(115200);
    delay(STARTUP_DELAY_MS);
    
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
    
    // Initialize Time Manager (before other components that need timestamps)
    #if TIME_MANAGEMENT_ENABLED
    Serial.println("\nInitializing Time Manager...");
    #if LOGGING_ENABLED
    LOG_INFO("Initializing Time Manager...");
    #endif
    if (timeManager.init(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC)) {
        #if LOGGING_ENABLED
        LOG_INFO("Time Manager initialized");
        #endif
        Serial.println("   ✓ Time Manager initialized");
        if (timeManager.hasExternalRTC()) {
            Serial.println("   External RTC (DS3231) detected");
        }
        if (timeManager.isTimeSet()) {
            char timestamp[30];
            timeManager.getTimestamp(timestamp, sizeof(timestamp));
            Serial.printf("   Current time: %s\n", timestamp);
        } else {
            Serial.println("   Time not set - will sync via NTP when WiFi connects");
        }
    } else {
        #if LOGGING_ENABLED
        LOG_WARN("Time Manager initialization failed (non-critical)");
        #endif
        Serial.println("   ⚠ Time Manager initialization failed");
    }
    #endif
    
    // Initialize watchdog timer for crash recovery
    #if LOGGING_ENABLED
    LOG_INFO("Initializing watchdog timer...");
    #endif
    Serial.println("Initializing watchdog timer...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    #if LOGGING_ENABLED
    LOG_INFO("Watchdog timer initialized");
    #endif
    Serial.println("   ✓ Watchdog timer initialized\n");
    
    // Initialize Power Manager first
    #if LOGGING_ENABLED
    LOG_INFO("Initializing Power Manager...");
    #endif
    Serial.println("1. Initializing Power Manager...");
    if (!power.init(BATTERY_ADC_PIN)) {
        #if LOGGING_ENABLED
        LOG_ERROR("Power Manager initialization failed!");
        #endif
        Serial.println("   ✗ Power Manager initialization failed!");
        Serial.println("\nSystem halted due to initialization failure.");
        while (true) {
            delay(ERROR_HALT_DELAY_MS);
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
            delay(ERROR_HALT_DELAY_MS);
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
            delay(ERROR_HALT_DELAY_MS);
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
            delay(ERROR_HALT_DELAY_MS);
        }
    }
    LOG_INFO("Motion detector ready");
    Serial.println("   ✓ Motion detector ready");
    
    // Initialize Web Server (if enabled)
    if (enableWebServer) {
        LOG_INFO("Initializing Web Server...");
        Serial.println("\n5. Initializing Web Server...");
        
        // Connect to WiFi with timeout
        Serial.print("   Connecting to WiFi");
        LOG_DEBUG("Connecting to WiFi SSID: %s", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        int attempts = 0;
        // Wait for WiFi connection with retry limit
        while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_MAX_ATTEMPTS) {
            delay(WIFI_CONNECT_RETRY_DELAY_MS);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_INFO("WiFi connected - IP: %s", WiFi.localIP().toString().c_str());
            Serial.println(" connected!");
            Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
            
            // Synchronize time via NTP after WiFi connection
            #if TIME_MANAGEMENT_ENABLED && NTP_AUTO_SYNC
            Serial.println("   Synchronizing time via NTP...");
            if (timeManager.syncNTP(NTP_SERVER_PRIMARY, NTP_SYNC_TIMEOUT_MS)) {
                #if LOGGING_ENABLED
                LOG_INFO("NTP time sync successful");
                #endif
                Serial.println("   ✓ Time synchronized via NTP");
                char timestamp[30];
                timeManager.getTimestamp(timestamp, sizeof(timestamp));
                Serial.printf("   Current time: %s\n", timestamp);
            } else {
                #if LOGGING_ENABLED
                LOG_WARN("NTP time sync failed - using RTC time if available");
                #endif
                Serial.println("   ⚠ NTP sync failed - using RTC time if available");
            }
            #endif
            
            // Initialize web server with manager references
            if (webServer.init(&storage, &camera, &power)) {
                webServer.begin();
                #if LOGGING_ENABLED
                LOG_INFO("Web server started");
                #endif
                Serial.println("   ✓ Web server started");
            } else {
                #if LOGGING_ENABLED
                LOG_ERROR("Web server initialization failed");
                #endif
                Serial.println("   ✗ Web server initialization failed");
            }
        } else {
            LOG_WARN("WiFi connection failed after %d attempts", attempts);
            Serial.println(" failed!");
            Serial.println("   ✗ WiFi connection failed");
            enableWebServer = false;  // Disable web server if WiFi failed
        }
    } else {
        LOG_INFO("Web Server disabled (low power mode)");
        Serial.println("\n5. Web Server disabled (low power mode)");
    }
    
    // Initialize LoRa Mesh Network (if enabled)
    #if LORA_ENABLED
    LOG_INFO("Initializing LoRa Mesh Network...");
    Serial.println("\n6. Initializing LoRa Mesh Network...");
    if (meshManager.init()) {
        // Set node name based on chip ID for identification
        String nodeName = "CAM" + String((uint32_t)(ESP.getEfuseMac() & 0xFFFF), HEX);
        meshManager.setNodeName(nodeName);
        LOG_INFO("LoRa mesh network ready - Node: %s", nodeName.c_str());
        Serial.printf("   ✓ Mesh network ready - Node: %s\n", nodeName.c_str());
        
        // If no coordinator exists after initial beacon, become coordinator
        // (First node to start typically becomes coordinator)
    } else {
        LOG_WARN("LoRa mesh network initialization failed (non-critical)");
        Serial.println("   ⚠ LoRa mesh network initialization failed");
        Serial.println("   (Continuing without mesh networking)");
    }
    #else
    Serial.println("\n6. LoRa Mesh Network: Disabled in config");
    LOG_INFO("LoRa mesh networking is disabled in config");
    #endif
    
    // Initialize Additional Sensors (BME280, GPS, Light Sensor)
    Serial.println("\n7. Initializing Additional Sensors...");
    LOG_INFO("Initializing Additional Sensors...");
    if (sensors.init()) {
        LOG_INFO("Additional sensors initialized");
        Serial.println("   ✓ Additional sensors initialized");
        sensors.printStatus();
    } else {
        LOG_INFO("No additional sensors configured (non-critical)");
        Serial.println("   ⚠ No additional sensors configured");
        Serial.println("   (Enable BME280/GPS/Light sensor in config.h)");
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
    lastSensorRead = millis();
    
    currentState = IDLE;
}

/**
 * @brief Main system loop - monitor motion, capture images, manage power
 * 
 * This function implements the main operational loop of the wildlife camera:
 * 
 * 1. Feeds watchdog timer to prevent system reset
 * 2. Checks for motion detection events
 * 3. When motion detected:
 *    - Stabilizes camera
 *    - Captures image
 *    - Saves image and metadata to SD card
 *    - Releases frame buffer
 * 4. Periodically checks battery level
 * 5. Enters deep sleep on:
 *    - Critical low battery
 *    - Extended inactivity period
 * 
 * State transitions:
 * - IDLE → MOTION_DETECTED → CAPTURING → SAVING → IDLE
 * - Any state → SLEEPING (on timeout or low battery)
 * 
 * @note This function runs continuously in an infinite loop
 * @note Loop delay of 100ms prevents CPU spinning
 */
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
                

                metadata["timestamp"] = millis();
                metadata["time_source"] = "millis";
                #endif
                
                metadata["image_path"] = filepath;
                metadata["battery_voltage"] = power.getBatteryVoltage();
                metadata["battery_percent"] = power.getBatteryPercentage();
                metadata["image_size"] = fb->len;
                metadata["image_count"] = imageCount + 1;
                
                // Add environmental sensor data if available
                if (sensors.isBME280Available()) {
                    EnvironmentalData envData = sensors.readEnvironmental();
                    if (envData.valid) {
                        JsonObject env = metadata.createNestedObject("environment");
                        env["temperature"] = envData.temperature;
                        env["humidity"] = envData.humidity;
                        env["pressure"] = envData.pressure;
                        env["altitude"] = envData.altitude;
                    }
                }
                
                // Add GPS location data if available
                if (sensors.isGPSAvailable() && sensors.hasGPSFix()) {
                    GPSData gpsData = sensors.readGPS();
                    JsonObject gps = metadata.createNestedObject("gps");
                    gps["latitude"] = gpsData.latitude;
                    gps["longitude"] = gpsData.longitude;
                    gps["altitude"] = gpsData.altitude;
                    gps["satellites"] = gpsData.satellites;
                }
                
                // Add light sensor data if available
                if (sensors.isLightSensorAvailable()) {
                    LightData lightData = sensors.readLight();
                    if (lightData.valid) {
                        JsonObject light = metadata.createNestedObject("light");
                        light["lux"] = lightData.lux;
                        light["is_daytime"] = lightData.isDaytime;
                    }
                }
                
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
                
                // Broadcast wildlife detection event over mesh network
                #if LORA_ENABLED
                if (meshManager.isInitialized()) {
                    WildlifeEvent event;
                    #if TIME_MANAGEMENT_ENABLED
                    event.timestamp = timeManager.isTimeSet() ? (uint32_t)timeManager.getUnixTime() : millis();
                    #else
                    event.timestamp = millis();
                    #endif
                    event.nodeId = meshManager.getNodeId();
                    event.species = "Unknown";  // Could be set by ML classifier
                    event.confidence = 0.0f;
                    
                    // Include GPS coordinates if available
                    if (sensors.isGPSAvailable() && sensors.hasGPSFix()) {
                        GPSData gpsData = sensors.readGPS();
                        event.latitude = gpsData.latitude;
                        event.longitude = gpsData.longitude;
                    } else {
                        event.latitude = 0.0f;
                        event.longitude = 0.0f;
                    }
                    
                    event.imageSize = fb->len;
                    event.hasImage = true;
                    
                    if (meshManager.sendWildlifeEvent(event)) {
                        LOG_INFO("Wildlife event broadcast to mesh network");
                        Serial.println("Wildlife event broadcast to mesh network");
                    }
                }
                #endif
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
    
    // Update GPS data regularly (if enabled)
    if (sensors.isGPSAvailable()) {
        sensors.updateGPS();
    }
    
    // Read environmental sensors periodically
    #ifdef SENSOR_READ_INTERVAL_MS
    if (millis() - lastSensorRead > SENSOR_READ_INTERVAL_MS) {
        if (sensors.isBME280Available() || sensors.isLightSensorAvailable()) {
            LOG_DEBUG("Periodic sensor reading");
            
            if (sensors.isBME280Available()) {
                EnvironmentalData env = sensors.readEnvironmental();
                if (env.valid) {
                    LOG_INFO("Environment: %.1f°C, %.1f%% RH, %.1f hPa", 
                             env.temperature, env.humidity, env.pressure);
                }
            }
            
            if (sensors.isLightSensorAvailable()) {
                LightData light = sensors.readLight();
                if (light.valid) {
                    LOG_INFO("Light: %.1f lux (%s)", light.lux, 
                             light.isDaytime ? "Day" : "Night");
                }
            }
        }
        lastSensorRead = millis();
    }
    #endif
    
    // Check battery status periodically (every 60 seconds)
    if (millis() - lastBatteryCheck > BATTERY_CHECK_INTERVAL_MS) {
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
    
    // Check for inactivity - enter deep sleep if no motion for extended period
    // Convert DEEP_SLEEP_DURATION from seconds to milliseconds for comparison
    if (millis() - lastMotionTime > (DEEP_SLEEP_DURATION * 1000)) {
        LOG_INFO("No motion detected for %d seconds - entering deep sleep", DEEP_SLEEP_DURATION);
        Serial.println("\nNo motion detected for extended period");
        Serial.println("Entering deep sleep mode...");
        Serial.printf("Will wake on motion or after %d seconds\n", DEEP_SLEEP_DURATION);
        
        currentState = SLEEPING;
        power.configureWakeOnMotion(PIR_SENSOR_PIN);
        power.enterDeepSleep(DEEP_SLEEP_DURATION);
    }
    
    // Process mesh network messages
    #if LORA_ENABLED
    if (meshManager.isInitialized()) {
        meshManager.process();
    }
    #endif
    
    // Small delay to prevent CPU spinning
    delay(MAIN_LOOP_DELAY_MS);
}