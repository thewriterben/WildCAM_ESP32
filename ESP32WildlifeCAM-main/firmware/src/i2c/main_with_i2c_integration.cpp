/**
 * @file main_with_i2c_integration.cpp
 * @brief Example integration of ESP-IDF I2C system with main wildlife camera
 */

#include "config.h"

// ESP-IDF I2C System Integration
#if ESP_IDF_I2C_ENABLED
#include "i2c/i2c_integration.h"
#include "i2c/i2c_wildlife_demo.h"
#include "i2c/i2c_system_test.h"
#endif

// Existing includes
#include "camera_handler.h"
#include "motion_filter.h"
#include "power_manager.h"
#include "hal/board_detector.h"
#include "debug_utils.h"

// Global I2C handles
#if ESP_IDF_I2C_ENABLED
static uint32_t g_bme280_handle = 0;
static uint32_t g_rtc_handle = 0;
static uint32_t g_display_handle = 0;
static bool g_i2c_system_active = false;
#endif

/**
 * @brief Enhanced setup with ESP-IDF I2C integration
 */
void setup() {
    Serial.begin(115200);
    DEBUG_PRINTLN("ESP32 Wildlife Camera with ESP-IDF I2C System");
    DEBUG_PRINTLN("===========================================");
    
    // Initialize basic systems
    DEBUG_PRINTLN("Initializing core systems...");
    
    // Board detection
    BoardType board_type = BoardDetector::detectBoardType();
    DEBUG_PRINTF("Detected board: %s\n", BoardDetector::getBoardName(board_type));
    
#if ESP_IDF_I2C_ENABLED
    // Initialize ESP-IDF I2C system
    DEBUG_PRINTLN("Initializing ESP-IDF I2C system...");
    
    esp_err_t i2c_ret = i2c_system_init(board_type);
    if (i2c_ret == ESP_OK) {
        g_i2c_system_active = true;
        DEBUG_PRINTLN("✓ ESP-IDF I2C system initialized successfully");
        
        // Initialize environmental sensor (BME280)
        if (i2c_init_bme280(&g_bme280_handle) == ESP_OK) {
            DEBUG_PRINTLN("✓ BME280 environmental sensor online");
            DEBUG_PRINTLN("  Temperature, humidity, and pressure monitoring enabled");
        } else {
            DEBUG_PRINTLN("⚠ BME280 not found - continuing without environmental monitoring");
        }
        
        // Initialize RTC if available
        if (i2c_init_rtc(&g_rtc_handle) == ESP_OK) {
            DEBUG_PRINTLN("✓ External RTC initialized");
            DEBUG_PRINTLN("  Accurate timestamping enabled");
        } else {
            DEBUG_PRINTLN("⚠ External RTC not found - using internal RTC");
        }
        
        // Initialize display if available
        if (i2c_init_display(&g_display_handle) == ESP_OK) {
            DEBUG_PRINTLN("✓ I2C display initialized");
            DEBUG_PRINTLN("  Wildlife status display enabled");
        } else {
            DEBUG_PRINTLN("⚠ I2C display not found - status display disabled");
        }
        
        // Show I2C system capabilities
        char status_buffer[256];
        i2c_get_system_status(status_buffer, sizeof(status_buffer));
        DEBUG_PRINTF("I2C System Status: %s\n", status_buffer);
        
        // Run comprehensive I2C test (optional)
        #ifdef DEBUG_ENABLED
        DEBUG_PRINTLN("Starting I2C system validation...");
        i2c_system_test_start();
        #endif
        
        // Initialize wildlife monitoring demo
        i2c_wildlife_demo_init();
        i2c_wildlife_demo_start_task();
        
    } else {
        DEBUG_PRINTF("✗ ESP-IDF I2C system initialization failed: %s\n", esp_err_to_name(i2c_ret));
        DEBUG_PRINTLN("Continuing with basic wildlife camera functionality");
    }
#else
    DEBUG_PRINTLN("ESP-IDF I2C system disabled in config.h");
#endif

    // Initialize camera system
    DEBUG_PRINTLN("Initializing camera system...");
    if (CameraHandler::init()) {
        DEBUG_PRINTLN("✓ Camera system initialized");
    } else {
        DEBUG_PRINTLN("✗ Camera initialization failed");
        // Handle camera failure
    }
    
    // Initialize motion detection
    DEBUG_PRINTLN("Initializing motion detection...");
    MotionFilter::init();
    DEBUG_PRINTLN("✓ Motion detection ready");
    
    // Initialize power management
    DEBUG_PRINTLN("Initializing power management...");
    PowerManager::init();
    DEBUG_PRINTLN("✓ Power management ready");
    
    DEBUG_PRINTLN("===========================================");
    DEBUG_PRINTLN("Wildlife camera system ready!");
    
#if ESP_IDF_I2C_ENABLED && BME280_ENABLED
    if (g_i2c_system_active && g_bme280_handle != 0) {
        DEBUG_PRINTLN("🌡️ Environmental monitoring: ACTIVE");
        DEBUG_PRINTLN("📊 Wildlife detection will be optimized based on environmental conditions");
    }
#endif
    
    DEBUG_PRINTLN("Starting wildlife monitoring...");
}

/**
 * @brief Enhanced main loop with environmental awareness
 */
void loop() {
    static unsigned long last_status_update = 0;
    static unsigned long last_environmental_check = 0;
    static uint32_t wildlife_detections = 0;
    
    // Check for motion/wildlife detection
    if (MotionFilter::checkMotion()) {
        wildlife_detections++;
        DEBUG_PRINTF("🦌 Wildlife detected! (Total: %lu)\n", wildlife_detections);
        
#if ESP_IDF_I2C_ENABLED && BME280_ENABLED
        // Check environmental conditions before capture
        if (g_i2c_system_active && g_bme280_handle != 0) {
            bool environmental_ok = i2c_wildlife_demo_is_environmental_ok_for_capture();
            
            if (environmental_ok) {
                DEBUG_PRINTLN("✓ Environmental conditions optimal for capture");
                
                // Get environmental context for metadata
                bme280_reading_t env_reading;
                if (i2c_get_environmental_reading(&env_reading) == ESP_OK) {
                    DEBUG_PRINTF("Capture conditions: %.1f°C, %.1f%%, %.1f hPa\n",
                                env_reading.temperature, env_reading.humidity, env_reading.pressure);
                }
                
                // Trigger camera capture
                CameraHandler::captureImage();
                
            } else {
                DEBUG_PRINTLN("⚠ Environmental conditions suboptimal - skipping capture");
            }
        } else {
            // Standard capture without environmental check
            CameraHandler::captureImage();
        }
#else
        // Standard capture without environmental awareness
        CameraHandler::captureImage();
#endif
    }
    
#if ESP_IDF_I2C_ENABLED
    // Periodic environmental monitoring
    if (g_i2c_system_active && millis() - last_environmental_check > 30000) { // Every 30 seconds
        last_environmental_check = millis();
        
        if (g_bme280_handle != 0) {
            bme280_reading_t reading;
            if (i2c_get_environmental_reading(&reading) == ESP_OK) {
                // Update display with current status
                if (g_display_handle != 0) {
                    i2c_update_wildlife_display(wildlife_detections, &reading);
                }
                
                // Environmental analysis logging
                if (reading.temperature < 0.0f) {
                    DEBUG_PRINTLN("❄️ Freezing conditions detected - reduced wildlife activity expected");
                } else if (reading.temperature > 35.0f) {
                    DEBUG_PRINTLN("🌡️ High temperature - animals may seek shade");
                }
                
                if (reading.humidity > 90.0f) {
                    DEBUG_PRINTLN("🌫️ High humidity - potential fog/condensation on camera lens");
                } else if (reading.humidity < 20.0f) {
                    DEBUG_PRINTLN("🏜️ Low humidity - dry conditions, dust possible");
                }
            }
        }
    }
#endif
    
    // Periodic status update
    if (millis() - last_status_update > 60000) { // Every minute
        last_status_update = millis();
        
        DEBUG_PRINTLN("=== Wildlife Camera Status ===");
        DEBUG_PRINTF("Wildlife detections: %lu\n", wildlife_detections);
        DEBUG_PRINTF("Free heap: %lu bytes\n", ESP.getFreeHeap());
        
#if ESP_IDF_I2C_ENABLED
        if (g_i2c_system_active) {
            // Get I2C system statistics
            i2c_manager_stats_t i2c_stats;
            if (i2c_manager_get_stats(&i2c_stats) == ESP_OK) {
                DEBUG_PRINTF("I2C transactions: %lu total, %lu successful\n", 
                            i2c_stats.total_transactions, i2c_stats.successful_transactions);
            }
            
            // Environmental status
            if (g_bme280_handle != 0) {
                char env_status[128];
                i2c_wildlife_demo_get_environmental_status(env_status, sizeof(env_status));
                DEBUG_PRINTF("Environment: %s\n", env_status);
            }
        }
#endif
        
        DEBUG_PRINTLN("============================");
    }
    
    // Power management
    PowerManager::update();
    
    // Small delay to prevent busy waiting
    delay(100);
}

/**
 * @brief Example function showing how to integrate I2C data with wildlife detection
 */
void enhanced_wildlife_detection_with_environmental_context() {
#if ESP_IDF_I2C_ENABLED && BME280_ENABLED
    if (!g_i2c_system_active || g_bme280_handle == 0) {
        return; // Environmental monitoring not available
    }
    
    bme280_reading_t env_data;
    if (i2c_get_environmental_reading(&env_data) != ESP_OK) {
        return; // Failed to read environmental data
    }
    
    // Enhanced wildlife detection logic with environmental awareness
    bool motion_detected = MotionFilter::checkMotion();
    
    if (motion_detected) {
        // Analyze environmental factors
        bool ideal_temperature = (env_data.temperature > 5.0f && env_data.temperature < 30.0f);
        bool good_visibility = (env_data.humidity < 90.0f); // Less fog/condensation
        bool stable_pressure = true; // Could track pressure changes for weather prediction
        
        if (ideal_temperature && good_visibility && stable_pressure) {
            DEBUG_PRINTLN("🌟 Optimal wildlife detection conditions!");
            
            // High-quality capture with detailed metadata
            CameraHandler::captureImage();
            
            // Log environmental context
            DEBUG_PRINTF("Detection context: %.1f°C, %.1f%% humidity, %.1f hPa\n",
                        env_data.temperature, env_data.humidity, env_data.pressure);
            
        } else {
            DEBUG_PRINTLN("📸 Wildlife detected but conditions suboptimal");
            
            if (!ideal_temperature) {
                DEBUG_PRINTLN("  Temperature may affect animal behavior");
            }
            if (!good_visibility) {
                DEBUG_PRINTLN("  High humidity may reduce image quality");
            }
            
            // Still capture but with different settings or metadata flags
            CameraHandler::captureImage();
        }
    }
#endif
}

/**
 * @brief Example multi-board coordination setup
 */
void setup_multiboard_coordination() {
#if ESP_IDF_I2C_ENABLED
    if (!g_i2c_system_active) {
        return;
    }
    
    // Determine role based on configuration or GPIO pin
    multiboard_role_t role = MULTIBOARD_ROLE_COORDINATOR; // Or detect automatically
    uint8_t node_addr = MULTIBOARD_COORDINATOR_ADDR;
    
    if (i2c_init_multiboard(role, node_addr) == ESP_OK) {
        DEBUG_PRINTLN("✓ Multi-board coordination initialized");
        
        if (role == MULTIBOARD_ROLE_COORDINATOR) {
            DEBUG_PRINTLN("  Operating as COORDINATOR - managing other camera nodes");
        } else {
            DEBUG_PRINTLN("  Operating as NODE - reporting to coordinator");
        }
    }
#endif
}