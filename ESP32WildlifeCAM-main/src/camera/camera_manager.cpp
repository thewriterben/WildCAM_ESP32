#include "camera_manager.h"
#include <SD_MMC.h>
#include <FS.h>
#include <time.h>
#include <ArduinoJson.h>

// Camera configuration constants
#define CAMERA_SHARPNESS_DEFAULT 0
#define CAMERA_DENOISE_DEFAULT 0
#define CAMERA_WB_MODE_DEFAULT 0
#define CAMERA_AEC_VALUE_DEFAULT 300
#define CAMERA_AGC_GAIN_DEFAULT 0
#define CAMERA_GAIN_CEILING_DEFAULT GAINCEILING_2X
#define LENS_CORRECTION_ENABLED true
#define CAMERA_GRAB_MODE CAMERA_GRAB_LATEST

bool CameraManager::initialize() {
    if (initialized) {
        return true;
    }

    Serial.println("Initializing camera...");

    // Get camera configuration
    camera_config_t config = getCameraConfig();

    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    // Get camera sensor for configuration
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        Serial.println("Failed to get camera sensor");
        return false;
    }

    // Configure initial sensor settings
    configureSensor();

    // Optimize for wildlife photography
    optimizeForWildlife();

    // Configure advanced GPIO pins
    configureAdvancedGPIOs();

    initialized = true;
    resetStatistics();

    Serial.println("Camera initialized successfully");
    Serial.printf("Camera configuration: %s\n", getConfiguration().c_str());

    return true;
}

CameraManager::CaptureResult CameraManager::captureImage(const String& folder) {
    CaptureResult result = {};
    result.success = false;

    if (!initialized) {
        Serial.println("Camera not initialized");
        return result;
    }

    uint32_t startTime = millis();

    // Capture image to frame buffer
    camera_fb_t* fb = captureToBuffer();
    if (fb == nullptr) {
        Serial.println("Camera capture failed");
        updateStats(false, millis() - startTime, 0);
        return result;
    }

    // Save to SD card
    String filename = saveFrameBuffer(fb, folder);
    if (filename.length() > 0) {
        result.success = true;
        result.filename = filename;
        result.imageSize = fb->len;
        result.captureTime = millis() - startTime;
        result.frameBuffer = fb;

        Serial.printf("Image captured: %s (%d bytes, %d ms)\n", 
                     filename.c_str(), fb->len, result.captureTime);
    }

    updateStats(result.success, result.captureTime, result.imageSize);
    return result;
}

camera_fb_t* CameraManager::captureToBuffer() {
    if (!initialized) {
        return nullptr;
    }

    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb == nullptr) {
        Serial.println("Camera capture failed");
        return nullptr;
    }

    // Validate frame buffer
    if (fb->len == 0 || fb->buf == nullptr) {
        Serial.println("Invalid frame buffer");
        esp_camera_fb_return(fb);
        return nullptr;
    }

    return fb;
}

void CameraManager::returnFrameBuffer(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

String CameraManager::saveFrameBuffer(camera_fb_t* fb, const String& folder, const String& filename) {
    if (fb == nullptr || fb->len == 0) {
        return "";
    }

    // Ensure directory exists
    if (!ensureDirectory(folder)) {
        Serial.printf("Failed to create directory: %s\n", folder.c_str());
        return "";
    }

    // Generate filename if not provided
    String saveFilename = filename;
    if (saveFilename.length() == 0) {
        saveFilename = generateFilename(folder);
    } else if (!saveFilename.startsWith("/")) {
        saveFilename = folder + "/" + saveFilename;
    }

    // Save to SD card
    File file = SD_MMC.open(saveFilename.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file: %s\n", saveFilename.c_str());
        return "";
    }

    size_t written = file.write(fb->buf, fb->len);
    file.close();

    if (written != fb->len) {
        Serial.printf("Write error: %d/%d bytes written\n", written, fb->len);
        return "";
    }

    return saveFilename;
}

void CameraManager::configureSensor(int brightness, int contrast, int saturation) {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return;
    }

    // Configure sensor settings
    sensor->set_brightness(sensor, brightness);   // -2 to 2
    sensor->set_contrast(sensor, contrast);       // -2 to 2
    sensor->set_saturation(sensor, saturation);   // -2 to 2
    
    // Wildlife-optimized settings
    sensor->set_sharpness(sensor, CAMERA_SHARPNESS_DEFAULT);
    sensor->set_denoise(sensor, CAMERA_DENOISE_DEFAULT);
    sensor->set_gain_ctrl(sensor, 1);             // Enable auto gain
    sensor->set_exposure_ctrl(sensor, 1);         // Enable auto exposure
    sensor->set_awb_gain(sensor, 1);              // Enable auto white balance gain
    sensor->set_wb_mode(sensor, CAMERA_WB_MODE_DEFAULT);
}

void CameraManager::optimizeForWildlife(int lightLevel, int motionLevel) {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return;
    }

    // Adjust settings based on light level
    if (lightLevel < 30) {  // Low light conditions
        sensor->set_aec_value(sensor, 600);       // Longer exposure
        sensor->set_agc_gain(sensor, 10);         // Higher gain
        sensor->set_gainceiling(sensor, GAINCEILING_16X);
    } else if (lightLevel > 70) {  // Bright conditions
        sensor->set_aec_value(sensor, 200);       // Shorter exposure
        sensor->set_agc_gain(sensor, 0);          // Lower gain
        sensor->set_gainceiling(sensor, GAINCEILING_2X);
    } else {  // Normal conditions
        sensor->set_aec_value(sensor, CAMERA_AEC_VALUE_DEFAULT);
        sensor->set_agc_gain(sensor, CAMERA_AGC_GAIN_DEFAULT);
        sensor->set_gainceiling(sensor, CAMERA_GAIN_CEILING_DEFAULT);
    }

    // Adjust for motion level
    if (motionLevel > 50) {  // High motion expected
        sensor->set_aec_value(sensor, 150);       // Faster shutter for motion blur reduction
    }

    // Enable lens correction for wildlife photography
    if (LENS_CORRECTION_ENABLED) {
        sensor->set_lenc(sensor, 1);
    }
}

void CameraManager::setNightMode(bool enable) {
    nightModeEnabled = enable;
    
    if (enable) {
        // Night mode optimizations
        optimizeForWildlife(10, 25);  // Low light, low motion
        configureSensor(1, 1, 0);     // Increased brightness and contrast
    } else {
        // Day mode optimizations
        optimizeForWildlife(50, 50);  // Normal conditions
        configureSensor(0, 0, 0);     // Default settings
    }
}

void CameraManager::resetStatistics() {
    stats = {};
    captureCounter = 0;
}

String CameraManager::getConfiguration() const {
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        return "Camera sensor not available";
    }

    String config = "Frame Size: ";
    switch (sensor->status.framesize) {
        case FRAMESIZE_UXGA: config += "UXGA(1600x1200)"; break;
        case FRAMESIZE_SXGA: config += "SXGA(1280x1024)"; break;
        case FRAMESIZE_XGA:  config += "XGA(1024x768)"; break;
        case FRAMESIZE_SVGA: config += "SVGA(800x600)"; break;
        case FRAMESIZE_VGA:  config += "VGA(640x480)"; break;
        default: config += "Unknown"; break;
    }
    
    config += ", Quality: " + String(sensor->status.quality);
    config += ", Night Mode: " + String(nightModeEnabled ? "ON" : "OFF");
    
    return config;
}

void CameraManager::cleanup() {
    if (initialized) {
        // Stop any active time-lapse
        if (timeLapseActive) {
            stopTimeLapse();
        }
        
        esp_camera_deinit();
        initialized = false;
        Serial.println("Camera cleaned up");
    }
}

CameraManager::BurstResult CameraManager::captureBurst(const BurstConfig& config, const String& folder) {
    BurstResult result = {};
    result.success = false;
    result.capturedCount = 0;
    result.folder = folder;

    if (!initialized) {
        Serial.println("Camera not initialized");
        return result;
    }

    // Validate burst configuration
    if (config.count < 1 || config.count > BURST_MAX_COUNT) {
        Serial.printf("Invalid burst count: %d (max: %d)\n", config.count, BURST_MAX_COUNT);
        return result;
    }

    if (config.intervalMs < BURST_MIN_INTERVAL_MS || config.intervalMs > BURST_MAX_INTERVAL_MS) {
        Serial.printf("Invalid burst interval: %d ms\n", config.intervalMs);
        return result;
    }

    // Generate sequence ID
    result.sequenceId = generateBurstSequenceId();
    
    // Ensure directory exists
    if (!ensureDirectory(folder)) {
        Serial.printf("Failed to create burst directory: %s\n", folder.c_str());
        return result;
    }

    uint32_t startTime = millis();
    Serial.printf("Starting burst capture: %d images, %d ms interval\n", config.count, config.intervalMs);

    // Capture burst sequence
    for (uint8_t i = 0; i < config.count; i++) {
        uint32_t captureStart = millis();
        
        // Capture frame to buffer
        camera_fb_t* fb = captureToBuffer();
        if (fb == nullptr) {
            Serial.printf("Burst capture %d/%d failed\n", i + 1, config.count);
            break;
        }

        // Generate filename for this burst image
        String filename = String(folder + "/" + result.sequenceId + "_" + String(i + 1, DEC) + ".jpg");
        
        // Save frame buffer
        String savedFilename = saveFrameBuffer(fb, folder, filename);
        if (savedFilename.length() > 0) {
            result.filenames[result.capturedCount] = savedFilename;
            result.totalSize += fb->len;
            result.capturedCount++;
            
            Serial.printf("Burst %d/%d captured: %s (%d bytes)\n", 
                         i + 1, config.count, savedFilename.c_str(), fb->len);
        } else {
            Serial.printf("Failed to save burst image %d/%d\n", i + 1, config.count);
        }

        // Return frame buffer
        returnFrameBuffer(fb);

        // Wait for interval (except for last image)
        if (i < config.count - 1) {
            uint32_t captureTime = millis() - captureStart;
            if (captureTime < config.intervalMs) {
                delay(config.intervalMs - captureTime);
            }
        }
    }

    result.totalTime = millis() - startTime;
    result.success = (result.capturedCount > 0);

    // Save burst metadata if requested
    if (config.createSequence && result.success) {
        saveBurstMetadata(result, folder);
    }

    // Update statistics
    updateStats(result.success, result.totalTime, result.totalSize);

    Serial.printf("Burst capture completed: %d/%d images, %d ms total\n", 
                 result.capturedCount, config.count, result.totalTime);

    return result;
}

CameraManager::VideoResult CameraManager::recordVideo(const VideoConfig& config, const String& folder) {
    VideoResult result = {};
    result.success = false;
    result.frameCount = 0;

    if (!initialized) {
        Serial.println("Camera not initialized");
        return result;
    }

    // Ensure directory exists
    if (!ensureDirectory(folder)) {
        Serial.printf("Failed to create video directory: %s\n", folder.c_str());
        return result;
    }

    // Generate video filename
    result.filename = generateVideoFilename(folder);
    
    // Calculate frame interval
    uint32_t frameIntervalMs = 1000 / config.frameRate;
    uint32_t totalFrames = config.durationSeconds * config.frameRate;
    
    Serial.printf("Starting video recording: %ds, %dfps, %d frames\n", 
                 config.durationSeconds, config.frameRate, totalFrames);

    // Open video file for writing
    File videoFile = SD_MMC.open(result.filename.c_str(), FILE_WRITE);
    if (!videoFile) {
        Serial.printf("Failed to create video file: %s\n", result.filename.c_str());
        return result;
    }

    // Write simple MJPEG header
    videoFile.write((uint8_t*)"MJPG", 4);  // Simple MJPEG marker

    uint32_t startTime = millis();
    uint32_t nextFrameTime = startTime;

    // Record video frames
    for (uint32_t frameNum = 0; frameNum < totalFrames; frameNum++) {
        uint32_t currentTime = millis();
        
        // Wait for next frame time
        if (currentTime < nextFrameTime) {
            delay(nextFrameTime - currentTime);
        }
        
        nextFrameTime = startTime + (frameNum + 1) * frameIntervalMs;

        // Capture frame
        camera_fb_t* fb = captureToBuffer();
        if (fb == nullptr) {
            Serial.printf("Video frame %d capture failed\n", frameNum + 1);
            continue;
        }

        // Write frame to video file
        if (writeVideoFrame(fb, videoFile)) {
            result.frameCount++;
            result.fileSize += fb->len + 8; // Frame data + header
        } else {
            Serial.printf("Failed to write video frame %d\n", frameNum + 1);
        }

        // Return frame buffer
        returnFrameBuffer(fb);

        // Check if recording time exceeded
        if (millis() - startTime > (config.durationSeconds * 1000 + 1000)) {
            Serial.println("Video recording timeout, stopping");
            break;
        }
    }

    videoFile.close();
    result.recordingTime = millis() - startTime;
    result.duration = result.recordingTime / 1000;
    result.success = (result.frameCount > 0);

    Serial.printf("Video recording completed: %d frames, %d seconds, %d bytes\n", 
                 result.frameCount, result.duration, result.fileSize);

    return result;
}

CameraManager::TimeLapseResult CameraManager::startTimeLapse(const TimeLapseConfig& config) {
    currentTimeLapseResult = {};
    currentTimeLapseResult.success = false;

    if (!initialized) {
        Serial.println("Camera not initialized");
        return currentTimeLapseResult;
    }

    if (timeLapseActive) {
        Serial.println("Time-lapse already active");
        return currentTimeLapseResult;
    }

    // Store configuration
    currentTimeLapseConfig = config;
    
    // Ensure directory exists
    if (!ensureDirectory(config.folder)) {
        Serial.printf("Failed to create time-lapse directory: %s\n", config.folder.c_str());
        return currentTimeLapseResult;
    }

    // Generate sequence ID
    currentTimeLapseResult.sequenceId = "TL_" + String(millis());
    currentTimeLapseResult.folder = config.folder;
    currentTimeLapseResult.success = true;
    
    timeLapseActive = true;
    lastTimeLapseCapture = millis();

    Serial.printf("Time-lapse started: %d second intervals, folder: %s\n", 
                 config.intervalSeconds, config.folder.c_str());

    return currentTimeLapseResult;
}

CameraManager::TimeLapseResult CameraManager::stopTimeLapse() {
    if (!timeLapseActive) {
        Serial.println("No active time-lapse to stop");
        return currentTimeLapseResult;
    }

    timeLapseActive = false;
    currentTimeLapseResult.totalDuration = (millis() - lastTimeLapseCapture) / 1000;

    Serial.printf("Time-lapse stopped: %d images captured over %d seconds\n", 
                 currentTimeLapseResult.capturedImages, currentTimeLapseResult.totalDuration);

    return currentTimeLapseResult;
}

CameraManager::TimeLapseResult CameraManager::updateTimeLapse() {
    if (!timeLapseActive || !initialized) {
        return currentTimeLapseResult;
    }

    uint32_t currentTime = millis();
    uint32_t elapsed = currentTime - lastTimeLapseCapture;
    
    // Check if it's time for next capture
    if (elapsed >= (currentTimeLapseConfig.intervalSeconds * 1000)) {
        // Check if we've reached maximum images
        if (currentTimeLapseResult.capturedImages >= currentTimeLapseConfig.maxImages) {
            Serial.println("Time-lapse reached maximum images, stopping");
            return stopTimeLapse();
        }

        // Capture image
        String filename = currentTimeLapseConfig.folder + "/" + currentTimeLapseResult.sequenceId + 
                         "_" + String(currentTimeLapseResult.capturedImages + 1, DEC) + ".jpg";
        
        CaptureResult capture = captureImage(currentTimeLapseConfig.folder);
        if (capture.success) {
            currentTimeLapseResult.capturedImages++;
            currentTimeLapseResult.totalSize += capture.imageSize;
            lastTimeLapseCapture = currentTime;
            
            Serial.printf("Time-lapse image %d captured: %s\n", 
                         currentTimeLapseResult.capturedImages, capture.filename.c_str());
        } else {
            Serial.printf("Time-lapse capture %d failed\n", currentTimeLapseResult.capturedImages + 1);
        }

        // Auto-cleanup if enabled
        if (currentTimeLapseConfig.autoCleanup && 
            currentTimeLapseResult.capturedImages > currentTimeLapseConfig.maxImages * 0.9) {
            // Clean up oldest images when approaching limit
            Serial.println("Time-lapse auto-cleanup triggered");
        }
    }

    return currentTimeLapseResult;
}

String CameraManager::generateFilename(const String& folder, const String& extension) {
    // Get current time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Generate filename with timestamp and counter
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/IMG_%04d%02d%02d_%02d%02d%02d_%04d%s",
             folder.c_str(),
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec,
             captureCounter++,
             extension.c_str());

    return String(filename);
}

bool CameraManager::ensureDirectory(const String& path) {
    if (SD_MMC.exists(path.c_str())) {
        return true;
    }

    return SD_MMC.mkdir(path.c_str());
}

void CameraManager::updateStats(bool success, uint32_t captureTime, size_t imageSize) {
    stats.totalCaptures++;
    
    if (success) {
        stats.successfulCaptures++;
        
        // Update running averages
        stats.averageCaptureTime = ((stats.averageCaptureTime * (stats.successfulCaptures - 1)) + captureTime) / stats.successfulCaptures;
        stats.averageImageSize = ((stats.averageImageSize * (stats.successfulCaptures - 1)) + imageSize) / stats.successfulCaptures;
    } else {
        stats.failedCaptures++;
    }
    
    stats.successRate = (float)stats.successfulCaptures / stats.totalCaptures;
}

camera_config_t CameraManager::getCameraConfig() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = CAMERA_PIXEL_FORMAT;
    config.frame_size = CAMERA_FRAME_SIZE;
    config.jpeg_quality = CAMERA_JPEG_QUALITY;
    config.fb_count = CAMERA_FB_COUNT;
    config.grab_mode = CAMERA_GRAB_MODE;

    return config;
}

void CameraManager::setIRLED(bool enable) {
    if (IR_LED_ENABLED) {
        irLedEnabled = enable;
        pinMode(IR_LED_PIN, OUTPUT);
        digitalWrite(IR_LED_PIN, enable ? HIGH : LOW);
        Serial.printf("IR LED %s\n", enable ? "enabled" : "disabled");
    }
}

uint8_t CameraManager::getLightLevel() const {
    if (!IR_LED_ENABLED) {
        return 50; // Default medium level
    }
    
    // Read light sensor (ADC)
    int rawValue = analogRead(LIGHT_SENSOR_PIN);
    // Convert 12-bit ADC (0-4095) to percentage (0-100)
    return map(rawValue, 0, 4095, 0, 100);
}

void CameraManager::autoAdjustNightMode() {
    if (!AUTO_NIGHT_MODE_ENABLED) {
        return;
    }
    
    uint8_t lightLevel = getLightLevel();
    bool shouldEnableNightMode = lightLevel < 30; // Night mode below 30%
    bool shouldEnableIR = lightLevel < 20;        // IR LED below 20%
    
    if (shouldEnableNightMode != nightModeEnabled) {
        setNightMode(shouldEnableNightMode);
        Serial.printf("Auto night mode: %s (light level: %d%%)\n", 
                     shouldEnableNightMode ? "ON" : "OFF", lightLevel);
    }
    
    if (shouldEnableIR != irLedEnabled) {
        setIRLED(shouldEnableIR);
        Serial.printf("Auto IR LED: %s (light level: %d%%)\n", 
                     shouldEnableIR ? "ON" : "OFF", lightLevel);
    }
}

String CameraManager::generateBurstSequenceId() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char sequenceId[30];
    snprintf(sequenceId, sizeof(sequenceId), "BURST_%04d%02d%02d_%02d%02d%02d",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
    
    return String(sequenceId);
}

bool CameraManager::saveBurstMetadata(const BurstResult& result, const String& folder) {
    String metadataFile = folder + "/" + result.sequenceId + "_metadata.json";
    
    // Create JSON document
    DynamicJsonDocument doc(1024);
    doc["sequenceId"] = result.sequenceId;
    doc["capturedCount"] = result.capturedCount;
    doc["totalTime"] = result.totalTime;
    doc["totalSize"] = result.totalSize;
    doc["timestamp"] = millis();
    
    JsonArray files = doc.createNestedArray("files");
    for (uint8_t i = 0; i < result.capturedCount; i++) {
        files.add(result.filenames[i]);
    }
    
    // Save to file
    File file = SD_MMC.open(metadataFile.c_str(), FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create metadata file: %s\n", metadataFile.c_str());
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.printf("Burst metadata saved: %s\n", metadataFile.c_str());
    return true;
}

void CameraManager::configureAdvancedGPIOs() {
    if (IR_LED_ENABLED) {
        pinMode(IR_LED_PIN, OUTPUT);
        digitalWrite(IR_LED_PIN, LOW); // Start with IR LED off
        Serial.printf("IR LED pin configured: GPIO%d\n", IR_LED_PIN);
    }
    
    if (IR_LED_ENABLED) {
        pinMode(LIGHT_SENSOR_PIN, INPUT);
        Serial.printf("Light sensor pin configured: GPIO%d\n", LIGHT_SENSOR_PIN);
    }
}

String CameraManager::generateVideoFilename(const String& folder) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/VID_%04d%02d%02d_%02d%02d%02d.mjpg",
             folder.c_str(),
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
    
    return String(filename);
}

bool CameraManager::initVideoEncoder(const VideoConfig& config) {
    // For MJPEG, we use the existing JPEG encoder
    // Additional video encoder initialization would go here
    Serial.printf("Video encoder initialized: MJPEG, %dfps, quality %d\n", 
                 config.frameRate, config.quality);
    return true;
}

bool CameraManager::writeVideoFrame(camera_fb_t* fb, File& file) {
    if (fb == nullptr || fb->len == 0) {
        return false;
    }
    
    // Write frame size (4 bytes)
    uint32_t frameSize = fb->len;
    file.write((uint8_t*)&frameSize, 4);
    
    // Write timestamp (4 bytes)
    uint32_t timestamp = millis();
    file.write((uint8_t*)&timestamp, 4);
    
    // Write frame data
    size_t written = file.write(fb->buf, fb->len);
    
    return (written == fb->len);
}