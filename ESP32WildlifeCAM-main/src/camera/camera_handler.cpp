/**
 * @file camera_handler.cpp
 * @brief Implementation of Camera Driver for Step 1.4
 * @author ESP32WildlifeCAM Project
 * @date 2025-08-31
 * 
 * This file implements the critical Camera Driver system as outlined in the
 * Unified Development Plan Step 1.4. Provides conflict-aware initialization,
 * PSRAM optimization, and proper frame buffer management.
 */

#include "camera_handler.h"
#include "camera_utils.cpp"
#include "../../firmware/src/hal/board_detector.h"
#include "../../firmware/src/hal/camera_board.h"
#include <esp_system.h>
#include <memory>

// Constructor
CameraHandler::CameraHandler() : 
    initialized(false), 
    board_instance(nullptr),
    frame_queue(nullptr),
    last_capture_time(0) {
    
    // Initialize configuration to default values
    memset(&config, 0, sizeof(camera_config_t));
    memset(&stats, 0, sizeof(CaptureStats));
    memset(&init_result, 0, sizeof(CameraInitResult));
    
    // Create frame queue (size 3 for triple buffering)
    frame_queue = xQueueCreate(3, sizeof(camera_fb_t*));
    
    Serial.println("CameraHandler: Constructor initialized");
}

// Destructor
CameraHandler::~CameraHandler() {
    deinitialize();
    
    if (frame_queue != nullptr) {
        vQueueDelete(frame_queue);
        frame_queue = nullptr;
    }
    
    Serial.println("CameraHandler: Destructor completed");
}

/**
 * @brief Maximum number of initialization retry attempts
 */
static const uint8_t MAX_INIT_RETRIES = 3;

/**
 * @brief Delay between retry attempts in milliseconds
 */
static const uint32_t RETRY_DELAY_MS = 500;

esp_err_t CameraHandler::initialize(const CameraConfig& user_config) {
    Serial.println("CameraHandler: Initializing with user configuration...");
    uint32_t start_time = millis();
    
    if (initialized) {
        Serial.println("CameraHandler: Already initialized");
        return ESP_OK;
    }
    
    // Validate configuration
    if (!CameraUtils::validateCameraConfig(user_config, "Unknown")) {
        Serial.println("CameraHandler: Configuration validation failed");
        init_result.error_code = ESP_ERR_INVALID_ARG;
        init_result.error_message = "Invalid configuration parameters";
        return ESP_ERR_INVALID_ARG;
    }
    
    // Detect hardware capabilities
    if (!detectHardwareCapabilities()) {
        Serial.println("CameraHandler: Hardware detection failed");
        init_result.error_code = ESP_FAIL;
        init_result.error_message = "Hardware detection failed";
        return ESP_FAIL;
    }
    
    // Apply configuration to camera_config_t
    config.pin_d0 = -1;      // Will be set by setupPins()
    config.pin_d1 = -1;
    config.pin_d2 = -1;
    config.pin_d3 = -1;
    config.pin_d4 = -1;
    config.pin_d5 = -1;
    config.pin_d6 = -1;
    config.pin_d7 = -1;
    config.pin_xclk = -1;
    config.pin_pclk = -1;
    config.pin_vsync = -1;
    config.pin_href = -1;
    config.pin_sscb_sda = -1;
    config.pin_sscb_scl = -1;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = user_config.xclk_freq_hz;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pixel_format = user_config.pixel_format;
    config.frame_size = user_config.max_framesize;
    config.jpeg_quality = user_config.jpeg_quality;
    config.fb_count = user_config.fb_count;
    config.grab_mode = user_config.grab_mode;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    
    // Setup pins using board detection
    esp_err_t pin_result = setupPins();
    if (pin_result != ESP_OK) {
        Serial.printf("CameraHandler: Pin setup failed: 0x%x\n", pin_result);
        init_result.error_code = pin_result;
        init_result.error_message = "Pin configuration failed";
        return pin_result;
    }
    
    // Initialize ESP camera with retry logic for edge cases
    esp_err_t err = ESP_FAIL;
    uint8_t retry_count = 0;
    
    while (retry_count < MAX_INIT_RETRIES && err != ESP_OK) {
        if (retry_count > 0) {
            Serial.printf("CameraHandler: Retry attempt %d/%d after %lu ms delay\n", 
                         retry_count, MAX_INIT_RETRIES - 1, RETRY_DELAY_MS);
            delay(RETRY_DELAY_MS);
            
            // Progressive recovery: adjust clock frequency on retries
            if (retry_count == 1 && config.xclk_freq_hz > 16000000) {
                Serial.println("CameraHandler: Reducing XCLK frequency for compatibility");
                config.xclk_freq_hz = 16000000; // Try lower clock
            } else if (retry_count == 2 && config.xclk_freq_hz > 10000000) {
                Serial.println("CameraHandler: Reducing XCLK to minimum for stability");
                config.xclk_freq_hz = 10000000; // Minimum stable clock
            }
        }
        
        err = esp_camera_init(&config);
        
        if (err != ESP_OK) {
            Serial.printf("CameraHandler: Camera init attempt %d failed: 0x%x\n", 
                         retry_count + 1, err);
            
            // Ensure deinit before retry (clean state)
            if (retry_count < MAX_INIT_RETRIES - 1) {
                esp_camera_deinit();
            }
        }
        
        retry_count++;
    }
    
    if (err != ESP_OK) {
        Serial.printf("CameraHandler: Camera init failed after %d attempts: 0x%x\n", 
                     MAX_INIT_RETRIES, err);
        init_result.error_code = err;
        init_result.error_message = "ESP camera initialization failed after retries";
        CameraUtils::diagnoseCameraIssues(err);
        return err;
    }
    
    // Configure sensor with error recovery
    esp_err_t sensor_result = configureSensor();
    if (sensor_result != ESP_OK) {
        // Try sensor configuration once more after brief delay
        Serial.println("CameraHandler: Sensor configuration failed, retrying...");
        delay(100);
        sensor_result = configureSensor();
        
        if (sensor_result != ESP_OK) {
            Serial.printf("CameraHandler: Sensor configuration failed: 0x%x\n", sensor_result);
            esp_camera_deinit();
            init_result.error_code = sensor_result;
            init_result.error_message = "Sensor configuration failed";
            return sensor_result;
        }
    }
    
    initialized = true;
    
    // Record successful initialization
    init_result.error_code = ESP_OK;
    init_result.initialized = true;
    init_result.error_message = "Successfully initialized";
    init_result.init_time_ms = millis() - start_time;
    
    Serial.printf("CameraHandler: Initialization successful (%lu ms)\n", init_result.init_time_ms);
    if (retry_count > 1) {
        Serial.printf("CameraHandler: Note - initialization required %d attempt(s)\n", retry_count);
    }
    CameraUtils::logCameraConfig(user_config);
    
    return ESP_OK;
}

/**
 * @brief Maximum number of capture retry attempts
 */
static const uint8_t MAX_CAPTURE_RETRIES = 2;

esp_err_t CameraHandler::captureFrame(uint32_t timeout_ms) {
    if (!initialized) {
        Serial.println("CameraHandler: Not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    uint32_t capture_start = millis();
    camera_fb_t* fb = nullptr;
    uint8_t retry_count = 0;
    
    // Capture frame with retry logic for edge cases (sensor timing issues)
    while (retry_count < MAX_CAPTURE_RETRIES && fb == nullptr) {
        if (retry_count > 0) {
            Serial.printf("CameraHandler: Capture retry attempt %d\n", retry_count);
            delay(50); // Brief delay before retry
        }
        
        fb = esp_camera_fb_get();
        
        if (!fb) {
            retry_count++;
            
            // On first failure, try to recover sensor state
            if (retry_count == 1) {
                sensor_t* sensor = esp_camera_sensor_get();
                if (sensor) {
                    // Attempt to reset exposure settings to recover from edge case
                    sensor->set_exposure_ctrl(sensor, 1);
                    sensor->set_gain_ctrl(sensor, 1);
                }
            }
        }
    }
    
    if (!fb) {
        Serial.printf("CameraHandler: Frame capture failed after %d attempts\n", MAX_CAPTURE_RETRIES);
        updateCaptureStats(millis() - capture_start, 0, false);
        return ESP_FAIL;
    }
    
    uint32_t capture_time = millis() - capture_start;
    
    // Check timeout
    if (capture_time > timeout_ms) {
        Serial.printf("CameraHandler: Capture timeout (%lu ms > %lu ms)\n", capture_time, timeout_ms);
        esp_camera_fb_return(fb);
        updateCaptureStats(capture_time, 0, false);
        return ESP_ERR_TIMEOUT;
    }
    
    // Validate frame buffer integrity (edge case protection)
    if (fb->len == 0 || fb->buf == nullptr) {
        Serial.println("CameraHandler: Invalid frame buffer received");
        esp_camera_fb_return(fb);
        updateCaptureStats(capture_time, 0, false);
        return ESP_ERR_INVALID_RESPONSE;
    }
    
    // Add to queue if space available
    if (xQueueSend(frame_queue, &fb, 0) != pdTRUE) {
        Serial.println("CameraHandler: Frame queue full, dropping frame");
        esp_camera_fb_return(fb);
        updateCaptureStats(capture_time, fb->len, false);
        return ESP_ERR_NO_MEM;
    }
    
    updateCaptureStats(capture_time, fb->len, true);
    if (retry_count > 0) {
        Serial.printf("CameraHandler: Frame captured after %d retry(s) (%dx%d, %d bytes, %lu ms)\n", 
                      retry_count, fb->width, fb->height, fb->len, capture_time);
    } else {
        Serial.printf("CameraHandler: Frame captured (%dx%d, %d bytes, %lu ms)\n", 
                      fb->width, fb->height, fb->len, capture_time);
    }
    
    return ESP_OK;
}

camera_fb_t* CameraHandler::getFrameBuffer() {
    if (frame_queue == nullptr) {
        Serial.println("CameraHandler: Frame queue not initialized");
        return nullptr;
    }
    
    camera_fb_t* fb = nullptr;
    if (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
        return fb;
    }
    
    return nullptr;
}

void CameraHandler::returnFrameBuffer(camera_fb_t* fb) {
    if (fb != nullptr) {
        esp_camera_fb_return(fb);
    }
}

esp_err_t CameraHandler::deinitialize() {
    Serial.println("CameraHandler: Deinitializing...");
    
    if (!initialized) {
        return ESP_OK;
    }
    
    // Return any pending frame buffers
    if (frame_queue != nullptr) {
        camera_fb_t* fb;
        while (xQueueReceive(frame_queue, &fb, 0) == pdTRUE) {
            esp_camera_fb_return(fb);
        }
    }
    
    // Deinitialize camera
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        Serial.printf("CameraHandler: Deinit failed: 0x%x\n", err);
        return err;
    }
    
    initialized = false;
    Serial.println("CameraHandler: Deinitialization complete");
    
    return ESP_OK;
}

bool CameraHandler::validatePinAssignment() {
    Serial.println("CameraHandler: Validating pin assignments...");
    
    // Create board instance for validation
    std::unique_ptr<CameraBoard> board = BoardDetector::createBoard();
    if (!board) {
        Serial.println("CameraHandler: Failed to create board for validation");
        return false;
    }
    
    GPIOMap gpio_map = board->getGPIOMap();
    return BoardDetector::validateGPIOConfiguration(gpio_map);
}

esp_err_t CameraHandler::initializeWithConflictCheck() {
    Serial.println("CameraHandler: Initializing with conflict checking...");
    
    if (!validatePinAssignment()) {
        Serial.println("CameraHandler: Pin validation failed");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Get recommended configuration based on hardware
    CameraConfig recommended_config = CameraUtils::getRecommendedConfig();
    return initialize(recommended_config);
}

CameraInitResult CameraHandler::getInitializationResult() const {
    return init_result;
}

CaptureStats CameraHandler::getCaptureStats() const {
    return stats;
}

bool CameraHandler::testCamera() {
    Serial.println("CameraHandler: Testing camera functionality...");
    
    if (!initialized) {
        Serial.println("CameraHandler: Camera not initialized for test");
        return false;
    }
    
    // Capture a test frame
    esp_err_t result = captureFrame(5000); // 5 second timeout
    if (result != ESP_OK) {
        Serial.printf("CameraHandler: Test capture failed: 0x%x\n", result);
        return false;
    }
    
    // Get and immediately return the frame
    camera_fb_t* fb = getFrameBuffer();
    if (!fb) {
        Serial.println("CameraHandler: Test frame buffer retrieval failed");
        return false;
    }
    
    Serial.printf("CameraHandler: Test successful - %dx%d, %d bytes\n", 
                  fb->width, fb->height, fb->len);
    
    returnFrameBuffer(fb);
    return true;
}

// Private methods

esp_err_t CameraHandler::setupPins() {
    Serial.println("CameraHandler: Setting up pins...");
    
    // Create board instance
    std::unique_ptr<CameraBoard> board = BoardDetector::createBoard();
    if (!board) {
        Serial.println("CameraHandler: Failed to create board instance");
        return ESP_FAIL;
    }
    
    // Store board reference for later use (note: unique_ptr manages the lifetime)
    
    // Get GPIO mapping
    GPIOMap gpio_map = board->getGPIOMap();
    
    // Apply pin configuration
    config.pin_d0 = gpio_map.y2_pin;
    config.pin_d1 = gpio_map.y3_pin;
    config.pin_d2 = gpio_map.y4_pin;
    config.pin_d3 = gpio_map.y5_pin;
    config.pin_d4 = gpio_map.y6_pin;
    config.pin_d5 = gpio_map.y7_pin;
    config.pin_d6 = gpio_map.y8_pin;
    config.pin_d7 = gpio_map.y9_pin;
    config.pin_xclk = gpio_map.xclk_pin;
    config.pin_pclk = gpio_map.pclk_pin;
    config.pin_vsync = gpio_map.vsync_pin;
    config.pin_href = gpio_map.href_pin;
    config.pin_sscb_sda = gpio_map.siod_pin;
    config.pin_sscb_scl = gpio_map.sioc_pin;
    config.pin_pwdn = gpio_map.pwdn_pin;
    config.pin_reset = gpio_map.reset_pin;
    
    Serial.println("CameraHandler: Pin setup complete");
    return ESP_OK;
}

esp_err_t CameraHandler::configureSensor() {
    Serial.println("CameraHandler: Configuring sensor...");
    
    sensor_t* sensor = esp_camera_sensor_get();
    if (!sensor) {
        Serial.println("CameraHandler: Failed to get sensor handle");
        return ESP_FAIL;
    }
    
    // Apply wildlife-optimized sensor settings
    sensor->set_brightness(sensor, 0);       // Normal brightness
    sensor->set_contrast(sensor, 0);         // Normal contrast
    sensor->set_saturation(sensor, 0);       // Normal saturation
    sensor->set_special_effect(sensor, 0);   // No special effects
    sensor->set_whitebal(sensor, 1);         // Enable white balance
    sensor->set_awb_gain(sensor, 1);         // Enable AWB gain
    sensor->set_wb_mode(sensor, 0);          // Auto white balance mode
    sensor->set_exposure_ctrl(sensor, 1);    // Enable exposure control
    sensor->set_aec2(sensor, 0);             // Disable AEC2
    sensor->set_ae_level(sensor, 0);         // Normal AE level
    sensor->set_aec_value(sensor, 300);      // AEC value
    sensor->set_gain_ctrl(sensor, 1);        // Enable gain control
    sensor->set_agc_gain(sensor, 0);         // AGC gain
    sensor->set_gainceiling(sensor, GAINCEILING_2X); // 2x gain ceiling
    sensor->set_bpc(sensor, 0);              // Disable black pixel correction
    sensor->set_wpc(sensor, 1);              // Enable white pixel correction
    sensor->set_raw_gma(sensor, 1);          // Enable gamma correction
    sensor->set_lenc(sensor, 1);             // Enable lens correction
    sensor->set_hmirror(sensor, 0);          // No horizontal mirror
    sensor->set_vflip(sensor, 0);            // No vertical flip
    sensor->set_dcw(sensor, 1);              // Enable downsize cropping
    sensor->set_colorbar(sensor, 0);         // Disable color bar
    
    Serial.println("CameraHandler: Sensor configuration complete");
    return ESP_OK;
}

bool CameraHandler::detectHardwareCapabilities() {
    Serial.println("CameraHandler: Detecting hardware capabilities...");
    
    // PSRAM detection
    init_result.psram_detected = psramFound();
    init_result.psram_size = init_result.psram_detected ? ESP.getPsramSize() : 0;
    
    Serial.printf("CameraHandler: PSRAM %s (%d bytes)\n", 
                  init_result.psram_detected ? "detected" : "not found",
                  init_result.psram_size);
    
    // Board detection
    BoardType board_type = BoardDetector::detectBoardType();
    init_result.board_name = BoardDetector::getBoardName(board_type);
    
    Serial.printf("CameraHandler: Board type: %s\n", init_result.board_name);
    
    // Note: Sensor detection will happen after esp_camera_init
    init_result.sensor_detected = false;
    init_result.sensor_name = "Unknown";
    
    return true;
}

void CameraHandler::updateCaptureStats(uint32_t capture_time, size_t image_size, bool success) {
    stats.total_captures++;
    
    if (success) {
        stats.successful_captures++;
        stats.total_data_captured += image_size;
        
        // Update timing statistics
        if (stats.successful_captures == 1) {
            stats.min_capture_time_ms = capture_time;
            stats.max_capture_time_ms = capture_time;
            stats.avg_capture_time_ms = capture_time;
            stats.avg_image_size = image_size;
        } else {
            if (capture_time < stats.min_capture_time_ms) {
                stats.min_capture_time_ms = capture_time;
            }
            if (capture_time > stats.max_capture_time_ms) {
                stats.max_capture_time_ms = capture_time;
            }
            
            // Running average
            stats.avg_capture_time_ms = 
                (stats.avg_capture_time_ms * (stats.successful_captures - 1) + capture_time) / 
                stats.successful_captures;
            
            stats.avg_image_size = 
                (stats.avg_image_size * (stats.successful_captures - 1) + image_size) / 
                stats.successful_captures;
        }
    } else {
        stats.failed_captures++;
    }
    
    stats.last_capture_time_ms = capture_time;
    last_capture_time = millis();
}

void CameraHandler::logDiagnosticInfo() const {
    Serial.println("=== CameraHandler Diagnostic Info ===");
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
    Serial.printf("Total captures: %lu\n", stats.total_captures);
    Serial.printf("Successful: %lu\n", stats.successful_captures);
    Serial.printf("Failed: %lu\n", stats.failed_captures);
    Serial.printf("Average capture time: %lu ms\n", stats.avg_capture_time_ms);
    Serial.printf("Average image size: %d bytes\n", stats.avg_image_size);
    Serial.println("=== End Diagnostic Info ===");
}