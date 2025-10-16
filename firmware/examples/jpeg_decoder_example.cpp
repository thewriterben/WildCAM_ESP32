/**
 * @file jpeg_decoder_example.cpp
 * @brief Example usage of JPEG decoding and image resizing
 * 
 * This example demonstrates:
 * 1. Decoding JPEG images from camera
 * 2. Resizing images for AI preprocessing
 * 3. Creating thumbnails for web server
 * 4. Proper memory management
 */

#include <Arduino.h>
#include "src/utils/jpeg_decoder.h"
#include "src/camera/camera_manager.h"
#include "src/ai/preprocessing.h"

// Example 1: Basic JPEG Decoding
void example_basic_decoding() {
    Serial.println("=== Example 1: Basic JPEG Decoding ===");
    
    // Simulate JPEG data (in real use, this comes from camera or storage)
    // For this example, assume we have JPEG data
    uint8_t* jpeg_data = nullptr;  // Would be actual JPEG data
    size_t jpeg_size = 0;          // Would be actual size
    
    // Decode JPEG to RGB888
    uint8_t* rgb_buffer = nullptr;
    int width, height;
    
    if (decodeJPEG(jpeg_data, jpeg_size, &rgb_buffer, &width, &height)) {
        Serial.printf("Successfully decoded JPEG: %dx%d pixels\n", width, height);
        Serial.printf("Buffer size: %d bytes (RGB888)\n", width * height * 3);
        
        // Process RGB data here...
        // For example: analyze colors, detect edges, etc.
        
        // Always free the buffer when done
        freeDecodedBuffer(rgb_buffer);
        Serial.println("Buffer freed successfully");
    } else {
        Serial.println("ERROR: Failed to decode JPEG");
    }
}

// Example 2: Create Thumbnail
void example_create_thumbnail() {
    Serial.println("\n=== Example 2: Create Thumbnail ===");
    
    // Assume we have a decoded image
    uint8_t* original_image = nullptr;  // RGB888 data
    int original_width = 640;
    int original_height = 480;
    
    // Create 128x128 thumbnail using bilinear interpolation
    uint8_t* thumbnail = nullptr;
    const int thumb_size = 128;
    
    if (resizeImageBilinear(original_image, original_width, original_height, 
                           thumb_size, thumb_size, &thumbnail)) {
        Serial.printf("Thumbnail created: %dx%d\n", thumb_size, thumb_size);
        
        // Send thumbnail to web server or save to storage
        // webServer.sendImage(thumbnail, thumb_size, thumb_size);
        
        freeDecodedBuffer(thumbnail);
        Serial.println("Thumbnail freed");
    } else {
        Serial.println("ERROR: Failed to create thumbnail");
    }
}

// Example 3: AI Preprocessing Pipeline
void example_ai_preprocessing() {
    Serial.println("\n=== Example 3: AI Preprocessing ===");
    
    // Configure preprocessor for species classification
    ImagePreprocessor preprocessor;
    
    if (!preprocessor.initialize(SPECIES_CLASSIFIER_CONFIG)) {
        Serial.println("ERROR: Failed to initialize preprocessor");
        return;
    }
    
    // Simulate JPEG image from camera
    uint8_t* jpeg_data = nullptr;  // Actual JPEG data
    size_t jpeg_size = 0;          // Actual size
    
    // Create input image descriptor
    ImageData input = {
        .data = jpeg_data,
        .width = 0,      // Unknown until decoded
        .height = 0,     // Unknown until decoded
        .channels = 3,
        .format = FORMAT_JPEG,
        .dataSize = jpeg_size
    };
    
    // Allocate output buffer for preprocessed data
    // Species classifier expects 224x224x3 float data
    float* ai_input = new float[224 * 224 * 3];
    
    // Process image (automatically decodes JPEG, resizes, normalizes)
    PreprocessingResult result = preprocessor.process(input, ai_input);
    
    if (result.success) {
        Serial.printf("Preprocessing successful: %dx%dx%d\n", 
                     result.width, result.height, result.channels);
        Serial.printf("Processing time: %lu ms\n", result.processingTime);
        
        // Now ai_input is ready for TensorFlow Lite inference
        // inference_engine.run(ai_input);
        
    } else {
        Serial.println("ERROR: Preprocessing failed");
    }
    
    delete[] ai_input;
}

// Example 4: Camera Integration
void example_camera_integration() {
    Serial.println("\n=== Example 4: Camera Integration ===");
    
    CameraManager camera;
    
    // Initialize camera
    if (!camera.initialize()) {
        Serial.println("ERROR: Camera initialization failed");
        return;
    }
    
    Serial.println("Camera initialized successfully");
    
    // Capture frame (RGB565 format)
    int frame_width, frame_height;
    uint8_t* frame_data = camera.captureFrame(&frame_width, &frame_height);
    
    if (!frame_data) {
        Serial.println("ERROR: Frame capture failed");
        return;
    }
    
    Serial.printf("Captured frame: %dx%d (RGB565)\n", frame_width, frame_height);
    
    // Note: Camera currently outputs RGB565, not JPEG
    // To use JPEG decoder, configure camera for JPEG output:
    // config.pixel_format = PIXFORMAT_JPEG;
    
    // For now, we can work with RGB565 data directly
    // or encode it to JPEG first if needed
    
    camera.releaseFrame(frame_data);
    Serial.println("Frame released");
}

// Example 5: Multi-Resolution Processing
void example_multi_resolution() {
    Serial.println("\n=== Example 5: Multi-Resolution Processing ===");
    
    // Assume we have a high-resolution decoded image
    uint8_t* original = nullptr;  // 1600x1200 RGB888
    int orig_width = 1600;
    int orig_height = 1200;
    
    // Create multiple resolutions for different purposes
    
    // 1. Thumbnail for web preview (128x128)
    uint8_t* thumbnail = nullptr;
    if (resizeImageBilinear(original, orig_width, orig_height, 128, 128, &thumbnail)) {
        Serial.println("Created 128x128 thumbnail");
        // Send to web server
        freeDecodedBuffer(thumbnail);
    }
    
    // 2. AI inference input (224x224)
    uint8_t* ai_size = nullptr;
    if (resizeImageBilinear(original, orig_width, orig_height, 224, 224, &ai_size)) {
        Serial.println("Created 224x224 AI input");
        // Process with AI model
        freeDecodedBuffer(ai_size);
    }
    
    // 3. Medium quality for storage (640x480)
    uint8_t* storage_size = nullptr;
    if (resizeImageBilinear(original, orig_width, orig_height, 640, 480, &storage_size)) {
        Serial.println("Created 640x480 storage version");
        // Save to SD card
        freeDecodedBuffer(storage_size);
    }
    
    Serial.println("All resolutions created successfully");
}

// Example 6: Error Handling Best Practices
void example_error_handling() {
    Serial.println("\n=== Example 6: Error Handling ===");
    
    uint8_t* jpeg_data = nullptr;
    size_t jpeg_size = 100;
    
    // Always check return values
    uint8_t* rgb_buffer = nullptr;
    int width = 0, height = 0;
    
    if (!decodeJPEG(jpeg_data, jpeg_size, &rgb_buffer, &width, &height)) {
        // Handle error gracefully
        Serial.println("Decoding failed - handling error");
        
        // Log error details
        Serial.printf("Input: jpeg_data=%p, size=%zu\n", jpeg_data, jpeg_size);
        
        // Don't try to use rgb_buffer - it's nullptr
        // Don't try to free it - freeDecodedBuffer handles nullptr
        freeDecodedBuffer(rgb_buffer);
        
        return;  // Exit early
    }
    
    // Success path
    Serial.printf("Decoded successfully: %dx%d\n", width, height);
    
    // Always use try-finally pattern (or RAII in C++)
    uint8_t* resized = nullptr;
    bool resize_success = resizeImageBilinear(rgb_buffer, width, height, 
                                              320, 240, &resized);
    
    // Clean up in reverse order
    if (resize_success && resized) {
        // Use resized image
        Serial.println("Resize successful");
    }
    
    freeDecodedBuffer(resized);   // Free resized buffer
    freeDecodedBuffer(rgb_buffer); // Free original buffer
    
    Serial.println("All buffers freed - no memory leaks");
}

// Example 7: Performance Monitoring
void example_performance_monitoring() {
    Serial.println("\n=== Example 7: Performance Monitoring ===");
    
    uint8_t* jpeg_data = nullptr;  // Actual data
    size_t jpeg_size = 0;
    
    // Measure decoding time
    unsigned long start_time = millis();
    
    uint8_t* rgb_buffer = nullptr;
    int width, height;
    
    if (decodeJPEG(jpeg_data, jpeg_size, &rgb_buffer, &width, &height)) {
        unsigned long decode_time = millis() - start_time;
        Serial.printf("JPEG decode time: %lu ms\n", decode_time);
        
        // Measure resize time
        start_time = millis();
        uint8_t* resized = nullptr;
        
        if (resizeImageBilinear(rgb_buffer, width, height, 224, 224, &resized)) {
            unsigned long resize_time = millis() - start_time;
            Serial.printf("Resize time: %lu ms\n", resize_time);
            
            // Report memory usage
            size_t original_size = width * height * 3;
            size_t resized_size = 224 * 224 * 3;
            Serial.printf("Memory: %zu bytes -> %zu bytes\n", 
                         original_size, resized_size);
            
            freeDecodedBuffer(resized);
        }
        
        freeDecodedBuffer(rgb_buffer);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== JPEG Decoder Examples ===\n");
    
    // Run examples
    // Note: These examples use placeholder data
    // In real use, provide actual JPEG data
    
    example_basic_decoding();
    example_create_thumbnail();
    example_ai_preprocessing();
    example_camera_integration();
    example_multi_resolution();
    example_error_handling();
    example_performance_monitoring();
    
    Serial.println("\n=== Examples Complete ===");
}

void loop() {
    // Nothing to do in loop
    delay(1000);
}
