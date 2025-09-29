/**
 * @file system_manager.cpp
 * @brief System manager implementation
 * @author thewriterben
 * @date 2025-08-31
 */

#include "system_manager.h"
#include "../src/utils/logger.h"
#include "../config.h"
#include "../src/detection/motion_coordinator.h"
#include "../camera/camera_manager.h"
#include <esp_system.h>
#include <esp_camera.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

// Global power manager instance
PowerManager* g_powerManager = nullptr;

SystemManager::SystemManager(BoardDetector::BoardType board) 
    : m_boardType(board), 
      m_state(STATE_UNINITIALIZED),
      m_initialized(false), 
      m_lastUpdate(0),
      m_cameraReady(false),
      m_storageReady(false),
      m_networkReady(false),
      m_sensorsReady(false),
      m_errorCount(0) {
    
    m_pinConfig = BoardDetector::getPinConfig(board);
    memset(m_lastError, 0, sizeof(m_lastError));
    
    // Initialize camera manager
    m_cameraManager = std::make_unique<CameraManager>();
}

SystemManager::~SystemManager() {
    // Cleanup resources
    if (m_storageReady) {
        SD_MMC.end();
        LittleFS.end();
    }
}

bool SystemManager::initialize() {
    m_state = STATE_INITIALIZING;
    
    Logger::info("=== ESP32WildlifeCAM System Initialization ===");
    Logger::info("Board: %s", BoardDetector::getBoardName(m_boardType));
    
    // Validate hardware configuration first
    if (!validateHardwareConfiguration()) {
        setError("Hardware validation failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize core hardware
    if (!initializeHardware()) {
        setError("Hardware initialization failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize storage (critical for logging)
    if (!initializeStorage()) {
        Logger::warning("Storage initialization failed - limited functionality");
        // Continue without storage for basic operation
    }
    
    // Initialize camera (core functionality)
    if (!initializeCamera()) {
        setError("Camera initialization failed");
        enterSafeMode();
        return false;
    }
    
    // Initialize sensors
    if (!initializeSensors()) {
        Logger::warning("Sensor initialization failed - limited sensing capability");
        // Continue without sensors
    }
    
    // Initialize power management
    if (!initializePowerManagement()) {
        Logger::warning("Power management initialization failed");
        // Continue with basic power monitoring
    }
    
    // Initialize network (optional)
    if (!initializeNetwork()) {
        Logger::warning("Network initialization failed - running in standalone mode");
        // Continue in offline mode
    }
    
    // Initialize tasks
    if (!initializeTasks()) {
        setError("Task initialization failed");
        enterSafeMode();
        return false;
    }
    
    Logger::info("Basic motion detection will use PIR sensor if available");
    
    m_initialized = true;
    m_state = STATE_RUNNING;
    
    printSystemInfo();
    Logger::info("=== System initialization complete ===");
    
    return true;
}

bool SystemManager::initializeHardware() {
    Logger::info("Initializing hardware...");
    
    // Configure GPIO pins based on board type
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    
    // Initialize watchdog timer
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);
    
    // Configure power pins if available
    if (m_pinConfig.battery_pin >= 0) {
        pinMode(m_pinConfig.battery_pin, INPUT);
    }
    
    // Reset watchdog
    esp_task_wdt_reset();
    
    Logger::info("Hardware initialization complete");
    return true;
}

bool SystemManager::initializeCamera() {
    Logger::info("Initializing camera...");
    
    // Use camera manager for actual camera initialization
    if (!m_cameraManager) {
        Logger::error("Camera manager not available");
        return false;
    }
    
    // Initialize camera using working camera manager implementation
    if (!m_cameraManager->initialize()) {
        Logger::error("Camera manager initialization failed");
        return false;
    }
    
    // Notify power manager of camera activation
    if (g_powerManager) {
        g_powerManager->onCameraActivation();
    }
    
    // Test camera capture to verify functionality
    camera_fb_t* test_fb = m_cameraManager->captureToBuffer();
    if (test_fb) {
        Logger::info("Camera test capture successful (%d bytes)", test_fb->len);
        m_cameraManager->returnFrameBuffer(test_fb);
        m_cameraReady = true;
    } else {
        Logger::error("Camera test capture failed");
        return false;
    }
    
    Logger::info("Camera initialized successfully");
    Logger::info("Configuration: %s", m_cameraManager->getConfiguration().c_str());
    
    return true;
}

bool SystemManager::initializeStorage() {
    Logger::info("Initializing storage...");
    
    bool success = false;
    
    // Try SD card first
    if (SD_MMC.begin()) {
        uint8_t cardType = SD_MMC.cardType();
        if (cardType != CARD_NONE) {
            uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
            Logger::info("SD Card initialized: %lluMB", cardSize);
            
            // Create required directories
            SD_MMC.mkdir("/wildlife");
            SD_MMC.mkdir("/wildlife/images");
            SD_MMC.mkdir("/wildlife/logs");
            
            success = true;
        } else {
            Logger::warning("SD Card detected but not accessible");
            SD_MMC.end();
        }
    } else {
        Logger::warning("SD Card initialization failed");
    }
    
    // Initialize LittleFS as fallback
    if (!LittleFS.begin()) {
        Logger::warning("LittleFS mount failed, formatting...");
        if (LittleFS.format()) {
            if (LittleFS.begin()) {
                Logger::info("LittleFS formatted and mounted as fallback storage");
                success = true;
            } else {
                Logger::error("LittleFS mount failed after format");
            }
        } else {
            Logger::error("LittleFS format failed");
        }
    } else {
        Logger::info("LittleFS mounted successfully");
        success = true;
    }
    
    m_storageReady = success;
    return success;
}

bool SystemManager::initializeSensors() {
    Logger::info("Initializing sensors...");
    
    // Initialize PIR sensor if available
    if (m_pinConfig.pir_pin >= 0) {
        pinMode(m_pinConfig.pir_pin, INPUT);
        Logger::info("PIR sensor configured on GPIO %d", m_pinConfig.pir_pin);
    }
    
    // Initialize I2C bus for environmental sensors
    Wire.begin(m_pinConfig.i2c_sda, m_pinConfig.i2c_scl);
    Wire.setClock(100000); // 100kHz for reliable operation
    
    // Scan I2C bus for connected sensors
    Logger::info("Scanning I2C bus for sensors...");
    int devicesFound = 0;
    
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            devicesFound++;
            Logger::info("  I2C device found at address 0x%02X", address);
            
            // Identify common sensor types
            switch (address) {
                case 0x76:
                case 0x77:
                    Logger::info("    Detected: BME280/BMP280 pressure sensor");
                    break;
                case 0x68:
                    Logger::info("    Detected: DS3231 RTC or MPU6050 IMU");
                    break;
                case 0x23:
                    Logger::info("    Detected: BH1750 light sensor");
                    break;
                default:
                    Logger::info("    Unknown sensor type");
                    break;
            }
        }
    }
    
    if (devicesFound == 0) {
        Logger::warning("No I2C sensors detected");
    } else {
        Logger::info("Found %d I2C sensor(s)", devicesFound);
    }
    
    // Configure analog sensors
    if (m_pinConfig.light_sensor_pin >= 0) {
        pinMode(m_pinConfig.light_sensor_pin, INPUT);
        Logger::info("Light sensor configured on GPIO %d", m_pinConfig.light_sensor_pin);
    }
    
    // Configure temperature sensor if available
    if (m_pinConfig.temp_sensor_pin >= 0) {
        pinMode(m_pinConfig.temp_sensor_pin, INPUT);
        Logger::info("Temperature sensor configured on GPIO %d", m_pinConfig.temp_sensor_pin);
    }
    
    // Notify power manager of sensor activity
    if (g_powerManager) {
        g_powerManager->onSensorActivity();
    }
    
    m_sensorsReady = (devicesFound > 0 || m_pinConfig.light_sensor_pin >= 0);
    Logger::info("Sensor initialization complete - %s", m_sensorsReady ? "Ready" : "Limited");
    return true;
}

bool SystemManager::initializePowerManagement() {
    Logger::info("Initializing power management...");
    
    // Initialize the global power manager (simplified)
    if (!g_powerManager) {
        g_powerManager = new PowerManager();
    }
    
    if (!g_powerManager->initialize()) {
        Logger::warning("Power manager initialization failed - continuing with basic power monitoring");
        // Continue without power management
        return true; // Don't fail initialization
    }
    
    // Get initial power status
    PowerManager::PowerStats stats = g_powerManager->getStatistics();
    Logger::info("Battery voltage: %.2fV (%.1f%%)", stats.batteryVoltage, stats.batteryPercentage);
    Logger::info("Solar voltage: %.2fV", stats.solarVoltage);
    Logger::info("Power status: %s", stats.isCharging ? "Charging" : "Not Charging");
    
    Logger::info("Power management initialization complete");
    return true;
}

bool SystemManager::initializeNetwork() {
    Logger::info("Initializing network...");
    
    bool networkAvailable = false;
    
    // WiFi initialization would depend on board capabilities
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_WIFI)) {
        Logger::info("WiFi capability detected");
        
        // Basic WiFi initialization (not connecting yet)
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(true);
        
        // Check if WiFi credentials are available
        // For production, this would read from secure storage
        Logger::info("WiFi configured for station mode - credentials required for connection");
        
        // Notify power manager of potential network activity
        if (g_powerManager) {
            g_powerManager->onNetworkActivity();
        }
        
        networkAvailable = true;
    }
    
    // LoRa initialization (if supported)
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_LORA)) {
        Logger::info("LoRa capability detected");
        
        // LoRa initialization would go here
        // Check if LoRa pins are available and not conflicting
        Logger::info("LoRa initialization deferred - pin conflicts with camera");
        
        // For AI-Thinker ESP32-CAM, LoRa may conflict with camera pins
        // This would be resolved in production with careful pin selection
    }
    
    // Bluetooth Low Energy for device configuration
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_BLUETOOTH)) {
        Logger::info("Bluetooth capability available");
        // BLE initialization would be implemented for device configuration
    }
    
    m_networkReady = networkAvailable;
    
    Logger::info("Network initialization complete - %s", 
                networkAvailable ? "WiFi ready (offline)" : "No network available");
    
    return true; // Return true to allow offline operation
}

bool SystemManager::initializeTasks() {
    Logger::info("Initializing tasks...");
    
    // Create FreeRTOS tasks for system operation
    BaseType_t result;
    
    // System monitoring task (highest priority for safety)
    result = xTaskCreate(
        systemMonitorTask,
        "SysMonitor",
        2048,  // Stack size
        this,  // Parameters
        3,     // Priority (high)
        NULL   // Task handle
    );
    if (result != pdPASS) {
        Logger::error("Failed to create system monitor task");
        return false;
    }
    
    // Power management task
    result = xTaskCreate(
        powerManagementTask,
        "PowerMgmt", 
        2048,
        this,
        2,     // Priority (medium-high)
        NULL
    );
    if (result != pdPASS) {
        Logger::error("Failed to create power management task");
        return false;
    }
    
    // Sensor monitoring task
    if (m_sensorsReady) {
        result = xTaskCreate(
            sensorMonitorTask,
            "SensorMon",
            2048,
            this,
            2,     // Priority (medium)
            NULL
        );
        if (result != pdPASS) {
            Logger::warning("Failed to create sensor monitor task");
            // Not critical - continue without sensor task
        }
    }
    
    // Motion detection task (if PIR available)
    if (m_pinConfig.pir_pin >= 0) {
        result = xTaskCreate(
            motionDetectionTask,
            "MotionDet",
            2048,
            this,
            2,     // Priority (medium)
            NULL
        );
        if (result != pdPASS) {
            Logger::warning("Failed to create motion detection task");
            // Not critical - continue without motion detection
        }
    }
    
    // Network communication task (if network available)
    if (m_networkReady) {
        result = xTaskCreate(
            networkCommTask,
            "NetComm",
            4096,  // Larger stack for network operations
            this,
            1,     // Priority (low)
            NULL
        );
        if (result != pdPASS) {
            Logger::warning("Failed to create network communication task");
            // Not critical - continue without network task
        }
    }
    
    Logger::info("Task initialization complete");
    Logger::info("  System Monitor: Running (Priority 3)");
    Logger::info("  Power Management: Running (Priority 2)");
    Logger::info("  Sensor Monitor: %s", m_sensorsReady ? "Running (Priority 2)" : "Disabled");
    Logger::info("  Motion Detection: %s", (m_pinConfig.pir_pin >= 0) ? "Running (Priority 2)" : "Disabled");
    Logger::info("  Network Communication: %s", m_networkReady ? "Running (Priority 1)" : "Disabled");
    
    return true;
}

void SystemManager::update() {
    if (!m_initialized || m_state != STATE_RUNNING) {
        return;
    }
    
    unsigned long now = millis();
    
    if (now - m_lastUpdate >= 1000) {
        // Update once per second
        m_lastUpdate = now;
        
        // Reset watchdog
        esp_task_wdt_reset();
        
        // System health checks
        performSystemHealthChecks();
        
        // Update telemetry
        updateSystemTelemetry();
        
        // Check for low power conditions
        checkPowerConditions();
        
        // Blink LED to show system is alive (if not in error state)
        if (m_state == STATE_RUNNING) {
            static bool ledState = false;
            digitalWrite(LED_BUILTIN, ledState);
            ledState = !ledState;
        }
    }
}

void SystemManager::performSystemHealthChecks() {
    // Check memory usage
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 5000) { // Less than 5KB free
        Logger::warning("Low memory: %d bytes free", freeHeap);
    }
    
    // Check component status
    if (!m_cameraReady && m_pinConfig.cam_xclk >= 0) {
        Logger::warning("Camera not ready despite configuration");
    }
}
}

void SystemManager::updateSystemTelemetry() {
    static int telemetryCounter = 0;
    
    // Log system status every 30 seconds
    if (++telemetryCounter >= 30) {
        telemetryCounter = 0;
        
        Logger::info("=== System Telemetry ===");
        Logger::info("  Uptime: %lu ms", millis());
        Logger::info("  Free Heap: %d bytes", ESP.getFreeHeap());
        
        if (g_powerManager) {
            PowerManager::PowerStats stats = g_powerManager->getStatistics();
            Logger::info("  Battery: %.2fV (%.1f%%)", stats.batteryVoltage, stats.batteryPercentage);
            Logger::info("  Solar: %.2fV (%s)", stats.solarVoltage, stats.isCharging ? "Charging" : "Not Charging");
        }
        
        Logger::info("  Components: Camera=%s, Storage=%s, Network=%s, Sensors=%s",
                    m_cameraReady ? "OK" : "ERR",
                    m_storageReady ? "OK" : "ERR", 
                    m_networkReady ? "OK" : "ERR",
                    m_sensorsReady ? "OK" : "ERR");
    }
}

void SystemManager::checkPowerConditions() {
    if (g_powerManager) {
        // Power checks are now handled by the power management task
        // This is just a backup check in the main loop
        PowerManager::PowerStats stats = g_powerManager->getStatistics();
        if (stats.currentState == PowerManager::PowerState::CRITICAL) {
            setError("Emergency shutdown required - critical battery");
            enterSafeMode();
        }
    }
}

void SystemManager::enterSafeMode() {
    m_state = STATE_SAFE_MODE;
    Logger::error("=== ENTERING SAFE MODE ===");
    Logger::error("Reason: %s", m_lastError);
    
    // Minimal system operation
    // Keep only essential functions running
    m_cameraReady = false;
    m_networkReady = false;
    
    // Flash LED in error pattern
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}

bool SystemManager::validateHardwareConfiguration() {
    Logger::info("Validating hardware configuration...");
    
    // Check for pin conflicts
    std::vector<int8_t> usedPins = BoardDetector::getUsedPins(m_boardType);
    
    // Basic validation - ensure camera pins are available
    if (m_pinConfig.cam_xclk < 0 || m_pinConfig.cam_siod < 0 || m_pinConfig.cam_sioc < 0) {
        Logger::error("Critical camera pins not configured");
        return false;
    }
    
    // Check PSRAM if required
    if (BoardDetector::hasFeature(m_boardType, BoardDetector::FEATURE_PSRAM)) {
        if (!psramFound()) {
            Logger::error("PSRAM required but not found");
            return false;
        }
        Logger::info("PSRAM validated: %d bytes", ESP.getPsramSize());
    }
    
    Logger::info("Hardware configuration validated");
    return true;
}

void SystemManager::printSystemInfo() {
    Logger::info("=== System Information ===");
    Logger::info("ESP32 Chip: %s", ESP.getChipModel());
    Logger::info("Chip Revision: %d", ESP.getChipRevision());
    Logger::info("Flash Size: %d bytes", ESP.getFlashChipSize());
    Logger::info("Free Heap: %d bytes", ESP.getFreeHeap());
    
    if (psramFound()) {
        Logger::info("PSRAM Size: %d bytes", ESP.getPsramSize());
        Logger::info("Free PSRAM: %d bytes", ESP.getFreePsram());
    }
    
    Logger::info("Components Status:");
    Logger::info("  Camera: %s", m_cameraReady ? "Ready" : "Not Ready");
    Logger::info("  Storage: %s", m_storageReady ? "Ready" : "Not Ready");
    Logger::info("  Network: %s", m_networkReady ? "Ready" : "Not Ready");
    Logger::info("  Sensors: %s", m_sensorsReady ? "Ready" : "Not Ready");
}

void SystemManager::setError(const char* error) {
    strncpy(m_lastError, error, sizeof(m_lastError) - 1);
    m_lastError[sizeof(m_lastError) - 1] = '\0';
    m_errorCount++;
    m_state = STATE_ERROR;
    Logger::error("System error: %s", error);
}

// FreeRTOS Task Implementations

void SystemManager::systemMonitorTask(void* parameter) {
    SystemManager* system = static_cast<SystemManager*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(5000); // 5 second intervals
    
    Logger::info("System Monitor Task started");
    
    for (;;) {
        // System health monitoring
        if (system->m_initialized && system->m_state == STATE_RUNNING) {
            // Check free heap memory
            size_t freeHeap = ESP.getFreeHeap();
            if (freeHeap < 10000) { // Less than 10KB free
                Logger::warning("Low memory warning: %d bytes free", freeHeap);
            }
            
            // Check power status if power manager is available
            if (g_powerManager) {
                PowerManager::PowerStats stats = g_powerManager->getStatistics();
                if (stats.currentState == PowerManager::PowerState::CRITICAL) {
                    Logger::error("Critical power warning detected");
                    system->setError("Critical battery level");
                }
            }
            
            // Reset watchdog
            esp_task_wdt_reset();
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

void SystemManager::powerManagementTask(void* parameter) {
    SystemManager* system = static_cast<SystemManager*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(10000); // 10 second intervals
    
    Logger::info("Power Management Task started");
    
    for (;;) {
        if (g_powerManager && system->m_initialized) {
            // Update power system
            g_powerManager->update();
            
            // Check power status
            PowerManager::PowerStats stats = g_powerManager->getStatistics();
            if (stats.currentState == PowerManager::PowerState::CRITICAL) {
                Logger::error("Critical battery level detected");
                system->setError("Critical battery level");
            } else if (stats.currentState == PowerManager::PowerState::LOW_BATTERY) {
                Logger::warning("Low battery level detected");
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

void SystemManager::sensorMonitorTask(void* parameter) {
    SystemManager* system = static_cast<SystemManager*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(30000); // 30 second intervals
    
    Logger::info("Sensor Monitor Task started");
    
    for (;;) {
        if (system->m_sensorsReady && system->m_state == STATE_RUNNING) {
            // Read environmental sensors
            // This would include BME280, light sensors, etc.
            
            // Notify power manager of sensor activity
            if (g_powerManager) {
                g_powerManager->onSensorActivity();
            }
            
            // Log sensor readings periodically
            static int sensorLogCounter = 0;
            if (++sensorLogCounter >= 10) { // Every 5 minutes
                Logger::info("Sensor monitoring active");
                sensorLogCounter = 0;
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

void SystemManager::motionDetectionTask(void* parameter) {
    SystemManager* system = static_cast<SystemManager*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(1000); // 1 second intervals
    
    Logger::info("Basic PIR Motion Detection Task started");
    
    for (;;) {
        if (system->m_pinConfig.pir_pin >= 0 && system->m_state == STATE_RUNNING) {
            // Basic PIR detection
            static bool lastPirState = false;
            static unsigned long lastMotionTime = 0;
            
            bool currentPirState = digitalRead(system->m_pinConfig.pir_pin);
            
            // Detect motion (PIR signal change)
            if (currentPirState && !lastPirState) {
                unsigned long now = millis();
                if (now - lastMotionTime > 5000) { // Debounce: 5 second minimum between detections
                    Logger::info("Basic PIR motion detected!");
                    lastMotionTime = now;
                    
                    // Trigger basic camera capture
                    if (system->captureImage()) {
                        Logger::info("PIR-triggered image captured successfully");
                    } else {
                        Logger::error("Failed to capture PIR-triggered image");
                    }
                }
            }
            
            lastPirState = currentPirState;
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

void SystemManager::networkCommTask(void* parameter) {
    SystemManager* system = static_cast<SystemManager*>(parameter);
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(60000); // 60 second intervals
    
    Logger::info("Network Communication Task started");
    
    for (;;) {
        if (system->m_networkReady && system->m_state == STATE_RUNNING) {
            // Handle network communications
            // This could include WiFi status checks, data uploads, etc.
            
            // Notify power manager of network activity
            if (g_powerManager) {
                g_powerManager->onNetworkActivity();
            }
            
            // Log network status periodically
            static int networkLogCounter = 0;
            if (++networkLogCounter >= 5) { // Every 5 minutes
                Logger::info("Network communication active");
                networkLogCounter = 0;
            }
        }
        
        vTaskDelayUntil(&lastWakeTime, frequency);
    }
}

// Camera operation methods

bool SystemManager::captureImage(const String& folder) {
    if (!m_cameraReady || !m_cameraManager) {
        Logger::error("Camera not ready for capture");
        return false;
    }
    
    Logger::info("Capturing image to folder: %s", folder.c_str());
    
    // Capture image using camera manager
    camera_fb_t* fb = m_cameraManager->captureToBuffer();
    if (!fb) {
        Logger::error("Failed to capture image");
        return false;
    }
    
    Logger::info("Image captured successfully - %dx%d, %d bytes", 
                 fb->width, fb->height, fb->len);
    
    // Save to SD card if storage is available
    String filename = "";
    if (m_storageReady) {
        filename = saveImageToSD(fb, folder.c_str());
        if (filename.length() > 0) {
            Logger::info("Image saved as: %s", filename.c_str());
        } else {
            Logger::warning("Failed to save image to storage");
        }
    } else {
        Logger::warning("Storage not available - image not saved");
    }
    
    // Return frame buffer to system
    m_cameraManager->returnFrameBuffer(fb);
    
    // Update statistics
    static int imageCount = 0;
    imageCount++;
    Logger::info("Total images captured: %d", imageCount);
    
    return filename.length() > 0 || !m_storageReady; // Success if saved or if we're just testing capture
}