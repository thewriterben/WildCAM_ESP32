#include "stream_manager.h"
#include <ArduinoJson.h>

// Constructor
StreamManager::StreamManager() :
    cameraManager_(nullptr),
    powerManager_(nullptr),
    motionManager_(nullptr),
    lastMotionTime_(0),
    lastPowerUpdate_(0),
    initialized_(false)
{
    // Initialize default configuration
    config_ = StreamConfig();
    state_ = StreamState();
    stats_ = StreamStats();
}

// Destructor
StreamManager::~StreamManager() {
    if (state_.isStreaming) {
        stopStream();
    }
    clients_.clear();
}

bool StreamManager::initialize(CameraManager* cameraManager, 
                              PowerManager* powerManager,
                              MotionDetectionManager* motionManager) {
    if (initialized_) {
        return true;
    }
    
    if (!cameraManager) {
        Serial.println("StreamManager: Camera manager is required");
        return false;
    }
    
    cameraManager_ = cameraManager;
    powerManager_ = powerManager;
    motionManager_ = motionManager;
    
    // Initialize frame interval based on default FPS
    state_.frameInterval = calculateFrameInterval(config_.targetFPS);
    
    // Set initial stream profile based on current battery level
    if (powerManager_) {
        // Get current battery level and update profile
        updateStreamProfile();
    } else {
        // Use medium battery profile as default when no power manager
        state_.currentProfile = StreamProfiles::MEDIUM_BATTERY;
    }
    
    initialized_ = true;
    
    Serial.println("StreamManager: Initialized successfully");
    notifyEvent(STREAM_EVENT_STARTED, "Stream manager initialized");
    
    return true;
}

void StreamManager::setEventCallback(StreamEventCallback callback) {
    eventCallback_ = callback;
}

bool StreamManager::startStream(const StreamConfig& config) {
    if (!initialized_) {
        Serial.println("StreamManager: Not initialized");
        return false;
    }
    
    if (state_.isStreaming) {
        Serial.println("StreamManager: Already streaming");
        return true;
    }
    
    // Validate and apply configuration
    if (!validateConfig(config)) {
        Serial.println("StreamManager: Invalid configuration");
        return false;
    }
    
    config_ = config;
    
    // Check power conditions
    if (!checkPowerConditions()) {
        Serial.println("StreamManager: Power conditions not adequate for streaming");
        notifyEvent(STREAM_EVENT_LOW_BATTERY, "Insufficient power for streaming");
        return false;
    }
    
    // Check motion conditions if motion-only mode is enabled
    if (config_.motionTriggerEnabled && state_.currentProfile.motionOnlyMode) {
        if (!checkMotionTrigger()) {
            Serial.println("StreamManager: Motion required but not detected");
            return false;
        }
    }
    
    // Update stream profile and apply camera settings
    updateStreamProfile();
    if (!applyProfileSettings()) {
        Serial.println("StreamManager: Failed to apply camera settings");
        return false;
    }
    
    // Start streaming
    state_.isStreaming = true;
    state_.startTime = millis();
    state_.lastFrameTime = 0;
    config_.active = true;
    config_.startTime = state_.startTime;
    
    // Reset statistics
    resetStats();
    
    Serial.printf("StreamManager: Streaming started - FPS: %d, Quality: %s, Size: %s\n",
                 state_.currentProfile.maxFPS,
                 streamQualityToString(state_.currentProfile.quality),
                 streamFrameSizeToString(state_.currentProfile.frameSize));
    
    notifyEvent(STREAM_EVENT_STARTED, "Live streaming started");
    
    return true;
}

bool StreamManager::stopStream() {
    if (!state_.isStreaming) {
        return true;
    }
    
    state_.isStreaming = false;
    config_.active = false;
    
    // Update statistics with final streaming time
    stats_.streamingTimeSeconds = (millis() - state_.startTime) / 1000;
    
    Serial.printf("StreamManager: Streaming stopped after %d seconds\n", 
                 stats_.streamingTimeSeconds);
    
    notifyEvent(STREAM_EVENT_STOPPED, "Live streaming stopped");
    
    return true;
}

bool StreamManager::addClient(AsyncWebSocketClient* client) {
    if (!client) {
        return false;
    }
    
    // Check maximum client limit
    if (clients_.size() >= config_.maxClients) {
        Serial.printf("StreamManager: Maximum clients (%d) reached\n", config_.maxClients);
        return false;
    }
    
    // Check if client already exists
    for (const auto& existingClient : clients_) {
        if (existingClient->client == client) {
            return true; // Client already exists
        }
    }
    
    // Add new client
    auto streamClient = std::make_unique<StreamClient>(client);
    clients_.push_back(std::move(streamClient));
    
    stats_.currentClients = clients_.size();
    if (stats_.currentClients > stats_.peakClients) {
        stats_.peakClients = stats_.currentClients;
    }
    
    Serial.printf("StreamManager: Client connected (total: %d)\n", stats_.currentClients);
    
    notifyEvent(STREAM_EVENT_CLIENT_CONNECTED, 
               String("Client connected - total: ") + String(stats_.currentClients));
    
    return true;
}

bool StreamManager::removeClient(AsyncWebSocketClient* client) {
    if (!client) {
        return false;
    }
    
    auto it = std::remove_if(clients_.begin(), clients_.end(),
        [client](const std::unique_ptr<StreamClient>& streamClient) {
            return streamClient->client == client;
        });
    
    if (it != clients_.end()) {
        clients_.erase(it, clients_.end());
        stats_.currentClients = clients_.size();
        
        Serial.printf("StreamManager: Client disconnected (remaining: %d)\n", 
                     stats_.currentClients);
        
        notifyEvent(STREAM_EVENT_CLIENT_DISCONNECTED, 
                   String("Client disconnected - remaining: ") + String(stats_.currentClients));
        
        // Stop streaming if no clients remain and not motion-triggered
        if (stats_.currentClients == 0 && !state_.motionTriggered) {
            stopStream();
        }
        
        return true;
    }
    
    return false;
}

void StreamManager::update() {
    if (!initialized_) {
        return;
    }
    
    // Clean up disconnected clients
    cleanupClients();
    
    // Update stream profile periodically
    uint32_t currentTime = millis();
    if (currentTime - lastPowerUpdate_ > 10000) { // Update every 10 seconds
        updateStreamProfile();
        lastPowerUpdate_ = currentTime;
    }
    
    // Check if streaming should continue
    if (state_.isStreaming) {
        // Check maximum duration
        if (checkMaxDuration()) {
            Serial.println("StreamManager: Maximum duration reached, stopping stream");
            stopStream();
            return;
        }
        
        // Check power conditions
        if (!checkPowerConditions()) {
            Serial.println("StreamManager: Power too low, stopping stream");
            notifyEvent(STREAM_EVENT_LOW_BATTERY, "Stopping due to low battery");
            stopStream();
            return;
        }
        
        // Check motion conditions for motion-only mode
        if (state_.currentProfile.motionOnlyMode && config_.motionTriggerEnabled) {
            if (!checkMotionTrigger()) {
                // No motion for auto-stop delay, stop streaming
                if (currentTime - lastMotionTime_ > STREAM_MOTION_AUTO_STOP_DELAY) {
                    Serial.println("StreamManager: No motion detected, stopping stream");
                    stopStream();
                    return;
                }
            }
        }
        
        // Capture and send frames if it's time
        if (currentTime - state_.lastFrameTime >= state_.frameInterval) {
            if (clients_.size() > 0) {
                captureAndSendFrame();
            }
        }
    }
}

void StreamManager::handleMotionEvent(bool motionDetected, float confidence) {
    if (!initialized_) {
        return;
    }
    
    if (motionDetected && confidence >= STREAM_MOTION_MINIMUM_CONFIDENCE) {
        lastMotionTime_ = millis();
        state_.motionTriggered = true;
        
        // Auto-start streaming if enabled and not already streaming
        if (config_.motionTriggerEnabled && !state_.isStreaming) {
            if (clients_.size() > 0) { // Only start if we have clients
                Serial.printf("StreamManager: Motion detected (%.1f%%), starting stream\n", confidence);
                
                // Delay before starting to allow motion to stabilize
                delay(STREAM_MOTION_AUTO_START_DELAY);
                
                if (startStream()) {
                    notifyEvent(STREAM_EVENT_MOTION_TRIGGERED, 
                               String("Motion triggered streaming - confidence: ") + String(confidence));
                }
            }
        }
    } else {
        // Motion ended, but keep flag for auto-stop delay
        state_.motionTriggered = false;
    }
}

void StreamManager::updatePowerStatus(int batteryPercentage, bool isCharging) {
    if (!initialized_) {
        return;
    }
    
    // Update stream profile based on new battery level
    StreamProfile newProfile = getProfileForBatteryLevel(batteryPercentage);
    
    // Check if profile changed significantly
    bool profileChanged = (newProfile.maxFPS != state_.currentProfile.maxFPS ||
                          newProfile.frameSize != state_.currentProfile.frameSize ||
                          newProfile.jpegQuality != state_.currentProfile.jpegQuality);
    
    if (profileChanged) {
        state_.currentProfile = newProfile;
        
        if (state_.isStreaming) {
            // Apply new settings immediately
            applyProfileSettings();
            state_.frameInterval = calculateFrameInterval(state_.currentProfile.maxFPS);
            
            Serial.printf("StreamManager: Profile updated for battery %d%% - FPS: %d\n",
                         batteryPercentage, state_.currentProfile.maxFPS);
        }
    }
}

bool StreamManager::setStreamConfig(const StreamConfig& config) {
    if (!validateConfig(config)) {
        return false;
    }
    
    config_ = config;
    
    // Update frame interval
    state_.frameInterval = calculateFrameInterval(config_.targetFPS);
    
    // Apply new settings if streaming
    if (state_.isStreaming) {
        updateStreamProfile();
        applyProfileSettings();
    }
    
    return true;
}

void StreamManager::resetStats() {
    stats_.reset();
    stats_.currentClients = clients_.size();
}

String StreamManager::getStatusJSON() const {
    StaticJsonDocument<512> doc;
    
    doc["streaming"] = state_.isStreaming;
    doc["clients"] = stats_.currentClients;
    doc["fps"] = state_.currentProfile.maxFPS;
    doc["quality"] = streamQualityToString(state_.currentProfile.quality);
    doc["frameSize"] = streamFrameSizeToString(state_.currentProfile.frameSize);
    doc["motionTriggered"] = state_.motionTriggered;
    doc["motionOnlyMode"] = state_.currentProfile.motionOnlyMode;
    doc["powerAware"] = config_.powerAwareMode;
    
    if (state_.isStreaming) {
        doc["duration"] = (millis() - state_.startTime) / 1000;
        doc["maxDuration"] = state_.currentProfile.maxDurationSeconds;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

String StreamManager::getStatsJSON() const {
    StaticJsonDocument<512> doc;
    
    doc["totalFrames"] = stats_.totalFramesSent;
    doc["totalBytes"] = stats_.totalBytesSent;
    doc["currentClients"] = stats_.currentClients;
    doc["peakClients"] = stats_.peakClients;
    doc["streamingTime"] = stats_.streamingTimeSeconds;
    doc["averageFrameSize"] = stats_.averageFrameSize;
    doc["averageFPS"] = stats_.averageFPS;
    doc["droppedFrames"] = stats_.droppedFrames;
    
    String result;
    serializeJson(doc, result);
    return result;
}

bool StreamManager::setFrameRate(uint8_t fps) {
    if (fps < STREAM_MIN_FPS || fps > STREAM_MAX_FPS) {
        return false;
    }
    
    config_.targetFPS = fps;
    state_.frameInterval = calculateFrameInterval(fps);
    
    return true;
}

bool StreamManager::setQuality(StreamQuality quality) {
    config_.quality = quality;
    updateStreamProfile();
    
    if (state_.isStreaming) {
        applyProfileSettings();
    }
    
    return true;
}

bool StreamManager::setFrameSize(StreamFrameSize frameSize) {
    config_.frameSize = frameSize;
    updateStreamProfile();
    
    if (state_.isStreaming) {
        applyProfileSettings();
    }
    
    return true;
}

void StreamManager::setMotionOnlyMode(bool enabled) {
    config_.motionTriggerEnabled = enabled;
}

bool StreamManager::shouldStreamBeActive() const {
    if (!initialized_ || clients_.size() == 0) {
        return false;
    }
    
    if (!checkPowerConditions()) {
        return false;
    }
    
    if (config_.motionTriggerEnabled && state_.currentProfile.motionOnlyMode) {
        return checkMotionTrigger();
    }
    
    return true;
}

// Private methods implementation

bool StreamManager::captureAndSendFrame() {
    if (!cameraManager_ || clients_.size() == 0) {
        return false;
    }
    
    // Capture frame from camera
    camera_fb_t* fb = cameraManager_->captureToBuffer();
    if (!fb) {
        stats_.droppedFrames++;
        state_.lastError = STREAM_ERROR_CAMERA_BUSY;
        return false;
    }
    
    // Send frame to all active clients
    uint8_t successfulSends = 0;
    for (auto& client : clients_) {
        if (client->active && client->client->status() == WS_CONNECTED) {
            if (sendFrameToClient(client.get(), fb)) {
                successfulSends++;
            }
        }
    }
    
    // Update statistics
    updateStats(fb->len, successfulSends);
    state_.lastFrameTime = millis();
    
    // Return frame buffer to camera manager
    cameraManager_->returnFrameBuffer(fb);
    
    return successfulSends > 0;
}

bool StreamManager::sendFrameToClient(StreamClient* client, camera_fb_t* frameBuffer) {
    if (!client || !frameBuffer || client->client->status() != WS_CONNECTED) {
        return false;
    }
    
    // Generate MJPEG frame header
    String header = generateFrameHeader(frameBuffer->len);
    
    // Send header
    if (!client->client->text(header)) {
        client->active = false;
        return false;
    }
    
    // Send frame data
    if (!client->client->binary(frameBuffer->buf, frameBuffer->len)) {
        client->active = false;
        return false;
    }
    
    // Update client statistics
    client->framesSent++;
    client->bytesSent += frameBuffer->len + header.length();
    
    return true;
}

void StreamManager::updateStreamProfile() {
    if (!config_.powerAwareMode) {
        return; // Use manual settings
    }
    
    // Get current battery level from power manager
    int batteryPercentage = 50; // Default to medium if no power manager
    if (powerManager_ && powerManager_->isInitialized()) {
        batteryPercentage = powerManager_->getBatteryPercentage();
    }
    
    // Update profile based on battery level
    state_.currentProfile = getProfileForBatteryLevel(batteryPercentage);
    
    // Apply user preferences if they don't conflict with power constraints
    if (config_.quality != STREAM_QUALITY_AUTO) {
        // Only allow higher quality if battery permits
        if (batteryPercentage > 50 || config_.quality <= state_.currentProfile.quality) {
            state_.currentProfile.quality = config_.quality;
        }
    }
    
    // Apply frame rate preference if within power limits
    if (config_.targetFPS <= state_.currentProfile.maxFPS) {
        state_.currentProfile.maxFPS = config_.targetFPS;
    }
    
    // Apply frame size preference if within power limits
    if (batteryPercentage > 30 && config_.frameSize != STREAM_FRAMESIZE_HD) {
        state_.currentProfile.frameSize = config_.frameSize;
    }
    
    // Update frame interval based on current profile
    state_.frameInterval = calculateFrameInterval(state_.currentProfile.maxFPS);
}

bool StreamManager::applyProfileSettings() {
    if (!cameraManager_) {
        return false;
    }
    
    // Convert stream frame size to ESP32 camera frame size
    framesize_t cameraFrameSize = streamFrameSizeToESP32(state_.currentProfile.frameSize);
    
    // Configure camera sensor for streaming
    // Note: This assumes CameraManager has methods to configure these settings
    // You may need to extend CameraManager or access the sensor directly
    
    Serial.printf("StreamManager: Applied profile - FPS: %d, Quality: %s, Frame: %s\n",
                 state_.currentProfile.maxFPS,
                 streamQualityToString(state_.currentProfile.quality),
                 streamFrameSizeToString(state_.currentProfile.frameSize));
    
    return true;
}

bool StreamManager::checkMotionTrigger() const {
    if (!motionManager_) {
        // No motion manager available, allow streaming if motion flag is set
        return state_.motionTriggered;
    }
    
    // Check if motion was detected recently
    if (!state_.motionTriggered) {
        return false;
    }
    
    // Motion is considered active for the auto-stop delay period
    uint32_t timeSinceMotion = millis() - lastMotionTime_;
    return timeSinceMotion < STREAM_MOTION_AUTO_STOP_DELAY;
}

bool StreamManager::checkPowerConditions() const {
    if (!powerManager_ || !powerManager_->isInitialized()) {
        return true; // Allow streaming if no power manager
    }
    
    // Check power state - don't stream in critical power state
    PowerState powerState = powerManager_->getPowerState();
    if (powerState == POWER_CRITICAL) {
        return false;
    }
    
    // Check if we should enter low power mode
    if (powerManager_->shouldEnterLowPower() && state_.currentProfile.motionOnlyMode) {
        // In low power mode, only allow streaming if motion was recently detected
        return checkMotionTrigger();
    }
    
    return true;
}

bool StreamManager::checkMaxDuration() const {
    if (!state_.isStreaming) {
        return false;
    }
    
    uint32_t streamingDuration = (millis() - state_.startTime) / 1000;
    return streamingDuration >= state_.currentProfile.maxDurationSeconds;
}

void StreamManager::cleanupClients() {
    auto it = std::remove_if(clients_.begin(), clients_.end(),
        [](const std::unique_ptr<StreamClient>& client) {
            return !client->active || client->client->status() != WS_CONNECTED;
        });
    
    if (it != clients_.end()) {
        size_t removedCount = std::distance(it, clients_.end());
        clients_.erase(it, clients_.end());
        stats_.currentClients = clients_.size();
        
        if (removedCount > 0) {
            Serial.printf("StreamManager: Cleaned up %d disconnected clients\n", removedCount);
        }
    }
}

uint32_t StreamManager::calculateFrameInterval(uint8_t fps) const {
    if (fps == 0) fps = 1;
    return 1000 / fps;
}

void StreamManager::notifyEvent(StreamEvent event, const String& message) {
    if (eventCallback_) {
        eventCallback_(event, message);
    }
}

void StreamManager::updateStats(size_t frameSize, uint8_t clientCount) {
    stats_.totalFramesSent++;
    stats_.totalBytesSent += frameSize * clientCount;
    
    // Update average frame size
    stats_.averageFrameSize = stats_.totalBytesSent / stats_.totalFramesSent;
    
    // Update average FPS
    if (state_.isStreaming) {
        uint32_t streamingTime = (millis() - state_.startTime) / 1000;
        if (streamingTime > 0) {
            stats_.averageFPS = (float)stats_.totalFramesSent / streamingTime;
        }
    }
}

String StreamManager::generateFrameHeader(size_t frameSize) const {
    return String(MJPEG_FRAME_HEADER) + String(frameSize) + "\r\n\r\n";
}

bool StreamManager::validateConfig(const StreamConfig& config) const {
    if (config.targetFPS < STREAM_MIN_FPS || config.targetFPS > STREAM_MAX_FPS) {
        return false;
    }
    
    if (config.maxClients > STREAM_MAX_CLIENTS) {
        return false;
    }
    
    if (config.maxStreamDuration == 0) {
        return false;
    }
    
    return true;
}