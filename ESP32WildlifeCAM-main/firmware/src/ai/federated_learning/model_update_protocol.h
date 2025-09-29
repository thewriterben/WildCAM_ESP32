/**
 * @file model_update_protocol.h
 * @brief Model Update Protocol for Federated Learning
 * 
 * Implements secure, bandwidth-efficient communication protocol for
 * federated learning model parameter exchange between devices.
 */

#ifndef MODEL_UPDATE_PROTOCOL_H
#define MODEL_UPDATE_PROTOCOL_H

#include "federated_common.h"
#include "../../network/ota_manager.h"
#include "../../production/security/data_protector.h"
#include <vector>
#include <map>
#include <memory>

/**
 * Protocol Version
 */
#define MODEL_UPDATE_PROTOCOL_VERSION "1.0.0"

/**
 * Message Types
 */
enum class MessageType {
    ROUND_ANNOUNCEMENT,         // Coordinator announces new round
    JOIN_REQUEST,              // Client requests to join round
    JOIN_RESPONSE,             // Coordinator responds to join request
    TRAINING_START,            // Signal to start training
    MODEL_UPDATE,              // Client sends model update
    UPDATE_ACK,                // Coordinator acknowledges update
    GLOBAL_MODEL,              // Coordinator sends global model
    ROUND_COMPLETE,            // Round completion notification
    ERROR_MESSAGE,             // Error notification
    HEARTBEAT,                 // Keep-alive message
    PEER_DISCOVERY,            // Peer discovery message
    LEAVE_ROUND               // Client leaves round
};

/**
 * Compression Methods
 */
enum class CompressionMethod {
    NONE,                      // No compression
    QUANTIZATION,              // Weight quantization
    SPARSIFICATION,            // Gradient sparsification
    HUFFMAN,                   // Huffman encoding
    LZ4,                       // LZ4 compression
    COMBINED                   // Multiple methods combined
};

/**
 * Message Header
 */
struct MessageHeader {
    uint32_t magic;            // Protocol magic number
    uint16_t version;          // Protocol version
    MessageType type;          // Message type
    uint32_t length;           // Payload length
    uint32_t checksum;         // CRC32 checksum
    String senderId;           // Sender device ID
    String receiverId;         // Receiver device ID (broadcast if empty)
    String roundId;            // Round ID (if applicable)
    uint32_t timestamp;        // Timestamp
    uint32_t sequenceNumber;   // Sequence number for ordering
    
    MessageHeader() : magic(0x46454441), version(0x0100), // "FEDA" v1.0
                     type(MessageType::HEARTBEAT), length(0), checksum(0),
                     timestamp(0), sequenceNumber(0) {}
};

/**
 * Message Payload Structures
 */
struct RoundAnnouncementPayload {
    FederatedRound roundInfo;
    RoundConfig config;
    std::vector<String> requiredCapabilities;
};

struct JoinRequestPayload {
    String deviceId;
    String capabilities;       // JSON string
    float reliability;
    uint32_t availableMemory;
    float batteryLevel;
};

struct JoinResponsePayload {
    bool accepted;
    String reason;             // Reason if rejected
    ModelUpdate globalModel;   // Current global model
    uint32_t expectedTrainingTime;
};

struct ModelUpdatePayload {
    ModelUpdate update;
    CompressionMethod compression;
    float compressionRatio;
    uint32_t originalSize;
};

struct GlobalModelPayload {
    ModelUpdate globalModel;
    float accuracyImprovement;
    uint32_t participantCount;
    String validationResults;
};

/**
 * Communication Statistics
 */
struct CommunicationStats {
    uint32_t messagesSent;
    uint32_t messagesReceived;
    uint32_t bytesTransmitted;
    uint32_t bytesReceived;
    uint32_t compressionRatio;
    uint32_t networkErrors;
    uint32_t timeouts;
    float averageLatency;
    
    CommunicationStats() : messagesSent(0), messagesReceived(0),
                          bytesTransmitted(0), bytesReceived(0),
                          compressionRatio(0), networkErrors(0),
                          timeouts(0), averageLatency(0.0f) {}
};

/**
 * Network Configuration
 */
struct NetworkConfig {
    std::vector<String> protocolPriority;  // WiFi, LoRa, Cellular order
    uint32_t connectionTimeout;
    uint32_t messageTimeout;
    uint32_t maxRetries;
    bool enableEncryption;
    bool enableCompression;
    CompressionMethod defaultCompression;
    uint32_t maxMessageSize;
    uint32_t heartbeatInterval;
    
    NetworkConfig() : connectionTimeout(30000), messageTimeout(10000),
                     maxRetries(3), enableEncryption(true),
                     enableCompression(true), 
                     defaultCompression(CompressionMethod::QUANTIZATION),
                     maxMessageSize(64 * 1024), heartbeatInterval(60000) {
        protocolPriority = {"WiFi", "LoRa", "Cellular"};
    }
};

/**
 * Model Update Protocol Handler
 * 
 * Handles secure communication for federated learning:
 * - Message serialization and deserialization
 * - Compression and decompression
 * - Encryption and authentication
 * - Network protocol selection
 * - Bandwidth optimization
 */
class ModelUpdateProtocol {
public:
    ModelUpdateProtocol();
    ~ModelUpdateProtocol();
    
    // Initialization
    bool init(const NetworkConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setNetworkConfig(const NetworkConfig& config);
    NetworkConfig getNetworkConfig() const { return config_; }
    void setEncryptionEnabled(bool enabled);
    void setCompressionMethod(CompressionMethod method);
    
    // Message sending
    bool sendRoundAnnouncement(const String& targetId, const FederatedRound& round, 
                              const RoundConfig& config);
    bool sendJoinRequest(const String& coordinatorId, const JoinRequestPayload& payload);
    bool sendJoinResponse(const String& clientId, const JoinResponsePayload& payload);
    bool sendModelUpdate(const String& coordinatorId, const ModelUpdate& update);
    bool sendGlobalModel(const String& clientId, const ModelUpdate& globalModel, 
                        float accuracyImprovement);
    bool sendError(const String& targetId, const String& error);
    bool sendHeartbeat(const String& targetId = "");
    
    // Broadcast methods
    bool broadcastRoundAnnouncement(const FederatedRound& round, const RoundConfig& config);
    bool broadcastPeerDiscovery();
    bool broadcastRoundComplete(const String& roundId);
    
    // Message receiving
    bool receiveMessage(MessageHeader& header, std::vector<uint8_t>& payload);
    bool processMessage(const MessageHeader& header, const std::vector<uint8_t>& payload);
    
    // Message processing callbacks
    typedef std::function<void(const String&, const FederatedRound&, const RoundConfig&)> RoundAnnouncementHandler;
    typedef std::function<void(const String&, const JoinRequestPayload&)> JoinRequestHandler;
    typedef std::function<void(const String&, const JoinResponsePayload&)> JoinResponseHandler;
    typedef std::function<void(const String&, const ModelUpdate&)> ModelUpdateHandler;
    typedef std::function<void(const String&, const ModelUpdate&, float)> GlobalModelHandler;
    typedef std::function<void(const String&, const String&)> ErrorHandler;
    typedef std::function<void(const String&)> HeartbeatHandler;
    typedef std::function<void(const String&)> PeerDiscoveryHandler;
    
    void setRoundAnnouncementHandler(RoundAnnouncementHandler handler) { roundAnnouncementHandler_ = handler; }
    void setJoinRequestHandler(JoinRequestHandler handler) { joinRequestHandler_ = handler; }
    void setJoinResponseHandler(JoinResponseHandler handler) { joinResponseHandler_ = handler; }
    void setModelUpdateHandler(ModelUpdateHandler handler) { modelUpdateHandler_ = handler; }
    void setGlobalModelHandler(GlobalModelHandler handler) { globalModelHandler_ = handler; }
    void setErrorHandler(ErrorHandler handler) { errorHandler_ = handler; }
    void setHeartbeatHandler(HeartbeatHandler handler) { heartbeatHandler_ = handler; }
    void setPeerDiscoveryHandler(PeerDiscoveryHandler handler) { peerDiscoveryHandler_ = handler; }
    
    // Compression and decompression
    std::vector<uint8_t> compressModelUpdate(const ModelUpdate& update, CompressionMethod method);
    ModelUpdate decompressModelUpdate(const std::vector<uint8_t>& compressed, CompressionMethod method);
    
    // Network management
    bool selectBestProtocol();
    String getCurrentProtocol() const { return currentProtocol_; }
    bool isConnected() const;
    bool reconnect();
    
    // Bandwidth optimization
    bool enableAdaptiveCompression(bool enable);
    CompressionMethod selectOptimalCompression(size_t dataSize, float batteryLevel);
    void setQualityOfService(uint8_t priority);
    
    // Security
    bool enableSecureMode(bool enable);
    bool authenticateMessage(const MessageHeader& header, const std::vector<uint8_t>& payload);
    std::vector<uint8_t> encryptPayload(const std::vector<uint8_t>& payload);
    std::vector<uint8_t> decryptPayload(const std::vector<uint8_t>& encrypted);
    
    // Statistics and monitoring
    CommunicationStats getStatistics() const { return stats_; }
    void resetStatistics();
    float getNetworkLatency() const;
    uint32_t getBandwidthUsage() const;
    bool isNetworkCongested() const;
    
    // Utility methods
    bool validateMessageIntegrity(const MessageHeader& header, const std::vector<uint8_t>& payload);
    uint32_t calculateChecksum(const std::vector<uint8_t>& data);
    String generateMessageId() const;

private:
    // Configuration
    NetworkConfig config_;
    bool initialized_;
    
    // Network state
    String currentProtocol_;
    bool connected_;
    String deviceId_;
    uint32_t sequenceNumber_;
    
    // Security components
    std::unique_ptr<DataProtector> dataProtector_;
    bool secureMode_;
    String encryptionKey_;
    
    // Communication statistics
    CommunicationStats stats_;
    
    // Message handlers
    RoundAnnouncementHandler roundAnnouncementHandler_;
    JoinRequestHandler joinRequestHandler_;
    JoinResponseHandler joinResponseHandler_;
    ModelUpdateHandler modelUpdateHandler_;
    GlobalModelHandler globalModelHandler_;
    ErrorHandler errorHandler_;
    HeartbeatHandler heartbeatHandler_;
    PeerDiscoveryHandler peerDiscoveryHandler_;
    
    // Private methods - Message serialization
    std::vector<uint8_t> serializeHeader(const MessageHeader& header);
    MessageHeader deserializeHeader(const std::vector<uint8_t>& data);
    std::vector<uint8_t> serializePayload(MessageType type, const void* payload);
    bool deserializePayload(MessageType type, const std::vector<uint8_t>& data, void* payload);
    
    // Private methods - Compression
    std::vector<uint8_t> quantizeWeights(const std::vector<float>& weights, uint8_t bits = 8);
    std::vector<float> dequantizeWeights(const std::vector<uint8_t>& quantized, uint8_t bits = 8);
    std::vector<uint8_t> sparsifyGradients(const std::vector<float>& gradients, float threshold = 0.01f);
    std::vector<float> densifyGradients(const std::vector<uint8_t>& sparse, size_t originalSize);
    std::vector<uint8_t> huffmanEncode(const std::vector<uint8_t>& data);
    std::vector<uint8_t> huffmanDecode(const std::vector<uint8_t>& encoded);
    
    // Private methods - Network protocols
    bool sendViaWiFi(const std::vector<uint8_t>& message, const String& targetId);
    bool sendViaLoRa(const std::vector<uint8_t>& message, const String& targetId);
    bool sendViaCellular(const std::vector<uint8_t>& message, const String& targetId);
    bool receiveFromNetwork(std::vector<uint8_t>& message, String& senderId);
    
    // Private methods - Utilities
    void updateStatistics(bool sent, size_t bytes, bool success);
    void logCommunicationEvent(const String& event, const String& details = "");
    bool isProtocolAvailable(const String& protocol) const;
    float estimateTransmissionTime(size_t bytes, const String& protocol) const;
};

// Global instance
extern ModelUpdateProtocol* g_modelUpdateProtocol;

// Utility functions
bool initializeModelUpdateProtocol(const NetworkConfig& config);
void cleanupModelUpdateProtocol();
bool sendFederatedMessage(MessageType type, const String& targetId, const void* payload);
bool broadcastFederatedMessage(MessageType type, const void* payload);
CompressionMethod selectBestCompression(float batteryLevel, uint32_t memoryAvailable);

#endif // MODEL_UPDATE_PROTOCOL_H