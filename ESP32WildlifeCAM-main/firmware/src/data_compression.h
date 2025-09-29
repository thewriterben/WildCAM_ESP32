/**
 * @file data_compression.h
 * @brief Header file for data compression utilities
 */

#ifndef DATA_COMPRESSION_H
#define DATA_COMPRESSION_H

#include <Arduino.h>

// Compression constants
#define RLE_MAX_COUNT 255
#define COMPRESSION_MIN_EFFICIENCY 3
#define JPEG_QUALITY_LOW 50
#define JPEG_QUALITY_MEDIUM 75
#define JPEG_QUALITY_HIGH 85

enum CompressionLevel {
    COMPRESSION_NONE = 0,
    COMPRESSION_LOW,
    COMPRESSION_MEDIUM,
    COMPRESSION_HIGH
};

enum MetadataLevel {
    METADATA_MINIMAL = 0,
    METADATA_STANDARD,
    METADATA_DETAILED
};

struct CameraMetadata {
    unsigned long timestamp;
    int batteryLevel;
    bool motionDetected;
    float temperature;
    float humidity;
    int lightLevel;
    int imageWidth;
    int imageHeight;
    float compressionRatio;
};

class DataCompression {
public:
    DataCompression();
    
    // Image compression
    size_t compressImage(const uint8_t* imageData, size_t imageSize, 
                        uint8_t* compressedData, size_t maxCompressedSize,
                        CompressionLevel level = COMPRESSION_MEDIUM);
    
    size_t decompressImage(const uint8_t* compressedData, size_t compressedSize,
                          uint8_t* imageData, size_t maxImageSize);
    
    // Metadata handling
    String createMetadataPacket(const CameraMetadata& metadata, 
                               MetadataLevel level = METADATA_STANDARD);
    CameraMetadata parseMetadataPacket(const String& packet);
    
    // Thumbnail creation
    size_t createThumbnail(const uint8_t* imageData, size_t imageSize,
                          uint8_t* thumbnailData, size_t maxThumbnailSize,
                          int targetWidth = 64, int targetHeight = 48);
    
    // Encoding utilities
    String encodeBase64(const uint8_t* data, size_t length);
    size_t decodeBase64(const String& encoded, uint8_t* data, size_t maxLength);
    
    // Network-specific optimization
    size_t optimizeForSatellite(const uint8_t* data, size_t dataSize,
                               uint8_t* optimizedData, size_t maxOptimizedSize);
    
    size_t optimizeForCellular(const uint8_t* data, size_t dataSize,
                              uint8_t* optimizedData, size_t maxOptimizedSize,
                              int compressionLevel = 2);

private:
    // Internal compression methods would be implemented here
};

#endif // DATA_COMPRESSION_H