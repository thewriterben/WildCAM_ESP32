/**
 * @file jpeg_decode_example.ino
 * @brief Example demonstrating JPEG decoding and image resizing
 * 
 * This example shows how to:
 * 1. Capture an image with ESP32-CAM
 * 2. Decode JPEG to RGB data
 * 3. Resize image using bilinear interpolation
 * 4. Prepare image for AI model input
 */

#include <esp_camera.h>
#include "src/utils/image_utils.h"

// AI-Thinker ESP32-CAM pin definitions
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== JPEG Decode and Image Resize Example ===\n");
    
    // Initialize camera
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
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    
    // Initialize with PSRAM if available
    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return;
    }
    
    Serial.println("Camera initialized successfully!");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    if (psramFound()) {
        Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
    }
    
    delay(2000);  // Let camera warm up
}

void loop() {
    Serial.println("\n--- Capturing and Processing Image ---");
    
    // Example 1: Basic JPEG decode
    example1_basic_decode();
    
    delay(3000);
    
    // Example 2: JPEG decode + resize for thumbnail
    example2_create_thumbnail();
    
    delay(3000);
    
    // Example 3: Complete preprocessing for AI model
    example3_ai_preprocessing();
    
    delay(10000);  // Wait 10 seconds before next iteration
}

/**
 * Example 1: Basic JPEG Decoding
 * Demonstrates how to decode a JPEG image to RGB data
 */
void example1_basic_decode() {
    Serial.println("\nExample 1: Basic JPEG Decode");
    Serial.println("=============================");
    
    // Capture frame
    camera_fb_t* frame = esp_camera_fb_get();
    if (!frame) {
        Serial.println("ERROR: Failed to capture frame");
        return;
    }
    
    Serial.printf("Captured JPEG: %d bytes, format: %d\n", frame->len, frame->format);
    
    // Decode JPEG
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    uint32_t startTime = millis();
    bool success = ImageUtils::decodeJPEG(frame->buf, frame->len, &width, &height, &rgbData);
    uint32_t decodeTime = millis() - startTime;
    
    if (success && rgbData) {
        Serial.printf("✓ Decoded to %dx%d RGB (%d bytes) in %d ms\n", 
                     width, height, width * height * 3, decodeTime);
        
        // Sample a few pixels to verify
        Serial.print("Sample pixels (R,G,B): ");
        for (int i = 0; i < 3; i++) {
            size_t idx = (i * 100) * 3;  // Sample every 100 pixels
            Serial.printf("(%d,%d,%d) ", rgbData[idx], rgbData[idx+1], rgbData[idx+2]);
        }
        Serial.println();
        
        // Free the decoded data
        ImageUtils::freeDecodedBuffer(rgbData);
        Serial.println("✓ Memory freed");
    } else {
        Serial.println("✗ JPEG decode failed");
    }
    
    esp_camera_fb_return(frame);
}

/**
 * Example 2: Create Thumbnail
 * Demonstrates resizing an image for thumbnail generation
 */
void example2_create_thumbnail() {
    Serial.println("\nExample 2: Create Thumbnail");
    Serial.println("============================");
    
    // Capture frame
    camera_fb_t* frame = esp_camera_fb_get();
    if (!frame) {
        Serial.println("ERROR: Failed to capture frame");
        return;
    }
    
    // Decode JPEG
    uint16_t width, height;
    uint8_t* rgbData = nullptr;
    
    if (!ImageUtils::decodeJPEG(frame->buf, frame->len, &width, &height, &rgbData)) {
        Serial.println("✗ JPEG decode failed");
        esp_camera_fb_return(frame);
        return;
    }
    
    Serial.printf("Original size: %dx%d\n", width, height);
    
    // Create 128x128 thumbnail using bilinear interpolation
    const uint16_t thumbSize = 128;
    uint8_t* thumbnail = (uint8_t*)malloc(thumbSize * thumbSize * 3);
    
    if (!thumbnail) {
        Serial.println("✗ Failed to allocate thumbnail buffer");
        ImageUtils::freeDecodedBuffer(rgbData);
        esp_camera_fb_return(frame);
        return;
    }
    
    uint32_t startTime = millis();
    bool success = ImageUtils::scaleImage(rgbData, width, height, 
                                         thumbnail, thumbSize, thumbSize);
    uint32_t scaleTime = millis() - startTime;
    
    if (success) {
        Serial.printf("✓ Created %dx%d thumbnail in %d ms\n", thumbSize, thumbSize, scaleTime);
        Serial.println("  (Using bilinear interpolation for smooth scaling)");
        
        // At this point, you could:
        // - Save thumbnail to SD card
        // - Send via WiFi/LoRa
        // - Display on screen
        
        free(thumbnail);
    } else {
        Serial.println("✗ Image scaling failed");
        free(thumbnail);
    }
    
    ImageUtils::freeDecodedBuffer(rgbData);
    esp_camera_fb_return(frame);
}

/**
 * Example 3: AI Preprocessing
 * Demonstrates complete preprocessing pipeline for AI models
 */
void example3_ai_preprocessing() {
    Serial.println("\nExample 3: AI Model Preprocessing");
    Serial.println("==================================");
    
    // Capture frame
    camera_fb_t* frame = esp_camera_fb_get();
    if (!frame) {
        Serial.println("ERROR: Failed to capture frame");
        return;
    }
    
    // Preprocess for typical AI model (224x224 input)
    const uint16_t modelWidth = 224;
    const uint16_t modelHeight = 224;
    
    Serial.printf("Preprocessing for %dx%d model input...\n", modelWidth, modelHeight);
    
    ImageUtils::PreprocessResult result = ImageUtils::preprocessFrameForModel(
        frame, modelWidth, modelHeight
    );
    
    if (result.success) {
        Serial.printf("✓ Preprocessing complete in %d ms\n", result.processingTime);
        Serial.printf("  - Tensor size: %d floats\n", result.tensorSize);
        Serial.printf("  - Value range: [-1.0, 1.0]\n");
        Serial.printf("  - Memory used: %d bytes\n", result.tensorSize * sizeof(float));
        
        // Sample tensor values
        Serial.print("  - Sample tensor values: ");
        for (int i = 0; i < 5 && i < result.tensorSize; i++) {
            Serial.printf("%.3f ", result.tensorData[i]);
        }
        Serial.println("...");
        
        // At this point, you would:
        // 1. Feed result.tensorData to TensorFlow Lite model
        // 2. Run inference
        // 3. Get classification results
        
        Serial.println("\n  Ready for TensorFlow Lite inference!");
        Serial.printf("  Free heap after preprocessing: %d bytes\n", ESP.getFreeHeap());
        
        // Free preprocessing result
        ImageUtils::freePreprocessResult(result);
        Serial.println("✓ Preprocessing memory freed");
    } else {
        Serial.printf("✗ Preprocessing failed: %s\n", result.errorMessage.c_str());
    }
    
    esp_camera_fb_return(frame);
}
