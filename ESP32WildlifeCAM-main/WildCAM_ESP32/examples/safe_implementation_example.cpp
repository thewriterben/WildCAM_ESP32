/**
 * @file safe_implementation_example.cpp
 * @brief Example demonstrating safe implementation patterns
 * @author WildCAM ESP32 Project
 * 
 * This example shows proper error handling, resource management,
 * and ESP32 best practices for the WildCAM project.
 */

#include <Arduino.h>
#include "../src/camera/camera_safety_impl.h"
#include "../src/power/power_safety_impl.h"
#include "../src/utils/logger.h"
#include "../src/core/system_manager.h"

/**
 * Example 1: Safe camera capture with RAII
 */
void example_safe_camera_capture() {
    Logger::info("=== Example 1: Safe Camera Capture ===");
    
    // Check memory before capture
    if (!checkCaptureMemory(100000)) {
        Logger::error("Insufficient memory for capture");
        return;
    }
    
    // Use RAII wrapper - automatically releases frame buffer
    {
        SafeCameraFrameBuffer frame;
        if (frame.capture()) {
            Logger::info("Captured image: %zu bytes", frame.size());
            
            // Process the image
            if (frame.isValid()) {
                // Save to SD card, process with AI, etc.
                Logger::info("Image processing successful");
            }
        } else {
            Logger::error("Capture failed");
        }
        // Frame buffer automatically released here when going out of scope
    }
    
    Logger::info("Capture complete, resources released");
}

/**
 * Example 2: Manual camera capture with explicit cleanup
 */
void example_manual_camera_capture() {
    Logger::info("=== Example 2: Manual Camera Capture ===");
    
    camera_fb_t* fb = nullptr;
    
    // Use helper function with validation
    if (!safeCameraCapture(&fb, 5000)) {
        Logger::error("Camera capture failed");
        return;
    }
    
    // Ensure cleanup in all paths
    bool success = false;
    
    if (fb && fb->buf && fb->len > 0) {
        Logger::info("Processing %zu bytes", fb->len);
        
        // Process image
        // ...
        
        success = true;
    }
    
    // CRITICAL: Always return frame buffer
    if (fb) {
        esp_camera_fb_return(fb);
        fb = nullptr;
    }
    
    if (success) {
        Logger::info("Image processed successfully");
    } else {
        Logger::error("Image processing failed");
    }
}

/**
 * Example 3: Safe SystemManager initialization
 */
bool example_safe_system_init() {
    Logger::info("=== Example 3: Safe System Initialization ===");
    
    // Detect board type
    BoardDetector::BoardType board = BoardDetector::detectBoard();
    Logger::info("Board type: %s", BoardDetector::getBoardName(board));
    
    // Create system manager with null check
    SystemManager* system = new (std::nothrow) SystemManager(board);
    if (!system) {
        Logger::error("Failed to allocate SystemManager");
        Logger::error("Free heap: %zu bytes", ESP.getFreeHeap());
        return false;
    }
    
    // Initialize with error checking
    if (!system->initialize()) {
        Logger::error("System initialization failed: %s", system->getLastError());
        
        // Cleanup on failure
        delete system;
        system = nullptr;
        return false;
    }
    
    Logger::info("System initialized successfully");
    
    // Use system...
    
    // Cleanup when done
    delete system;
    system = nullptr;
    
    return true;
}

/**
 * Example 4: Safe power management with battery monitoring
 */
void example_safe_power_management() {
    Logger::info("=== Example 4: Safe Power Management ===");
    
    // Create battery monitor (ADC pin 35, voltage divider 2:1)
    SafeBatteryMonitor battery(35, 2.0f);
    
    // Read battery voltage with filtering
    float voltage = battery.readVoltage();
    if (voltage < 0.0f) {
        Logger::error("Failed to read battery voltage");
        return;
    }
    
    Logger::info("Battery voltage: %.2fV", voltage);
    
    // Calculate percentage
    int8_t percentage = SafeBatteryMonitor::voltageToPercentage(voltage);
    if (percentage >= 0) {
        Logger::info("Battery level: %d%%", percentage);
    }
    
    // Validate power state
    float temperature = 25.0f;  // Read from sensor
    if (!validatePowerState(voltage, temperature)) {
        Logger::error("Power state invalid - entering safe mode");
        // Enter safe mode or shutdown
        return;
    }
    
    // Estimate runtime
    float runtime = estimateRuntime(2000, 100.0f, percentage);
    Logger::info("Estimated runtime: %.1f hours", runtime);
    
    // Check if we should enter low power mode
    if (percentage < 20) {
        Logger::warning("Low battery - entering power save mode");
        // Reduce functionality, disable non-essential features
    }
}

/**
 * Example 5: Safe deep sleep with wake sources
 */
void example_safe_deep_sleep() {
    Logger::info("=== Example 5: Safe Deep Sleep ===");
    
    // Log wake up cause
    const char* wakeup_cause = SafeDeepSleep::getWakeupCause();
    Logger::info("Wake up cause: %s", wakeup_cause);
    
    // Validate power down state
    if (!SafeDeepSleep::validatePowerDownOptions()) {
        Logger::error("Power down validation failed");
        return;
    }
    
    // Configure and enter deep sleep
    // Wake up after 3600 seconds (1 hour) OR on GPIO trigger
    Logger::info("Entering deep sleep for 1 hour...");
    
    // This call does not return (ESP32 enters deep sleep)
    SafeDeepSleep::enter(
        3600,              // Sleep for 1 hour
        true,              // Enable GPIO wakeup
        GPIO_NUM_13,       // Wake on GPIO 13
        0                  // Wake on LOW level
    );
}

/**
 * Example 6: Safe watchdog timer usage
 */
void example_safe_watchdog() {
    Logger::info("=== Example 6: Safe Watchdog Timer ===");
    
    // Create watchdog with 30 second timeout
    SafeWatchdog watchdog(30);
    
    if (!watchdog.enable()) {
        Logger::error("Failed to enable watchdog");
        return;
    }
    
    // Perform long-running operations
    for (int i = 0; i < 10; i++) {
        Logger::info("Processing iteration %d", i);
        
        // Do work
        delay(2000);
        
        // Reset watchdog to prevent timeout
        watchdog.reset();
    }
    
    Logger::info("Processing complete");
    
    // Watchdog automatically disabled when going out of scope
}

/**
 * Example 7: Safe mutex usage with RAII
 */
class SharedCounter {
private:
    int m_count;
    SemaphoreHandle_t m_mutex;
    
public:
    SharedCounter() : m_count(0), m_mutex(nullptr) {
        m_mutex = xSemaphoreCreateMutex();
        if (!m_mutex) {
            Logger::error("Failed to create mutex");
        }
    }
    
    ~SharedCounter() {
        if (m_mutex) {
            vSemaphoreDelete(m_mutex);
            m_mutex = nullptr;
        }
    }
    
    void increment() {
        // Use RAII mutex lock
        SafeMutexLock lock(m_mutex, "SharedCounter::increment");
        if (lock.isLocked()) {
            m_count++;
            Logger::debug("Count incremented to %d", m_count);
        } else {
            Logger::error("Failed to acquire mutex");
        }
        // Mutex automatically released when lock goes out of scope
    }
    
    int getCount() const {
        int value = -1;
        // Safe mutex lock with timeout
        SafeMutexLock lock(m_mutex, "SharedCounter::getCount", 100);
        if (lock.isLocked()) {
            value = m_count;
        } else {
            Logger::warning("Timeout acquiring mutex");
        }
        return value;
    }
};

void example_safe_mutex_usage() {
    Logger::info("=== Example 7: Safe Mutex Usage ===");
    
    SharedCounter counter;
    
    // Safe increment operations
    for (int i = 0; i < 5; i++) {
        counter.increment();
        delay(100);
    }
    
    // Safe read operation
    int count = counter.getCount();
    Logger::info("Final count: %d", count);
}

/**
 * Example 8: Comprehensive error handling
 */
bool example_comprehensive_error_handling() {
    Logger::info("=== Example 8: Comprehensive Error Handling ===");
    
    // Stage 1: Validate prerequisites
    if (!psramFound()) {
        Logger::error("PSRAM not found - required for operation");
        return false;
    }
    
    size_t free_heap = ESP.getFreeHeap();
    if (free_heap < 50000) {
        Logger::error("Insufficient heap: %zu bytes", free_heap);
        return false;
    }
    
    // Stage 2: Initialize camera
    camera_config_t config = {}; // Configure appropriately
    if (!safeCameraInit(config)) {
        Logger::error("Camera initialization failed");
        return false;
    }
    
    // Stage 3: Perform operations with cleanup on error
    bool success = false;
    camera_fb_t* fb = nullptr;
    
    if (!safeCameraCapture(&fb, 5000)) {
        Logger::error("Capture failed");
        goto cleanup;
    }
    
    if (!fb || !fb->buf || fb->len == 0) {
        Logger::error("Invalid frame buffer");
        goto cleanup;
    }
    
    // Process image
    Logger::info("Processing %zu byte image", fb->len);
    // ... processing logic ...
    
    success = true;
    
cleanup:
    // Cleanup resources
    if (fb) {
        esp_camera_fb_return(fb);
        fb = nullptr;
    }
    
    if (!success) {
        safeCameraDeinit();
    }
    
    return success;
}

/**
 * Main setup function
 */
void setup() {
    // Initialize serial
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    Logger::info("========================================");
    Logger::info("WildCAM Safe Implementation Examples");
    Logger::info("========================================");
    
    // Run examples (comment out as needed)
    example_safe_camera_capture();
    delay(1000);
    
    example_manual_camera_capture();
    delay(1000);
    
    example_safe_system_init();
    delay(1000);
    
    example_safe_power_management();
    delay(1000);
    
    example_safe_watchdog();
    delay(1000);
    
    example_safe_mutex_usage();
    delay(1000);
    
    example_comprehensive_error_handling();
    delay(1000);
    
    Logger::info("========================================");
    Logger::info("All examples completed");
    Logger::info("========================================");
    
    // Uncomment to test deep sleep (WARNING: will sleep for 1 hour!)
    // example_safe_deep_sleep();
}

/**
 * Main loop function
 */
void loop() {
    // Examples run once in setup
    delay(1000);
}
