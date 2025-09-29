/**
 * @file image_mesh.h
 * @brief Image Transmission over Mesh Network for ESP32WildlifeCAM
 * 
 * Handles efficient transmission of wildlife camera images over LoRa mesh network:
 * - JPEG compression optimization
 * - Packet chunking for large images
 * - Progressive image transmission
 * - Error correction and retransmission
 * - Thumbnail-first delivery
 */

#ifndef IMAGE_MESH_H
#define IMAGE_MESH_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_camera.h>
#include <vector>
#include <map>
#include "mesh_interface.h"

// ===========================
// IMAGE TRANSMISSION STRUCTURES
// ===========================

// Image transmission modes
enum ImageTransmissionMode {
    MODE_THUMBNAIL_ONLY,        // Send only thumbnail
    MODE_PROGRESSIVE,           // Send thumbnail first, then full image
    MODE_FULL_IMMEDIATE,        // Send full image immediately
    MODE_METADATA_ONLY          // Send only image metadata
};

// Image compression levels
enum ImageCompressionLevel {
    COMPRESSION_HIGHEST_QUALITY = 10,   // Largest file, best quality
    COMPRESSION_HIGH_QUALITY = 25,      // Good balance
    COMPRESSION_MEDIUM_QUALITY = 50,    // Default balance
    COMPRESSION_LOW_QUALITY = 75,       // Smaller file, lower quality
    COMPRESSION_LOWEST_QUALITY = 95     // Smallest file, lowest quality
};

// Image chunk structure
struct ImageChunk {
    uint32_t imageId;               // Unique image identifier
    uint16_t chunkIndex;            // Chunk sequence number
    uint16_t totalChunks;           // Total number of chunks
    uint16_t chunkSize;             // Size of this chunk
    uint32_t crc32;                 // CRC32 checksum of chunk data
    std::vector<uint8_t> data;      // Chunk data
    unsigned long timestamp;        // Transmission timestamp
};

// Image metadata
struct ImageMetadata {
    uint32_t imageId;               // Unique identifier
    String filename;                // Original filename
    uint32_t fileSize;              // Total file size in bytes
    uint16_t width;                 // Image width in pixels
    uint16_t height;                // Image height in pixels
    uint8_t jpegQuality;            // JPEG quality setting
    uint16_t totalChunks;           // Number of chunks
    uint16_t chunkSize;             // Size of each chunk (except last)
    uint32_t crc32;                 // CRC32 of entire image
    unsigned long captureTime;      // Image capture timestamp
    float latitude;                 // GPS coordinates (if available)
    float longitude;
    bool motionTriggered;           // Motion triggered capture
    String detectedSpecies;         // AI detected species (if any)
    float detectionConfidence;      // AI detection confidence
};

// Image transmission status
enum ImageTransmissionStatus {
    STATUS_PENDING,                 // Waiting to start transmission
    STATUS_IN_PROGRESS,             // Currently transmitting
    STATUS_COMPLETED,               // Successfully transmitted
    STATUS_FAILED,                  // Transmission failed
    STATUS_CANCELLED,               // Transmission cancelled
    STATUS_PAUSED                   // Transmission paused
};

// Image transmission request
struct ImageTransmissionRequest {
    uint32_t imageId;
    uint32_t destination;           // 0 = broadcast
    ImageTransmissionMode mode;
    ImageCompressionLevel compression;
    uint8_t priority;               // 0-255, higher = more priority
    uint8_t maxRetries;             // Maximum retry attempts per chunk
    uint32_t retryDelay;            // Delay between retries (ms)
    bool requireAck;                // Require chunk acknowledgments
    unsigned long requestTime;      // Request timestamp
};

// Image transmission progress
struct ImageTransmissionProgress {
    uint32_t imageId;
    ImageTransmissionStatus status;
    uint16_t chunksTransmitted;
    uint16_t chunksAcknowledged;
    uint16_t totalChunks;
    uint32_t bytesTransmitted;
    uint32_t totalBytes;
    uint8_t retryCount;
    float progressPercentage;
    unsigned long startTime;
    unsigned long estimatedTimeRemaining;
    uint32_t transmissionRate;      // Bytes per second
};

// Image reception state
struct ImageReceptionState {
    uint32_t imageId;
    ImageMetadata metadata;
    std::vector<bool> chunksReceived;
    std::vector<ImageChunk> chunks;
    uint16_t chunksComplete;
    uint32_t bytesReceived;
    unsigned long lastChunkTime;
    bool metadataReceived;
    bool thumbnailReceived;
    bool imageComplete;
};

// Image transmission configuration
struct ImageMeshConfig {
    ImageTransmissionMode defaultMode;
    ImageCompressionLevel defaultCompression;
    uint16_t maxChunkSize;          // Maximum chunk size in bytes
    uint8_t maxConcurrentTransmissions;
    uint32_t ackTimeout;            // Acknowledgment timeout (ms)
    uint8_t maxRetries;             // Maximum retry attempts
    uint32_t retryDelay;            // Base retry delay (ms)
    bool adaptiveRetry;             // Use adaptive retry intervals
    uint32_t thumbnailMaxSize;      // Maximum thumbnail size
    uint8_t thumbnailQuality;       // Thumbnail JPEG quality
    bool enableProgressiveTransmission;
    bool enableErrorCorrection;
    uint32_t transmissionTimeout;   // Overall transmission timeout
    bool storeThumbnailsLocally;
    bool storeMetadataLocally;
};

// ===========================
// IMAGE MESH CLASS
// ===========================

class ImageMesh {
public:
    ImageMesh();
    ~ImageMesh();
    
    // Initialization and configuration
    bool init(MeshInterface* meshInterface);
    bool configure(const ImageMeshConfig& config);
    void cleanup();
    
    // Image transmission
    uint32_t transmitImage(camera_fb_t* frameBuffer, 
                          uint32_t destination = 0,
                          ImageTransmissionMode mode = MODE_PROGRESSIVE);
    uint32_t transmitImageFile(const String& filename,
                              uint32_t destination = 0,
                              ImageTransmissionMode mode = MODE_PROGRESSIVE);
    bool transmitThumbnail(uint32_t imageId, uint32_t destination = 0);
    bool transmitMetadata(uint32_t imageId, uint32_t destination = 0);
    
    // Transmission control
    bool pauseTransmission(uint32_t imageId);
    bool resumeTransmission(uint32_t imageId);
    bool cancelTransmission(uint32_t imageId);
    bool retryFailedChunks(uint32_t imageId);
    
    // Image reception
    bool requestImage(uint32_t imageId, uint32_t sourceNode);
    bool requestThumbnail(uint32_t imageId, uint32_t sourceNode);
    bool requestMetadata(uint32_t imageId, uint32_t sourceNode);
    bool acknowledgeChunk(uint32_t imageId, uint16_t chunkIndex, uint32_t sourceNode);
    
    // Image management
    std::vector<uint32_t> getAvailableImages() const;
    ImageMetadata getImageMetadata(uint32_t imageId) const;
    bool saveReceivedImage(uint32_t imageId, const String& filename);
    bool deleteImage(uint32_t imageId);
    
    // Progress and status
    ImageTransmissionProgress getTransmissionProgress(uint32_t imageId) const;
    std::vector<ImageTransmissionProgress> getAllTransmissionProgress() const;
    ImageReceptionState getReceptionState(uint32_t imageId) const;
    std::vector<ImageReceptionState> getAllReceptionStates() const;
    
    // Statistics
    struct ImageMeshStatistics {
        uint32_t imagesTransmitted;
        uint32_t imagesReceived;
        uint32_t chunksTransmitted;
        uint32_t chunksReceived;
        uint32_t bytesTransmitted;
        uint32_t bytesReceived;
        uint32_t transmissionErrors;
        uint32_t receptionErrors;
        uint32_t retransmissions;
        float averageTransmissionTime;
        float averageCompressionRatio;
        uint32_t thumbnailsGenerated;
        uint32_t progressiveTransmissions;
    };
    
    ImageMeshStatistics getStatistics() const;
    void resetStatistics();
    
    // Configuration management
    ImageMeshConfig getConfig() const;
    bool setConfig(const ImageMeshConfig& config);
    
    // Processing and maintenance
    void process();                 // Call regularly in main loop
    void performMaintenance();      // Periodic maintenance
    
    // Event callbacks
    typedef void (*ImageTransmittedCallback)(uint32_t imageId, bool success);
    typedef void (*ImageReceivedCallback)(uint32_t imageId, const ImageMetadata& metadata);
    typedef void (*ChunkReceivedCallback)(uint32_t imageId, uint16_t chunkIndex, uint16_t totalChunks);
    typedef void (*TransmissionProgressCallback)(uint32_t imageId, float progressPercentage);
    typedef void (*ErrorCallback)(uint32_t imageId, const String& errorMessage);
    
    void setImageTransmittedCallback(ImageTransmittedCallback callback);
    void setImageReceivedCallback(ImageReceivedCallback callback);
    void setChunkReceivedCallback(ChunkReceivedCallback callback);
    void setTransmissionProgressCallback(TransmissionProgressCallback callback);
    void setErrorCallback(ErrorCallback callback);

private:
    // Internal state
    bool initialized_;
    MeshInterface* meshInterface_;
    ImageMeshConfig config_;
    
    // Image storage and management
    std::map<uint32_t, ImageMetadata> imageMetadata_;
    std::map<uint32_t, std::vector<uint8_t>> imageData_;
    std::map<uint32_t, std::vector<uint8_t>> thumbnailData_;
    
    // Transmission state
    std::map<uint32_t, ImageTransmissionRequest> transmissionRequests_;
    std::map<uint32_t, ImageTransmissionProgress> transmissionProgress_;
    std::map<uint32_t, std::vector<ImageChunk>> transmissionChunks_;
    
    // Reception state
    std::map<uint32_t, ImageReceptionState> receptionStates_;
    
    // Timing and scheduling
    unsigned long lastMaintenanceTime_;
    std::map<uint32_t, unsigned long> chunkRetryTimes_;
    std::map<uint32_t, unsigned long> ackTimeouts_;
    
    // Statistics
    ImageMeshStatistics statistics_;
    
    // Callbacks
    ImageTransmittedCallback imageTransmittedCallback_;
    ImageReceivedCallback imageReceivedCallback_;
    ChunkReceivedCallback chunkReceivedCallback_;
    TransmissionProgressCallback transmissionProgressCallback_;
    ErrorCallback errorCallback_;
    
    // Internal transmission methods
    uint32_t generateImageId();
    bool startImageTransmission(const ImageTransmissionRequest& request);
    bool processTransmissionQueue();
    bool transmitNextChunk(uint32_t imageId);
    bool transmitChunk(const ImageChunk& chunk, uint32_t destination);
    
    // Image processing methods
    bool createImageChunks(uint32_t imageId, const std::vector<uint8_t>& imageData);
    std::vector<uint8_t> generateThumbnail(const std::vector<uint8_t>& imageData);
    std::vector<uint8_t> compressImage(const std::vector<uint8_t>& imageData, 
                                      ImageCompressionLevel compression);
    
    // Reception methods
    bool processReceivedChunk(const ImageChunk& chunk, uint32_t sourceNode);
    bool assembleImage(uint32_t imageId);
    bool validateImageIntegrity(uint32_t imageId);
    
    // Packet handling
    bool handleImagePacket(const MeshPacket& packet);
    bool handleChunkPacket(const MeshPacket& packet);
    bool handleMetadataPacket(const MeshPacket& packet);
    bool handleAckPacket(const MeshPacket& packet);
    bool handleRequestPacket(const MeshPacket& packet);
    
    // Error handling and retry logic
    bool handleTransmissionError(uint32_t imageId, const String& error);
    bool shouldRetryChunk(uint32_t imageId, uint16_t chunkIndex);
    unsigned long calculateRetryDelay(uint8_t retryCount);
    
    // Utility methods
    uint32_t calculateCRC32(const std::vector<uint8_t>& data);
    bool validateCRC32(const std::vector<uint8_t>& data, uint32_t expectedCRC);
    std::vector<uint8_t> serializeMetadata(const ImageMetadata& metadata);
    ImageMetadata deserializeMetadata(const std::vector<uint8_t>& data);
    std::vector<uint8_t> serializeChunk(const ImageChunk& chunk);
    ImageChunk deserializeChunk(const std::vector<uint8_t>& data);
    
    // Storage management
    bool saveImageToFile(uint32_t imageId, const String& filename);
    bool loadImageFromFile(const String& filename, uint32_t& imageId);
    void cleanupExpiredData();
    void limitMemoryUsage();
    
    // Mesh interface integration
    static void onMeshMessage(const MeshPacket& packet);
    static ImageMesh* instance_;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function
ImageMesh* createImageMesh(MeshInterface* meshInterface);

// Configuration helpers
ImageMeshConfig createDefaultImageMeshConfig();
ImageMeshConfig createLowBandwidthConfig();
ImageMeshConfig createHighQualityConfig();
ImageMeshConfig createBatteryOptimizedConfig();

// Image processing utilities
std::vector<uint8_t> resizeJpegImage(const std::vector<uint8_t>& jpegData, 
                                     uint16_t maxWidth, uint16_t maxHeight);
uint32_t estimateTransmissionTime(uint32_t imageSize, uint32_t chunkSize, 
                                 uint32_t chunkDelay, uint8_t spreadingFactor);
float calculateCompressionRatio(uint32_t originalSize, uint32_t compressedSize);

// Validation functions
bool isValidImageId(uint32_t imageId);
bool isValidChunkIndex(uint16_t chunkIndex, uint16_t totalChunks);
bool isValidImageMetadata(const ImageMetadata& metadata);
bool isValidImageChunk(const ImageChunk& chunk);

// Format utilities
String formatImageSize(uint32_t bytes);
String formatTransmissionRate(uint32_t bytesPerSecond);
String formatImageDimensions(uint16_t width, uint16_t height);
String imageTransmissionStatusToString(ImageTransmissionStatus status);

#endif // IMAGE_MESH_H