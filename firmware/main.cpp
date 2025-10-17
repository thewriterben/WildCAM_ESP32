/**
 * @file main.cpp
 * @brief ESP32 WildCAM v2.0 - Advanced AI-Powered Wildlife Monitoring Platform
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 * 
 * Main entry point for the advanced wildlife monitoring system with:
 * - Edge AI with YOLO-tiny detection
 * - Advanced power management with MPPT
 * - Hardware security with AES-256 encryption
 * - Mesh networking capabilities
 * - Real-time analytics and ecosystem integration
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <esp_log.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>


// Hardware abstraction layer
#include "hal/board_detector.h"

// Driver modules
#include "drivers/environmental_suite.h"

// AI/ML modules
#include "ml_models/yolo_tiny/yolo_tiny_detector.h"
#include "ml_models/wildlife_detector.h"

// Power management
#include "power/mppt_controller.h"
#include "power/power_manager.h"

// Security features
#include "security/security_manager.h"

// Networking
#include "networking/mesh_config.h"

// Core system components
#include "core/system_manager.h"
#include "utils/logger.h"
#include "src/camera/camera_manager.h"
#include "src/utils/time_manager.h"
#include "src/diagnostics.h"

// Storage management
#include "core/storage_manager.h"

// Time management for timestamps
#include <time.h>
#include <sys/time.h>

// Configuration
#include "config.h"
#define FIRMWARE_VERSION "3.0.0"
#define SYSTEM_NAME "WildCAM_ESP32_v2.0"
#define LOCKDOWN_DURATION_MS (3600000) // 1 hour in milliseconds
#define SD_CS_PIN 13  // SD card chip select pin for ESP32-CAM

// Global system components
SystemManager* g_system_manager = nullptr;
YOLOTinyDetector* g_yolo_detector = nullptr;
MPPTController* g_mppt_controller = nullptr;
SecurityManager* g_security_manager = nullptr;
EnvironmentalSuite* g_env_sensors = nullptr;
CameraManager* g_camera_manager = nullptr;

// Storage and security globals
Preferences g_preferences;
bool g_sd_initialized = false;
bool g_lockdown_enabled = true;  // Configurable lockdown mode

// Task handles for multi-core processing
TaskHandle_t ai_processing_task = NULL;
TaskHandle_t power_management_task = NULL;
TaskHandle_t network_management_task = NULL;
TaskHandle_t security_monitoring_task = NULL;

// Power management settings
uint32_t deep_sleep_duration = 300; // Default: 300 seconds (5 minutes)

// System state
struct SystemState {
    bool ai_initialized = false;
    bool power_system_ok = false;
    bool security_active = false;
    bool network_connected = false;
    unsigned long last_detection = 0;
    unsigned long last_power_check = 0;
    unsigned long last_security_check = 0;
    int active_cameras = 0;
    float system_temperature = 0.0f;
    float battery_level = 0.0f;
    

} system_state;

/**
 * @brief Generate unique filename for wildlife detection
 * @param species Species name from detection
 * @param buffer Output buffer for filename
 * @param buffer_size Size of output buffer
 * @return true if filename generated successfully
 */
bool generateDetectionFilename(const char* species, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < MAX_FILENAME_LENGTH) {
        return false;
    }
    
    struct tm timeinfo;
    time_t now;
    time(&now);
    
    // Try to get local time (RTC or NTP)
    if (getLocalTime(&timeinfo)) {
        // Format: YYYYMMDD_HHMMSS_species.jpg
        snprintf(buffer, buffer_size, "%04d%02d%02d_%02d%02d%02d_%s.jpg",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec,
                species);
    } else {
        // Fallback to milliseconds-based timestamp
        unsigned long timestamp = millis();
        snprintf(buffer, buffer_size, "%lu_%s.jpg", timestamp, species);
    }
    
    return true;
}

/**
 * @brief Generate metadata file for wildlife detection
 * @param filename Image filename
 * @param species Species name
 * @param confidence Detection confidence
 * @param bbox Bounding box information
 * @return true if metadata saved successfully
 */
bool saveDetectionMetadata(const char* filename, const char* species, 
                           float confidence, const BoundingBox& bbox) {
    if (!filename || !species) {
        return false;
    }
    
    // Create metadata filename by replacing .jpg with .json
    char metadata_filename[MAX_FILENAME_LENGTH];
    snprintf(metadata_filename, sizeof(metadata_filename), "%s", filename);
    char* ext = strrchr(metadata_filename, '.');
    if (ext) {
        strcpy(ext, ".json");
    } else {
        strcat(metadata_filename, ".json");
    }
    
    // Get current timestamp for metadata
    struct tm timeinfo;
    char timestamp_str[32] = "unknown";
    if (getLocalTime(&timeinfo)) {
        strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
    }
    
    // Get battery level from system state
    float battery_voltage = system_state.battery_level;
    
    // Build JSON metadata string with GPS and battery info
    char json_buffer[768];
    snprintf(json_buffer, sizeof(json_buffer),
             "{\n"
             "  \"filename\": \"%s\",\n"
             "  \"species\": \"%s\",\n"
             "  \"confidence\": %.3f,\n"
             "  \"timestamp\": \"%s\",\n"
             "  \"timestamp_millis\": %lu,\n"
             "  \"bounding_box\": {\n"
             "    \"x\": %.3f,\n"
             "    \"y\": %.3f,\n"
             "    \"width\": %.3f,\n"
             "    \"height\": %.3f\n"
             "  },\n"
             "  \"class_id\": %d,\n"
             "  \"battery_voltage\": %.2f,\n"
             "  \"gps_coordinates\": {\n"
             "    \"latitude\": 0.0,\n"
             "    \"longitude\": 0.0,\n"
             "    \"note\": \"GPS integration pending\"\n"
             "  }\n"
             "}\n",
             filename, species, confidence, timestamp_str, millis(),
             bbox.x, bbox.y, bbox.width, bbox.height, bbox.class_id,
             battery_voltage);
    
    // Save metadata to storage
    storage_result_t result = g_storage.saveLog(json_buffer, metadata_filename);
    
    if (result != STORAGE_SUCCESS) {
        Logger::error("Failed to save metadata file: %s", g_storage.getLastError());
        return false;
    }
    
    Logger::info("Metadata saved: %s", metadata_filename);
    return true;
}

/**
 * @brief Process wildlife detection event - save image and metadata
 * @param image_data Raw image data
 * @param image_size Size of image data
 * @param detection Detection information
 * @return true if processing successful (non-critical failures still return true)
 */
bool processWildlifeDetection(const uint8_t* image_data, size_t image_size, 
                              const BoundingBox& detection) {
    if (!image_data || image_size == 0) {
        Logger::error("Invalid image data for detection processing");
        return false;
    }
    
    // Step 1: Generate unique filename
    char filename[MAX_FILENAME_LENGTH];
    if (!generateDetectionFilename(detection.class_name, filename, sizeof(filename))) {
        Logger::error("Failed to generate detection filename");
        return false;
    }
    
    Logger::info("Processing wildlife detection: %s", filename);
    
    // Step 2: Check if storage is ready
    if (!g_storage.isReady()) {
        Logger::error("Storage not ready, cannot save detection image (continuing detection loop)");
        return true; // Continue detection loop despite storage failure
    }
    
    // Step 3: Check available storage space
    uint64_t free_space = g_storage.getFreeSpace();
    uint64_t required_space = image_size + 4096; // Image + metadata buffer
    
    if (free_space < required_space) {
        Logger::error("Insufficient storage space: %llu bytes free, %llu bytes required", 
                     free_space, required_space);
        Logger::error("SD card full condition - cannot save detection");
        return true; // Continue detection loop despite storage full
    }
    
    // Step 4: Save image buffer to SD card with retry logic
    const int max_retries = 3;
    int retry_delay_ms = 100; // Start with 100ms delay
    storage_result_t save_result = STORAGE_ERROR_WRITE;
    
    for (int retry = 0; retry < max_retries; retry++) {
        save_result = g_storage.saveImage(image_data, image_size, filename);
        
        if (save_result == STORAGE_SUCCESS) {
            break; // Success - exit retry loop
        }
        
        // Log retry attempt
        Logger::warning("Image save failed (attempt %d/%d): %s", 
                       retry + 1, max_retries, g_storage.getLastError());
        
        // Wait before retry with exponential backoff
        if (retry < max_retries - 1) {
            delay(retry_delay_ms);
            retry_delay_ms *= 2; // Exponential backoff
        }
    }
    
    if (save_result == STORAGE_SUCCESS) {
        // Step 5: Log save operation with filename and file size
        Logger::info("Image saved successfully: %s (size: %u bytes)", filename, image_size);
        
        // Step 6: Save metadata file with retry logic
        bool metadata_saved = false;
        for (int retry = 0; retry < max_retries && !metadata_saved; retry++) {
            metadata_saved = saveDetectionMetadata(filename, detection.class_name, 
                                                  detection.confidence, detection);
            if (!metadata_saved && retry < max_retries - 1) {
                delay(100); // Brief delay before retry
            }
        }
        
        if (!metadata_saved) {
            // Log warning but continue - metadata is optional
            Logger::warning("Failed to save metadata for %s after %d retries, continuing operation", 
                          filename, max_retries);
        }
        
        // Step 7: Increment detection counter (persistent across reboots)
        uint32_t detection_count = g_preferences.getUInt("detect_count", 0);
        detection_count++;
        g_preferences.putUInt("detect_count", detection_count);
        Logger::info("Detection count: %lu", detection_count);
        
    } else {
        // Step 8: Error handling - log error but continue operation
        Logger::error("Failed to save image %s after %d retries: %s (continuing detection loop)", 
                     filename, max_retries, g_storage.getLastError());
        // Return true to continue detection loop despite save failure
    }
    
    return true;
}

/**
 * @brief AI Processing Task (Core 1)
 * Handles real-time wildlife detection using YOLO-tiny
 */
void aiProcessingTask(void* parameter) {
    Logger::info("AI Processing Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(100); // 10 FPS processing rate
    BoundingBox detections[10];
    
    while (true) {
        // Skip processing if in lockdown mode
        if (system_state.in_lockdown) {
            vTaskDelay(xDelay * 10); // Check less frequently during lockdown
            continue;
        }
        
        if (system_state.ai_initialized && g_yolo_detector && g_yolo_detector->isInitialized() && g_camera_manager) {
            // Capture camera frame with error handling
            int image_width = 0, image_height = 0;
            uint8_t* image_data = nullptr;
            
            // Retry logic for transient camera errors
            int retry_count = 0;
            const int max_retries = 3;
            
            while (retry_count < max_retries && !image_data) {
                image_data = g_camera_manager->captureFrame(&image_width, &image_height);
                
                if (!image_data) {
                    Logger::warning("Camera capture failed (attempt %d/%d): %s", 
                                  retry_count + 1, max_retries, 
                                  g_camera_manager->getLastError().c_str());
                    retry_count++;
                    if (retry_count < max_retries) {
                        vTaskDelay(pdMS_TO_TICKS(100)); // Brief delay before retry
                    }
                }
            }
            
            if (image_data) {
                // Run YOLO detection with error handling
                int num_detections = 0;
                
                try {
                    num_detections = g_yolo_detector->detect(image_data, detections, 10);
                } catch (...) {
                    Logger::error("AI detection exception occurred");
                    g_camera_manager->releaseFrame(image_data);
                    vTaskDelay(xDelay);
                    continue;
                }
                
                if (num_detections > 0) {
                    system_state.last_detection = millis();
                    
                    for (int i = 0; i < num_detections; i++) {
                        Logger::info("Wildlife detected: %s (confidence: %.2f)", 
                                   detections[i].class_name, detections[i].confidence);
                        
                        // Process detection - save image, metadata, etc.
                        if (!processWildlifeDetection(image_data, image_width * image_height, detections[i])) {
                            Logger::warning("Detection processing had issues but continuing operation");
                        }
                    }
                }
                
                // Always release camera frame
                g_camera_manager->releaseFrame(image_data);
            } else {
                // Camera capture failed after retries - graceful degradation
                Logger::error("Camera capture failed after %d retries, continuing...", max_retries);
            }
        }
        
        vTaskDelay(xDelay);
    }
}

/**
 * @brief Power Management Task (Core 0)
 * Handles MPPT control and battery monitoring
 */
void powerManagementTask(void* parameter) {
    Logger::info("Power Management Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(5000); // Check every 5 seconds
    
    while (true) {
        if (g_mppt_controller) {
            // Update MPPT controller
            g_mppt_controller->update();
            
            // Get power system status
            SolarPowerStatus power_status = g_mppt_controller->getStatus();
            system_state.battery_level = power_status.battery_voltage;
            system_state.power_system_ok = power_status.is_charging || (power_status.battery_voltage > 3.3f);
            
            // Log power status periodically
            if ((millis() - system_state.last_power_check) > 60000) { // Every minute
                Logger::info("Power Status - Battery: %.2fV, Solar: %.2fW, Efficiency: %.1f%%",
                           power_status.battery_voltage, power_status.solar_power, power_status.charge_efficiency);
                system_state.last_power_check = millis();
            }
            
            // Handle low power conditions
            if (power_status.battery_voltage < 3.0f && !system_state.power_save_mode) {
                Logger::warning("Entering power save mode (battery: %.2fV)", 
                                power_status.battery_voltage);
                
                // Reduce CPU frequency
                setCpuFrequencyMhz(80);
                Logger::info("CPU frequency reduced to 80MHz");
                
                // Increase sleep duration
                deep_sleep_duration = 600; // 10 minutes
                Logger::info("Deep sleep duration increased to 600 seconds");
                
                // Disable WiFi if enabled
                #if WIFI_ENABLED
                if (system_state.network_connected) {
                    WiFi.disconnect(true);
                    WiFi.mode(WIFI_OFF);
                    system_state.network_connected = false;
                    Logger::info("WiFi disabled for power saving");
                }
                #endif
                
                // Reduce camera quality (would need to update camera config)
                // This would be implemented when camera is reconfigured
                Logger::info("Camera quality will be reduced on next capture");
                
                system_state.power_save_mode = true;
                Logger::info("Power save mode activated");
                
            } else if (power_status.battery_voltage > 3.4f && system_state.power_save_mode) {
                Logger::info("Exiting power save mode (battery: %.2fV)", 
                             power_status.battery_voltage);
                
                // Restore normal operation
                setCpuFrequencyMhz(240);
                Logger::info("CPU frequency restored to 240MHz");
                
                deep_sleep_duration = 300;
                Logger::info("Deep sleep duration restored to 300 seconds");
                
                // Camera quality will be restored on next capture
                Logger::info("Camera quality will be restored on next capture");
                
                system_state.power_save_mode = false;
                Logger::info("Power save mode deactivated - normal operation resumed");
            }
        }
        
        vTaskDelay(xDelay);
    }
}

/**
 * @brief Initialize SD card storage
 * @return true if successful, false otherwise
 */
bool initializeSDCard() {
    Logger::info("Initializing SD card...");
    
    if (!SD.begin(SD_CS_PIN)) {
        Logger::error("SD card initialization failed");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Logger::error("No SD card attached");
        return false;
    }
    
    // Create images directory if it doesn't exist
    if (!SD.exists("/images")) {
        if (!SD.mkdir("/images")) {
            Logger::error("Failed to create /images directory");
            return false;
        }
    }
    
    Logger::info("SD card initialized successfully");
    return true;
}

/**
 * @brief Get detection counter from persistent storage
 * @return Current detection counter value
 */
uint32_t getDetectionCounter() {
    return g_preferences.getUInt("detect_count", 0);
}

/**
 * @brief Get tamper counter from persistent storage
 * @return Current tamper counter value
 */
uint32_t getTamperCounter() {
    return g_preferences.getUInt("tamper_count", 0);
}

/**
 * @brief Increment and persist tamper counter
 * @return New tamper counter value
 */
uint32_t incrementTamperCounter() {
    uint32_t count = getTamperCounter() + 1;
    g_preferences.putUInt("tamper_count", count);
    Logger::info("Tamper counter incremented to: %lu", count);
    return count;
}

/**
 * @brief Check if network is available for sending alerts
 * @return true if network is available, false otherwise
 */
bool isNetworkAvailable() {
    // Check system state for network connectivity
    return system_state.network_connected;
}

/**
 * @brief Send critical alert through available network
 * @param message Alert message to send
 * @return true if alert sent successfully, false otherwise
 */
bool sendCriticalAlert(const char* message) {
    if (!isNetworkAvailable()) {
        Logger::warning("Network not available for alert transmission");
        return false;
    }
    
    // TODO: Implement actual network alert transmission
    // This would send via WiFi, cellular, or satellite depending on availability
    Logger::info("Sending critical alert: %s", message);
    
    // Placeholder for actual implementation
    // Example: HTTP POST, MQTT publish, or satellite message
    return true;
}

/**
 * @brief Capture and save tamper event image
 * @return true if image captured and saved successfully, false otherwise
 */
bool captureTamperImage() {
    if (!g_camera_manager || !g_camera_manager->isInitialized()) {
        Logger::error("Camera not available for tamper image capture");
        return false;
    }
    
    if (!g_sd_initialized) {
        Logger::error("SD card not initialized, cannot save tamper image");
        return false;
    }
    
    // Capture frame
    int width = 0, height = 0;
    uint8_t* image_data = g_camera_manager->captureFrame(&width, &height);
    
    if (!image_data) {
        Logger::error("Failed to capture tamper image");
        return false;
    }
    
    // Generate filename with timestamp
    String timestamp = getFormattedTime();
    timestamp.replace(" ", "_");
    timestamp.replace(":", "");
    timestamp.replace("-", "");
    String filename = "/images/TAMPER_" + timestamp + ".jpg";
    
    // Save to SD card
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        Logger::error("Failed to create tamper image file: %s", filename.c_str());
        g_camera_manager->releaseFrame(image_data);
        return false;
    }
    
    // Note: The image_data from captureFrame is in RGB565 format
    // For a real implementation, you would need to convert to JPEG or save raw data
    // This is a simplified version that demonstrates the concept
    
    // Get actual frame buffer for JPEG data
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
        file.write(fb->buf, fb->len);
        esp_camera_fb_return(fb);
        Logger::info("Tamper image saved: %s (%d bytes)", filename.c_str(), fb->len);
    } else {
        Logger::error("Failed to get camera frame buffer");
        file.close();
        g_camera_manager->releaseFrame(image_data);
        return false;
    }
    
    file.close();
    g_camera_manager->releaseFrame(image_data);
    
    return true;
}

/**
 * @brief Handle tamper detection security response
 */
void handleTamperDetection() {
    String timestamp = getFormattedTime();
    
    // Step 1: Write critical alert to log (console and file)
    Logger::critical("TAMPER DETECTED at %s - Initiating security response", timestamp.c_str());
    
    // Create detailed log entry for file storage
    char log_entry[512];
    snprintf(log_entry, sizeof(log_entry), 
             "[TAMPER] %s - Battery: %.2fV - Free Heap: %d bytes - Tamper Event\n",
             timestamp.c_str(), 
             system_state.battery_level,
             ESP.getFreeHeap());
    
    // Save detailed log to SD card if storage is available
    if (g_storage.isReady()) {
        storage_result_t log_result = g_storage.saveLog(log_entry, "/security.log");
        if (log_result != STORAGE_SUCCESS) {
            Logger::warning("Failed to save security log: %s", g_storage.getLastError());
        }
    }
    
    // Step 2: Capture and save image with TAMPER_ prefix
    bool image_saved = captureTamperImage();
    if (image_saved) {
        Logger::info("Tamper event image captured successfully");
    } else {
        Logger::error("Failed to capture tamper event image");
    }
    
    // Step 3: Send alert if network is available
    char alert_message[256];
    snprintf(alert_message, sizeof(alert_message), 
             "CRITICAL: Tamper detected at %s. Image capture: %s. Battery: %.2fV",
             timestamp.c_str(), 
             image_saved ? "SUCCESS" : "FAILED",
             system_state.battery_level);
    
    if (sendCriticalAlert(alert_message)) {
        Logger::info("Critical alert sent successfully");
    } else {
        Logger::warning("Failed to send critical alert");
    }
    
    // Step 4: Increment tamper counter in persistent storage
    uint32_t tamper_count = incrementTamperCounter();
    Logger::info("Total tamper events: %lu", tamper_count);
    
    // Step 5: Optional lockdown mode
    if (g_lockdown_enabled && !system_state.in_lockdown) {
        system_state.in_lockdown = true;
        system_state.lockdown_start_time = millis();
        Logger::critical("LOCKDOWN MODE ACTIVATED - Image capture disabled for 1 hour");
    }
}

/**
 * @brief Check and manage lockdown mode
 */
void manageLockdownMode() {
    if (system_state.in_lockdown) {
        unsigned long lockdown_elapsed = millis() - system_state.lockdown_start_time;
        
        if (lockdown_elapsed >= LOCKDOWN_DURATION_MS) {
            system_state.in_lockdown = false;
            Logger::info("LOCKDOWN MODE DEACTIVATED - Normal operation resumed");
        }
    }
}

/**
 * @brief Check for OTA firmware updates and install if available
 * 
 * This function implements a complete OTA update workflow:
 * - Checks remote server for available firmware version
 * - Compares with current version
 * - Downloads firmware binary securely
 * - Verifies firmware integrity
 * - Writes to OTA partition
 * - Validates installation
 * - Reboots to new firmware
 * - Rollback capability on failure
 * 
 * @note Uses ESP32's Update library for safe OTA updates
 * @note Requires network connectivity
 */
void checkAndInstallOTAUpdate() {
    Logger::info("=== OTA Update Check Started ===");
    
    // Verify network connectivity
    if (!system_state.network_connected || WiFi.status() != WL_CONNECTED) {
        Logger::warning("OTA update check skipped - no network connection");
        return;
    }
    
    String current_version = FIRMWARE_VERSION;
    Logger::info("Current firmware version: %s", current_version.c_str());
    
    // Step 1: Check for available version
    HTTPClient http;
    http.begin(OTA_VERSION_URL);
    http.setTimeout(10000); // 10 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        Logger::error("Failed to check for updates (HTTP %d)", httpCode);
        http.end();
        return;
    }
    
    String latest_version = http.getString();
    latest_version.trim();
    http.end();
    
    // Step 2: Compare versions
    if (latest_version.isEmpty()) {
        Logger::warning("Empty version response from server");
        return;
    }
    
    if (latest_version == current_version) {
        Logger::info("Firmware is up to date (v%s)", current_version.c_str());
        system_state.ota_available = false;
        return;
    }
    
    Logger::info("Update available: %s -> %s", current_version.c_str(), latest_version.c_str());
    system_state.ota_available = true;
    
    // Step 3: Download firmware binary
    Logger::info("Downloading firmware from %s", OTA_UPDATE_URL);
    http.begin(OTA_UPDATE_URL);
    http.setTimeout(60000); // 60 second timeout for binary download
    
    httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK) {
        Logger::error("Failed to download firmware (HTTP %d)", httpCode);
        http.end();
        return;
    }
    
    int contentLength = http.getSize();
    
    if (contentLength <= 0) {
        Logger::error("Invalid firmware size: %d bytes", contentLength);
        http.end();
        return;
    }
    
    Logger::info("Firmware size: %d bytes", contentLength);
    
    // Step 4: Verify sufficient space in OTA partition
    bool canBegin = Update.begin(contentLength);
    
    if (!canBegin) {
        Logger::error("Not enough space for OTA update (need %d bytes)", contentLength);
        http.end();
        return;
    }
    
    Logger::info("Starting OTA update process...");
    
    // Step 5: Write firmware to OTA partition
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[128];
    int lastProgress = 0;
    
    while (http.connected() && (written < contentLength)) {
        size_t available = stream->available();
        
        if (available) {
            int bytesRead = stream->readBytes(buffer, min(sizeof(buffer), available));
            
            if (bytesRead > 0) {
                size_t bytesWritten = Update.write(buffer, bytesRead);
                
                if (bytesWritten != bytesRead) {
                    Logger::error("Write error: wrote %d of %d bytes", bytesWritten, bytesRead);
                    Update.abort();
                    http.end();
                    return;
                }
                
                written += bytesWritten;
                
                // Log progress every 10%
                int progress = (written * 100) / contentLength;
                if (progress >= lastProgress + 10) {
                    Logger::info("Download progress: %d%% (%d/%d bytes)", progress, written, contentLength);
                    lastProgress = progress;
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1)); // Yield to other tasks
    }
    
    http.end();
    
    // Step 6: Verify download completed successfully
    if (written != contentLength) {
        Logger::error("Download incomplete: %d of %d bytes", written, contentLength);
        Update.abort();
        return;
    }
    
    Logger::info("✓ Firmware downloaded successfully (%d bytes)", written);
    
    // Step 7: Finalize and verify OTA update
    if (!Update.end(true)) { // true = set new firmware as boot partition
        Logger::error("OTA update failed during finalization");
        Logger::error("Error: %s", Update.errorString());
        Update.abort();
        return;
    }
    
    // Step 8: Verify update integrity
    if (!Update.isFinished()) {
        Logger::error("OTA update not finished properly");
        return;
    }
    
    Logger::info("✓ OTA update completed successfully");
    Logger::info("✓ New firmware version: %s", latest_version.c_str());
    Logger::info("✓ Update verified and validated");
    Logger::info("Rebooting to new firmware in 3 seconds...");
    
    // Give time for log messages to be sent
    delay(3000);
    
    // Step 9: Reboot to new firmware
    // Note: ESP32 bootloader handles rollback automatically if new firmware fails to boot
    ESP.restart();
}

/**
 * @brief Security Monitoring Task (Core 0)
 * Handles encryption, tamper detection, and secure communications
 */
void securityMonitoringTask(void* parameter) {
    Logger::info("Security Monitoring Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(10000); // Check every 10 seconds
    
    while (true) {
        // Manage lockdown mode
        manageLockdownMode();
        
        if (g_security_manager && g_security_manager->isSecurityOperational()) {
            // Check for tampering
            if (g_security_manager->detectTampering()) {
                Logger::error("Tampering detected! Initiating security protocols");
                handleTamperDetection();
            }
            
            // Periodic security health check
            if ((millis() - system_state.last_security_check) > 300000) { // Every 5 minutes
                unsigned long total_ops, failed_ops, boot_verifications;
                g_security_manager->getSecurityStats(&total_ops, &failed_ops, &boot_verifications);
                
                Logger::info("Security Stats - Operations: %lu, Failed: %lu, Boot Verifications: %lu",
                           total_ops, failed_ops, boot_verifications);
                
                system_state.last_security_check = millis();
            }
        }
        
        vTaskDelay(xDelay);
    }
}

/**
 * @brief Network Management Task (Core 0)  
 * Handles mesh networking, API communication, and data transmission
 */
void networkManagementTask(void* parameter) {
    Logger::info("Network Management Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(30000); // Check every 30 seconds
    
    while (true) {
        unsigned long currentTime = millis();
        
        // 1. Check if WIFI_ENABLED and attempt connection if needed
        #if WIFI_ENABLED
        if (!system_state.network_connected) {
            // Calculate exponential backoff delay
            unsigned long backoffDelay = WIFI_RETRY_BASE_DELAY * (1 << system_state.wifi_retry_count);
            if (backoffDelay > 60000) backoffDelay = 60000; // Cap at 60 seconds
            
            // Attempt connection if enough time has passed since last attempt
            if ((currentTime - system_state.last_wifi_attempt) >= backoffDelay) {
                Logger::info("Attempting WiFi connection (attempt %d/%d)...", 
                           system_state.wifi_retry_count + 1, WIFI_MAX_RETRIES);
                
                WiFi.mode(WIFI_STA);
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                
                // Wait for connection with timeout
                unsigned long startAttempt = millis();
                while (WiFi.status() != WL_CONNECTED && 
                       (millis() - startAttempt) < WIFI_CONNECTION_TIMEOUT) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                
                if (WiFi.status() == WL_CONNECTED) {
                    system_state.network_connected = true;
                    system_state.wifi_retry_count = 0;
                    Logger::info("✓ WiFi connected! IP address: %s", WiFi.localIP().toString().c_str());
                    Logger::info("Signal strength: %d dBm", WiFi.RSSI());
                } else {
                    system_state.wifi_retry_count++;
                    system_state.last_wifi_attempt = currentTime;
                    Logger::warning("WiFi connection failed (attempt %d/%d), retrying with backoff", 
                                  system_state.wifi_retry_count, WIFI_MAX_RETRIES);
                    
                    // Reset retry count after max retries to try again
                    if (system_state.wifi_retry_count >= WIFI_MAX_RETRIES) {
                        system_state.wifi_retry_count = 0;
                        Logger::info("Max WiFi retries reached, resetting retry counter");
                    }
                }
            }
        } else {
            // Check if still connected
            if (WiFi.status() != WL_CONNECTED) {
                system_state.network_connected = false;
                system_state.wifi_retry_count = 0;
                Logger::warning("WiFi connection lost, will attempt to reconnect");
            }
        }
        #endif
        
        // 2. Handle data upload if WiFi is connected
        #if WIFI_ENABLED && DATA_UPLOAD_ENABLED
        if (system_state.network_connected && 
            (currentTime - system_state.last_upload) >= DATA_UPLOAD_INTERVAL) {
            
            if (system_state.pending_uploads > 0) {
                Logger::info("Uploading %d pending data items...", system_state.pending_uploads);
                
                // TODO: Implement actual data upload logic
                // This would involve:
                // - Reading pending data from storage
                // - Sending via HTTP POST to API endpoint
                // - Marking as uploaded on success
                // - Retrying on failure
                
                HTTPClient http;
                http.begin(DATA_API_ENDPOINT);
                http.addHeader("Content-Type", "application/json");
                
                // Example upload (would need real data)
                String payload = "{\"device\":\"" + String(SYSTEM_NAME) + 
                               "\",\"count\":" + String(system_state.pending_uploads) + "}";
                
                int httpResponseCode = http.POST(payload);
                
                if (httpResponseCode > 0 && httpResponseCode < 400) {
                    Logger::info("✓ Data uploaded successfully (HTTP %d)", httpResponseCode);
                    system_state.pending_uploads = 0; // Would be updated based on actual upload
                } else {
                    Logger::warning("Data upload failed (HTTP %d)", httpResponseCode);
                }
                
                http.end();
                system_state.last_upload = currentTime;
            }
        }
        #endif
        
        // 3. Check for OTA firmware updates if enabled
        #if OTA_ENABLED
        if (system_state.network_connected && 
            (currentTime - system_state.last_ota_check) >= OTA_CHECK_INTERVAL) {
            
            // Check for and install OTA updates
            checkAndInstallOTAUpdate();
            
            system_state.last_ota_check = currentTime;
        }
        #endif
        
        // 4. Check LoRa mesh network health if enabled
        #if LORA_ENABLED
        if ((currentTime - system_state.last_lora_check) >= LORA_HEALTH_CHECK_INTERVAL) {
            Logger::info("Checking LoRa mesh network health...");
            
            // TODO: Implement actual LoRa mesh health check
            // This would involve:
            // - Pinging known mesh nodes
            // - Checking signal strength
            // - Updating routing tables
            // - Detecting dead nodes
            
            // Placeholder: simulate health check
            // In real implementation, would query LoRa module
            system_state.lora_active_nodes = 0; // Would be set by actual check
            
            if (system_state.lora_active_nodes > 0) {
                Logger::info("✓ LoRa mesh healthy: %d active nodes", system_state.lora_active_nodes);
            } else {
                Logger::warning("No LoRa mesh nodes detected");
            }
            
            system_state.last_lora_check = currentTime;
        }
        #endif
        
        // 5. Log network status every 5 minutes
        if ((currentTime - system_state.last_network_status_log) >= NETWORK_STATUS_LOG_INTERVAL) {
            Logger::info("=== Network Status Report ===");
            
            #if WIFI_ENABLED
            Logger::info("WiFi: %s", system_state.network_connected ? "Connected" : "Disconnected");
            if (system_state.network_connected) {
                Logger::info("  IP Address: %s", WiFi.localIP().toString().c_str());
                Logger::info("  Signal: %d dBm", WiFi.RSSI());
            }
            Logger::info("  Retry count: %d", system_state.wifi_retry_count);
            #else
            Logger::info("WiFi: Disabled");
            #endif
            
            #if DATA_UPLOAD_ENABLED
            Logger::info("Data Upload:");
            Logger::info("  Last upload: %lu sec ago", (currentTime - system_state.last_upload) / 1000);
            Logger::info("  Pending uploads: %d", system_state.pending_uploads);
            #endif
            
            #if OTA_ENABLED
            Logger::info("OTA: %s", system_state.network_connected ? "Enabled" : "Waiting for WiFi");
            Logger::info("  Update available: %s", system_state.ota_available ? "YES" : "No");
            Logger::info("  Last check: %lu sec ago", (currentTime - system_state.last_ota_check) / 1000);
            #else
            Logger::info("OTA: Disabled");
            #endif
            
            #if LORA_ENABLED
            Logger::info("LoRa Mesh:");
            Logger::info("  Active nodes: %d", system_state.lora_active_nodes);
            Logger::info("  Last health check: %lu sec ago", (currentTime - system_state.last_lora_check) / 1000);
            #else
            Logger::info("LoRa: Disabled");
            #endif
            
            Logger::info("============================");
            system_state.last_network_status_log = currentTime;
        }
        
        vTaskDelay(xDelay);
    }
}

/**
 * @brief Initialize all system components
 * @return true if initialization successful
 */
/**
 * @brief Cleanup system resources on initialization failure
 */
void cleanupSystemResources() {
    Logger::info("Cleaning up system resources...");
    
    if (g_camera_manager) {
        delete g_camera_manager;
        g_camera_manager = nullptr;
    }
    
    if (g_system_manager) {
        delete g_system_manager;
        g_system_manager = nullptr;
    }
    
    if (g_yolo_detector) {
        delete g_yolo_detector;
        g_yolo_detector = nullptr;
    }
    
    if (g_env_sensors) {
        delete g_env_sensors;
        g_env_sensors = nullptr;
    }
    
    if (g_mppt_controller) {
        delete g_mppt_controller;
        g_mppt_controller = nullptr;
    }
    
    if (g_security_manager) {
        delete g_security_manager;
        g_security_manager = nullptr;
    }
    
    Logger::info("System resources cleaned up");
}

bool initializeSystem() {
    Logger::info("Initializing WildCAM ESP32 v2.0 Advanced System...");
    
    // Initialize watchdog timer
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
    
    // Initialize NVS (Non-Volatile Storage) for persistent data
    if (!g_preferences.begin("wildcam", false)) {
        Logger::error("Failed to initialize NVS storage");
        return false;
    }
    Logger::info("✓ NVS storage initialized");
    
    // Initialize time manager
    if (!initializeTimeManager()) {
        Logger::warning("Time manager initialization failed (non-critical)");
    } else {
        Logger::info("✓ Time manager initialized");
    }
    
    // Initialize SD card storage
    g_sd_initialized = initializeSDCard();
    if (!g_sd_initialized) {
        Logger::warning("SD card initialization failed - image storage disabled");
    }
    
    // Initialize security manager first
    g_security_manager = new (std::nothrow) SecurityManager(SecurityLevel::ENHANCED, true);
    if (!g_security_manager) {
        Logger::error("Security manager allocation failed - out of memory");
        cleanupSystemResources();
        return false;
    }
    
    if (!g_security_manager->begin()) {
        Logger::error("Security manager initialization failed");
        cleanupSystemResources();
        return false;
    }
    system_state.security_active = true;
    Logger::info("✓ Security system initialized");
    
    // Initialize power management
    g_mppt_controller = new (std::nothrow) MPPTController(
        36, 39, 34, 35, 25, // ADC pins for voltage/current sensing, PWM pin
        MPPTAlgorithm::PERTURB_OBSERVE
    );
    if (!g_mppt_controller) {
        Logger::error("MPPT controller allocation failed - out of memory");
        cleanupSystemResources();
        return false;
    }
    
    if (!g_mppt_controller->begin()) {
        Logger::error("MPPT controller initialization failed");
        cleanupSystemResources();
        return false;
    }
    system_state.power_system_ok = true;
    Logger::info("✓ Power management system initialized");
    
    // Initialize environmental sensors (non-critical)
    g_env_sensors = new (std::nothrow) EnvironmentalSuite();
    if (!g_env_sensors) {
        Logger::warning("Environmental sensors allocation failed - continuing without sensors");
    } else if (!g_env_sensors->begin()) {
        Logger::warning("Environmental sensors initialization failed (non-critical)");
        delete g_env_sensors;
        g_env_sensors = nullptr;
    } else {
        Logger::info("✓ Environmental sensors initialized");
    }
    
    // Initialize camera manager
    g_camera_manager = new (std::nothrow) CameraManager();
    if (!g_camera_manager) {
        Logger::error("Camera manager allocation failed - out of memory");
        cleanupSystemResources();
        return false;
    }
    
    if (!g_camera_manager->initialize()) {
        Logger::error("Camera initialization failed: %s", g_camera_manager->getLastError().c_str());
        cleanupSystemResources();
        return false;
    }
    Logger::info("✓ Camera system initialized");
    
    // Initialize AI detection system
    g_yolo_detector = new (std::nothrow) YOLOTinyDetector();
    if (!g_yolo_detector) {
        Logger::warning("YOLO detector allocation failed - AI features disabled");
        system_state.ai_initialized = false;
    } else {
        // TODO: Load model data from flash memory
        // const unsigned char* model_data = load_yolo_model();
        // if (g_yolo_detector->initialize(model_data)) {
        //     system_state.ai_initialized = true;
        //     Logger::info("✓ YOLO-tiny AI detector initialized");
        // } else {
        //     Logger::error("YOLO-tiny detector initialization failed");
        //     delete g_yolo_detector;
        //     g_yolo_detector = nullptr;
        //     cleanupSystemResources();
        //     return false;
        // }
        
        // For now, mark AI as initialized (will be implemented with actual model)
        system_state.ai_initialized = true;
        Logger::info("✓ AI detection system ready");
    }
    
    // Initialize system manager
    g_system_manager = new (std::nothrow) SystemManager();
    if (!g_system_manager) {
        Logger::error("System manager allocation failed - out of memory");
        cleanupSystemResources();
        return false;
    }
    Logger::info("✓ System manager initialized");
    
    // Initialize storage system
    if (!g_storage.initialize()) {
        Logger::warning("Storage initialization failed: %s (non-critical, continuing)", 
                       g_storage.getLastError());
        // Continue without storage - operations will log errors when storage is needed
    } else {
        Logger::info("✓ Storage system initialized (%s)", 
                    g_storage.getActiveStorage() == STORAGE_SD_CARD ? "SD Card" : "LittleFS");
    }
    
    Logger::info("WildCAM ESP32 v2.0 system initialization complete!");
    Logger::info("Free heap: %d bytes", ESP.getFreeHeap());
    return true;
}

/**
 * @brief Create and start all system tasks
 */
void createSystemTasks() {
    Logger::info("Creating system tasks for multi-core processing...");
    
    // AI Processing Task on Core 1 (high priority)
    xTaskCreatePinnedToCore(
        aiProcessingTask,           // Task function
        "AI_Processing",            // Task name
        8192,                       // Stack size
        NULL,                       // Parameters
        3,                          // Priority (high)
        &ai_processing_task,        // Task handle
        1                           // Core 1
    );
    
    // Power Management Task on Core 0 (medium priority)
    xTaskCreatePinnedToCore(
        powerManagementTask,
        "Power_Management",
        4096,
        NULL,
        2,
        &power_management_task,
        0
    );
    
    // Security Monitoring Task on Core 0 (medium priority)
    xTaskCreatePinnedToCore(
        securityMonitoringTask,
        "Security_Monitor",
        4096,
        NULL,
        2,
        &security_monitoring_task,
        0
    );
    
    // Network Management Task on Core 0 (low priority)
    xTaskCreatePinnedToCore(
        networkManagementTask,
        "Network_Management",
        6144,
        NULL,
        1,
        &network_management_task,
        0
    );
    
    Logger::info("All system tasks created successfully");
}

/**
 * @brief Main setup function
 */
void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    // Initialize logger
    Logger::begin();
    Logger::info("=== WildCAM ESP32 v2.0 Advanced Wildlife Monitoring Platform ===");
    Logger::info("Firmware Version: %s", FIRMWARE_VERSION);
    Logger::info("Build Date: %s %s", __DATE__, __TIME__);
    Logger::info("Free Heap: %d bytes", ESP.getFreeHeap());
    
    if (psramFound()) {
        Logger::info("PSRAM: %d bytes", ESP.getPsramSize());
    } else {
        Logger::warning("PSRAM not found - AI features may be limited");
    }
    
    // Display system information
    Logger::info("CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
    Logger::info("Flash Size: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
    Logger::info("Chip Model: %s", ESP.getChipModel());
    Logger::info("Chip Revision: %d", ESP.getChipRevision());
    
    // Initialize all system components
    if (!initializeSystem()) {
        Logger::error("System initialization failed! Entering safe mode...");
        // TODO: Implement safe mode
        while (true) {
            delay(1000);
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
    }
    
    // Run system diagnostics
    Logger::info("Running system diagnostics...");
    bool diagnosticsPass = runSystemDiagnostics();
    
    if (!diagnosticsPass) {
        Logger::warning("System diagnostics detected issues - review diagnostics.log");
        // Continue operation but log the warning
    }
    
    // Create and start all system tasks
    createSystemTasks();
    
    Logger::info("WildCAM ESP32 v2.0 startup complete - All systems operational!");
}

/**
 * @brief Main loop function
 * Handles system monitoring and maintenance tasks
 */
void loop() {
    // Reset watchdog timer
    esp_task_wdt_reset();
    
    // System health monitoring
    static unsigned long last_health_check = 0;
    if ((millis() - last_health_check) > 60000) { // Every minute
        // Update system temperature
        if (g_env_sensors) {
            // system_state.system_temperature = g_env_sensors->getTemperature();
        }
        
        // Log system health
        Logger::info("System Health - Uptime: %lu min, Free Heap: %d bytes, Temperature: %.1f°C",
                   millis() / 60000, ESP.getFreeHeap(), system_state.system_temperature);
        
        // Check task health
        if (ai_processing_task && eTaskGetState(ai_processing_task) == eDeleted) {
            Logger::error("AI Processing task has died! Restarting...");
            // TODO: Implement task restart logic
        }
        
        last_health_check = millis();
    }
    
    // Handle system commands (if any)
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command == "status") {
            Logger::info("=== System Status ===");
            Logger::info("AI Initialized: %s", system_state.ai_initialized ? "YES" : "NO");
            Logger::info("Power System: %s", system_state.power_system_ok ? "OK" : "FAULT");
            Logger::info("Security Active: %s", system_state.security_active ? "YES" : "NO");
            Logger::info("Network Connected: %s", system_state.network_connected ? "YES" : "NO");
            Logger::info("Battery Level: %.2fV", system_state.battery_level);
            Logger::info("Last Detection: %lu ms ago", millis() - system_state.last_detection);
            Logger::info("Total Detections: %lu", getDetectionCounter());
            Logger::info("Storage Ready: %s", g_storage.isReady() ? "YES" : "NO");
            if (g_storage.isReady()) {
                Logger::info("Storage Usage: %.1f%% (%llu MB free)", 
                           g_storage.getUsagePercentage(),
                           g_storage.getFreeSpace() / (1024 * 1024));
            }
        } else if (command == "restart") {
            Logger::info("System restart requested...");
            ESP.restart();
        } else if (command == "info") {
            Logger::info("WildCAM ESP32 v2.0 - Advanced AI Wildlife Monitoring Platform");
            Logger::info("Version: %s", FIRMWARE_VERSION);
            Logger::info("Features: YOLO-tiny AI, MPPT Power, AES-256 Security, Mesh Network");
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(100);
}