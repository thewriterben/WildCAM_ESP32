#include "image_utils.h"
#include <esp_heap_caps.h>
#include <cmath>

namespace ImageUtils {

bool decodeJPEG(const uint8_t* jpegData, size_t jpegSize, 
                uint16_t* outWidth, uint16_t* outHeight, uint8_t** outRgb) {
    if (!jpegData || jpegSize == 0 || !outWidth || !outHeight || !outRgb) {
        Serial.println("Invalid parameters for JPEG decode");
        return false;
    }
    
    // TODO: Implement proper JPEG decoding using a library like TJpgDec
    // For now, this is a placeholder that assumes frame buffer is already decoded
    // In practice, esp_camera provides JPEG data that needs decoding
    
    // Placeholder: Allocate minimal RGB buffer
    // Actual implementation would decode the JPEG and populate this
    *outWidth = 320;   // Placeholder dimensions
    *outHeight = 240;
    size_t rgbSize = (*outWidth) * (*outHeight) * 3;
    
    *outRgb = (uint8_t*)heap_caps_malloc(rgbSize, MALLOC_CAP_8BIT);
    if (*outRgb == nullptr) {
        Serial.println("Failed to allocate RGB buffer");
        return false;
    }
    
    // Placeholder: Fill with dummy data
    // Real implementation would decode JPEG here
    memset(*outRgb, 128, rgbSize);
    
    Serial.println("JPEG decode (placeholder) completed");
    return true;
}

bool scaleImage(const uint8_t* srcRgb, uint16_t srcWidth, uint16_t srcHeight,
                uint8_t* dstRgb, uint16_t dstWidth, uint16_t dstHeight) {
    if (!srcRgb || !dstRgb || srcWidth == 0 || srcHeight == 0 || 
        dstWidth == 0 || dstHeight == 0) {
        Serial.println("Invalid parameters for image scaling");
        return false;
    }
    
    /**
     * PLACEHOLDER: Simple nearest-neighbor scaling
     * 
     * NOTE: This is a basic implementation using nearest-neighbor interpolation.
     * For production use, consider implementing:
     * - Bilinear interpolation for better quality
     * - Bicubic interpolation for highest quality
     * - Hardware-accelerated scaling if available
     * 
     * The current implementation prioritizes simplicity and speed over quality.
     */
    
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;
    
    for (uint16_t y = 0; y < dstHeight; y++) {
        for (uint16_t x = 0; x < dstWidth; x++) {
            // Find nearest source pixel
            uint16_t srcX = (uint16_t)(x * xRatio);
            uint16_t srcY = (uint16_t)(y * yRatio);
            
            // Clamp to source bounds
            if (srcX >= srcWidth) srcX = srcWidth - 1;
            if (srcY >= srcHeight) srcY = srcHeight - 1;
            
            // Copy RGB values
            size_t srcIndex = (srcY * srcWidth + srcX) * 3;
            size_t dstIndex = (y * dstWidth + x) * 3;
            
            dstRgb[dstIndex + 0] = srcRgb[srcIndex + 0];  // R
            dstRgb[dstIndex + 1] = srcRgb[srcIndex + 1];  // G
            dstRgb[dstIndex + 2] = srcRgb[srcIndex + 2];  // B
        }
    }
    
    Serial.printf("Image scaled from %dx%d to %dx%d (nearest-neighbor)\n", 
                 srcWidth, srcHeight, dstWidth, dstHeight);
    return true;
}

bool normalizeToTensor(const uint8_t* rgbData, uint16_t width, uint16_t height,
                       float* tensorData) {
    if (!rgbData || !tensorData || width == 0 || height == 0) {
        Serial.println("Invalid parameters for tensor normalization");
        return false;
    }
    
    size_t pixelCount = width * height;
    size_t rgbSize = pixelCount * 3;
    
    // Normalize RGB values from [0, 255] to [-1, 1]
    for (size_t i = 0; i < rgbSize; i++) {
        tensorData[i] = (rgbData[i] / 127.5f) - 1.0f;
    }
    
    Serial.printf("Normalized %d pixels to tensor [-1, 1]\n", pixelCount);
    return true;
}

PreprocessResult preprocessFrameForModel(camera_fb_t* frame, 
                                        uint16_t targetWidth, 
                                        uint16_t targetHeight) {
    PreprocessResult result = {};
    result.success = false;
    result.tensorData = nullptr;
    result.tensorSize = 0;
    
    uint32_t startTime = millis();
    
    if (!frame || !frame->buf || frame->len == 0) {
        result.errorMessage = "Invalid frame buffer";
        return result;
    }
    
    // Step 1: Decode JPEG to RGB
    uint16_t decodedWidth, decodedHeight;
    uint8_t* rgbData = nullptr;
    
    if (!decodeJPEG(frame->buf, frame->len, &decodedWidth, &decodedHeight, &rgbData)) {
        result.errorMessage = "JPEG decode failed";
        return result;
    }
    
    // Step 2: Scale to target dimensions
    size_t scaledSize = targetWidth * targetHeight * 3;
    uint8_t* scaledRgb = (uint8_t*)heap_caps_malloc(scaledSize, MALLOC_CAP_8BIT);
    
    if (scaledRgb == nullptr) {
        free(rgbData);
        result.errorMessage = "Failed to allocate scaled buffer";
        return result;
    }
    
    if (!scaleImage(rgbData, decodedWidth, decodedHeight, 
                   scaledRgb, targetWidth, targetHeight)) {
        free(rgbData);
        free(scaledRgb);
        result.errorMessage = "Image scaling failed";
        return result;
    }
    
    free(rgbData);  // No longer need decoded image
    
    // Step 3: Normalize to tensor
    result.tensorSize = targetWidth * targetHeight * 3;
    result.tensorData = (float*)heap_caps_malloc(result.tensorSize * sizeof(float), 
                                                  MALLOC_CAP_8BIT);
    
    if (result.tensorData == nullptr) {
        free(scaledRgb);
        result.errorMessage = "Failed to allocate tensor buffer";
        return result;
    }
    
    if (!normalizeToTensor(scaledRgb, targetWidth, targetHeight, result.tensorData)) {
        free(scaledRgb);
        free(result.tensorData);
        result.tensorData = nullptr;
        result.errorMessage = "Tensor normalization failed";
        return result;
    }
    
    free(scaledRgb);  // No longer need scaled RGB image
    
    result.success = true;
    result.processingTime = millis() - startTime;
    
    Serial.printf("Image preprocessing completed in %d ms\n", result.processingTime);
    return result;
}

void freePreprocessResult(PreprocessResult& result) {
    if (result.tensorData != nullptr) {
        free(result.tensorData);
        result.tensorData = nullptr;
    }
    result.tensorSize = 0;
    result.success = false;
}

bool grayscaleToRGB(const uint8_t* grayscaleData, uint16_t width, uint16_t height,
                    uint8_t* rgbData) {
    if (!grayscaleData || !rgbData || width == 0 || height == 0) {
        Serial.println("Invalid parameters for grayscale to RGB conversion");
        return false;
    }
    
    size_t pixelCount = width * height;
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t gray = grayscaleData[i];
        rgbData[i * 3 + 0] = gray;  // R
        rgbData[i * 3 + 1] = gray;  // G
        rgbData[i * 3 + 2] = gray;  // B
    }
    
    Serial.printf("Converted %d grayscale pixels to RGB\n", pixelCount);
    return true;
}

bool enhanceImage(uint8_t* rgbData, uint16_t width, uint16_t height,
                  int brightness, float contrast) {
    if (!rgbData || width == 0 || height == 0) {
        Serial.println("Invalid parameters for image enhancement");
        return false;
    }
    
    // Clamp parameters
    brightness = constrain(brightness, -100, 100);
    contrast = constrain(contrast, 0.5f, 2.0f);
    
    size_t pixelCount = width * height * 3;
    
    for (size_t i = 0; i < pixelCount; i++) {
        // Apply contrast
        float value = rgbData[i];
        value = (value - 128.0f) * contrast + 128.0f;
        
        // Apply brightness
        value += brightness;
        
        // Clamp to valid range
        value = constrain(value, 0.0f, 255.0f);
        
        rgbData[i] = (uint8_t)value;
    }
    
    Serial.printf("Enhanced image: brightness=%d, contrast=%.2f\n", brightness, contrast);
    return true;
}

} // namespace ImageUtils
