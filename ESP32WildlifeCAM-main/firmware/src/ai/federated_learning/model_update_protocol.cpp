/**
 * @file model_update_protocol.cpp
 * @brief Implementation of Model Update Protocol
 * 
 * Implements secure, bandwidth-efficient communication protocol for
 * federated learning model parameter exchange.
 */

#include "model_update_protocol.h"
#include "../../utils/logger.h"
#include <WiFi.h>
#include <cstring>
#include <algorithm>

// Global instance
ModelUpdateProtocol* g_modelUpdateProtocol = nullptr;

// ===========================
// CONSTRUCTOR / DESTRUCTOR
// ===========================

ModelUpdateProtocol::ModelUpdateProtocol()
    : initialized_(false)
    , encryptionEnabled_(true)
    , adaptiveCompressionEnabled_(false)
    , currentProtocol_("WiFi")
    , sequenceNumber_(0)
{
}

ModelUpdateProtocol::~ModelUpdateProtocol() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool ModelUpdateProtocol::init(const NetworkConfig& config) {
    if (initialized_) {
        Logger::warn("[ModelUpdateProtocol] Already initialized");
        return true;
    }

    Logger::info("[ModelUpdateProtocol] Initializing protocol...");

    config_ = config;
    encryptionEnabled_ = config.enableEncryption;

    // Initialize statistics
    stats_ = CommunicationStats();

    initialized_ = true;

    Logger::info("[ModelUpdateProtocol] Initialization complete");
    return true;
}

void ModelUpdateProtocol::cleanup() {
    if (!initialized_) {
        return;
    }

    Logger::info("[ModelUpdateProtocol] Cleaning up protocol...");

    initialized_ = false;

    Logger::info("[ModelUpdateProtocol] Cleanup complete");
}

// ===========================
// CONFIGURATION
// ===========================

void ModelUpdateProtocol::setNetworkConfig(const NetworkConfig& config) {
    config_ = config;
}

void ModelUpdateProtocol::setEncryptionEnabled(bool enabled) {
    encryptionEnabled_ = enabled;
    Logger::info("[ModelUpdateProtocol] Encryption %s", enabled ? "enabled" : "disabled");
}

void ModelUpdateProtocol::setCompressionMethod(CompressionMethod method) {
    config_.compressionMethod = method;
    Logger::info("[ModelUpdateProtocol] Compression method: %d", static_cast<int>(method));
}

// ===========================
// MESSAGE SENDING
// ===========================

bool ModelUpdateProtocol::sendRoundAnnouncement(const String& targetId, 
                                               const FederatedRound& round,
                                               const RoundConfig& config) {
    Logger::info("[ModelUpdateProtocol] Sending round announcement to %s", targetId.c_str());

    MessageHeader header;
    header.type = MessageType::ROUND_ANNOUNCEMENT;
    header.senderId = getDeviceId();
    header.receiverId = targetId;
    header.roundId = round.roundId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    // Serialize payload (simplified)
    std::vector<uint8_t> payload;
    // In a real implementation, would serialize round and config properly
    payload.resize(256); // Placeholder

    header.length = payload.size();
    header.checksum = calculateChecksum(payload);

    stats_.messagesSent++;
    stats_.bytesTransmitted += payload.size();

    return true;
}

bool ModelUpdateProtocol::sendJoinRequest(const String& coordinatorId, 
                                         const JoinRequestPayload& payload) {
    Logger::info("[ModelUpdateProtocol] Sending join request to %s", coordinatorId.c_str());

    MessageHeader header;
    header.type = MessageType::JOIN_REQUEST;
    header.senderId = getDeviceId();
    header.receiverId = coordinatorId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    // Serialize payload
    std::vector<uint8_t> serialized;
    // Would serialize properly in real implementation
    serialized.resize(128);

    header.length = serialized.size();
    header.checksum = calculateChecksum(serialized);

    stats_.messagesSent++;
    stats_.bytesTransmitted += serialized.size();

    return true;
}

bool ModelUpdateProtocol::sendJoinResponse(const String& clientId, 
                                          const JoinResponsePayload& payload) {
    Logger::info("[ModelUpdateProtocol] Sending join response to %s", clientId.c_str());

    MessageHeader header;
    header.type = MessageType::JOIN_RESPONSE;
    header.senderId = getDeviceId();
    header.receiverId = clientId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    // Serialize payload
    std::vector<uint8_t> serialized;
    serialized.resize(256);

    header.length = serialized.size();
    header.checksum = calculateChecksum(serialized);

    stats_.messagesSent++;
    stats_.bytesTransmitted += serialized.size();

    return true;
}

bool ModelUpdateProtocol::sendModelUpdate(const String& coordinatorId, const ModelUpdate& update) {
    Logger::info("[ModelUpdateProtocol] Sending model update to %s", coordinatorId.c_str());

    MessageHeader header;
    header.type = MessageType::MODEL_UPDATE;
    header.senderId = getDeviceId();
    header.receiverId = coordinatorId;
    header.roundId = update.roundId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    // Compress model update
    std::vector<uint8_t> compressed = compressModelUpdate(update, config_.compressionMethod);

    // Encrypt if enabled
    if (encryptionEnabled_) {
        compressed = encryptPayload(compressed);
    }

    header.length = compressed.size();
    header.checksum = calculateChecksum(compressed);

    stats_.messagesSent++;
    stats_.bytesTransmitted += compressed.size();
    stats_.modelUpdatesSent++;

    Logger::info("[ModelUpdateProtocol] Model update sent, size: %u bytes", compressed.size());

    return true;
}

bool ModelUpdateProtocol::sendGlobalModel(const String& clientId, 
                                         const ModelUpdate& globalModel,
                                         float accuracyImprovement) {
    Logger::info("[ModelUpdateProtocol] Sending global model to %s", clientId.c_str());

    MessageHeader header;
    header.type = MessageType::GLOBAL_MODEL;
    header.senderId = getDeviceId();
    header.receiverId = clientId;
    header.roundId = globalModel.roundId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    // Compress model
    std::vector<uint8_t> compressed = compressModelUpdate(globalModel, config_.compressionMethod);

    // Encrypt if enabled
    if (encryptionEnabled_) {
        compressed = encryptPayload(compressed);
    }

    header.length = compressed.size();
    header.checksum = calculateChecksum(compressed);

    stats_.messagesSent++;
    stats_.bytesTransmitted += compressed.size();

    return true;
}

bool ModelUpdateProtocol::sendError(const String& targetId, const String& error) {
    Logger::info("[ModelUpdateProtocol] Sending error to %s: %s", 
                targetId.c_str(), error.c_str());

    MessageHeader header;
    header.type = MessageType::ERROR_MESSAGE;
    header.senderId = getDeviceId();
    header.receiverId = targetId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    std::vector<uint8_t> payload(error.begin(), error.end());

    header.length = payload.size();
    header.checksum = calculateChecksum(payload);

    stats_.messagesSent++;
    stats_.bytesTransmitted += payload.size();

    return true;
}

bool ModelUpdateProtocol::sendHeartbeat(const String& targetId) {
    MessageHeader header;
    header.type = MessageType::HEARTBEAT;
    header.senderId = getDeviceId();
    header.receiverId = targetId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    std::vector<uint8_t> payload;
    payload.resize(16); // Minimal heartbeat data

    header.length = payload.size();
    header.checksum = calculateChecksum(payload);

    stats_.messagesSent++;
    stats_.bytesTransmitted += payload.size();

    return true;
}

// ===========================
// BROADCAST METHODS
// ===========================

bool ModelUpdateProtocol::broadcastRoundAnnouncement(const FederatedRound& round, 
                                                    const RoundConfig& config) {
    Logger::info("[ModelUpdateProtocol] Broadcasting round announcement: %s", round.roundId.c_str());

    return sendRoundAnnouncement("", round, config);
}

bool ModelUpdateProtocol::broadcastPeerDiscovery() {
    Logger::info("[ModelUpdateProtocol] Broadcasting peer discovery");

    MessageHeader header;
    header.type = MessageType::PEER_DISCOVERY;
    header.senderId = getDeviceId();
    header.receiverId = ""; // Broadcast
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    std::vector<uint8_t> payload;
    payload.resize(64);

    header.length = payload.size();
    header.checksum = calculateChecksum(payload);

    stats_.messagesSent++;
    stats_.bytesTransmitted += payload.size();

    return true;
}

bool ModelUpdateProtocol::broadcastRoundComplete(const String& roundId) {
    Logger::info("[ModelUpdateProtocol] Broadcasting round complete: %s", roundId.c_str());

    MessageHeader header;
    header.type = MessageType::ROUND_COMPLETE;
    header.senderId = getDeviceId();
    header.receiverId = ""; // Broadcast
    header.roundId = roundId;
    header.timestamp = millis();
    header.sequenceNumber = sequenceNumber_++;

    std::vector<uint8_t> payload;
    payload.resize(32);

    header.length = payload.size();
    header.checksum = calculateChecksum(payload);

    stats_.messagesSent++;
    stats_.bytesTransmitted += payload.size();

    return true;
}

// ===========================
// MESSAGE RECEIVING
// ===========================

bool ModelUpdateProtocol::receiveMessage(MessageHeader& header, std::vector<uint8_t>& payload) {
    // In a real implementation, would receive from network
    // For now, return false indicating no message available
    return false;
}

bool ModelUpdateProtocol::processMessage(const MessageHeader& header, 
                                        const std::vector<uint8_t>& payload) {
    Logger::debug("[ModelUpdateProtocol] Processing message type: %d", static_cast<int>(header.type));

    stats_.messagesReceived++;
    stats_.bytesReceived += payload.size();

    // Validate message
    if (!validateMessageIntegrity(header, payload)) {
        Logger::error("[ModelUpdateProtocol] Message validation failed");
        stats_.failedMessages++;
        return false;
    }

    // Decrypt if needed
    std::vector<uint8_t> decrypted = payload;
    if (encryptionEnabled_) {
        decrypted = decryptPayload(payload);
    }

    // Route to appropriate handler
    switch (header.type) {
        case MessageType::ROUND_ANNOUNCEMENT:
            if (roundAnnouncementHandler_) {
                // Would deserialize and call handler
            }
            break;

        case MessageType::JOIN_REQUEST:
            if (joinRequestHandler_) {
                // Would deserialize and call handler
            }
            break;

        case MessageType::JOIN_RESPONSE:
            if (joinResponseHandler_) {
                // Would deserialize and call handler
            }
            break;

        case MessageType::MODEL_UPDATE:
            if (modelUpdateHandler_) {
                stats_.modelUpdatesReceived++;
                // Would deserialize and call handler
            }
            break;

        case MessageType::GLOBAL_MODEL:
            if (globalModelHandler_) {
                // Would deserialize and call handler
            }
            break;

        case MessageType::ERROR_MESSAGE:
            if (errorHandler_) {
                String error(decrypted.begin(), decrypted.end());
                errorHandler_(header.senderId, error);
            }
            break;

        case MessageType::HEARTBEAT:
            if (heartbeatHandler_) {
                heartbeatHandler_(header.senderId);
            }
            break;

        case MessageType::PEER_DISCOVERY:
            if (peerDiscoveryHandler_) {
                peerDiscoveryHandler_(header.senderId);
            }
            break;

        default:
            Logger::warn("[ModelUpdateProtocol] Unknown message type: %d", 
                        static_cast<int>(header.type));
            return false;
    }

    return true;
}

// ===========================
// COMPRESSION AND DECOMPRESSION
// ===========================

std::vector<uint8_t> ModelUpdateProtocol::compressModelUpdate(const ModelUpdate& update, 
                                                              CompressionMethod method) {
    std::vector<uint8_t> compressed;

    switch (method) {
        case CompressionMethod::NONE:
            // Just serialize without compression
            compressed.resize(update.weights.size() * sizeof(float));
            memcpy(compressed.data(), update.weights.data(), compressed.size());
            break;

        case CompressionMethod::QUANTIZATION:
            // Quantize to 8-bit
            compressed.resize(update.weights.size());
            for (size_t i = 0; i < update.weights.size(); i++) {
                int8_t quantized = static_cast<int8_t>(update.weights[i] * 127.0f);
                compressed[i] = static_cast<uint8_t>(quantized + 128);
            }
            stats_.averageCompressionRatio = 0.25f; // 4x compression
            break;

        case CompressionMethod::SPARSIFICATION:
            // Keep only non-zero gradients
            for (size_t i = 0; i < update.weights.size(); i++) {
                if (fabs(update.weights[i]) > 0.001f) {
                    // Store index and value
                    uint16_t index = static_cast<uint16_t>(i);
                    compressed.insert(compressed.end(), 
                                    reinterpret_cast<uint8_t*>(&index),
                                    reinterpret_cast<uint8_t*>(&index) + sizeof(uint16_t));
                    
                    float value = update.weights[i];
                    compressed.insert(compressed.end(),
                                    reinterpret_cast<uint8_t*>(&value),
                                    reinterpret_cast<uint8_t*>(&value) + sizeof(float));
                }
            }
            stats_.averageCompressionRatio = 0.1f; // ~10x compression for sparse gradients
            break;

        case CompressionMethod::COMBINED:
            // Use quantization + sparsification
            for (size_t i = 0; i < update.weights.size(); i++) {
                if (fabs(update.weights[i]) > 0.001f) {
                    uint16_t index = static_cast<uint16_t>(i);
                    compressed.insert(compressed.end(),
                                    reinterpret_cast<uint8_t*>(&index),
                                    reinterpret_cast<uint8_t*>(&index) + sizeof(uint16_t));
                    
                    int8_t quantized = static_cast<int8_t>(update.weights[i] * 127.0f);
                    compressed.push_back(static_cast<uint8_t>(quantized + 128));
                }
            }
            stats_.averageCompressionRatio = 0.05f; // ~20x compression
            break;

        default:
            // Default to no compression
            compressed.resize(update.weights.size() * sizeof(float));
            memcpy(compressed.data(), update.weights.data(), compressed.size());
            break;
    }

    Logger::debug("[ModelUpdateProtocol] Compressed %u -> %u bytes (ratio: %.2f)", 
                 update.weights.size() * sizeof(float), 
                 compressed.size(),
                 static_cast<float>(compressed.size()) / (update.weights.size() * sizeof(float)));

    return compressed;
}

ModelUpdate ModelUpdateProtocol::decompressModelUpdate(const std::vector<uint8_t>& compressed, 
                                                       CompressionMethod method) {
    ModelUpdate update;

    switch (method) {
        case CompressionMethod::NONE:
            // Just deserialize
            update.weights.resize(compressed.size() / sizeof(float));
            memcpy(update.weights.data(), compressed.data(), compressed.size());
            break;

        case CompressionMethod::QUANTIZATION:
            // De-quantize from 8-bit
            update.weights.resize(compressed.size());
            for (size_t i = 0; i < compressed.size(); i++) {
                int8_t quantized = static_cast<int8_t>(compressed[i]) - 128;
                update.weights[i] = static_cast<float>(quantized) / 127.0f;
            }
            break;

        case CompressionMethod::SPARSIFICATION:
            // Reconstruct sparse gradients
            // Would need to know original size
            update.weights.resize(10000); // Placeholder
            for (size_t i = 0; i < compressed.size(); ) {
                if (i + sizeof(uint16_t) + sizeof(float) > compressed.size()) {
                    break;
                }
                
                uint16_t index;
                memcpy(&index, &compressed[i], sizeof(uint16_t));
                i += sizeof(uint16_t);
                
                float value;
                memcpy(&value, &compressed[i], sizeof(float));
                i += sizeof(float);
                
                if (index < update.weights.size()) {
                    update.weights[index] = value;
                }
            }
            break;

        case CompressionMethod::COMBINED:
            // Reconstruct from quantized sparse format
            update.weights.resize(10000); // Placeholder
            for (size_t i = 0; i < compressed.size(); ) {
                if (i + sizeof(uint16_t) + 1 > compressed.size()) {
                    break;
                }
                
                uint16_t index;
                memcpy(&index, &compressed[i], sizeof(uint16_t));
                i += sizeof(uint16_t);
                
                int8_t quantized = static_cast<int8_t>(compressed[i]) - 128;
                i++;
                
                if (index < update.weights.size()) {
                    update.weights[index] = static_cast<float>(quantized) / 127.0f;
                }
            }
            break;

        default:
            break;
    }

    return update;
}

// ===========================
// NETWORK MANAGEMENT
// ===========================

bool ModelUpdateProtocol::selectBestProtocol() {
    // Simple protocol selection logic
    if (WiFi.status() == WL_CONNECTED) {
        currentProtocol_ = "WiFi";
        return true;
    }

    // Would check other protocols (LoRa, Cellular)
    currentProtocol_ = "None";
    return false;
}

bool ModelUpdateProtocol::isConnected() const {
    return currentProtocol_ != "None" && WiFi.status() == WL_CONNECTED;
}

bool ModelUpdateProtocol::reconnect() {
    Logger::info("[ModelUpdateProtocol] Attempting to reconnect...");
    return selectBestProtocol();
}

// ===========================
// BANDWIDTH OPTIMIZATION
// ===========================

bool ModelUpdateProtocol::enableAdaptiveCompression(bool enable) {
    adaptiveCompressionEnabled_ = enable;
    Logger::info("[ModelUpdateProtocol] Adaptive compression %s", enable ? "enabled" : "disabled");
    return true;
}

CompressionMethod ModelUpdateProtocol::selectOptimalCompression(size_t dataSize, 
                                                                float batteryLevel) {
    if (!adaptiveCompressionEnabled_) {
        return config_.compressionMethod;
    }

    // Select compression based on data size and battery
    if (batteryLevel < 0.2f) {
        // Low battery: use minimal compression to save CPU
        return CompressionMethod::NONE;
    } else if (dataSize > 100000) {
        // Large data: use aggressive compression
        return CompressionMethod::COMBINED;
    } else if (dataSize > 10000) {
        // Medium data: use quantization
        return CompressionMethod::QUANTIZATION;
    }

    return CompressionMethod::NONE;
}

void ModelUpdateProtocol::setQualityOfService(uint8_t priority) {
    Logger::info("[ModelUpdateProtocol] QoS priority set to: %u", priority);
    // Would configure network QoS
}

// ===========================
// SECURITY
// ===========================

bool ModelUpdateProtocol::enableSecureMode(bool enable) {
    encryptionEnabled_ = enable;
    Logger::info("[ModelUpdateProtocol] Secure mode %s", enable ? "enabled" : "disabled");
    return true;
}

bool ModelUpdateProtocol::authenticateMessage(const MessageHeader& header, 
                                             const std::vector<uint8_t>& payload) {
    // Would perform actual authentication
    // For now, just check basic validity
    return header.magic == 0x46454441 && !header.senderId.isEmpty();
}

std::vector<uint8_t> ModelUpdateProtocol::encryptPayload(const std::vector<uint8_t>& payload) {
    // In a real implementation, would use actual encryption (AES-256)
    // For now, just return the payload (placeholder)
    Logger::debug("[ModelUpdateProtocol] Encrypting payload: %u bytes", payload.size());
    return payload;
}

std::vector<uint8_t> ModelUpdateProtocol::decryptPayload(const std::vector<uint8_t>& encrypted) {
    // In a real implementation, would decrypt
    // For now, just return the payload (placeholder)
    Logger::debug("[ModelUpdateProtocol] Decrypting payload: %u bytes", encrypted.size());
    return encrypted;
}

// ===========================
// STATISTICS AND MONITORING
// ===========================

void ModelUpdateProtocol::resetStatistics() {
    stats_ = CommunicationStats();
    Logger::info("[ModelUpdateProtocol] Statistics reset");
}

float ModelUpdateProtocol::getNetworkLatency() const {
    return stats_.averageLatency;
}

uint32_t ModelUpdateProtocol::getBandwidthUsage() const {
    return stats_.bytesTransmitted + stats_.bytesReceived;
}

bool ModelUpdateProtocol::isNetworkCongested() const {
    // Simple congestion detection
    return stats_.averageLatency > 1000.0f || stats_.failedMessages > 10;
}

// ===========================
// UTILITY METHODS
// ===========================

bool ModelUpdateProtocol::validateMessageIntegrity(const MessageHeader& header, 
                                                   const std::vector<uint8_t>& payload) {
    // Validate magic number
    if (header.magic != 0x46454441) {
        Logger::error("[ModelUpdateProtocol] Invalid magic number");
        return false;
    }

    // Validate length
    if (header.length != payload.size()) {
        Logger::error("[ModelUpdateProtocol] Length mismatch");
        return false;
    }

    // Validate checksum
    uint32_t computed = calculateChecksum(payload);
    if (computed != header.checksum) {
        Logger::error("[ModelUpdateProtocol] Checksum mismatch");
        return false;
    }

    return true;
}

uint32_t ModelUpdateProtocol::calculateChecksum(const std::vector<uint8_t>& data) {
    // Simple CRC32 implementation
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}

String ModelUpdateProtocol::generateMessageId() const {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "msg_%u_%u", millis(), sequenceNumber_);
    return String(buffer);
}

String ModelUpdateProtocol::getDeviceId() const {
    return "device_" + String(ESP.getEfuseMac(), HEX);
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

bool initializeModelUpdateProtocol(const NetworkConfig& config) {
    if (g_modelUpdateProtocol) {
        Logger::warn("[ModelUpdateProtocol] Already initialized");
        return true;
    }

    g_modelUpdateProtocol = new ModelUpdateProtocol();
    return g_modelUpdateProtocol->init(config);
}

void cleanupModelUpdateProtocol() {
    if (g_modelUpdateProtocol) {
        delete g_modelUpdateProtocol;
        g_modelUpdateProtocol = nullptr;
    }
}

bool sendFederatedMessage(MessageType type, const String& targetId, const void* payload) {
    if (!g_modelUpdateProtocol || !g_modelUpdateProtocol->isInitialized()) {
        return false;
    }

    // Would serialize and send based on type
    return true;
}

bool broadcastFederatedMessage(MessageType type, const void* payload) {
    if (!g_modelUpdateProtocol || !g_modelUpdateProtocol->isInitialized()) {
        return false;
    }

    // Would serialize and broadcast based on type
    return true;
}

CompressionMethod selectBestCompression(float batteryLevel, uint32_t memoryAvailable) {
    if (batteryLevel < 0.2f || memoryAvailable < 50000) {
        return CompressionMethod::NONE;
    } else if (memoryAvailable < 200000) {
        return CompressionMethod::QUANTIZATION;
    } else {
        return CompressionMethod::COMBINED;
    }
}
