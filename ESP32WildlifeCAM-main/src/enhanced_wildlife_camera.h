/**
 * @file enhanced_wildlife_camera.h
 * @brief Enhanced camera class with OV5640 5MP optimization and IR Cut control
 * 
 * Provides comprehensive camera functionality for wildlife monitoring including:
 * - Full 5MP OV5640 sensor support with auto focus
 * - IR Cut filter control for day/night operation
 * - Wildlife-optimized settings and image processing
 * - Power management integration
 * - Motion-triggered capture
 */

#ifndef ENHANCED_WILDLIFE_CAMERA_H
#define ENHANCED_WILDLIFE_CAMERA_H

#include <Arduino.h>
#include <esp_camera.h>
#include "firmware/src/configs/sensor_configs.h"
#include "firmware/src/hal/camera_board.h"

// IR Cut control pin for T-Camera Plus S3
#define IR_CUT_PIN 16  // AP1511B_FBC pin

// Auto focus control pins and settings
#define AUTOFOCUS_ENABLE_PIN -1  // Not available on all modules
#define DEFAULT_FOCUS_POSITION 0x200  // Infinity focus
#define FOCUS_STEP_SIZE 0x10
#define MAX_FOCUS_ITERATIONS 20

// Frame size definitions for OV5640
enum WildlifeFrameSize {
    WILDLIFE_QVGA = FRAMESIZE_QVGA,     // 320x240
    WILDLIFE_VGA = FRAMESIZE_VGA,       // 640x480
    WILDLIFE_SVGA = FRAMESIZE_SVGA,     // 800x600
    WILDLIFE_HD = FRAMESIZE_HD,         // 1280x720
    WILDLIFE_SXGA = FRAMESIZE_SXGA,     // 1280x1024
    WILDLIFE_UXGA = FRAMESIZE_UXGA,     // 1600x1200
    WILDLIFE_FHD = FRAMESIZE_FHD,       // 1920x1080
    WILDLIFE_QSXGA = FRAMESIZE_QSXGA    // 2592x1944 (5MP)
};

// IR Cut filter states
enum IRCutState {
    IR_CUT_DISABLED = 0,  // IR filter removed (night mode)
    IR_CUT_ENABLED = 1    // IR filter enabled (day mode)
};

// Wildlife capture modes
enum WildlifeCaptureMode {
    MODE_CONTINUOUS,      // Continuous monitoring
    MODE_MOTION_TRIGGER,  // PIR motion triggered
    MODE_TIME_LAPSE,      // Time-lapse photography
    MODE_MANUAL          // Manual capture only
};

// Camera status structure
struct CameraStatus {
    bool initialized;
    bool ir_cut_enabled;
    SensorType sensor_type;
    WildlifeFrameSize current_frame_size;
    uint8_t image_quality;
    uint16_t focus_position;
    bool autofocus_enabled;
    unsigned long last_capture_time;
    uint32_t total_captures;
    uint32_t failed_captures;
};

// Wildlife capture settings
struct WildlifeCaptureSettings {
    WildlifeFrameSize frame_size;
    uint8_t quality;            // 1-63 (lower = better quality)
    bool enable_autofocus;
    uint16_t focus_position;    // Manual focus position
    int8_t brightness;          // -2 to 2
    int8_t contrast;            // -2 to 2
    int8_t saturation;          // -2 to 2
    bool auto_ir_cut;           // Automatic day/night switching
    uint8_t capture_interval_s; // Seconds between captures in time-lapse
};

class EnhancedWildlifeCamera {
public:
    EnhancedWildlifeCamera();
    ~EnhancedWildlifeCamera();

    // Initialization and configuration
    bool init(BoardType board_type = BOARD_LILYGO_T_CAMERA_PLUS);
    bool detectSensor();
    void cleanup();
    
    // Camera configuration
    bool configureSensor(const WildlifeCaptureSettings& settings);
    bool setFrameSize(WildlifeFrameSize size);
    bool setImageQuality(uint8_t quality);
    bool setWildlifeOptimizations();
    
    // IR Cut filter control
    bool setIRCutFilter(IRCutState state);
    bool toggleIRCutFilter();
    IRCutState getIRCutState() const;
    bool autoAdjustIRCut();  // Based on ambient light
    
    // Auto focus functionality
    bool enableAutoFocus(bool enable);
    bool setFocusPosition(uint16_t position);
    uint16_t getCurrentFocusPosition() const;
    bool performAutoFocus();
    
    // Image capture
    camera_fb_t* captureImage();
    bool captureToFile(const char* filename);
    bool captureWildlifeImage(const WildlifeCaptureSettings& settings, const char* filename = nullptr);
    
    // Wildlife monitoring features
    bool startContinuousMonitoring(WildlifeCaptureMode mode, const WildlifeCaptureSettings& settings);
    void stopContinuousMonitoring();
    bool isMonitoring() const;
    
    // Status and diagnostics
    CameraStatus getStatus() const;
    bool testCamera();
    void printDiagnostics() const;
    const char* getSensorName() const;
    
    // Power management
    bool enterLowPowerMode();
    bool exitLowPowerMode();
    void powerDown();
    void powerUp();
    
    // Time-based operations
    void setTimeOfDay(uint8_t hour, uint8_t minute);
    bool isDaytime() const;
    bool shouldSwitchIRCut() const;

private:
    CameraStatus status_;
    WildlifeCaptureSettings current_settings_;
    WildlifeCaptureMode monitoring_mode_;
    bool monitoring_active_;
    uint8_t current_hour_;
    uint8_t current_minute_;
    
    // Hardware configuration
    BoardType board_type_;
    camera_config_t camera_config_;
    
    // Internal methods
    bool initializeHardware();
    bool configureGPIOs();
    bool setupCameraConfig();
    bool applyOV5640Optimizations(sensor_t* sensor);
    bool applySensorSettings(sensor_t* sensor, const WildlifeCaptureSettings& settings);
    
    // IR Cut control
    void setIRCutGPIO(bool state);
    
    // Auto focus helpers
    bool focusStep(int16_t step);
    uint16_t calculateOptimalFocus();
    float calculateImageSharpness(camera_fb_t* fb);
    
    // Utility methods
    bool validateFrameSize(WildlifeFrameSize size) const;
    const char* frameSizeToString(WildlifeFrameSize size) const;
    void updateStatistics(bool capture_success);
};

// Utility functions
const char* irCutStateToString(IRCutState state);
const char* captureModeToString(WildlifeCaptureMode mode);
WildlifeCaptureSettings getDefaultWildlifeSettings();
WildlifeCaptureSettings getLowLightWildlifeSettings();
WildlifeCaptureSettings getHighQualityWildlifeSettings();

#endif // ENHANCED_WILDLIFE_CAMERA_H