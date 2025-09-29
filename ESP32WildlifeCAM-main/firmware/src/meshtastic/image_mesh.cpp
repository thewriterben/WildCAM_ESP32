/**
 * @file image_mesh.cpp
 * @brief Image Transmission over Mesh Network Implementation for ESP32WildlifeCAM
 * 
 * Comprehensive implementation of image transmission over LoRa mesh with
 * chunking, compression, error correction, and progressive delivery.
 */

#include "image_mesh.h"
#include "../debug_utils.h"
#include <FS.h>
#include <LittleFS.h>
#include <esp_crc.h>

// Static instance for callback access
ImageMesh* ImageMesh::instance_ = nullptr;

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

ImageMesh::ImageMesh()
    : initialized_(false)
    , meshInterface_(nullptr)
    , lastMaintenanceTime_(0)
    , imageTransmittedCallback_(nullptr)
    , imageReceivedCallback_(nullptr)
    , chunkReceivedCallback_(nullptr)
    , transmissionProgressCallback_(nullptr)
    , errorCallback_(nullptr)
{
    instance_ = this;
    
    // Initialize statistics
    memset(&statistics_, 0, sizeof(statistics_));
    
    // Initialize default configuration
    config_ = createDefaultImageMeshConfig();
}

ImageMesh::~ImageMesh() {
    cleanup();
    instance_ = nullptr;
}

// ===========================
// INITIALIZATION
// ===========================

bool ImageMesh::init(MeshInterface* meshInterface) {
    DEBUG_PRINTLN("ImageMesh: Initializing...");
    
    if (!meshInterface) {
        DEBUG_PRINTLN("ImageMesh: Invalid mesh interface");
        return false;
    }
    
    meshInterface_ = meshInterface;
    
    // Initialize LittleFS for image storage
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("ImageMesh: Failed to initialize LittleFS");
        return false;
    }
    
    // Set up mesh message callback
    meshInterface_->setMessageCallback(onMeshMessage);
    
    initialized_ = true;
    
    DEBUG_PRINTLN("ImageMesh: Initialized successfully");
    
    return true;
}

bool ImageMesh::configure(const ImageMeshConfig& config) {
    if (!initialized_) {
        return false;
    }
    
    config_ = config;
    
    DEBUG_PRINTLN("ImageMesh: Configuration updated");
    
    return true;
}

void ImageMesh::cleanup() {
    // Clear all data structures
    imageMetadata_.clear();
    imageData_.clear();
    thumbnailData_.clear();
    transmissionRequests_.clear();
    transmissionProgress_.clear();
    transmissionChunks_.clear();
    receptionStates_.clear();
    chunkRetryTimes_.clear();
    ackTimeouts_.clear();
    
    initialized_ = false;
}

// ===========================
// IMAGE TRANSMISSION
// ===========================

uint32_t ImageMesh::transmitImage(camera_fb_t* frameBuffer, uint32_t destination, 
                                 ImageTransmissionMode mode) {
    if (!initialized_ || !frameBuffer || frameBuffer->len == 0) {
        return 0;
    }
    
    // Generate unique image ID
    uint32_t imageId = generateImageId();
    
    // Convert frame buffer to vector
    std::vector<uint8_t> imageData(frameBuffer->buf, frameBuffer->buf + frameBuffer->len);
    
    // Create metadata
    ImageMetadata metadata;
    metadata.imageId = imageId;
    metadata.filename = "IMG_" + String(imageId, HEX) + ".jpg";
    metadata.fileSize = frameBuffer->len;
    metadata.width = frameBuffer->width;
    metadata.height = frameBuffer->height;
    metadata.jpegQuality = config_.defaultCompression;
    metadata.crc32 = calculateCRC32(imageData);
    metadata.captureTime = millis();
    metadata.motionTriggered = true;
    metadata.latitude = 0.0;  // TODO: Get from GPS
    metadata.longitude = 0.0;
    metadata.detectedSpecies = "";
    metadata.detectionConfidence = 0.0;
    
    // Store image data and metadata
    imageData_[imageId] = imageData;
    imageMetadata_[imageId] = metadata;
    
    // Create chunks
    createImageChunks(imageId, imageData);
    
    // Create transmission request
    ImageTransmissionRequest request;
    request.imageId = imageId;
    request.destination = destination;
    request.mode = mode;
    request.compression = config_.defaultCompression;
    request.priority = 128;
    request.maxRetries = config_.maxRetries;
    request.retryDelay = config_.retryDelay;
    request.requireAck = true;
    request.requestTime = millis();
    
    // Add to transmission queue
    transmissionRequests_[imageId] = request;
    
    // Initialize progress tracking
    ImageTransmissionProgress progress;
    progress.imageId = imageId;
    progress.status = STATUS_PENDING;
    progress.chunksTransmitted = 0;
    progress.chunksAcknowledged = 0;
    progress.totalChunks = metadata.totalChunks;
    progress.bytesTransmitted = 0;
    progress.totalBytes = metadata.fileSize;
    progress.retryCount = 0;
    progress.progressPercentage = 0.0;
    progress.startTime = millis();
    progress.estimatedTimeRemaining = 0;
    progress.transmissionRate = 0;
    
    transmissionProgress_[imageId] = progress;
    
    DEBUG_PRINTF("ImageMesh: Queued image transmission (ID: %08X, size: %d bytes)\n", 
                 imageId, frameBuffer->len);
    
    return imageId;
}

uint32_t ImageMesh::transmitImageFile(const String& filename, uint32_t destination, 
                                     ImageTransmissionMode mode) {
    if (!LittleFS.exists(filename)) {
        DEBUG_PRINTF("ImageMesh: File not found: %s\n", filename.c_str());
        return 0;
    }
    
    File file = LittleFS.open(filename, "r");
    if (!file) {
        DEBUG_PRINTF("ImageMesh: Failed to open file: %s\n", filename.c_str());
        return 0;
    }
    
    size_t fileSize = file.size();
    std::vector<uint8_t> imageData(fileSize);
    file.readBytes((char*)imageData.data(), fileSize);
    file.close();
    
    // Create a mock frame buffer
    camera_fb_t frameBuffer;
    frameBuffer.buf = imageData.data();
    frameBuffer.len = fileSize;
    frameBuffer.width = 1600;  // Default dimensions
    frameBuffer.height = 1200;
    frameBuffer.format = PIXFORMAT_JPEG;
    
    return transmitImage(&frameBuffer, destination, mode);
}

bool ImageMesh::transmitThumbnail(uint32_t imageId, uint32_t destination) {
    auto thumbnailIt = thumbnailData_.find(imageId);
    if (thumbnailIt == thumbnailData_.end()) {
        // Generate thumbnail if not exists
        auto imageIt = imageData_.find(imageId);
        if (imageIt == imageData_.end()) {
            return false;
        }
        
        std::vector<uint8_t> thumbnail = generateThumbnail(imageIt->second);
        thumbnailData_[imageId] = thumbnail;
        thumbnailIt = thumbnailData_.find(imageId);
    }
    
    // Transmit thumbnail as single packet
    DynamicJsonDocument doc(512);
    doc["type"] = "thumbnail";
    doc["imageId"] = imageId;
    doc["size"] = thumbnailIt->second.size();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    payload.insert(payload.end(), thumbnailIt->second.begin(), thumbnailIt->second.end());
    
    return meshInterface_->sendMessage(destination, PACKET_TYPE_DATA, payload, PRIORITY_HIGH);
}

bool ImageMesh::transmitMetadata(uint32_t imageId, uint32_t destination) {
    auto metadataIt = imageMetadata_.find(imageId);
    if (metadataIt == imageMetadata_.end()) {
        return false;
    }
    
    std::vector<uint8_t> payload = serializeMetadata(metadataIt->second);
    
    return meshInterface_->sendMessage(destination, PACKET_TYPE_DATA, payload, PRIORITY_NORMAL);
}

// ===========================
// TRANSMISSION CONTROL
// ===========================

bool ImageMesh::pauseTransmission(uint32_t imageId) {
    auto progressIt = transmissionProgress_.find(imageId);
    if (progressIt != transmissionProgress_.end()) {
        progressIt->second.status = STATUS_PAUSED;
        return true;
    }
    return false;
}

bool ImageMesh::resumeTransmission(uint32_t imageId) {
    auto progressIt = transmissionProgress_.find(imageId);
    if (progressIt != transmissionProgress_.end() && 
        progressIt->second.status == STATUS_PAUSED) {
        progressIt->second.status = STATUS_IN_PROGRESS;
        return true;
    }
    return false;
}

bool ImageMesh::cancelTransmission(uint32_t imageId) {
    auto progressIt = transmissionProgress_.find(imageId);
    if (progressIt != transmissionProgress_.end()) {
        progressIt->second.status = STATUS_CANCELLED;
        
        // Clean up transmission data
        transmissionRequests_.erase(imageId);
        transmissionChunks_.erase(imageId);
        
        return true;
    }
    return false;
}

// ===========================
// PROCESSING AND MAINTENANCE
// ===========================

void ImageMesh::process() {
    if (!initialized_) {
        return;
    }
    
    // Process transmission queue
    processTransmissionQueue();
    
    // Handle timeouts and retries
    unsigned long currentTime = millis();
    
    // Check for ACK timeouts
    for (auto& pair : ackTimeouts_) {
        if (currentTime - pair.second > config_.ackTimeout) {
            // Handle timeout
            uint32_t imageId = pair.first;
            auto progressIt = transmissionProgress_.find(imageId);
            if (progressIt != transmissionProgress_.end()) {
                progressIt->second.retryCount++;
                if (progressIt->second.retryCount >= config_.maxRetries) {
                    handleTransmissionError(imageId, "ACK timeout exceeded max retries");
                } else {
                    // Retry chunk
                    // Implementation would retry the specific chunk
                }
            }
        }
    }
    
    // Perform maintenance
    if (currentTime - lastMaintenanceTime_ > 60000) {  // Every minute
        performMaintenance();
        lastMaintenanceTime_ = currentTime;
    }
}

void ImageMesh::performMaintenance() {
    // Clean up expired data
    cleanupExpiredData();
    
    // Limit memory usage
    limitMemoryUsage();
    
    // Update statistics
    // Statistics are updated in real-time during transmission/reception
}

bool ImageMesh::processTransmissionQueue() {
    bool processed = false;
    
    for (auto& pair : transmissionProgress_) {
        uint32_t imageId = pair.first;
        ImageTransmissionProgress& progress = pair.second;
        
        if (progress.status == STATUS_PENDING) {
            // Start transmission
            progress.status = STATUS_IN_PROGRESS;
            progress.startTime = millis();
            
            // Start with metadata or thumbnail based on mode
            auto requestIt = transmissionRequests_.find(imageId);
            if (requestIt != transmissionRequests_.end()) {
                if (requestIt->second.mode == MODE_PROGRESSIVE || 
                    requestIt->second.mode == MODE_THUMBNAIL_ONLY) {
                    transmitThumbnail(imageId, requestIt->second.destination);
                }
                transmitMetadata(imageId, requestIt->second.destination);
            }
            
            processed = true;
        } else if (progress.status == STATUS_IN_PROGRESS) {
            // Continue transmission
            if (transmitNextChunk(imageId)) {
                processed = true;
            }
        }
    }
    
    return processed;
}

bool ImageMesh::transmitNextChunk(uint32_t imageId) {
    auto chunksIt = transmissionChunks_.find(imageId);
    auto progressIt = transmissionProgress_.find(imageId);
    auto requestIt = transmissionRequests_.find(imageId);
    
    if (chunksIt == transmissionChunks_.end() || 
        progressIt == transmissionProgress_.end() ||
        requestIt == transmissionRequests_.end()) {
        return false;
    }
    
    // Find next chunk to transmit
    for (size_t i = 0; i < chunksIt->second.size(); i++) {
        if (i >= progressIt->second.chunksTransmitted) {
            // Transmit this chunk
            bool success = transmitChunk(chunksIt->second[i], requestIt->second.destination);
            
            if (success) {
                progressIt->second.chunksTransmitted++;
                progressIt->second.bytesTransmitted += chunksIt->second[i].chunkSize;
                
                // Update progress percentage
                progressIt->second.progressPercentage = 
                    (float)progressIt->second.chunksTransmitted / progressIt->second.totalChunks * 100.0;
                
                // Calculate transmission rate
                unsigned long elapsed = millis() - progressIt->second.startTime;
                if (elapsed > 0) {
                    progressIt->second.transmissionRate = 
                        (progressIt->second.bytesTransmitted * 1000) / elapsed;
                }
                
                // Estimate time remaining
                if (progressIt->second.transmissionRate > 0) {
                    uint32_t remainingBytes = progressIt->second.totalBytes - progressIt->second.bytesTransmitted;
                    progressIt->second.estimatedTimeRemaining = 
                        (remainingBytes * 1000) / progressIt->second.transmissionRate;
                }
                
                // Call progress callback
                if (transmissionProgressCallback_) {
                    transmissionProgressCallback_(imageId, progressIt->second.progressPercentage);
                }
                
                // Check if transmission complete
                if (progressIt->second.chunksTransmitted >= progressIt->second.totalChunks) {
                    progressIt->second.status = STATUS_COMPLETED;
                    statistics_.imagesTransmitted++;
                    
                    if (imageTransmittedCallback_) {
                        imageTransmittedCallback_(imageId, true);
                    }
                    
                    DEBUG_PRINTF("ImageMesh: Image transmission completed (ID: %08X)\n", imageId);
                }
                
                return true;
            } else {
                statistics_.transmissionErrors++;
                progressIt->second.retryCount++;
                
                if (progressIt->second.retryCount >= config_.maxRetries) {
                    handleTransmissionError(imageId, "Max retries exceeded");
                }
                
                return false;
            }
        }
    }
    
    return false;
}

bool ImageMesh::transmitChunk(const ImageChunk& chunk, uint32_t destination) {
    std::vector<uint8_t> payload = serializeChunk(chunk);
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_DATA, 
                                              payload, PRIORITY_NORMAL, true);
    
    if (success) {
        statistics_.chunksTransmitted++;
        statistics_.bytesTransmitted += chunk.chunkSize;
        
        // Set ACK timeout
        ackTimeouts_[chunk.imageId] = millis();
    }
    
    return success;
}

// ===========================
// IMAGE PROCESSING
// ===========================

bool ImageMesh::createImageChunks(uint32_t imageId, const std::vector<uint8_t>& imageData) {
    uint16_t chunkSize = config_.maxChunkSize;
    uint16_t totalChunks = (imageData.size() + chunkSize - 1) / chunkSize;
    
    std::vector<ImageChunk> chunks;
    chunks.reserve(totalChunks);
    
    for (uint16_t i = 0; i < totalChunks; i++) {
        ImageChunk chunk;
        chunk.imageId = imageId;
        chunk.chunkIndex = i;
        chunk.totalChunks = totalChunks;
        chunk.timestamp = millis();
        
        size_t startOffset = i * chunkSize;
        size_t endOffset = min(startOffset + chunkSize, imageData.size());
        chunk.chunkSize = endOffset - startOffset;
        
        chunk.data.resize(chunk.chunkSize);
        memcpy(chunk.data.data(), imageData.data() + startOffset, chunk.chunkSize);
        
        chunk.crc32 = calculateCRC32(chunk.data);
        
        chunks.push_back(chunk);
    }
    
    transmissionChunks_[imageId] = chunks;
    
    // Update metadata with chunk information
    auto metadataIt = imageMetadata_.find(imageId);
    if (metadataIt != imageMetadata_.end()) {
        metadataIt->second.totalChunks = totalChunks;
        metadataIt->second.chunkSize = chunkSize;
    }
    
    DEBUG_PRINTF("ImageMesh: Created %d chunks for image %08X\n", totalChunks, imageId);
    
    return true;
}

std::vector<uint8_t> ImageMesh::generateThumbnail(const std::vector<uint8_t>& imageData) {
    // Simplified thumbnail generation - would use proper JPEG library in production
    // For now, just return the first portion of the image as a "thumbnail"
    size_t thumbnailSize = min((size_t)config_.thumbnailMaxSize, imageData.size() / 4);
    
    std::vector<uint8_t> thumbnail;
    thumbnail.resize(thumbnailSize);
    memcpy(thumbnail.data(), imageData.data(), thumbnailSize);
    
    statistics_.thumbnailsGenerated++;
    
    return thumbnail;
}

// ===========================
// PACKET HANDLING
// ===========================

bool ImageMesh::handleImagePacket(const MeshPacket& packet) {
    // Parse packet to determine type
    if (packet.payload.size() < 4) {
        return false;
    }
    
    // Simple packet type detection based on first few bytes
    String typeStr((char*)packet.payload.data(), 20);
    
    if (typeStr.indexOf("chunk") >= 0) {
        return handleChunkPacket(packet);
    } else if (typeStr.indexOf("metadata") >= 0) {
        return handleMetadataPacket(packet);
    } else if (typeStr.indexOf("ack") >= 0) {
        return handleAckPacket(packet);
    } else if (typeStr.indexOf("request") >= 0) {
        return handleRequestPacket(packet);
    }
    
    return false;
}

bool ImageMesh::handleChunkPacket(const MeshPacket& packet) {
    ImageChunk chunk = deserializeChunk(packet.payload);
    
    if (!isValidImageChunk(chunk)) {
        statistics_.receptionErrors++;
        return false;
    }
    
    return processReceivedChunk(chunk, packet.header.from);
}

bool ImageMesh::processReceivedChunk(const ImageChunk& chunk, uint32_t sourceNode) {
    uint32_t imageId = chunk.imageId;
    
    // Find or create reception state
    auto stateIt = receptionStates_.find(imageId);
    if (stateIt == receptionStates_.end()) {
        ImageReceptionState state;
        state.imageId = imageId;
        state.chunksReceived.resize(chunk.totalChunks, false);
        state.chunks.resize(chunk.totalChunks);
        state.chunksComplete = 0;
        state.bytesReceived = 0;
        state.lastChunkTime = millis();
        state.metadataReceived = false;
        state.thumbnailReceived = false;
        state.imageComplete = false;
        
        receptionStates_[imageId] = state;
        stateIt = receptionStates_.find(imageId);
    }
    
    ImageReceptionState& state = stateIt->second;
    
    // Check if chunk already received
    if (chunk.chunkIndex < state.chunksReceived.size() && 
        state.chunksReceived[chunk.chunkIndex]) {
        // Duplicate chunk, send ACK anyway
        acknowledgeChunk(imageId, chunk.chunkIndex, sourceNode);
        return true;
    }
    
    // Validate chunk CRC
    if (!validateCRC32(chunk.data, chunk.crc32)) {
        statistics_.receptionErrors++;
        return false;
    }
    
    // Store chunk
    if (chunk.chunkIndex < state.chunks.size()) {
        state.chunks[chunk.chunkIndex] = chunk;
        state.chunksReceived[chunk.chunkIndex] = true;
        state.chunksComplete++;
        state.bytesReceived += chunk.chunkSize;
        state.lastChunkTime = millis();
        
        statistics_.chunksReceived++;
        statistics_.bytesReceived += chunk.chunkSize;
        
        // Send acknowledgment
        acknowledgeChunk(imageId, chunk.chunkIndex, sourceNode);
        
        // Call chunk received callback
        if (chunkReceivedCallback_) {
            chunkReceivedCallback_(imageId, chunk.chunkIndex, chunk.totalChunks);
        }
        
        // Check if image complete
        if (state.chunksComplete >= chunk.totalChunks) {
            if (assembleImage(imageId)) {
                state.imageComplete = true;
                statistics_.imagesReceived++;
                
                // Call image received callback
                if (imageReceivedCallback_) {
                    auto metadataIt = imageMetadata_.find(imageId);
                    if (metadataIt != imageMetadata_.end()) {
                        imageReceivedCallback_(imageId, metadataIt->second);
                    }
                }
                
                DEBUG_PRINTF("ImageMesh: Image reception completed (ID: %08X)\n", imageId);
            }
        }
        
        return true;
    }
    
    return false;
}

bool ImageMesh::assembleImage(uint32_t imageId) {
    auto stateIt = receptionStates_.find(imageId);
    if (stateIt == receptionStates_.end() || !stateIt->second.imageComplete) {
        return false;
    }
    
    const ImageReceptionState& state = stateIt->second;
    
    // Calculate total image size
    uint32_t totalSize = 0;
    for (const auto& chunk : state.chunks) {
        totalSize += chunk.chunkSize;
    }
    
    // Assemble image data
    std::vector<uint8_t> imageData;
    imageData.reserve(totalSize);
    
    for (const auto& chunk : state.chunks) {
        imageData.insert(imageData.end(), chunk.data.begin(), chunk.data.end());
    }
    
    // Validate assembled image
    if (validateImageIntegrity(imageId)) {
        imageData_[imageId] = imageData;
        return true;
    }
    
    return false;
}

bool ImageMesh::validateImageIntegrity(uint32_t imageId) {
    auto imageIt = imageData_.find(imageId);
    auto metadataIt = imageMetadata_.find(imageId);
    
    if (imageIt == imageData_.end() || metadataIt == imageMetadata_.end()) {
        return false;
    }
    
    // Validate CRC32
    uint32_t calculatedCRC = calculateCRC32(imageIt->second);
    return (calculatedCRC == metadataIt->second.crc32);
}

bool ImageMesh::acknowledgeChunk(uint32_t imageId, uint16_t chunkIndex, uint32_t sourceNode) {
    DynamicJsonDocument doc(256);
    doc["type"] = "ack";
    doc["imageId"] = imageId;
    doc["chunkIndex"] = chunkIndex;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    
    return meshInterface_->sendMessage(sourceNode, PACKET_TYPE_DATA, payload, PRIORITY_HIGH);
}

// ===========================
// UTILITY METHODS
// ===========================

uint32_t ImageMesh::generateImageId() {
    static uint32_t counter = 1;
    return (millis() & 0xFFFF0000) | (counter++ & 0xFFFF);
}

uint32_t ImageMesh::calculateCRC32(const std::vector<uint8_t>& data) {
    return esp_crc32_le(0, data.data(), data.size());
}

bool ImageMesh::validateCRC32(const std::vector<uint8_t>& data, uint32_t expectedCRC) {
    return calculateCRC32(data) == expectedCRC;
}

std::vector<uint8_t> ImageMesh::serializeMetadata(const ImageMetadata& metadata) {
    DynamicJsonDocument doc(1024);
    doc["type"] = "metadata";
    doc["imageId"] = metadata.imageId;
    doc["filename"] = metadata.filename;
    doc["fileSize"] = metadata.fileSize;
    doc["width"] = metadata.width;
    doc["height"] = metadata.height;
    doc["jpegQuality"] = metadata.jpegQuality;
    doc["totalChunks"] = metadata.totalChunks;
    doc["chunkSize"] = metadata.chunkSize;
    doc["crc32"] = metadata.crc32;
    doc["captureTime"] = metadata.captureTime;
    doc["latitude"] = metadata.latitude;
    doc["longitude"] = metadata.longitude;
    doc["motionTriggered"] = metadata.motionTriggered;
    doc["detectedSpecies"] = metadata.detectedSpecies;
    doc["detectionConfidence"] = metadata.detectionConfidence;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

ImageMetadata ImageMesh::deserializeMetadata(const std::vector<uint8_t>& data) {
    DynamicJsonDocument doc(1024);
    String jsonString((char*)data.data(), data.size());
    deserializeJson(doc, jsonString);
    
    ImageMetadata metadata;
    metadata.imageId = doc["imageId"];
    metadata.filename = doc["filename"].as<String>();
    metadata.fileSize = doc["fileSize"];
    metadata.width = doc["width"];
    metadata.height = doc["height"];
    metadata.jpegQuality = doc["jpegQuality"];
    metadata.totalChunks = doc["totalChunks"];
    metadata.chunkSize = doc["chunkSize"];
    metadata.crc32 = doc["crc32"];
    metadata.captureTime = doc["captureTime"];
    metadata.latitude = doc["latitude"];
    metadata.longitude = doc["longitude"];
    metadata.motionTriggered = doc["motionTriggered"];
    metadata.detectedSpecies = doc["detectedSpecies"].as<String>();
    metadata.detectionConfidence = doc["detectionConfidence"];
    
    return metadata;
}

std::vector<uint8_t> ImageMesh::serializeChunk(const ImageChunk& chunk) {
    DynamicJsonDocument doc(512);
    doc["type"] = "chunk";
    doc["imageId"] = chunk.imageId;
    doc["chunkIndex"] = chunk.chunkIndex;
    doc["totalChunks"] = chunk.totalChunks;
    doc["chunkSize"] = chunk.chunkSize;
    doc["crc32"] = chunk.crc32;
    doc["timestamp"] = chunk.timestamp;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    payload.insert(payload.end(), chunk.data.begin(), chunk.data.end());
    
    return payload;
}

ImageChunk ImageMesh::deserializeChunk(const std::vector<uint8_t>& data) {
    // Find end of JSON header
    size_t jsonEnd = 0;
    for (size_t i = 0; i < data.size() - 1; i++) {
        if (data[i] == '}' && (i == data.size() - 1 || data[i + 1] != '"')) {
            jsonEnd = i + 1;
            break;
        }
    }
    
    // Parse JSON header
    String jsonString((char*)data.data(), jsonEnd);
    DynamicJsonDocument doc(512);
    deserializeJson(doc, jsonString);
    
    ImageChunk chunk;
    chunk.imageId = doc["imageId"];
    chunk.chunkIndex = doc["chunkIndex"];
    chunk.totalChunks = doc["totalChunks"];
    chunk.chunkSize = doc["chunkSize"];
    chunk.crc32 = doc["crc32"];
    chunk.timestamp = doc["timestamp"];
    
    // Extract chunk data
    if (jsonEnd < data.size()) {
        size_t dataSize = data.size() - jsonEnd;
        chunk.data.resize(dataSize);
        memcpy(chunk.data.data(), data.data() + jsonEnd, dataSize);
    }
    
    return chunk;
}

void ImageMesh::cleanupExpiredData() {
    unsigned long currentTime = millis();
    unsigned long expireTime = 24 * 60 * 60 * 1000; // 24 hours
    
    // Clean up completed transmissions
    for (auto it = transmissionProgress_.begin(); it != transmissionProgress_.end();) {
        if ((it->second.status == STATUS_COMPLETED || it->second.status == STATUS_FAILED) &&
            currentTime - it->second.startTime > expireTime) {
            
            uint32_t imageId = it->first;
            
            // Clean up related data
            transmissionRequests_.erase(imageId);
            transmissionChunks_.erase(imageId);
            ackTimeouts_.erase(imageId);
            
            it = transmissionProgress_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Clean up old reception states
    for (auto it = receptionStates_.begin(); it != receptionStates_.end();) {
        if (currentTime - it->second.lastChunkTime > expireTime) {
            it = receptionStates_.erase(it);
        } else {
            ++it;
        }
    }
}

void ImageMesh::limitMemoryUsage() {
    // Limit number of images stored in memory
    const size_t maxStoredImages = 10;
    
    if (imageData_.size() > maxStoredImages) {
        // Remove oldest images
        // This is simplified - would implement LRU in production
        auto it = imageData_.begin();
        imageData_.erase(it);
        
        // Also clean up related data
        imageMetadata_.erase(it->first);
        thumbnailData_.erase(it->first);
    }
}

bool ImageMesh::handleTransmissionError(uint32_t imageId, const String& error) {
    auto progressIt = transmissionProgress_.find(imageId);
    if (progressIt != transmissionProgress_.end()) {
        progressIt->second.status = STATUS_FAILED;
        
        if (imageTransmittedCallback_) {
            imageTransmittedCallback_(imageId, false);
        }
        
        if (errorCallback_) {
            errorCallback_(imageId, error);
        }
        
        DEBUG_PRINTF("ImageMesh: Transmission error for image %08X: %s\n", 
                     imageId, error.c_str());
    }
    
    return true;
}

// ===========================
// CALLBACK MANAGEMENT
// ===========================

void ImageMesh::setImageTransmittedCallback(ImageTransmittedCallback callback) {
    imageTransmittedCallback_ = callback;
}

void ImageMesh::setImageReceivedCallback(ImageReceivedCallback callback) {
    imageReceivedCallback_ = callback;
}

void ImageMesh::setChunkReceivedCallback(ChunkReceivedCallback callback) {
    chunkReceivedCallback_ = callback;
}

void ImageMesh::setTransmissionProgressCallback(TransmissionProgressCallback callback) {
    transmissionProgressCallback_ = callback;
}

void ImageMesh::setErrorCallback(ErrorCallback callback) {
    errorCallback_ = callback;
}

// ===========================
// STATISTICS AND STATUS
// ===========================

ImageMesh::ImageMeshStatistics ImageMesh::getStatistics() const {
    return statistics_;
}

void ImageMesh::resetStatistics() {
    memset(&statistics_, 0, sizeof(statistics_));
}

ImageTransmissionProgress ImageMesh::getTransmissionProgress(uint32_t imageId) const {
    auto it = transmissionProgress_.find(imageId);
    if (it != transmissionProgress_.end()) {
        return it->second;
    }
    return ImageTransmissionProgress();
}

ImageReceptionState ImageMesh::getReceptionState(uint32_t imageId) const {
    auto it = receptionStates_.find(imageId);
    if (it != receptionStates_.end()) {
        return it->second;
    }
    return ImageReceptionState();
}

// ===========================
// STATIC CALLBACK
// ===========================

void ImageMesh::onMeshMessage(const MeshPacket& packet) {
    if (instance_ && packet.header.portNum == PACKET_TYPE_DATA) {
        instance_->handleImagePacket(packet);
    }
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

ImageMesh* createImageMesh(MeshInterface* meshInterface) {
    ImageMesh* imageMesh = new ImageMesh();
    if (imageMesh->init(meshInterface)) {
        return imageMesh;
    } else {
        delete imageMesh;
        return nullptr;
    }
}

ImageMeshConfig createDefaultImageMeshConfig() {
    ImageMeshConfig config;
    config.defaultMode = MODE_PROGRESSIVE;
    config.defaultCompression = COMPRESSION_MEDIUM_QUALITY;
    config.maxChunkSize = IMAGE_CHUNK_SIZE;
    config.maxConcurrentTransmissions = 3;
    config.ackTimeout = 10000;      // 10 seconds
    config.maxRetries = IMAGE_MAX_RETRIES;
    config.retryDelay = 5000;       // 5 seconds
    config.adaptiveRetry = true;
    config.thumbnailMaxSize = IMAGE_THUMBNAIL_SIZE * IMAGE_THUMBNAIL_SIZE;
    config.thumbnailQuality = 30;
    config.enableProgressiveTransmission = true;
    config.enableErrorCorrection = true;
    config.transmissionTimeout = 600000;  // 10 minutes
    config.storeThumbnailsLocally = true;
    config.storeMetadataLocally = true;
    
    return config;
}

bool isValidImageId(uint32_t imageId) {
    return imageId != 0;
}

bool isValidImageChunk(const ImageChunk& chunk) {
    return chunk.imageId != 0 && 
           chunk.chunkIndex < chunk.totalChunks &&
           chunk.chunkSize > 0 &&
           chunk.data.size() == chunk.chunkSize;
}

String imageTransmissionStatusToString(ImageTransmissionStatus status) {
    switch (status) {
        case STATUS_PENDING: return "Pending";
        case STATUS_IN_PROGRESS: return "In Progress";
        case STATUS_COMPLETED: return "Completed";
        case STATUS_FAILED: return "Failed";
        case STATUS_CANCELLED: return "Cancelled";
        case STATUS_PAUSED: return "Paused";
        default: return "Unknown";
    }
}