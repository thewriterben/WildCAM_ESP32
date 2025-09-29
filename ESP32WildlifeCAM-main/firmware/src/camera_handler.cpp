/**
 * Camera Handler Module
 * 
 * Manages ESP32 camera initialization, configuration, and image capture
 * for the wildlife monitoring system using Hardware Abstraction Layer (HAL).
 */

#include "camera_handler.h"
#include "config.h"
#include "hal/board_detector.h"
#include "hal/esp32_cam.h"
#include "configs/sensor_configs.h"
#include <esp_camera.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Constructor
CameraHandler::CameraHandler() 
    : initialized(false), imageCounter(0), frame_queue(nullptr) {
    // Initialize camera_config to default values
    memset(&camera_config, 0, sizeof(camera_config_t));
    applyConfigurationSettings();
    
    // Create frame queue for buffer management
    frame_queue = xQueueCreate(3, sizeof(camera_fb_t*));
}

// Destructor
CameraHandler::~CameraHandler() {
    cleanup();
}

/**
 * Initialize camera with automatic board detection
 */
bool CameraHandler::init() {
    DEBUG_PRINTLN("Initializing camera with automatic board detection...");
    
    // Create board instance using detection
    board = BoardDetector::createBoard();
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return false;
    }
    
    DEBUG_PRINTF("Detected board: %s\n", board->getBoardName());
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return false;
    }
    
    return initializeCamera();
}

/**
 * Initialize camera with specific board type
 */
bool CameraHandler::init(BoardType boardType) {
    DEBUG_PRINTF("Initializing camera with specific board type: %s\n", 
                BoardDetector::getBoardName(boardType));
    
    // Create board instance for specific type
    board = BoardDetector::createBoard(boardType);
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return false;
    }
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return false;
    }
    
    return initializeCamera();
}

/**
 * Get the current camera board instance
 */
CameraBoard* CameraHandler::getBoard() {
    return board.get();
}

/**
 * Internal function to initialize camera with current board configuration
 */
bool CameraHandler::initializeCamera() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Configuring camera with board-specific settings...");
    
    // Get board-specific configuration
    GPIOMap gpio_map = board->getGPIOMap();
    CameraConfig cam_config = board->getCameraConfig();
    
    // Configure camera_config with board-specific pins
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.pin_d0 = gpio_map.y2_pin;
    camera_config.pin_d1 = gpio_map.y3_pin;
    camera_config.pin_d2 = gpio_map.y4_pin;
    camera_config.pin_d3 = gpio_map.y5_pin;
    camera_config.pin_d4 = gpio_map.y6_pin;
    camera_config.pin_d5 = gpio_map.y7_pin;
    camera_config.pin_d6 = gpio_map.y8_pin;
    camera_config.pin_d7 = gpio_map.y9_pin;
    camera_config.pin_xclk = gpio_map.xclk_pin;
    camera_config.pin_pclk = gpio_map.pclk_pin;
    camera_config.pin_vsync = gpio_map.vsync_pin;
    camera_config.pin_href = gpio_map.href_pin;
    camera_config.pin_sscb_sda = gpio_map.siod_pin;
    camera_config.pin_sscb_scl = gpio_map.sioc_pin;
    camera_config.pin_pwdn = gpio_map.pwdn_pin;
    camera_config.pin_reset = gpio_map.reset_pin;
    camera_config.xclk_freq_hz = cam_config.xclk_freq_hz;
    camera_config.pixel_format = CAMERA_PIXEL_FORMAT;
    
    // Frame size and quality settings based on board capabilities
    if (board->hasPSRAM() && !cam_config.psram_required) {
        camera_config.frame_size = CAMERA_FRAME_SIZE;
        camera_config.jpeg_quality = CAMERA_JPEG_QUALITY;
        camera_config.fb_count = CAMERA_FB_COUNT;
        DEBUG_PRINTLN("PSRAM found - using high quality settings");
    } else if (board->hasPSRAM() && cam_config.psram_required) {
        camera_config.frame_size = CAMERA_FRAME_SIZE;
        camera_config.jpeg_quality = CAMERA_JPEG_QUALITY;
        camera_config.fb_count = CAMERA_FB_COUNT;
        DEBUG_PRINTLN("PSRAM required and found - using optimal settings");
    } else if (!board->hasPSRAM() && cam_config.psram_required) {
        DEBUG_PRINTLN("PSRAM required but not found - using conservative settings");
        camera_config.frame_size = FRAMESIZE_SVGA;
        camera_config.jpeg_quality = 15;
        camera_config.fb_count = 1;
    } else {
        camera_config.frame_size = FRAMESIZE_SVGA;
        camera_config.jpeg_quality = 15;
        camera_config.fb_count = 1;
        DEBUG_PRINTLN("PSRAM not found - using conservative settings");
    }
    
    camera_config.grab_mode = CAMERA_GRAB_MODE;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor
    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor == NULL) {
        DEBUG_PRINTLN("Failed to get camera sensor");
        return false;
    }
    
    // Configure sensor settings using board-specific implementation
    if (!board->configureSensor(sensor)) {
        DEBUG_PRINTLN("Failed to configure sensor");
        return false;
    }
    
    // Apply additional configuration settings
    configureSensorSettings(sensor);
    
    initialized = true;
    DEBUG_PRINTLN("Camera initialized successfully");
    
    return true;
}

/**
 * Configure camera sensor settings optimized for wildlife detection
 */
void CameraHandler::configureSensorSettings(sensor_t* sensor) {
    DEBUG_PRINTLN("Configuring camera sensor settings...");
    
    // Apply configuration values from config.h
    sensor->set_brightness(sensor, CAMERA_BRIGHTNESS_DEFAULT);
    sensor->set_contrast(sensor, CAMERA_CONTRAST_DEFAULT);
    sensor->set_saturation(sensor, CAMERA_SATURATION_DEFAULT);
    sensor->set_special_effect(sensor, 0); // No effects for wildlife
    sensor->set_whitebal(sensor, CAMERA_AWB_GAIN_DEFAULT);
    sensor->set_awb_gain(sensor, CAMERA_AWB_GAIN_DEFAULT);
    sensor->set_wb_mode(sensor, CAMERA_WB_MODE_DEFAULT);
    
    // Exposure and gain settings for outdoor conditions
    sensor->set_exposure_ctrl(sensor, AUTO_EXPOSURE_ENABLED ? 1 : 0);
    sensor->set_aec2(sensor, 0);           // Disable AEC2 for simpler control
    sensor->set_ae_level(sensor, CAMERA_AE_LEVEL_DEFAULT);
    sensor->set_aec_value(sensor, CAMERA_AEC_VALUE_DEFAULT);
    sensor->set_gain_ctrl(sensor, 1);      // Enable gain control
    sensor->set_agc_gain(sensor, CAMERA_AGC_GAIN_DEFAULT);
    sensor->set_gainceiling(sensor, CAMERA_GAIN_CEILING_DEFAULT);
    
    // Image enhancement for wildlife photography
    sensor->set_bpc(sensor, 0);            // Disable bad pixel correction
    sensor->set_wpc(sensor, 1);            // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);        // Enable gamma correction
    sensor->set_lenc(sensor, LENS_CORRECTION_ENABLED ? 1 : 0);
    
    // Motion detection optimizations
    sensor->set_hmirror(sensor, 0);        // No horizontal mirror
    sensor->set_vflip(sensor, 0);          // No vertical flip
    sensor->set_dcw(sensor, 1);            // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);       // Disable color bar test
    
    DEBUG_PRINTLN("Camera sensor configured for wildlife photography");
}

/**
 * Apply configuration settings from config.h
 */
void CameraHandler::applyConfigurationSettings() {
    // This method initializes camera_config with values from config.h
    camera_config.pixel_format = CAMERA_PIXEL_FORMAT;
    camera_config.frame_size = CAMERA_FRAME_SIZE;
    camera_config.jpeg_quality = CAMERA_JPEG_QUALITY;
    camera_config.fb_count = CAMERA_FB_COUNT;
    camera_config.grab_mode = CAMERA_GRAB_MODE;
}

/**
 * Capture a single image
 */
camera_fb_t* CameraHandler::captureImage() {
    if (!initialized) {
        DEBUG_PRINTLN("Error: Camera not initialized");
        return nullptr;
    }
    
    DEBUG_PRINTLN("Capturing image...");
    
    // Flash the LED briefly to indicate capture
    flashLED();
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        DEBUG_PRINTLN("Error: Camera capture failed");
        return nullptr;
    }
    
    DEBUG_PRINTF("Image captured: %dx%d, %d bytes, format: %d\n", 
                 fb->width, fb->height, fb->len, fb->format);
    
    imageCounter++;
    return fb;
}

/**
 * Save image to storage with timestamp and metadata
 */
String CameraHandler::saveImage(camera_fb_t* fb, const char* folder) {
    if (!fb) {
        DEBUG_PRINTLN("Error: No image buffer to save");
        return "";
    }
    
    // Create filename with timestamp
    String filename = generateFilename(folder);
    
    File file;
    
    #ifdef SD_CARD_ENABLED
    #if SD_CARD_ENABLED
    // Use SD card if enabled and available
    file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        DEBUG_PRINTF("Error: Failed to create SD file %s, trying LittleFS\n", filename.c_str());
        file = LittleFS.open(filename.c_str(), FILE_WRITE);
    }
    #else
    // Use LittleFS when SD card is disabled
    file = LittleFS.open(filename.c_str(), FILE_WRITE);
    #endif
    #else
    // Fallback to LittleFS if SD_CARD_ENABLED not defined
    file = LittleFS.open(filename.c_str(), FILE_WRITE);
    #endif
    
    if (!file) {
        DEBUG_PRINTF("Error: Failed to create file %s\n", filename.c_str());
        return "";
    }
    
    size_t written = file.write(fb->buf, fb->len);
    file.close();
    
    if (written != fb->len) {
        DEBUG_PRINTF("Error: Failed to write complete image (%d/%d bytes)\n", written, fb->len);
        return "";
    }
    
    // Save metadata
    if (IMAGE_TIMESTAMP_ENABLED) {
        saveImageMetadata(filename, fb);
    }
    
    DEBUG_PRINTF("Image saved successfully: %s (%d bytes)\n", filename.c_str(), written);
    return filename;
}

/**
 * Generate timestamped filename
 */
String CameraHandler::generateFilename(const char* folder) {
    struct tm timeinfo;
    char filename[100];
    
    if (getLocalTime(&timeinfo)) {
        snprintf(filename, sizeof(filename), "%s/" FILENAME_TIMESTAMP_FORMAT "_%04d.jpg",
                folder,
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec,
                imageCounter);
    } else {
        // Fallback if time is not available
        snprintf(filename, sizeof(filename), "%s/img_%08lu_%04d.jpg",
                folder, millis(), imageCounter);
    }
    
    return String(filename);
}

/**
 * Save image metadata as JSON
 */
void CameraHandler::saveImageMetadata(const String& imageFilename, camera_fb_t* fb) {
    String metaFilename = imageFilename;
    metaFilename.replace(".jpg", ".json");
    
    // Create metadata JSON
    DynamicJsonDocument doc(512);
    
    doc["timestamp"] = millis();
    doc["image_file"] = imageFilename;
    doc["width"] = fb->width;
    doc["height"] = fb->height;
    doc["size_bytes"] = fb->len;
    doc["format"] = fb->format;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["device_name"] = DEVICE_NAME;
    doc["node_id"] = NODE_ID;
    
    // Add time information if available
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        doc["datetime"] = timeStr;
    }
    
    // Save metadata file
    File metaFile = SD_MMC.open(metaFilename.c_str(), FILE_WRITE);
    if (metaFile) {
        serializeJson(doc, metaFile);
        metaFile.close();
        DEBUG_PRINTF("Metadata saved: %s\n", metaFilename.c_str());
    } else {
        DEBUG_PRINTF("Warning: Failed to save metadata %s\n", metaFilename.c_str());
    }
}

/**
 * Get camera status information
 */
CameraStatus CameraHandler::getStatus() const {
    CameraStatus status;
    status.initialized = initialized;
    status.imageCount = imageCounter;
    status.lastError = ESP_OK;  // Would need to track actual errors
    
    if (board) {
        status.boardType = board->getBoardType();
        status.sensorType = board->getSensorType();
        status.boardName = board->getBoardName();
        const SensorCapabilities* sensor_caps = getSensorCapabilities(board->getSensorType());
        status.sensorName = sensor_caps ? sensor_caps->name : "Unknown";
    } else {
        status.boardType = BOARD_UNKNOWN;
        status.sensorType = SENSOR_UNKNOWN;
        status.boardName = "Not Detected";
        status.sensorName = "Not Detected";
    }
    
    if (initialized) {
        sensor_t* sensor = esp_camera_sensor_get();
        if (sensor) {
            status.sensorDetected = true;
        }
    }
    
    return status;
}

/**
 * Take a test image and return basic info
 */
bool CameraHandler::testCamera() {
    DEBUG_PRINTLN("Testing camera...");
    
    camera_fb_t* fb = captureImage();
    if (!fb) {
        DEBUG_PRINTLN("Camera test failed - no image captured");
        return false;
    }
    
    DEBUG_PRINTF("Camera test successful: %dx%d, %d bytes\n", 
                 fb->width, fb->height, fb->len);
    
    esp_camera_fb_return(fb);
    return true;
}

/**
 * Adjust camera settings for different lighting conditions
 */
void CameraHandler::adjustForLighting(LightingCondition condition) {
    if (!initialized) return;
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) return;
    
    DEBUG_PRINTF("Adjusting camera for lighting condition: %d\n", condition);
    
    switch (condition) {
        case LIGHTING_BRIGHT:
            sensor->set_ae_level(sensor, -1);      // Reduce exposure
            sensor->set_agc_gain(sensor, 0);       // Low gain
            sensor->set_brightness(sensor, -1);    // Reduce brightness
            break;
            
        case LIGHTING_NORMAL:
            sensor->set_ae_level(sensor, 0);       // Normal exposure
            sensor->set_agc_gain(sensor, 5);       // Medium gain
            sensor->set_brightness(sensor, 0);     // Normal brightness
            break;
            
        case LIGHTING_DIM:
            sensor->set_ae_level(sensor, 1);       // Increase exposure
            sensor->set_agc_gain(sensor, 15);      // Higher gain
            sensor->set_brightness(sensor, 1);     // Increase brightness
            break;
            
        case LIGHTING_DARK:
            sensor->set_ae_level(sensor, 2);       // Maximum exposure
            sensor->set_agc_gain(sensor, 25);      // High gain
            sensor->set_brightness(sensor, 2);     // Maximum brightness
            break;
    }
}

/**
 * Flash camera LED briefly
 */
void CameraHandler::flashLED() {
    #ifdef CAMERA_LED_PIN
    pinMode(CAMERA_LED_PIN, OUTPUT);
    digitalWrite(CAMERA_LED_PIN, HIGH);
    delay(50);  // Brief flash duration from config
    digitalWrite(CAMERA_LED_PIN, LOW);
    #endif
}

/**
 * Cleanup camera resources
 */
void CameraHandler::cleanup() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
        imageCounter = 0;
        DEBUG_PRINTLN("Camera deinitialized");
    }
    
    // Clean up frame queue
    if (frame_queue != nullptr) {
        vQueueDelete(frame_queue);
        frame_queue = nullptr;
    }
}

/**
 * Capture image with environmental awareness
 */
camera_fb_t* CameraHandler::captureImageWithEnvironmentalAwareness() {
    if (!initialized) {
        DEBUG_PRINTLN("Camera not initialized");
        return nullptr;
    }
    
    // Check environmental conditions first
    if (!areEnvironmentalConditionsSuitable()) {
        DEBUG_PRINTLN("Environmental conditions not suitable for photography");
        return nullptr;
    }
    
    // Adjust camera settings based on environmental conditions
    adjustForEnvironmentalConditions();
    
    // Capture the image
    return captureImage();
}

/**
 * Initialize camera with user-provided configuration
 */
esp_err_t CameraHandler::initialize(const CameraConfig& user_config) {
    DEBUG_PRINTLN("Initializing camera with user configuration...");
    
    if (initialized) {
        return ESP_OK;  // Already initialized
    }
    
    // Create board instance using detection
    board = BoardDetector::createBoard();
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return ESP_FAIL;
    }
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return ESP_FAIL;
    }
    
    // Apply user configuration
    camera_config.frame_size = user_config.max_framesize;
    camera_config.pixel_format = user_config.pixel_format;
    camera_config.jpeg_quality = user_config.jpeg_quality;
    camera_config.fb_count = user_config.fb_count;
    camera_config.xclk_freq_hz = user_config.xclk_freq_hz;
    
    return initializeCamera() ? ESP_OK : ESP_FAIL;
}

/**
 * Validate pin assignment for conflict-free operation
 */
bool CameraHandler::validatePinAssignment() {
    if (!board) {
        DEBUG_PRINTLN("Board not initialized");
        return false;
    }
    
    GPIOMap gpio_map = board->getGPIOMap();
    
    // Use BoardDetector validation functionality
    return BoardDetector::validateGPIOConfiguration(gpio_map);
}

/**
 * Initialize camera with conflict checking
 */
esp_err_t CameraHandler::initializeWithConflictCheck() {
    DEBUG_PRINTLN("Initializing camera with conflict checking...");
    
    if (initialized) {
        return ESP_OK;  // Already initialized
    }
    
    // Create board instance using detection
    board = BoardDetector::createBoard();
    if (!board) {
        DEBUG_PRINTLN("Failed to create board instance");
        return ESP_FAIL;
    }
    
    // Validate pin assignments before proceeding
    if (!validatePinAssignment()) {
        DEBUG_PRINTLN("Pin validation failed - conflicts detected");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Initialize the specific board
    if (!board->init()) {
        DEBUG_PRINTLN("Board initialization failed");
        return ESP_FAIL;
    }
    
    return initializeCamera() ? ESP_OK : ESP_FAIL;
}

/**
 * Capture frame with timeout
 */
esp_err_t CameraHandler::captureFrame(uint32_t timeout_ms) {
    if (!initialized) {
        DEBUG_PRINTLN("Error: Camera not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    DEBUG_PRINTF("Capturing frame with %lu ms timeout...\n", timeout_ms);
    
    // Flash the LED briefly to indicate capture
    flashLED();
    
    // Capture image with timeout handling
    uint32_t start_time = millis();
    camera_fb_t* fb = esp_camera_fb_get();
    uint32_t elapsed = millis() - start_time;
    
    if (!fb) {
        DEBUG_PRINTLN("Error: Camera capture failed");
        return ESP_FAIL;
    }
    
    if (elapsed > timeout_ms) {
        DEBUG_PRINTF("Warning: Capture took %lu ms (timeout was %lu ms)\n", elapsed, timeout_ms);
        esp_camera_fb_return(fb);
        return ESP_ERR_TIMEOUT;
    }
    
    // Add frame buffer to queue if space available
    if (frame_queue != nullptr) {
        if (xQueueSend(frame_queue, &fb, 0) != pdTRUE) {
            DEBUG_PRINTLN("Frame queue full - returning buffer immediately");
            esp_camera_fb_return(fb);
            return ESP_ERR_NO_MEM;
        }
    } else {
        // No queue, return buffer immediately
        esp_camera_fb_return(fb);
    }
    
    DEBUG_PRINTF("Frame captured: %dx%d, %d bytes, format: %d\n", 
                 fb->width, fb->height, fb->len, fb->format);
    
    imageCounter++;
    return ESP_OK;
}

/**
 * Get frame buffer from internal queue
 */
camera_fb_t* CameraHandler::getFrameBuffer() {
    if (frame_queue == nullptr) {
        DEBUG_PRINTLN("Frame queue not initialized");
        return nullptr;
    }
    
    camera_fb_t* fb = nullptr;
    if (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
        return fb;
    }
    
    return nullptr;
}

/**
 * Return frame buffer to system
 */
void CameraHandler::returnFrameBuffer(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

/**
 * Deinitialize camera and clean up resources
 */
esp_err_t CameraHandler::deinitialize() {
    DEBUG_PRINTLN("Deinitializing camera...");
    
    // Clean up any pending frame buffers in queue
    if (frame_queue != nullptr) {
        camera_fb_t* fb;
        while (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
            esp_camera_fb_return(fb);
        }
    }
    
    cleanup();
    return ESP_OK;
}

/**
 * Adjust camera settings based on environmental conditions
 */
bool CameraHandler::adjustForEnvironmentalConditions() {
    // This requires environmental integration
    #ifdef __has_include
    #if __has_include("../sensors/environmental_integration.cpp")
    extern AdvancedEnvironmentalData getLatestEnvironmentalData();
    
    AdvancedEnvironmentalData env_data = getLatestEnvironmentalData();
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        return false;
    }
    
    // Adjust exposure based on light conditions
    if (env_data.visible_light < 50.0) {
        // Low light - increase exposure
        sensor->set_aec_value(sensor, 600);
        sensor->set_gain_ctrl(sensor, 1);
        DEBUG_PRINTLN("Adjusted for low light conditions");
    } else if (env_data.visible_light > 1000.0) {
        // Bright light - reduce exposure
        sensor->set_aec_value(sensor, 200);
        sensor->set_gain_ctrl(sensor, 0);
        DEBUG_PRINTLN("Adjusted for bright light conditions");
    }
    
    // Adjust for humidity (condensation protection)
    if (env_data.humidity > 90.0) {
        // High humidity - use faster settings to minimize condensation exposure
        sensor->set_aec_value(sensor, 300);
        DEBUG_PRINTLN("Adjusted for high humidity");
    }
    
    return true;
    #endif
    #endif
    
    return false;
}

/**
 * Check if environmental conditions are suitable for photography
 */
bool CameraHandler::areEnvironmentalConditionsSuitable() {
    // This requires environmental integration
    #ifdef __has_include
    #if __has_include("../sensors/environmental_integration.cpp")
    extern bool environmentallyAwareCameraCapture();
    extern AdvancedEnvironmentalData getLatestEnvironmentalData();
    
    AdvancedEnvironmentalData env_data = getLatestEnvironmentalData();
    
    // Check photography conditions score
    if (env_data.photography_conditions < 40) {
        DEBUG_PRINTF("Photography conditions too poor: %d%%\n", env_data.photography_conditions);
        return false;
    }
    
    // Check for condensation risk
    if (env_data.humidity > 95.0) {
        DEBUG_PRINTLN("High condensation risk - postponing capture");
        return false;
    }
    
    // Check for extreme temperatures
    if (env_data.temperature < -10.0 || env_data.temperature > 50.0) {
        DEBUG_PRINTLN("Extreme temperature - postponing capture");
        return false;
    }
    
    // Check for sufficient light
    if (env_data.visible_light < 5.0) {
        DEBUG_PRINTLN("Insufficient light for photography");
        return false;
    }
    
    return true;
    #endif
    #endif
    
    // Fallback - always allow capture if environmental system not available
    return true;
}