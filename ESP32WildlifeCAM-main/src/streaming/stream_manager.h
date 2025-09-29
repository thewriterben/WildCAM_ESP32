#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncWebSocket.h>
#include <esp_camera.h>
#include <vector>
#include <memory>

#include "stream_config.h"
#include "../camera/camera_manager.h"
#include "../../firmware/src/power_manager.h"
#include "../detection/motion_detection_manager.h"

/**
 * @brief Live streaming management system for ESP32WildlifeCAM
 * 
 * Provides real-time MJPEG streaming with power-aware operation,
 * motion detection integration, and multi-client support.
 * 
 * Key Features:
 * - MJPEG streaming protocol for web browser compatibility
 * - Power-aware streaming profiles based on battery level
 * - Motion-triggered streaming activation
 * - Multi-client support (up to 3 concurrent viewers)
 * - Automatic quality adjustment based on conditions
 * - Integration with existing camera and power management systems
 */
class StreamManager {
public:
    /**
     * @brief Stream client information
     */
    struct StreamClient {
        AsyncWebSocketClient* client;
        uint32_t connectedTime;
        uint32_t framesSent;
        uint32_t bytesSent;
        bool active;
        
        StreamClient(AsyncWebSocketClient* c) : 
            client(c), connectedTime(millis()), framesSent(0), 
            bytesSent(0), active(true) {}
    };
    
    /**
     * @brief Stream state information
     */
    struct StreamState {
        bool isStreaming = false;
        bool motionTriggered = false;
        uint32_t startTime = 0;
        uint32_t lastFrameTime = 0;
        uint32_t frameInterval = 200; // ms between frames (5 FPS default)
        StreamProfile currentProfile = StreamProfiles::MEDIUM_BATTERY;
        StreamError lastError = STREAM_OK;
        
        StreamState() {}
    };
    
    /**
     * @brief Stream event callback function type
     */
    typedef std::function<void(StreamEvent event, const String& message)> StreamEventCallback;

public:
    /**
     * @brief Constructor
     */
    StreamManager();
    
    /**
     * @brief Destructor
     */
    ~StreamManager();
    
    /**
     * @brief Initialize the stream manager
     * @param cameraManager Pointer to camera manager instance
     * @param powerManager Pointer to power manager instance
     * @param motionManager Pointer to motion detection manager instance
     * @return true if initialization successful
     */
    bool initialize(CameraManager* cameraManager, 
                   PowerManager* powerManager = nullptr,
                   MotionDetectionManager* motionManager = nullptr);
    
    /**
     * @brief Set stream event callback
     * @param callback Function to call on stream events
     */
    void setEventCallback(StreamEventCallback callback);
    
    /**
     * @brief Start streaming
     * @param config Optional stream configuration
     * @return true if streaming started successfully
     */
    bool startStream(const StreamConfig& config = StreamConfig());
    
    /**
     * @brief Stop streaming
     * @return true if streaming stopped successfully
     */
    bool stopStream();
    
    /**
     * @brief Check if currently streaming
     * @return true if actively streaming
     */
    bool isStreaming() const { return state_.isStreaming; }
    
    /**
     * @brief Add a client to the stream
     * @param client WebSocket client pointer
     * @return true if client added successfully
     */
    bool addClient(AsyncWebSocketClient* client);
    
    /**
     * @brief Remove a client from the stream
     * @param client WebSocket client pointer
     * @return true if client removed successfully
     */
    bool removeClient(AsyncWebSocketClient* client);
    
    /**
     * @brief Get current number of connected clients
     * @return Number of active clients
     */
    uint8_t getClientCount() const { return clients_.size(); }
    
    /**
     * @brief Update stream (call from main loop)
     * This handles frame capture and distribution
     */
    void update();
    
    /**
     * @brief Handle motion detection event
     * @param motionDetected true if motion was detected
     * @param confidence Motion detection confidence (0-100)
     */
    void handleMotionEvent(bool motionDetected, float confidence = 0.0);
    
    /**
     * @brief Update power status (called by power manager)
     * @param batteryPercentage Current battery level (0-100)
     * @param isCharging true if currently charging
     */
    void updatePowerStatus(int batteryPercentage, bool isCharging = false);
    
    /**
     * @brief Set stream configuration
     * @param config New stream configuration
     * @return true if configuration applied successfully
     */
    bool setStreamConfig(const StreamConfig& config);
    
    /**
     * @brief Get current stream configuration
     * @return Current stream configuration
     */
    StreamConfig getStreamConfig() const { return config_; }
    
    /**
     * @brief Get stream statistics
     * @return Current stream statistics
     */
    StreamStats getStreamStats() const { return stats_; }
    
    /**
     * @brief Get stream state
     * @return Current stream state
     */
    StreamState getStreamState() const { return state_; }
    
    /**
     * @brief Reset stream statistics
     */
    void resetStats();
    
    /**
     * @brief Generate JSON status for API endpoints
     * @return JSON string with current status
     */
    String getStatusJSON() const;
    
    /**
     * @brief Generate JSON statistics for API endpoints
     * @return JSON string with current statistics
     */
    String getStatsJSON() const;
    
    /**
     * @brief Set frame rate
     * @param fps Target frames per second (1-10)
     * @return true if frame rate set successfully
     */
    bool setFrameRate(uint8_t fps);
    
    /**
     * @brief Set stream quality
     * @param quality Stream quality level
     * @return true if quality set successfully
     */
    bool setQuality(StreamQuality quality);
    
    /**
     * @brief Set frame size
     * @param frameSize Stream frame size
     * @return true if frame size set successfully
     */
    bool setFrameSize(StreamFrameSize frameSize);
    
    /**
     * @brief Enable/disable motion-only streaming
     * @param enabled true to enable motion-only mode
     */
    void setMotionOnlyMode(bool enabled);
    
    /**
     * @brief Check if stream should be active based on current conditions
     * @return true if stream should be active
     */
    bool shouldStreamBeActive() const;

private:
    // Core components
    CameraManager* cameraManager_;
    PowerManager* powerManager_;
    MotionDetectionManager* motionManager_;
    
    // Stream configuration and state
    StreamConfig config_;
    StreamState state_;
    StreamStats stats_;
    
    // Client management
    std::vector<std::unique_ptr<StreamClient>> clients_;
    
    // Event callback
    StreamEventCallback eventCallback_;
    
    // Timing and control
    uint32_t lastMotionTime_;
    uint32_t lastPowerUpdate_;
    bool initialized_;
    
    // Private methods
    
    /**
     * @brief Capture and distribute frame to all clients
     * @return true if frame captured and sent successfully
     */
    bool captureAndSendFrame();
    
    /**
     * @brief Send frame to a specific client
     * @param client Stream client to send frame to
     * @param frameBuffer Camera frame buffer
     * @return true if frame sent successfully
     */
    bool sendFrameToClient(StreamClient* client, camera_fb_t* frameBuffer);
    
    /**
     * @brief Update stream profile based on current conditions
     * This considers battery level, motion state, and user preferences
     */
    void updateStreamProfile();
    
    /**
     * @brief Apply current stream profile settings to camera
     * @return true if settings applied successfully
     */
    bool applyProfileSettings();
    
    /**
     * @brief Check if motion trigger conditions are met
     * @return true if motion should trigger streaming
     */
    bool checkMotionTrigger() const;
    
    /**
     * @brief Check if power conditions allow streaming
     * @return true if power conditions are adequate
     */
    bool checkPowerConditions() const;
    
    /**
     * @brief Check if maximum stream duration has been reached
     * @return true if maximum duration exceeded
     */
    bool checkMaxDuration() const;
    
    /**
     * @brief Clean up disconnected clients
     */
    void cleanupClients();
    
    /**
     * @brief Calculate frame interval based on target FPS
     * @param fps Target frames per second
     * @return Frame interval in milliseconds
     */
    uint32_t calculateFrameInterval(uint8_t fps) const;
    
    /**
     * @brief Send event notification
     * @param event Stream event type
     * @param message Optional message
     */
    void notifyEvent(StreamEvent event, const String& message = "");
    
    /**
     * @brief Update stream statistics
     * @param frameSize Size of frame in bytes
     * @param clientCount Number of clients frame was sent to
     */
    void updateStats(size_t frameSize, uint8_t clientCount);
    
    /**
     * @brief Generate MJPEG frame header
     * @param frameSize Size of JPEG frame
     * @return MJPEG header string
     */
    String generateFrameHeader(size_t frameSize) const;
    
    /**
     * @brief Validate stream configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    bool validateConfig(const StreamConfig& config) const;
};

#endif // STREAM_MANAGER_H