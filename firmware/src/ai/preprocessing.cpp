/**
 * @file preprocessing.cpp
 * @brief Image preprocessing pipeline implementation for ESP32 wildlife classification
 */

#include "preprocessing.h"
#include "../utils/jpeg_decoder.h"
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <string.h>

static const char* TAG = "ImagePreprocessor";

// Default preprocessing configurations 
const PreprocessingConfig SPECIES_CLASSIFIER_CONFIG = {
    .enableResize = true,
    .targetWidth = 224,
    .targetHeight = 224,
    .enableNormalization = true,
    .meanRGB = {0.485f, 0.456f, 0.406f},
    .stdRGB = {0.229f, 0.224f, 0.225f},
    .scaleRange = {0.0f, 1.0f},
    .convertToGrayscale = false,
    .convertColorSpace = false,
    .enableAugmentation = false,
    .rotationAngle = 0.0f,
    .brightnessAdjust = 0.0f,
    .contrastMultiplier = 1.0f,
    .fastMode = true,
    .preserveAspectRatio = false
};

const PreprocessingConfig BEHAVIOR_ANALYZER_CONFIG = {
    .enableResize = true,
    .targetWidth = 160,
    .targetHeight = 160,
    .enableNormalization = true,
    .meanRGB = {0.5f, 0.5f, 0.5f},
    .stdRGB = {0.5f, 0.5f, 0.5f},
    .scaleRange = {-1.0f, 1.0f},
    .convertToGrayscale = false,
    .convertColorSpace = false,
    .enableAugmentation = false,
    .rotationAngle = 0.0f,
    .brightnessAdjust = 0.0f,
    .contrastMultiplier = 1.0f,
    .fastMode = true,
    .preserveAspectRatio = false
};

const PreprocessingConfig MOTION_DETECTOR_CONFIG = {
    .enableResize = true,
    .targetWidth = 96,
    .targetHeight = 96,
    .enableNormalization = true,
    .meanRGB = {0.5f, 0.5f, 0.5f},
    .stdRGB = {0.5f, 0.5f, 0.5f},
    .scaleRange = {0.0f, 1.0f},
    .convertToGrayscale = true,
    .convertColorSpace = false,
    .enableAugmentation = false,
    .rotationAngle = 0.0f,
    .brightnessAdjust = 0.0f,
    .contrastMultiplier = 1.0f,
    .fastMode = true,
    .preserveAspectRatio = false
};

ImagePreprocessor::ImagePreprocessor()
    : initialized_(false)
    , tempBuffer_(nullptr)
    , tempBufferSize_(0)
    , totalProcessed_(0)
    , totalProcessingTime_(0)
    , minProcessingTime_(UINT32_MAX)
    , maxProcessingTime_(0) {
    
    memset(&config_, 0, sizeof(config_));
}

ImagePreprocessor::~ImagePreprocessor() {
    if (tempBuffer_) {
        heap_caps_free(tempBuffer_);
        tempBuffer_ = nullptr;
    }
}

bool ImagePreprocessor::initialize(const PreprocessingConfig& config) {
    config_ = config;
    
    // Calculate required buffer size
    size_t maxImageSize = MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * RGB_CHANNELS;
    tempBufferSize_ = maxImageSize * sizeof(float);
    
    // Allocate temporary buffer preferring PSRAM
    tempBuffer_ = static_cast<uint8_t*>(heap_caps_malloc(tempBufferSize_, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!tempBuffer_) {
        tempBuffer_ = static_cast<uint8_t*>(heap_caps_malloc(tempBufferSize_, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    }
    
    if (!tempBuffer_) {
        ESP_LOGE(TAG, "Failed to allocate temporary buffer of size %zu", tempBufferSize_);
        return false;
    }
    
    initialized_ = true;
    ESP_LOGI(TAG, "Image preprocessor initialized successfully");
    ESP_LOGI(TAG, "Buffer size: %zu bytes, Target size: %dx%d", 
             tempBufferSize_, config_.targetWidth, config_.targetHeight);
    
    return true;
}

PreprocessingResult ImagePreprocessor::process(const ImageData& input, float* output) {
    PreprocessingResult result = {};
    result.success = false;
    
    if (!initialized_ || !input.data || !output) {
        ESP_LOGE(TAG, "Invalid parameters for preprocessing");
        return result;
    }
    
    uint32_t startTime = esp_timer_get_time() / 1000; // Convert to milliseconds
    
    ESP_LOGD(TAG, "Processing image: %dx%dx%d, format: %d", 
             input.width, input.height, input.channels, input.format);
    
    // Decode image if necessary
    const uint8_t* imageData = input.data;
    uint16_t imageWidth = input.width;
    uint16_t imageHeight = input.height;
    uint8_t imageChannels = input.channels;
    uint8_t* decodedData = nullptr;
    
    if (input.format == FORMAT_JPEG) {
        // Decode JPEG to RGB888
        int decoded_width, decoded_height;
        if (!decodeJPEG(input.data, input.dataSize, &decodedData, &decoded_width, &decoded_height)) {
            ESP_LOGE(TAG, "Failed to decode JPEG image");
            return result;
        }
        imageData = decodedData;
        imageWidth = decoded_width;
        imageHeight = decoded_height;
        imageChannels = 3; // RGB888
        ESP_LOGI(TAG, "JPEG decoded to %dx%d RGB888", imageWidth, imageHeight);
    }
    
    // Convert to float and store in output
    size_t pixelCount = imageWidth * imageHeight * imageChannels;
    convertToFloat(imageData, output, pixelCount);
    
    // Apply resize if needed
    if (config_.enableResize && (imageWidth != config_.targetWidth || imageHeight != config_.targetHeight)) {
        if (!resizeImage(imageData, imageWidth, imageHeight, 
                        tempBuffer_, config_.targetWidth, config_.targetHeight, imageChannels)) {
            ESP_LOGE(TAG, "Image resize failed");
            return result;
        }
        
        // Convert resized data to float
        size_t resizedPixelCount = config_.targetWidth * config_.targetHeight * imageChannels;
        convertToFloat(tempBuffer_, output, resizedPixelCount);
        
        result.width = config_.targetWidth;
        result.height = config_.targetHeight;
    } else {
        result.width = imageWidth;
        result.height = imageHeight;
    }
    
    // Convert to grayscale if needed
    if (config_.convertToGrayscale && imageChannels == 3) {
        convertToGrayscale(reinterpret_cast<const uint8_t*>(output), 
                          reinterpret_cast<uint8_t*>(output), result.width, result.height);
        result.channels = 1;
    } else {
        result.channels = imageChannels;
    }
    
    // Apply normalization
    if (config_.enableNormalization) {
        normalizePixels(output, result.width, result.height, result.channels);
    }
    
    // Apply brightness adjustment
    if (config_.enableAugmentation && config_.brightnessAdjust != 0.0f) {
        size_t dataSize = result.width * result.height * result.channels;
        adjustBrightness(output, dataSize, config_.brightnessAdjust);
    }
    
    // Apply contrast adjustment
    if (config_.enableAugmentation && config_.contrastMultiplier != 1.0f) {
        size_t dataSize = result.width * result.height * result.channels;
        adjustContrast(output, dataSize, config_.contrastMultiplier);
    }
    
    uint32_t endTime = esp_timer_get_time() / 1000;
    result.processingTime = endTime - startTime;
    result.dataSize = result.width * result.height * result.channels * sizeof(float);
    result.processedData = output;
    result.success = true;
    
    // Update statistics
    updateStatistics(result.processingTime);
    
    // Free decoded JPEG buffer if allocated
    if (decodedData) {
        freeDecodedBuffer(decodedData);
    }
    
    ESP_LOGD(TAG, "Preprocessing completed in %lu ms, output: %dx%dx%d", 
             result.processingTime, result.width, result.height, result.channels);
    
    return result;
}

PreprocessingResult ImagePreprocessor::processFrame(const uint8_t* frameBuffer, uint16_t width, 
                                                   uint16_t height, ImageFormat format, float* output) {
    ImageData imageData = {
        .data = const_cast<uint8_t*>(frameBuffer),
        .width = width,
        .height = height,
        .channels = (format == FORMAT_GRAYSCALE) ? 1 : 3,
        .format = format,
        .dataSize = width * height * ((format == FORMAT_GRAYSCALE) ? 1 : 3)
    };
    
    return process(imageData, output);
}

void ImagePreprocessor::updateConfig(const PreprocessingConfig& config) {
    config_ = config;
    ESP_LOGI(TAG, "Preprocessing configuration updated");
}

size_t ImagePreprocessor::getMemoryUsage() const {
    return tempBufferSize_;
}

void ImagePreprocessor::resetStatistics() {
    totalProcessed_ = 0;
    totalProcessingTime_ = 0;
    minProcessingTime_ = UINT32_MAX;
    maxProcessingTime_ = 0;
}

void ImagePreprocessor::getStatistics(uint32_t* totalProcessed, uint32_t* averageTime, 
                                     uint32_t* minTime, uint32_t* maxTime) const {
    if (totalProcessed) *totalProcessed = totalProcessed_;
    if (averageTime) *averageTime = totalProcessed_ > 0 ? totalProcessingTime_ / totalProcessed_ : 0;
    if (minTime) *minTime = minProcessingTime_ == UINT32_MAX ? 0 : minProcessingTime_;
    if (maxTime) *maxTime = maxProcessingTime_;
}

bool ImagePreprocessor::resizeImage(const uint8_t* input, uint16_t inputWidth, uint16_t inputHeight,
                                   uint8_t* output, uint16_t targetWidth, uint16_t targetHeight, 
                                   uint8_t channels) {
    if (!input || !output) return false;
    
    // Simple nearest neighbor interpolation for fast mode
    if (config_.fastMode) {
        float scaleX = (float)inputWidth / targetWidth;
        float scaleY = (float)inputHeight / targetHeight;
        
        for (uint16_t y = 0; y < targetHeight; y++) {
            for (uint16_t x = 0; x < targetWidth; x++) {
                uint16_t srcX = (uint16_t)(x * scaleX);
                uint16_t srcY = (uint16_t)(y * scaleY);
                
                // Clamp to input bounds
                srcX = (srcX >= inputWidth) ? inputWidth - 1 : srcX;
                srcY = (srcY >= inputHeight) ? inputHeight - 1 : srcY;
                
                for (uint8_t c = 0; c < channels; c++) {
                    size_t srcIdx = (srcY * inputWidth + srcX) * channels + c;
                    size_t dstIdx = (y * targetWidth + x) * channels + c;
                    output[dstIdx] = input[srcIdx];
                }
            }
        }
    } else {
        // Bilinear interpolation for better quality
        float scaleX = (float)(inputWidth - 1) / (targetWidth - 1);
        float scaleY = (float)(inputHeight - 1) / (targetHeight - 1);
        
        for (uint16_t y = 0; y < targetHeight; y++) {
            for (uint16_t x = 0; x < targetWidth; x++) {
                float srcX = x * scaleX;
                float srcY = y * scaleY;
                
                uint16_t x1 = (uint16_t)srcX;
                uint16_t y1 = (uint16_t)srcY;
                uint16_t x2 = (x1 + 1 < inputWidth) ? x1 + 1 : x1;
                uint16_t y2 = (y1 + 1 < inputHeight) ? y1 + 1 : y1;
                
                float wx = srcX - x1;
                float wy = srcY - y1;
                
                for (uint8_t c = 0; c < channels; c++) {
                    size_t idx11 = (y1 * inputWidth + x1) * channels + c;
                    size_t idx12 = (y1 * inputWidth + x2) * channels + c;
                    size_t idx21 = (y2 * inputWidth + x1) * channels + c;
                    size_t idx22 = (y2 * inputWidth + x2) * channels + c;
                    
                    float val = (1 - wx) * (1 - wy) * input[idx11] +
                               wx * (1 - wy) * input[idx12] +
                               (1 - wx) * wy * input[idx21] +
                               wx * wy * input[idx22];
                    
                    size_t dstIdx = (y * targetWidth + x) * channels + c;
                    output[dstIdx] = (uint8_t)(val + 0.5f); // Round to nearest
                }
            }
        }
    }
    
    return true;
}

void ImagePreprocessor::normalizePixels(float* data, uint16_t width, uint16_t height, uint8_t channels) {
    size_t pixelCount = width * height;
    
    for (uint8_t c = 0; c < channels; c++) {
        float mean = (c < 3) ? config_.meanRGB[c] : config_.meanRGB[0];
        float std = (c < 3) ? config_.stdRGB[c] : config_.stdRGB[0];
        float scale = config_.scaleRange[1] - config_.scaleRange[0];
        float offset = config_.scaleRange[0];
        
        for (size_t i = 0; i < pixelCount; i++) {
            size_t idx = i * channels + c;
            // First normalize to 0-1 range
            data[idx] = data[idx] / 255.0f;
            // Then apply mean/std normalization and scale
            data[idx] = ((data[idx] - mean) / std) * scale + offset;
        }
    }
}

void ImagePreprocessor::convertToGrayscale(const uint8_t* rgbData, uint8_t* grayscaleData, 
                                          uint16_t width, uint16_t height) {
    size_t pixelCount = width * height;
    
    for (size_t i = 0; i < pixelCount; i++) {
        size_t rgbIdx = i * 3;
        uint8_t r = rgbData[rgbIdx];
        uint8_t g = rgbData[rgbIdx + 1];
        uint8_t b = rgbData[rgbIdx + 2];
        
        // Standard grayscale conversion weights
        grayscaleData[i] = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
    }
}

void ImagePreprocessor::adjustBrightness(float* data, size_t size, float adjustment) {
    for (size_t i = 0; i < size; i++) {
        data[i] = fmaxf(0.0f, fminf(1.0f, data[i] + adjustment));
    }
}

void ImagePreprocessor::adjustContrast(float* data, size_t size, float multiplier) {
    for (size_t i = 0; i < size; i++) {
        data[i] = fmaxf(0.0f, fminf(1.0f, (data[i] - 0.5f) * multiplier + 0.5f));
    }
}

void ImagePreprocessor::convertToFloat(const uint8_t* input, float* output, size_t size) {
    for (size_t i = 0; i < size; i++) {
        output[i] = (float)input[i];
    }
}

bool ImagePreprocessor::decodeJPEG(const uint8_t* jpegData, size_t jpegSize, uint8_t* output,
                                  uint16_t* outputWidth, uint16_t* outputHeight) {
    // Use the standalone JPEG decoder from utils
    uint8_t* decoded_buffer = nullptr;
    int width, height;
    
    if (!::decodeJPEG(jpegData, jpegSize, &decoded_buffer, &width, &height)) {
        return false;
    }
    
    // Copy decoded data to output buffer
    size_t data_size = width * height * 3;
    memcpy(output, decoded_buffer, data_size);
    
    if (outputWidth) *outputWidth = width;
    if (outputHeight) *outputHeight = height;
    
    freeDecodedBuffer(decoded_buffer);
    return true;
}

void ImagePreprocessor::updateStatistics(uint32_t processingTime) {
    totalProcessed_++;
    totalProcessingTime_ += processingTime;
    
    if (processingTime < minProcessingTime_) {
        minProcessingTime_ = processingTime;
    }
    
    if (processingTime > maxProcessingTime_) {
        maxProcessingTime_ = processingTime;
    }
}