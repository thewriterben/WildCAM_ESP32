/**
 * @file preprocessing.h  
 * @brief Image preprocessing pipeline for ESP32 wildlife classification
 * 
 * Optimized image preprocessing including normalization, resizing, and
 * data augmentation specifically designed for ESP32-S3 constraints.
 */

#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <stdint.h>
#include <stddef.h>

// Image format definitions
#define RGB_CHANNELS 3
#define GRAYSCALE_CHANNELS 1
#define MAX_IMAGE_WIDTH 1600
#define MAX_IMAGE_HEIGHT 1200

/**
 * Image format enumeration
 */
typedef enum {
    FORMAT_RGB888 = 0,      // 24-bit RGB
    FORMAT_RGB565 = 1,      // 16-bit RGB  
    FORMAT_GRAYSCALE = 2,   // 8-bit grayscale
    FORMAT_YUV422 = 3,      // YUV 4:2:2
    FORMAT_JPEG = 4         // JPEG compressed
} ImageFormat;

/**
 * Preprocessing pipeline configuration
 */
typedef struct {
    // Resize parameters
    bool enableResize;
    uint16_t targetWidth;
    uint16_t targetHeight;
    
    // Normalization parameters
    bool enableNormalization;
    float meanRGB[3];
    float stdRGB[3];
    float scaleRange[2];    // [min, max] scaling range
    
    // Color space conversion
    bool convertToGrayscale;
    bool convertColorSpace;
    
    // Data augmentation (for training/federated learning)
    bool enableAugmentation;
    float rotationAngle;    // degrees
    float brightnessAdjust; // -1.0 to 1.0
    float contrastMultiplier; // 0.5 to 2.0
    
    // Quality/performance trade-offs
    bool fastMode;          // Use fast algorithms with lower quality
    bool preserveAspectRatio;
    
} PreprocessingConfig;

/**
 * Image data structure
 */
typedef struct {
    uint8_t* data;          // Image data buffer
    uint16_t width;         // Image width
    uint16_t height;        // Image height
    uint8_t channels;       // Number of channels
    ImageFormat format;     // Image format
    size_t dataSize;        // Size of data buffer
} ImageData;

/**
 * Preprocessing result structure
 */
typedef struct {
    float* processedData;   // Processed image data (float format)
    uint16_t width;         // Processed image width
    uint16_t height;        // Processed image height
    uint8_t channels;       // Number of channels
    size_t dataSize;        // Size of processed data
    uint32_t processingTime; // Processing time in milliseconds
    bool success;           // Whether preprocessing succeeded
} PreprocessingResult;

/**
 * Image Preprocessing Pipeline Class
 * 
 * Handles complete image preprocessing pipeline optimized for ESP32-S3
 * with memory-efficient algorithms and hardware acceleration where available.
 */
class ImagePreprocessor {
public:
    ImagePreprocessor();
    ~ImagePreprocessor();

    /**
     * Initialize the preprocessor
     * @param config Preprocessing configuration
     * @return true if initialization successful
     */
    bool initialize(const PreprocessingConfig& config);

    /**
     * Process a single image
     * @param input Input image data
     * @param output Output buffer for processed data
     * @return Preprocessing result
     */
    PreprocessingResult process(const ImageData& input, float* output);

    /**
     * Process camera frame buffer directly
     * @param frameBuffer Camera frame buffer
     * @param width Frame width
     * @param height Frame height
     * @param format Frame format
     * @param output Output buffer for processed data
     * @return Preprocessing result
     */
    PreprocessingResult processFrame(const uint8_t* frameBuffer, uint16_t width, 
                                   uint16_t height, ImageFormat format, float* output);

    /**
     * Update preprocessing configuration
     * @param config New configuration
     */
    void updateConfig(const PreprocessingConfig& config);

    /**
     * Get current configuration
     * @return Current preprocessing configuration
     */
    PreprocessingConfig getConfig() const { return config_; }

    /**
     * Get memory usage of preprocessing buffers
     * @return Memory usage in bytes
     */
    size_t getMemoryUsage() const;

    /**
     * Reset preprocessing statistics
     */
    void resetStatistics();

    /**
     * Get preprocessing performance statistics
     */
    void getStatistics(uint32_t* totalProcessed, uint32_t* averageTime, 
                      uint32_t* minTime, uint32_t* maxTime) const;

private:
    PreprocessingConfig config_;    // Current configuration
    bool initialized_;              // Initialization status
    uint8_t* tempBuffer_;          // Temporary processing buffer
    size_t tempBufferSize_;        // Size of temporary buffer
    
    // Performance statistics
    uint32_t totalProcessed_;      // Total images processed
    uint32_t totalProcessingTime_; // Total processing time
    uint32_t minProcessingTime_;   // Minimum processing time
    uint32_t maxProcessingTime_;   // Maximum processing time

    /**
     * Resize image using optimized algorithm
     * @param input Input image data
     * @param output Output buffer
     * @param targetWidth Target width
     * @param targetHeight Target height
     * @return true if resize successful
     */
    bool resizeImage(const uint8_t* input, uint16_t inputWidth, uint16_t inputHeight,
                    uint8_t* output, uint16_t targetWidth, uint16_t targetHeight, 
                    uint8_t channels);

    /**
     * Normalize pixel values
     * @param data Image data (will be modified)
     * @param width Image width
     * @param height Image height  
     * @param channels Number of channels
     */
    void normalizePixels(float* data, uint16_t width, uint16_t height, uint8_t channels);

    /**
     * Convert RGB to grayscale
     * @param rgbData RGB image data
     * @param grayscaleData Output grayscale data
     * @param width Image width
     * @param height Image height
     */
    void convertToGrayscale(const uint8_t* rgbData, uint8_t* grayscaleData, 
                           uint16_t width, uint16_t height);

    /**
     * Apply brightness adjustment
     * @param data Image data (will be modified)
     * @param size Size of data array
     * @param adjustment Brightness adjustment (-1.0 to 1.0)
     */
    void adjustBrightness(float* data, size_t size, float adjustment);

    /**
     * Apply contrast adjustment
     * @param data Image data (will be modified)
     * @param size Size of data array
     * @param multiplier Contrast multiplier (0.5 to 2.0)
     */
    void adjustContrast(float* data, size_t size, float multiplier);

    /**
     * Convert uint8 data to float
     * @param input Input uint8 data
     * @param output Output float data
     * @param size Size of data arrays
     */
    void convertToFloat(const uint8_t* input, float* output, size_t size);

    /**
     * Decode JPEG image data
     * @param jpegData JPEG image data
     * @param jpegSize Size of JPEG data
     * @param output Output buffer for decoded data
     * @param outputWidth Output image width
     * @param outputHeight Output image height
     * @return true if decode successful
     */
    bool decodeJPEG(const uint8_t* jpegData, size_t jpegSize, uint8_t* output,
                   uint16_t* outputWidth, uint16_t* outputHeight);

    /**
     * Update processing statistics
     * @param processingTime Time taken for processing
     */
    void updateStatistics(uint32_t processingTime);
};

// Default preprocessing configurations for different use cases
extern const PreprocessingConfig SPECIES_CLASSIFIER_CONFIG;
extern const PreprocessingConfig BEHAVIOR_ANALYZER_CONFIG;
extern const PreprocessingConfig MOTION_DETECTOR_CONFIG;

#endif // PREPROCESSING_H