#ifndef STREAM_CONFIG_H
#define STREAM_CONFIG_H

#include <Arduino.h>

// ===========================
// LIVE STREAMING CONFIGURATION
// ===========================

// Streaming Protocol Configuration
#define MJPEG_BOUNDARY "wildlifecam_stream"
#define MJPEG_CONTENT_TYPE "multipart/x-mixed-replace; boundary=" MJPEG_BOUNDARY
#define MJPEG_FRAME_HEADER "\r\n--" MJPEG_BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: "

// Streaming Performance Limits
#define STREAM_MAX_CLIENTS 3              // Maximum concurrent viewers
#define STREAM_MAX_FPS 10                 // Maximum frames per second
#define STREAM_MIN_FPS 1                  // Minimum frames per second
#define STREAM_DEFAULT_FPS 5              // Default frame rate
#define STREAM_FRAME_TIMEOUT 10000        // Frame timeout in milliseconds
#define STREAM_CLIENT_BUFFER_SIZE 32768   // Client buffer size (32KB)
#define STREAM_MAX_FRAME_SIZE 65536       // Maximum frame size (64KB)

// Stream Quality Levels
enum StreamQuality {
    STREAM_QUALITY_LOW = 0,     // Low quality for power saving
    STREAM_QUALITY_MEDIUM = 1,  // Medium quality for balanced operation  
    STREAM_QUALITY_HIGH = 2,    // High quality for optimal viewing
    STREAM_QUALITY_AUTO = 3     // Automatic quality based on conditions
};

// Stream Frame Sizes
enum StreamFrameSize {
    STREAM_FRAMESIZE_QVGA = 0,  // 320x240 - Low power
    STREAM_FRAMESIZE_VGA = 1,   // 640x480 - Medium quality
    STREAM_FRAMESIZE_SVGA = 2,  // 800x600 - High quality  
    STREAM_FRAMESIZE_HD = 3     // 1280x720 - Maximum quality
};

// Power-Aware Streaming Profiles
struct StreamProfile {
    uint8_t maxFPS;
    StreamFrameSize frameSize;
    StreamQuality quality;
    uint32_t maxDurationSeconds;
    uint8_t jpegQuality;          // 10-63 (lower = better quality)
    bool motionOnlyMode;          // Only stream when motion detected
    
    StreamProfile(uint8_t fps = 5, StreamFrameSize fs = STREAM_FRAMESIZE_VGA, 
                  StreamQuality q = STREAM_QUALITY_MEDIUM, uint32_t duration = 1800,
                  uint8_t jpeg = 15, bool motionOnly = false) :
        maxFPS(fps), frameSize(fs), quality(q), maxDurationSeconds(duration),
        jpegQuality(jpeg), motionOnlyMode(motionOnly) {}
};

// Battery Level Based Stream Profiles (matches PowerManager's 5-tier system)
namespace StreamProfiles {
    // High battery (>70%): Full quality streaming
    const StreamProfile HIGH_BATTERY(10, STREAM_FRAMESIZE_HD, STREAM_QUALITY_HIGH, 3600, 12, false);
    
    // Good battery (50-70%): Balanced streaming
    const StreamProfile GOOD_BATTERY(7, STREAM_FRAMESIZE_VGA, STREAM_QUALITY_MEDIUM, 2400, 15, false);
    
    // Medium battery (30-50%): Conservative streaming
    const StreamProfile MEDIUM_BATTERY(5, STREAM_FRAMESIZE_VGA, STREAM_QUALITY_MEDIUM, 1800, 18, true);
    
    // Low battery (15-30%): Minimal streaming
    const StreamProfile LOW_BATTERY(3, STREAM_FRAMESIZE_QVGA, STREAM_QUALITY_LOW, 900, 25, true);
    
    // Critical battery (<15%): Emergency streaming only
    const StreamProfile CRITICAL_BATTERY(1, STREAM_FRAMESIZE_QVGA, STREAM_QUALITY_LOW, 300, 35, true);
}

// Motion Detection Integration
#define STREAM_MOTION_TRIGGER_ENABLED true
#define STREAM_MOTION_AUTO_START_DELAY 1000    // ms - delay before starting stream after motion
#define STREAM_MOTION_AUTO_STOP_DELAY 30000    // ms - delay before stopping stream after no motion
#define STREAM_MOTION_MINIMUM_CONFIDENCE 50    // Minimum motion confidence to trigger stream

// Stream Statistics Tracking
struct StreamStats {
    uint32_t totalFramesSent = 0;
    uint32_t totalBytesSent = 0;
    uint32_t currentClients = 0;
    uint32_t peakClients = 0;
    uint32_t streamingTimeSeconds = 0;
    uint32_t averageFrameSize = 0;
    float averageFPS = 0.0;
    uint32_t droppedFrames = 0;
    uint32_t lastFrameTime = 0;
    
    void reset() {
        totalFramesSent = 0;
        totalBytesSent = 0;
        streamingTimeSeconds = 0;
        averageFrameSize = 0;
        averageFPS = 0.0;
        droppedFrames = 0;
        lastFrameTime = 0;
        // Don't reset currentClients, peakClients as they represent current state
    }
};

// Stream Events for Integration
enum StreamEvent {
    STREAM_EVENT_STARTED = 0,
    STREAM_EVENT_STOPPED = 1,
    STREAM_EVENT_CLIENT_CONNECTED = 2,
    STREAM_EVENT_CLIENT_DISCONNECTED = 3,
    STREAM_EVENT_MOTION_TRIGGERED = 4,
    STREAM_EVENT_LOW_BATTERY = 5,
    STREAM_EVENT_ERROR = 6
};

// Stream Error Codes
enum StreamError {
    STREAM_OK = 0,
    STREAM_ERROR_NO_CLIENTS = 1,
    STREAM_ERROR_CAMERA_BUSY = 2,
    STREAM_ERROR_LOW_MEMORY = 3,
    STREAM_ERROR_POWER_SAVE = 4,
    STREAM_ERROR_MOTION_REQUIRED = 5,
    STREAM_ERROR_MAX_DURATION = 6,
    STREAM_ERROR_NETWORK = 7
};

// Stream Configuration Structure
struct StreamConfig {
    bool enabled = true;
    uint8_t targetFPS = STREAM_DEFAULT_FPS;
    StreamFrameSize frameSize = STREAM_FRAMESIZE_VGA;
    StreamQuality quality = STREAM_QUALITY_AUTO;
    bool powerAwareMode = true;
    bool motionTriggerEnabled = STREAM_MOTION_TRIGGER_ENABLED;
    uint32_t maxStreamDuration = 1800; // seconds
    uint8_t maxClients = STREAM_MAX_CLIENTS;
    
    // Runtime state
    bool active = false;
    uint32_t startTime = 0;
    StreamProfile currentProfile = StreamProfiles::MEDIUM_BATTERY;
    
    StreamConfig() {}
};

// Utility Functions
inline const char* streamQualityToString(StreamQuality quality) {
    switch (quality) {
        case STREAM_QUALITY_LOW: return "Low";
        case STREAM_QUALITY_MEDIUM: return "Medium";
        case STREAM_QUALITY_HIGH: return "High";
        case STREAM_QUALITY_AUTO: return "Auto";
        default: return "Unknown";
    }
}

inline const char* streamFrameSizeToString(StreamFrameSize frameSize) {
    switch (frameSize) {
        case STREAM_FRAMESIZE_QVGA: return "QVGA (320x240)";
        case STREAM_FRAMESIZE_VGA: return "VGA (640x480)";
        case STREAM_FRAMESIZE_SVGA: return "SVGA (800x600)";
        case STREAM_FRAMESIZE_HD: return "HD (1280x720)";
        default: return "Unknown";
    }
}

inline framesize_t streamFrameSizeToESP32(StreamFrameSize frameSize) {
    switch (frameSize) {
        case STREAM_FRAMESIZE_QVGA: return FRAMESIZE_QVGA;
        case STREAM_FRAMESIZE_VGA: return FRAMESIZE_VGA;
        case STREAM_FRAMESIZE_SVGA: return FRAMESIZE_SVGA;
        case STREAM_FRAMESIZE_HD: return FRAMESIZE_HD;
        default: return FRAMESIZE_VGA;
    }
}

inline StreamProfile getProfileForBatteryLevel(int batteryPercentage) {
    if (batteryPercentage > 70) return StreamProfiles::HIGH_BATTERY;
    else if (batteryPercentage > 50) return StreamProfiles::GOOD_BATTERY;
    else if (batteryPercentage > 30) return StreamProfiles::MEDIUM_BATTERY;
    else if (batteryPercentage > 15) return StreamProfiles::LOW_BATTERY;
    else return StreamProfiles::CRITICAL_BATTERY;
}

#endif // STREAM_CONFIG_H