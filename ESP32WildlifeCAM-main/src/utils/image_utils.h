#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include <Arduino.h>
#include <esp_camera.h>

/**
 * @brief Image preprocessing utilities for AI classifier
 * 
 * Provides image decoding, scaling, and preprocessing functions
 * for TensorFlow Lite model input preparation.
 */
namespace ImageUtils {

/**
 * @brief Image preprocessing result
 */
struct PreprocessResult {
    bool success;
    float* tensorData;           // Preprocessed tensor data (normalized)
    size_t tensorSize;           // Size of tensor data in floats
    uint32_t processingTime;     // Time taken for preprocessing (ms)
    String errorMessage;         // Error message if failed
};

/**
 * @brief Decode JPEG image to RGB data
 * @param jpegData JPEG image data
 * @param jpegSize Size of JPEG data
 * @param outWidth Output image width
 * @param outHeight Output image height
 * @param outRgb Output RGB buffer (allocated by function, caller must free)
 * @return true if decoding successful
 */
bool decodeJPEG(const uint8_t* jpegData, size_t jpegSize, 
                uint16_t* outWidth, uint16_t* outHeight, uint8_t** outRgb);

/**
 * @brief Scale image to target dimensions
 * PLACEHOLDER: Currently uses simple nearest-neighbor interpolation
 * TODO: Implement bilinear or bicubic interpolation for better quality
 * 
 * @param srcRgb Source RGB data
 * @param srcWidth Source width
 * @param srcHeight Source height
 * @param dstRgb Destination RGB buffer (must be pre-allocated)
 * @param dstWidth Target width
 * @param dstHeight Target height
 * @return true if scaling successful
 */
bool scaleImage(const uint8_t* srcRgb, uint16_t srcWidth, uint16_t srcHeight,
                uint8_t* dstRgb, uint16_t dstWidth, uint16_t dstHeight);

/**
 * @brief Normalize RGB image to float tensor [-1, 1]
 * @param rgbData RGB image data (0-255)
 * @param width Image width
 * @param height Image height
 * @param tensorData Output tensor data (normalized to [-1, 1])
 * @return true if normalization successful
 */
bool normalizeToTensor(const uint8_t* rgbData, uint16_t width, uint16_t height,
                       float* tensorData);

/**
 * @brief Preprocess camera frame for TensorFlow Lite model
 * Performs: JPEG decode -> scale -> normalize
 * 
 * @param frame Camera frame buffer
 * @param targetWidth Target width for model input
 * @param targetHeight Target height for model input
 * @return Preprocessing result with tensor data
 */
PreprocessResult preprocessFrameForModel(camera_fb_t* frame, 
                                        uint16_t targetWidth, 
                                        uint16_t targetHeight);

/**
 * @brief Free preprocessing result resources
 * @param result Preprocessing result to free
 */
void freePreprocessResult(PreprocessResult& result);

/**
 * @brief Convert grayscale to RGB (for models expecting RGB input)
 * @param grayscaleData Grayscale image data
 * @param width Image width
 * @param height Image height
 * @param rgbData Output RGB data (must be pre-allocated, 3x size of grayscale)
 * @return true if conversion successful
 */
bool grayscaleToRGB(const uint8_t* grayscaleData, uint16_t width, uint16_t height,
                    uint8_t* rgbData);

/**
 * @brief Apply basic image enhancements (contrast, brightness)
 * @param rgbData RGB image data (modified in-place)
 * @param width Image width
 * @param height Image height
 * @param brightness Brightness adjustment (-100 to 100)
 * @param contrast Contrast adjustment (0.5 to 2.0)
 * @return true if enhancement successful
 */
bool enhanceImage(uint8_t* rgbData, uint16_t width, uint16_t height,
                  int brightness = 0, float contrast = 1.0f);

} // namespace ImageUtils

#endif // IMAGE_UTILS_H
