/**
 * @file camera_utils.cpp
 * @brief Camera utility functions for ESP32WildlifeCAM
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file provides utility functions for camera operations including
 * validation, diagnostics, and helper functions.
 */

#include "camera_config.h"
#include <esp_camera.h>
#include <esp_system.h>
#include <SD_MMC.h>
#include <LittleFS.h>

namespace CameraUtils {

/**
 * Validate camera configuration for compatibility with hardware
 */
bool validateCameraConfig(const CameraConfig& config, const char* board_name) {
    // Check frame size compatibility
    if (config.max_framesize > FRAMESIZE_QXGA) {
        Serial.printf("ERROR: Frame size %d too large for most ESP32 cameras\n", config.max_framesize);
        return false;
    }
    
    // Check PSRAM requirements
    if (config.psram_required && !psramFound()) {
        Serial.println("ERROR: Configuration requires PSRAM but none detected");
        return false;
    }
    
    // Check frame buffer count vs memory
    if (config.fb_count > 3 && !psramFound()) {
        Serial.printf("WARNING: fb_count %d may cause memory issues without PSRAM\n", config.fb_count);
    }
    
    // Validate JPEG quality
    if (config.jpeg_quality < 1 || config.jpeg_quality > 63) {
        Serial.printf("ERROR: Invalid JPEG quality %d (must be 1-63)\n", config.jpeg_quality);
        return false;
    }
    
    // Validate clock frequency
    if (config.xclk_freq_hz < 10000000 || config.xclk_freq_hz > 30000000) {
        Serial.printf("WARNING: Clock frequency %lu Hz is outside recommended range 10-30MHz\n", config.xclk_freq_hz);
    }
    
    return true;
}

/**
 * Get recommended configuration based on available hardware
 */
CameraConfig getRecommendedConfig() {
    // Check available memory
    size_t free_heap = ESP.getFreeHeap();
    bool has_psram = psramFound();
    size_t psram_size = has_psram ? ESP.getPsramSize() : 0;
    
    Serial.printf("Memory analysis: Free heap: %d bytes, PSRAM: %s (%d bytes)\n", 
                  free_heap, has_psram ? "Yes" : "No", psram_size);
    
    if (has_psram && psram_size >= 4 * 1024 * 1024) {
        Serial.println("Recommending high performance configuration");
        return CameraConfig::getHighPerformanceConfig();
    } else if (free_heap > 200 * 1024) {
        Serial.println("Recommending default AI-Thinker configuration");
        return CameraConfig::getDefaultAIThinkerConfig();
    } else {
        Serial.println("Recommending low power configuration due to memory constraints");
        return CameraConfig::getLowPowerConfig();
    }
}

/**
 * Diagnose camera initialization issues
 */
void diagnoseCameraIssues(esp_err_t init_error) {
    Serial.println("=== Camera Diagnostic Information ===");
    
    // Chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    Serial.printf("Chip: %s, cores: %d, revision: %d\n", 
                  CONFIG_IDF_TARGET, chip_info.cores, chip_info.revision);
    
    // Memory information
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("PSRAM: Not found");
    }
    
    // Analyze the specific error
    switch (init_error) {
        case ESP_OK:
            Serial.println("Status: Camera initialized successfully");
            break;
        case ESP_ERR_INVALID_ARG:
            Serial.println("ERROR: Invalid camera configuration parameters");
            Serial.println("SUGGESTION: Check pin assignments and configuration values");
            break;
        case ESP_ERR_NO_MEM:
            Serial.println("ERROR: Insufficient memory for camera initialization");
            Serial.println("SUGGESTION: Reduce frame buffer count or frame size");
            break;
        case ESP_FAIL:
            Serial.println("ERROR: Camera hardware initialization failed");
            Serial.println("SUGGESTION: Check hardware connections and power supply");
            break;
        default:
            Serial.printf("ERROR: Camera initialization failed with code 0x%x\n", init_error);
            break;
    }
    
    // Check storage availability
    if (SD_MMC.begin()) {
        uint64_t card_size = SD_MMC.cardSize() / (1024 * 1024);
        Serial.printf("SD Card: %lluMB available\n", card_size);
        SD_MMC.end();
    } else {
        Serial.println("SD Card: Not available");
    }
    
    if (LittleFS.begin()) {
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        Serial.printf("LittleFS: %d/%d bytes used\n", used, total);
        LittleFS.end();
    } else {
        Serial.println("LittleFS: Not available");
    }
    
    Serial.println("=== End Diagnostic Information ===");
}

/**
 * Calculate optimal frame buffer count based on available memory
 */
uint8_t calculateOptimalFrameBufferCount(framesize_t frame_size) {
    size_t frame_size_bytes = 0;
    
    // Estimate frame size in bytes (JPEG compression assumed)
    switch (frame_size) {
        case FRAMESIZE_QQVGA:   frame_size_bytes = 10 * 1024; break;    // ~10KB
        case FRAMESIZE_QCIF:    frame_size_bytes = 15 * 1024; break;    // ~15KB
        case FRAMESIZE_HQVGA:   frame_size_bytes = 20 * 1024; break;    // ~20KB
        case FRAMESIZE_240X240: frame_size_bytes = 25 * 1024; break;    // ~25KB
        case FRAMESIZE_QVGA:    frame_size_bytes = 30 * 1024; break;    // ~30KB
        case FRAMESIZE_CIF:     frame_size_bytes = 50 * 1024; break;    // ~50KB
        case FRAMESIZE_HVGA:    frame_size_bytes = 70 * 1024; break;    // ~70KB
        case FRAMESIZE_VGA:     frame_size_bytes = 100 * 1024; break;   // ~100KB
        case FRAMESIZE_SVGA:    frame_size_bytes = 150 * 1024; break;   // ~150KB
        case FRAMESIZE_XGA:     frame_size_bytes = 200 * 1024; break;   // ~200KB
        case FRAMESIZE_HD:      frame_size_bytes = 300 * 1024; break;   // ~300KB
        case FRAMESIZE_SXGA:    frame_size_bytes = 400 * 1024; break;   // ~400KB
        case FRAMESIZE_UXGA:    frame_size_bytes = 600 * 1024; break;   // ~600KB
        case FRAMESIZE_FHD:     frame_size_bytes = 800 * 1024; break;   // ~800KB
        case FRAMESIZE_P_HD:    frame_size_bytes = 900 * 1024; break;   // ~900KB
        case FRAMESIZE_P_3MP:   frame_size_bytes = 1200 * 1024; break;  // ~1.2MB
        case FRAMESIZE_QXGA:    frame_size_bytes = 1500 * 1024; break;  // ~1.5MB
        default:                frame_size_bytes = 200 * 1024; break;   // Default to ~200KB
    }
    
    size_t available_memory = ESP.getFreeHeap();
    if (psramFound()) {
        available_memory += ESP.getFreePsram();
    }
    
    // Reserve 50% of memory for other operations
    size_t memory_for_buffers = available_memory / 2;
    uint8_t max_buffers = memory_for_buffers / frame_size_bytes;
    
    // Limit to reasonable range
    if (max_buffers < 1) max_buffers = 1;
    if (max_buffers > 3) max_buffers = 3;
    
    Serial.printf("Frame buffer calculation: %d bytes per frame, %d max buffers\n", 
                  frame_size_bytes, max_buffers);
    
    return max_buffers;
}

/**
 * Convert frame size enum to human-readable string
 */
const char* getFrameSizeName(framesize_t frame_size) {
    switch (frame_size) {
        case FRAMESIZE_96X96:   return "96x96";
        case FRAMESIZE_QQVGA:   return "QQVGA (160x120)";
        case FRAMESIZE_QCIF:    return "QCIF (176x144)";
        case FRAMESIZE_HQVGA:   return "HQVGA (240x176)";
        case FRAMESIZE_240X240: return "240x240";
        case FRAMESIZE_QVGA:    return "QVGA (320x240)";
        case FRAMESIZE_CIF:     return "CIF (400x296)";
        case FRAMESIZE_HVGA:    return "HVGA (480x320)";
        case FRAMESIZE_VGA:     return "VGA (640x480)";
        case FRAMESIZE_SVGA:    return "SVGA (800x600)";
        case FRAMESIZE_XGA:     return "XGA (1024x768)";
        case FRAMESIZE_HD:      return "HD (1280x720)";
        case FRAMESIZE_SXGA:    return "SXGA (1280x1024)";
        case FRAMESIZE_UXGA:    return "UXGA (1600x1200)";
        case FRAMESIZE_FHD:     return "FHD (1920x1080)";
        case FRAMESIZE_P_HD:    return "P_HD (720x1280)";
        case FRAMESIZE_P_3MP:   return "P_3MP (864x1536)";
        case FRAMESIZE_QXGA:    return "QXGA (2048x1536)";
        default:                return "Unknown";
    }
}

/**
 * Log camera configuration details
 */
void logCameraConfig(const CameraConfig& config) {
    Serial.println("=== Camera Configuration ===");
    Serial.printf("Frame size: %s\n", getFrameSizeName(config.max_framesize));
    Serial.printf("Pixel format: %s\n", 
                  config.pixel_format == PIXFORMAT_JPEG ? "JPEG" : "RGB565");
    Serial.printf("JPEG quality: %d\n", config.jpeg_quality);
    Serial.printf("Frame buffers: %d\n", config.fb_count);
    Serial.printf("Clock frequency: %lu Hz\n", config.xclk_freq_hz);
    Serial.printf("PSRAM required: %s\n", config.psram_required ? "Yes" : "No");
    Serial.printf("Flash available: %s\n", config.has_flash ? "Yes" : "No");
    Serial.printf("Motion detection: %s\n", config.motion_detection_enabled ? "Enabled" : "Disabled");
    Serial.printf("Night mode: %s\n", config.night_mode ? "Enabled" : "Disabled");
    Serial.printf("Low power mode: %s\n", config.low_power_mode ? "Enabled" : "Disabled");
    Serial.println("=== End Configuration ===");
}

} // namespace CameraUtils