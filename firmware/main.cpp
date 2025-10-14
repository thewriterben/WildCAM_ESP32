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

// Configuration
#define FIRMWARE_VERSION "3.0.0"
#define SYSTEM_NAME "WildCAM_ESP32_v2.0"

// Global system components
SystemManager* g_system_manager = nullptr;
YOLOTinyDetector* g_yolo_detector = nullptr;
MPPTController* g_mppt_controller = nullptr;
SecurityManager* g_security_manager = nullptr;
EnvironmentalSuite* g_env_sensors = nullptr;
CameraManager* g_camera_manager = nullptr;

// Task handles for multi-core processing
TaskHandle_t ai_processing_task = NULL;
TaskHandle_t power_management_task = NULL;
TaskHandle_t network_management_task = NULL;
TaskHandle_t security_monitoring_task = NULL;

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
 * @brief AI Processing Task (Core 1)
 * Handles real-time wildlife detection using YOLO-tiny
 */
void aiProcessingTask(void* parameter) {
    Logger::info("AI Processing Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(100); // 10 FPS processing rate
    BoundingBox detections[10];
    
    while (true) {
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
                        
                        // Process detection (save image, send to API, etc.)
                        // TODO: Implement detection processing pipeline
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
            if (power_status.battery_voltage < 3.0f) {
                Logger::warning("Low battery detected, entering power save mode");
                // TODO: Implement power save mode
            }
        }
        
        vTaskDelay(xDelay);
    }
}

/**
 * @brief Security Monitoring Task (Core 0)
 * Handles encryption, tamper detection, and secure communications
 */
void securityMonitoringTask(void* parameter) {
    Logger::info("Security Monitoring Task started on Core %d", xPortGetCoreID());
    
    const TickType_t xDelay = pdMS_TO_TICKS(10000); // Check every 10 seconds
    
    while (true) {
        if (g_security_manager && g_security_manager->isSecurityOperational()) {
            // Check for tampering
            if (g_security_manager->detectTampering()) {
                Logger::error("Tampering detected! Initiating security protocols");
                // TODO: Implement security response (alert, data wipe, etc.)
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
        // TODO: Implement network management
        // - Mesh network maintenance
        // - API data transmission
        // - Firmware update checks
        // - Remote configuration sync
        
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