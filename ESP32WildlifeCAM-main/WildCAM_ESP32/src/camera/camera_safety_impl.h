/**
 * @file camera_safety_impl.h
 * @brief Safe Camera Implementation Patterns
 * @author WildCAM ESP32 Project
 * 
 * This file provides implementation patterns and guidelines for safe
 * camera operations with proper error handling and resource management.
 */

#ifndef CAMERA_SAFETY_IMPL_H
#define CAMERA_SAFETY_IMPL_H

#include <esp_camera.h>
#include "../utils/logger.h"

/**
 * @brief Safe camera frame buffer wrapper with RAII
 * 
 * Automatically returns frame buffer when going out of scope,
 * preventing memory leaks.
 * 
 * Example usage:
 * @code
 * {
 *     SafeCameraFrameBuffer frame;
 *     if (frame.capture()) {
 *         processImage(frame.get());
 *     }
 *     // Frame buffer automatically returned when frame goes out of scope
 * }
 * @endcode
 */
class SafeCameraFrameBuffer {
private:
    camera_fb_t* m_fb;
    
public:
    SafeCameraFrameBuffer() : m_fb(nullptr) {}
    
    ~SafeCameraFrameBuffer() {
        release();
    }
    
    // Prevent copying to avoid double-free
    SafeCameraFrameBuffer(const SafeCameraFrameBuffer&) = delete;
    SafeCameraFrameBuffer& operator=(const SafeCameraFrameBuffer&) = delete;
    
    /**
     * @brief Capture a frame from camera
     * @return true if capture successful, false otherwise
     */
    bool capture() {
        // Release any existing buffer first
        release();
        
        m_fb = esp_camera_fb_get();
        if (!m_fb) {
            Logger::error("Camera capture failed");
            return false;
        }
        
        // Validate frame buffer content
        if (!m_fb->buf || m_fb->len == 0) {
            Logger::error("Invalid frame buffer: buf=%p, len=%zu", m_fb->buf, m_fb->len);
            release();
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Get the frame buffer pointer
     * @return Frame buffer pointer or nullptr if not captured
     */
    camera_fb_t* get() const {
        return m_fb;
    }
    
    /**
     * @brief Check if frame buffer is valid
     * @return true if valid frame buffer exists
     */
    bool isValid() const {
        return m_fb != nullptr && m_fb->buf != nullptr && m_fb->len > 0;
    }
    
    /**
     * @brief Release the frame buffer
     */
    void release() {
        if (m_fb) {
            esp_camera_fb_return(m_fb);
            m_fb = nullptr;
        }
    }
    
    /**
     * @brief Get frame buffer size
     * @return Size in bytes, or 0 if invalid
     */
    size_t size() const {
        return (m_fb && m_fb->buf) ? m_fb->len : 0;
    }
};

/**
 * @brief Safe mutex lock with RAII
 * 
 * Automatically releases mutex when going out of scope.
 * 
 * Example usage:
 * @code
 * {
 *     SafeMutexLock lock(m_mutex, "CameraOperation");
 *     if (lock.isLocked()) {
 *         // Perform critical section operations
 *         updateSharedState();
 *     }
 *     // Mutex automatically released when lock goes out of scope
 * }
 * @endcode
 */
class SafeMutexLock {
private:
    SemaphoreHandle_t m_mutex;
    bool m_locked;
    const char* m_name;
    
public:
    /**
     * @brief Constructor - attempts to take mutex
     * @param mutex Semaphore handle to lock
     * @param name Optional name for debugging
     * @param timeout_ms Timeout in milliseconds (default: wait forever)
     */
    SafeMutexLock(SemaphoreHandle_t mutex, const char* name = nullptr, 
                  uint32_t timeout_ms = portMAX_DELAY) 
        : m_mutex(mutex), m_locked(false), m_name(name) {
        
        if (m_mutex) {
            TickType_t ticks = (timeout_ms == portMAX_DELAY) ? portMAX_DELAY 
                                                               : pdMS_TO_TICKS(timeout_ms);
            m_locked = (xSemaphoreTake(m_mutex, ticks) == pdTRUE);
            
            if (!m_locked && m_name) {
                Logger::warning("Failed to acquire mutex: %s", m_name);
            }
        }
    }
    
    ~SafeMutexLock() {
        if (m_locked && m_mutex) {
            xSemaphoreGive(m_mutex);
            m_locked = false;
        }
    }
    
    // Prevent copying
    SafeMutexLock(const SafeMutexLock&) = delete;
    SafeMutexLock& operator=(const SafeMutexLock&) = delete;
    
    /**
     * @brief Check if mutex was successfully locked
     * @return true if locked, false otherwise
     */
    bool isLocked() const {
        return m_locked;
    }
};

/**
 * @brief Camera initialization helper with comprehensive error checking
 * 
 * Example usage:
 * @code
 * camera_config_t config = getCameraConfig();
 * if (!safeCameraInit(config)) {
 *     Logger::error("Camera initialization failed");
 *     return false;
 * }
 * @endcode
 */
inline bool safeCameraInit(const camera_config_t& config) {
    // Validate configuration
    if (!config.pin_d0 || !config.pin_vsync || !config.pin_href) {
        Logger::error("Invalid camera pin configuration");
        return false;
    }
    
    // Check PSRAM requirement
    if (config.fb_location == CAMERA_FB_IN_PSRAM && !psramFound()) {
        Logger::error("PSRAM required but not found");
        return false;
    }
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Logger::error("Camera init failed: 0x%x", err);
        return false;
    }
    
    // Validate camera sensor
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        Logger::error("Failed to get camera sensor");
        esp_camera_deinit();
        return false;
    }
    
    Logger::info("Camera initialized successfully");
    return true;
}

/**
 * @brief Safe camera deinitialize
 * Ensures proper cleanup of camera resources
 */
inline void safeCameraDeinit() {
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        Logger::warning("Camera deinit returned error: 0x%x", err);
    } else {
        Logger::info("Camera deinitialized successfully");
    }
}

/**
 * @brief Safe image capture with timeout and validation
 * 
 * @param fb Output frame buffer pointer (set to nullptr on failure)
 * @param timeout_ms Capture timeout in milliseconds
 * @return true if capture successful, false otherwise
 * 
 * Example usage:
 * @code
 * camera_fb_t* fb = nullptr;
 * if (safeCameraCapture(&fb, 5000)) {
 *     processImage(fb);
 *     esp_camera_fb_return(fb);
 * }
 * @endcode
 */
inline bool safeCameraCapture(camera_fb_t** fb, uint32_t timeout_ms = 5000) {
    if (!fb) {
        Logger::error("Null frame buffer pointer");
        return false;
    }
    
    *fb = nullptr;
    
    // Capture with timeout
    uint32_t start = millis();
    *fb = esp_camera_fb_get();
    uint32_t elapsed = millis() - start;
    
    if (!*fb) {
        Logger::error("Camera capture failed (timeout or error)");
        return false;
    }
    
    if (elapsed > timeout_ms) {
        Logger::warning("Camera capture exceeded timeout: %ums", elapsed);
        esp_camera_fb_return(*fb);
        *fb = nullptr;
        return false;
    }
    
    // Validate frame buffer
    if (!(*fb)->buf || (*fb)->len == 0) {
        Logger::error("Invalid frame buffer captured");
        esp_camera_fb_return(*fb);
        *fb = nullptr;
        return false;
    }
    
    Logger::debug("Capture successful: %zu bytes in %ums", (*fb)->len, elapsed);
    return true;
}

/**
 * @brief Memory-aware capture check
 * Verifies sufficient memory is available before capture
 * 
 * @param required_bytes Minimum required free heap
 * @return true if sufficient memory available
 */
inline bool checkCaptureMemory(size_t required_bytes = 100000) {
    size_t free_heap = ESP.getFreeHeap();
    
    if (free_heap < required_bytes) {
        Logger::warning("Insufficient heap for capture: %zu < %zu", 
                       free_heap, required_bytes);
        return false;
    }
    
    // Check PSRAM if available
    if (psramFound()) {
        size_t free_psram = ESP.getFreePsram();
        Logger::debug("Free memory - Heap: %zu, PSRAM: %zu", free_heap, free_psram);
    }
    
    return true;
}

#endif // CAMERA_SAFETY_IMPL_H
