/**
 * @file data_compression.cpp
 * @brief Data compression utilities for efficient transmission
 * 
 * Provides image compression and metadata optimization for
 * bandwidth-limited cellular and satellite connections.
 */

#include "data_compression.h"
#include <Arduino.h>

DataCompression::DataCompression() {
    // Initialize compression parameters
}

size_t DataCompression::compressImage(const uint8_t* imageData, size_t imageSize, 
                                    uint8_t* compressedData, size_t maxCompressedSize,
                                    CompressionLevel level) {
    if (!imageData || !compressedData || imageSize == 0) {
        return 0;
    }
    
    // Simple run-length encoding for demonstration
    // In a real implementation, you would use JPEG compression or similar
    size_t compressedSize = 0;
    size_t i = 0;
    
    while (i < imageSize && compressedSize < maxCompressedSize - 2) {
        uint8_t currentByte = imageData[i];
        uint8_t count = 1;
        
        // Count consecutive identical bytes
        while (i + count < imageSize && 
               imageData[i + count] == currentByte && 
               count < RLE_MAX_COUNT) {
            count++;
        }
        
        if (count > COMPRESSION_MIN_EFFICIENCY || currentByte == 0xFF) {
            // Use RLE encoding
            compressedData[compressedSize++] = 0xFF; // Escape sequence
            compressedData[compressedSize++] = count;
            compressedData[compressedSize++] = currentByte;
        } else {
            // Copy bytes directly
            for (uint8_t j = 0; j < count && compressedSize < maxCompressedSize; j++) {
                compressedData[compressedSize++] = currentByte;
            }
        }
        
        i += count;
    }
    
    float compressionRatio = (float)compressedSize / imageSize;
    Serial.printf("Image compressed: %d -> %d bytes (%.1f%%)\n", 
                  imageSize, compressedSize, compressionRatio * 100);
    
    return compressedSize;
}

size_t DataCompression::decompressImage(const uint8_t* compressedData, size_t compressedSize,
                                      uint8_t* imageData, size_t maxImageSize) {
    if (!compressedData || !imageData || compressedSize == 0) {
        return 0;
    }
    
    size_t imageSize = 0;
    size_t i = 0;
    
    while (i < compressedSize && imageSize < maxImageSize) {
        if (compressedData[i] == 0xFF && i + 2 < compressedSize) {
            // RLE sequence
            uint8_t count = compressedData[i + 1];
            uint8_t value = compressedData[i + 2];
            
            for (uint8_t j = 0; j < count && imageSize < maxImageSize; j++) {
                imageData[imageSize++] = value;
            }
            
            i += 3;
        } else {
            // Direct copy
            imageData[imageSize++] = compressedData[i++];
        }
    }
    
    return imageSize;
}

String DataCompression::createMetadataPacket(const CameraMetadata& metadata, 
                                           MetadataLevel level) {
    String packet = "";
    
    switch (level) {
        case METADATA_MINIMAL:
            packet = String(metadata.timestamp) + "," +
                    String(metadata.batteryLevel) + "," +
                    String(metadata.motionDetected ? 1 : 0);
            break;
            
        case METADATA_STANDARD:
            packet = String(metadata.timestamp) + "," +
                    String(metadata.batteryLevel) + "," +
                    String(metadata.motionDetected ? 1 : 0) + "," +
                    String(metadata.temperature, 1) + "," +
                    String(metadata.humidity, 1);
            break;
            
        case METADATA_DETAILED:
            packet = String(metadata.timestamp) + "," +
                    String(metadata.batteryLevel) + "," +
                    String(metadata.motionDetected ? 1 : 0) + "," +
                    String(metadata.temperature, 1) + "," +
                    String(metadata.humidity, 1) + "," +
                    String(metadata.lightLevel) + "," +
                    String(metadata.imageWidth) + "," +
                    String(metadata.imageHeight) + "," +
                    String(metadata.compressionRatio, 2);
            break;
    }
    
    return packet;
}

CameraMetadata DataCompression::parseMetadataPacket(const String& packet) {
    CameraMetadata metadata = {};
    
    int lastIndex = 0;
    int fieldIndex = 0;
    
    while (lastIndex < packet.length()) {
        int commaIndex = packet.indexOf(',', lastIndex);
        if (commaIndex == -1) commaIndex = packet.length();
        
        String field = packet.substring(lastIndex, commaIndex);
        
        switch (fieldIndex) {
            case 0: metadata.timestamp = field.toInt(); break;
            case 1: metadata.batteryLevel = field.toInt(); break;
            case 2: metadata.motionDetected = field.toInt() == 1; break;
            case 3: metadata.temperature = field.toFloat(); break;
            case 4: metadata.humidity = field.toFloat(); break;
            case 5: metadata.lightLevel = field.toInt(); break;
            case 6: metadata.imageWidth = field.toInt(); break;
            case 7: metadata.imageHeight = field.toInt(); break;
            case 8: metadata.compressionRatio = field.toFloat(); break;
        }
        
        fieldIndex++;
        lastIndex = commaIndex + 1;
    }
    
    return metadata;
}

size_t DataCompression::createThumbnail(const uint8_t* imageData, size_t imageSize,
                                      uint8_t* thumbnailData, size_t maxThumbnailSize,
                                      int targetWidth, int targetHeight) {
    // Simplified thumbnail creation - decimation
    // In a real implementation, you would use proper image scaling algorithms
    
    if (!imageData || !thumbnailData || imageSize == 0) {
        return 0;
    }
    
    // Assume original image dimensions (would normally be parsed from header)
    int originalWidth = 640;  // Default camera resolution
    int originalHeight = 480;
    
    int skipX = originalWidth / targetWidth;
    int skipY = originalHeight / targetHeight;
    
    if (skipX < 1) skipX = 1;
    if (skipY < 1) skipY = 1;
    
    size_t thumbnailSize = 0;
    
    for (int y = 0; y < originalHeight && y < targetHeight * skipY; y += skipY) {
        for (int x = 0; x < originalWidth && x < targetWidth * skipX; x += skipX) {
            if (thumbnailSize < maxThumbnailSize) {
                size_t pixelIndex = (y * originalWidth + x);
                if (pixelIndex < imageSize) {
                    thumbnailData[thumbnailSize++] = imageData[pixelIndex];
                }
            }
        }
    }
    
    Serial.printf("Thumbnail created: %dx%d -> %dx%d (%d bytes)\n",
                  originalWidth, originalHeight, targetWidth, targetHeight, thumbnailSize);
    
    return thumbnailSize;
}

String DataCompression::encodeBase64(const uint8_t* data, size_t length) {
    const char* chars = BASE64_CHARS;
    String encoded = "";
    
    for (size_t i = 0; i < length; i += 3) {
        uint32_t value = 0;
        int padding = 0;
        
        // Pack 3 bytes into 24 bits
        for (int j = 0; j < 3; j++) {
            if (i + j < length) {
                value = (value << 8) | data[i + j];
            } else {
                value = value << 8;
                padding++;
            }
        }
        
        // Extract 4 6-bit values
        for (int j = 3; j >= 0; j--) {
            if (j <= padding) {
                encoded += '=';
            } else {
                encoded += chars[(value >> (j * 6)) & 0x3F];
            }
        }
    }
    
    return encoded;
}

size_t DataCompression::decodeBase64(const String& encoded, uint8_t* data, size_t maxLength) {
    size_t decodedLength = 0;
    size_t encodedLength = encoded.length();
    
    for (size_t i = 0; i < encodedLength && decodedLength < maxLength; i += 4) {
        uint32_t value = 0;
        int validChars = 0;
        
        // Decode 4 characters into 24 bits
        for (int j = 0; j < 4; j++) {
            char c = encoded.charAt(i + j);
            uint8_t val = 0;
            
            if (c >= 'A' && c <= 'Z') {
                val = c - 'A';
                validChars++;
            } else if (c >= 'a' && c <= 'z') {
                val = c - 'a' + 26;
                validChars++;
            } else if (c >= '0' && c <= '9') {
                val = c - '0' + 52;
                validChars++;
            } else if (c == '+') {
                val = 62;
                validChars++;
            } else if (c == '/') {
                val = 63;
                validChars++;
            } else if (c == '=') {
                break;
            }
            
            value = (value << 6) | val;
        }
        
        // Extract bytes
        for (int j = 2; j >= 0 && decodedLength < maxLength && j < validChars - 1; j--) {
            data[decodedLength++] = (value >> (j * 8)) & 0xFF;
        }
    }
    
    return decodedLength;
}

size_t DataCompression::optimizeForSatellite(const uint8_t* data, size_t dataSize,
                                           uint8_t* optimizedData, size_t maxOptimizedSize) {
    // Optimize data for satellite transmission (160-byte message limit)
    if (dataSize <= 160) {
        // Data fits in one message, copy directly
        size_t copySize = min(dataSize, maxOptimizedSize);
        memcpy(optimizedData, data, copySize);
        return copySize;
    }
    
    // Data too large, create summary
    size_t optimizedSize = 0;
    
    // Header: data type and original size
    optimizedData[optimizedSize++] = 0x01; // Data type: image summary
    optimizedData[optimizedSize++] = (dataSize >> 8) & 0xFF;
    optimizedData[optimizedSize++] = dataSize & 0xFF;
    
    // Sample key bytes from the data
    size_t sampleInterval = dataSize / 150; // Leave room for header
    if (sampleInterval < 1) sampleInterval = 1;
    
    for (size_t i = 0; i < dataSize && optimizedSize < maxOptimizedSize - 1; i += sampleInterval) {
        optimizedData[optimizedSize++] = data[i];
    }
    
    Serial.printf("Satellite optimization: %d -> %d bytes\n", dataSize, optimizedSize);
    return optimizedSize;
}

size_t DataCompression::optimizeForCellular(const uint8_t* data, size_t dataSize,
                                          uint8_t* optimizedData, size_t maxOptimizedSize,
                                          int compressionLevel) {
    // Apply different compression based on level
    switch (compressionLevel) {
        case 1: // Light compression
            return compressImage(data, dataSize, optimizedData, maxOptimizedSize, COMPRESSION_LOW);
            
        case 2: // Medium compression
            return compressImage(data, dataSize, optimizedData, maxOptimizedSize, COMPRESSION_MEDIUM);
            
        case 3: // Heavy compression
            return compressImage(data, dataSize, optimizedData, maxOptimizedSize, COMPRESSION_HIGH);
            
        default: // No compression
            size_t copySize = min(dataSize, maxOptimizedSize);
            memcpy(optimizedData, data, copySize);
            return copySize;
    }
}