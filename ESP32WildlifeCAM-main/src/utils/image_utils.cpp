#include "image_utils.h"
#include <esp_heap_caps.h>
#include <TJpg_Decoder.h>
#include <cmath>

namespace ImageUtils {

// Callback function for TJpgDec to output decoded image data
static uint8_t* g_rgbBuffer = nullptr;
static uint16_t g_currentRow = 0;
static uint16_t g_imageWidth = 0;

// TJpgDec callback: Called when a block of image is decoded
bool tjpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!g_rgbBuffer) return false;
    
    // Convert RGB565 to RGB888
    for (uint16_t row = 0; row < h; row++) {
        for (uint16_t col = 0; col < w; col++) {
            uint16_t pixel565 = bitmap[row * w + col];
            
            // Extract RGB components from RGB565
            uint8_t r = ((pixel565 >> 11) & 0x1F) << 3;  // 5 bits -> 8 bits
            uint8_t g = ((pixel565 >> 5) & 0x3F) << 2;   // 6 bits -> 8 bits
            uint8_t b = (pixel565 & 0x1F) << 3;          // 5 bits -> 8 bits
            
            // Calculate position in RGB888 buffer
            size_t pixelIndex = ((y + row) * g_imageWidth + (x + col)) * 3;
            g_rgbBuffer[pixelIndex + 0] = r;
            g_rgbBuffer[pixelIndex + 1] = g;
            g_rgbBuffer[pixelIndex + 2] = b;
        }
    }
    
    return true;
}

bool decodeJPEG(const uint8_t* jpegData, size_t jpegSize, 
                uint16_t* outWidth, uint16_t* outHeight, uint8_t** outRgb) {
    if (!jpegData || jpegSize == 0 || !outWidth || !outHeight || !outRgb) {
        Serial.println("ERROR: Invalid parameters for JPEG decode");
        return false;
    }
    
    // Initialize TJpgDec
    TJpgDec.setJpgScale(1);  // No scaling (1:1)
    TJpgDec.setCallback(tjpgOutput);
    
    // Get JPEG dimensions without decoding
    uint16_t w, h;
    if (TJpgDec.getJpgSize(&w, &h, jpegData, jpegSize) != JDR_OK) {
        Serial.println("ERROR: Failed to get JPEG dimensions");
        return false;
    }
    
    *outWidth = w;
    *outHeight = h;
    
    // Allocate RGB buffer
    size_t rgbSize = w * h * 3;
    g_rgbBuffer = (uint8_t*)heap_caps_malloc(rgbSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (g_rgbBuffer == nullptr) {
        Serial.printf("ERROR: Failed to allocate RGB buffer (%d bytes)\n", rgbSize);
        return false;
    }
    
    // Set global variables for callback
    g_imageWidth = w;
    g_currentRow = 0;
    
    // Decode JPEG
    if (TJpgDec.drawJpg(0, 0, jpegData, jpegSize) != JDR_OK) {
        Serial.println("ERROR: JPEG decode failed");
        free(g_rgbBuffer);
        g_rgbBuffer = nullptr;
        return false;
    }
    
    *outRgb = g_rgbBuffer;
    g_rgbBuffer = nullptr;  // Transfer ownership to caller
    
    Serial.printf("JPEG decoded successfully: %dx%d (%d bytes)\n", w, h, rgbSize);
    return true;
}

void freeDecodedBuffer(uint8_t* rgbBuffer) {
    if (rgbBuffer != nullptr) {
        free(rgbBuffer);
    }
}

bool scaleImage(const uint8_t* srcRgb, uint16_t srcWidth, uint16_t srcHeight,
                uint8_t* dstRgb, uint16_t dstWidth, uint16_t dstHeight) {
    if (!srcRgb || !dstRgb || srcWidth == 0 || srcHeight == 0 || 
        dstWidth == 0 || dstHeight == 0) {
        Serial.println("ERROR: Invalid parameters for image scaling");
        return false;
    }
    
    /**
     * Bilinear Interpolation Implementation
     * 
     * This implementation uses bilinear interpolation for high-quality image resizing.
     * For each destination pixel, we:
     * 1. Calculate the corresponding floating-point position in the source image
     * 2. Find the four nearest source pixels (top-left, top-right, bottom-left, bottom-right)
     * 3. Interpolate horizontally between top pixels and between bottom pixels
     * 4. Interpolate vertically between the two horizontal results
     * 
     * This provides much better quality than nearest-neighbor, especially for downscaling.
     */
    
    float xRatio = (float)srcWidth / dstWidth;
    float yRatio = (float)srcHeight / dstHeight;
    
    for (uint16_t dstY = 0; dstY < dstHeight; dstY++) {
        for (uint16_t dstX = 0; dstX < dstWidth; dstX++) {
            // Calculate corresponding position in source image
            float srcXf = dstX * xRatio;
            float srcYf = dstY * yRatio;
            
            // Get integer and fractional parts
            uint16_t srcX = (uint16_t)srcXf;
            uint16_t srcY = (uint16_t)srcYf;
            float fracX = srcXf - srcX;
            float fracY = srcYf - srcY;
            
            // Clamp to source bounds
            uint16_t srcX1 = srcX;
            uint16_t srcY1 = srcY;
            uint16_t srcX2 = (srcX + 1 < srcWidth) ? srcX + 1 : srcX;
            uint16_t srcY2 = (srcY + 1 < srcHeight) ? srcY + 1 : srcY;
            
            // Get the four surrounding pixels for each RGB channel
            for (int c = 0; c < 3; c++) {
                // Top-left, top-right, bottom-left, bottom-right pixels
                float p11 = srcRgb[(srcY1 * srcWidth + srcX1) * 3 + c];
                float p21 = srcRgb[(srcY1 * srcWidth + srcX2) * 3 + c];
                float p12 = srcRgb[(srcY2 * srcWidth + srcX1) * 3 + c];
                float p22 = srcRgb[(srcY2 * srcWidth + srcX2) * 3 + c];
                
                // Bilinear interpolation
                // First interpolate horizontally
                float top = p11 * (1.0f - fracX) + p21 * fracX;
                float bottom = p12 * (1.0f - fracX) + p22 * fracX;
                
                // Then interpolate vertically
                float result = top * (1.0f - fracY) + bottom * fracY;
                
                // Write to destination
                size_t dstIndex = (dstY * dstWidth + dstX) * 3 + c;
                dstRgb[dstIndex] = (uint8_t)(result + 0.5f);  // Round to nearest
            }
        }
    }
    
    Serial.printf("Image scaled from %dx%d to %dx%d (bilinear interpolation)\n", 
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
        freeDecodedBuffer(rgbData);
        result.errorMessage = "Failed to allocate scaled buffer";
        return result;
    }
    
    if (!scaleImage(rgbData, decodedWidth, decodedHeight, 
                   scaledRgb, targetWidth, targetHeight)) {
        freeDecodedBuffer(rgbData);
        free(scaledRgb);
        result.errorMessage = "Image scaling failed";
        return result;
    }
    
    freeDecodedBuffer(rgbData);  // No longer need decoded image
    
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
