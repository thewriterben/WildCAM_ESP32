/**
 * @file reliable_transmission_protocol.h
 * @brief Reliable Data Transmission Protocol for ESP32WildlifeCAM
 * 
 * Implements reliable data transmission protocols optimized for:
 * - Low-bandwidth, lossy LoRa mesh networks
 * - Error recovery and automatic retransmission
 * - Bandwidth management and prioritization
 * - Integration with WiFi and mesh backbones
 * 
 * Features:
 * - Automatic retry with exponential backoff
 * - Acknowledgment-based confirmation
 * - Packet loss detection and recovery
 * - Bandwidth throttling and QoS
 * - Transmission statistics and monitoring
 */

#ifndef RELIABLE_TRANSMISSION_PROTOCOL_H
#define RELIABLE_TRANSMISSION_PROTOCOL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <queue>
#include "mesh_interface.h"

// ===========================
// PROTOCOL CONSTANTS
// ===========================

// Retry configuration
#define RTP_MAX_RETRIES              5          // Maximum retry attempts
#define RTP_INITIAL_RETRY_DELAY      1000       // Initial retry delay (ms)
#define RTP_MAX_RETRY_DELAY          60000      // Maximum retry delay (ms)
#define RTP_BACKOFF_MULTIPLIER       2.0        // Exponential backoff multiplier

// Acknowledgment configuration
#define RTP_ACK_TIMEOUT              5000       // ACK timeout (ms)
#define RTP_ACK_BATCH_SIZE           10         // Batch ACK size

// Bandwidth management
#define RTP_MAX_BYTES_PER_SECOND     1024       // Maximum bytes per second (1KB/s)
#define RTP_BURST_SIZE               4096       // Maximum burst size
#define RTP_MIN_PACKET_INTERVAL      100        // Minimum interval between packets (ms)

// Queue configuration
#define RTP_MAX_QUEUE_SIZE           50         // Maximum queued transmissions
#define RTP_QUEUE_TIMEOUT            300000     // Queue timeout (5 minutes)

// Statistics configuration
#define RTP_STATS_WINDOW_SIZE        100        // Statistics window size

// ===========================
// TRANSMISSION DATA STRUCTURES
// ===========================

// Transmission priority levels
enum TransmissionPriority {
    PRIORITY_CRITICAL = 0,      // Emergency data (endangered species alert)
    PRIORITY_HIGH = 1,          // Important data (wildlife detection, capture events)
    PRIORITY_NORMAL = 2,        // Standard data (motion events, telemetry)
    PRIORITY_LOW = 3,           // Background data (routine status updates)
    PRIORITY_BACKGROUND = 4     // Best-effort data (periodic health checks)
};

// Transmission states
enum TransmissionState {
    TX_STATE_QUEUED,            // Waiting in queue
    TX_STATE_TRANSMITTING,      // Currently transmitting
    TX_STATE_AWAITING_ACK,      // Waiting for acknowledgment
    TX_STATE_RETRYING,          // Retrying after failure
    TX_STATE_COMPLETED,         // Successfully completed
    TX_STATE_FAILED,            // Failed after max retries
    TX_STATE_CANCELLED          // Cancelled by user
};

// Transmission result codes
enum TransmissionResult {
    TX_SUCCESS,                 // Transmission successful
    TX_PENDING,                 // Transmission in progress
    TX_TIMEOUT,                 // ACK timeout
    TX_MAX_RETRIES,             // Maximum retries exceeded
    TX_QUEUE_FULL,              // Queue full
    TX_INVALID_DATA,            // Invalid data
    TX_NETWORK_ERROR,           // Network error
    TX_CANCELLED                // Cancelled
};

// Packet structure
struct ReliablePacket {
    uint32_t packetId;          // Unique packet identifier
    uint32_t transmissionId;    // Associated transmission ID
    uint32_t destination;       // Destination node (0 = broadcast)
    MeshPacketType type;        // Packet type
    TransmissionPriority priority;
    std::vector<uint8_t> payload;
    uint32_t payloadSize;
    uint32_t crc32;             // Payload CRC32 checksum
    uint16_t sequenceNumber;    // Sequence number
    uint16_t totalPackets;      // Total packets in transmission
    bool requiresAck;           // Requires acknowledgment
    unsigned long timestamp;    // Creation timestamp
};

// Acknowledgment structure
struct PacketAck {
    uint32_t packetId;          // Acknowledged packet ID
    uint32_t transmissionId;    // Associated transmission ID
    uint32_t fromNode;          // Node that sent ACK
    bool success;               // Success flag
    unsigned long timestamp;    // ACK timestamp
    int16_t rssi;               // Signal strength
    uint8_t snr;                // Signal-to-noise ratio
};

// Transmission request
struct TransmissionRequest {
    uint32_t transmissionId;    // Unique transmission identifier
    uint32_t destination;       // Destination node (0 = broadcast)
    MeshPacketType type;        // Data type
    TransmissionPriority priority;
    std::vector<uint8_t> data;  // Data to transmit
    bool requiresAck;           // Requires acknowledgment
    uint8_t maxRetries;         // Maximum retry attempts
    uint32_t retryDelay;        // Initial retry delay (ms)
    bool splitIntoPackets;      // Split large data into packets
    unsigned long queueTime;    // Time added to queue
};

// Transmission status
struct TransmissionStatus {
    uint32_t transmissionId;
    TransmissionState state;
    TransmissionPriority priority;
    uint16_t packetsTransmitted;
    uint16_t packetsAcknowledged;
    uint16_t totalPackets;
    uint32_t bytesTransmitted;
    uint32_t totalBytes;
    uint8_t retryCount;
    float progressPercentage;
    unsigned long startTime;
    unsigned long lastActivityTime;
    unsigned long estimatedTimeRemaining;
    TransmissionResult result;
    String errorMessage;
};

// Bandwidth usage statistics
struct BandwidthStats {
    uint32_t bytesTransmitted;      // Total bytes transmitted
    uint32_t bytesReceived;         // Total bytes received
    uint32_t packetsTransmitted;    // Total packets transmitted
    uint32_t packetsReceived;       // Total packets received
    uint32_t packetsLost;           // Total packets lost
    uint32_t packetsRetransmitted;  // Total retransmissions
    float packetLossRate;           // Packet loss rate (0-1)
    float averageLatency;           // Average latency (ms)
    float throughput;               // Current throughput (bytes/s)
    unsigned long lastUpdateTime;   // Last update timestamp
};

// Protocol configuration
struct ReliableTransmissionConfig {
    bool enableRetransmission;      // Enable automatic retransmission
    uint8_t maxRetries;             // Maximum retry attempts
    uint32_t initialRetryDelay;     // Initial retry delay (ms)
    uint32_t maxRetryDelay;         // Maximum retry delay (ms)
    float backoffMultiplier;        // Exponential backoff multiplier
    
    bool enableAcknowledgments;     // Enable ACK mechanism
    uint32_t ackTimeout;            // ACK timeout (ms)
    uint8_t ackBatchSize;           // Batch ACK size
    
    bool enableBandwidthManagement; // Enable bandwidth throttling
    uint32_t maxBytesPerSecond;     // Maximum bandwidth (bytes/s)
    uint32_t burstSize;             // Maximum burst size
    uint32_t minPacketInterval;     // Minimum packet interval (ms)
    
    uint16_t maxQueueSize;          // Maximum queue size
    uint32_t queueTimeout;          // Queue timeout (ms)
    
    bool enableStatistics;          // Enable statistics tracking
    uint16_t statsWindowSize;       // Statistics window size
    
    bool priorityBasedScheduling;   // Enable priority scheduling
    bool adaptiveRetry;             // Adaptive retry based on conditions
};

// ===========================
// RELIABLE TRANSMISSION PROTOCOL CLASS
// ===========================

class ReliableTransmissionProtocol {
public:
    ReliableTransmissionProtocol();
    ~ReliableTransmissionProtocol();
    
    // Initialization and configuration
    bool init(MeshInterface* meshInterface);
    bool configure(const ReliableTransmissionConfig& config);
    void cleanup();
    
    // Transmission methods
    uint32_t transmitData(uint32_t destination, 
                         MeshPacketType type,
                         const std::vector<uint8_t>& data,
                         TransmissionPriority priority = PRIORITY_NORMAL,
                         bool requiresAck = true);
    
    uint32_t transmitDataAsync(const TransmissionRequest& request);
    
    bool cancelTransmission(uint32_t transmissionId);
    bool retryTransmission(uint32_t transmissionId);
    
    // Status and monitoring
    TransmissionStatus getTransmissionStatus(uint32_t transmissionId);
    std::vector<TransmissionStatus> getAllTransmissions();
    bool isTransmissionComplete(uint32_t transmissionId);
    bool isTransmissionActive(uint32_t transmissionId);
    
    // Packet handling
    bool handleReceivedPacket(const ReliablePacket& packet);
    bool handleReceivedAck(const PacketAck& ack);
    
    // Bandwidth management
    bool setBandwidthLimit(uint32_t bytesPerSecond);
    uint32_t getBandwidthLimit() const;
    float getCurrentBandwidthUsage() const;
    bool throttleTransmission();
    
    // Statistics
    BandwidthStats getStatistics() const;
    void resetStatistics();
    float getPacketLossRate() const;
    float getAverageLatency() const;
    float getThroughput() const;
    
    // Configuration
    ReliableTransmissionConfig getConfig() const;
    bool setConfig(const ReliableTransmissionConfig& config);
    
    // Processing
    void process();                 // Call regularly in main loop
    void processTransmissionQueue();
    void processRetries();
    void processAcknowledgments();
    void processBandwidthManagement();
    
    // Callbacks
    typedef void (*TransmissionCompleteCallback)(uint32_t transmissionId, bool success);
    typedef void (*TransmissionProgressCallback)(uint32_t transmissionId, float progress);
    typedef void (*PacketReceivedCallback)(const ReliablePacket& packet);
    typedef void (*TransmissionErrorCallback)(uint32_t transmissionId, TransmissionResult error);
    
    void setTransmissionCompleteCallback(TransmissionCompleteCallback callback);
    void setTransmissionProgressCallback(TransmissionProgressCallback callback);
    void setPacketReceivedCallback(PacketReceivedCallback callback);
    void setTransmissionErrorCallback(TransmissionErrorCallback callback);

private:
    // Internal state
    bool initialized_;
    MeshInterface* meshInterface_;
    ReliableTransmissionConfig config_;
    
    // Transmission queue and management
    std::queue<TransmissionRequest> transmissionQueue_;
    std::map<uint32_t, TransmissionStatus> activeTransmissions_;
    std::map<uint32_t, std::vector<ReliablePacket>> transmissionPackets_;
    std::map<uint32_t, std::vector<bool>> packetAckStatus_;
    
    // Retry management
    std::map<uint32_t, unsigned long> nextRetryTime_;
    std::map<uint32_t, unsigned long> retryDelays_;
    
    // ACK tracking
    std::map<uint32_t, unsigned long> ackTimeouts_;
    std::map<uint32_t, PacketAck> receivedAcks_;
    
    // Bandwidth management
    uint32_t bandwidthLimit_;
    unsigned long lastTransmissionTime_;
    uint32_t currentBurstSize_;
    std::vector<uint32_t> recentTransmissions_;  // For throughput calculation
    std::vector<unsigned long> transmissionTimes_;
    
    // Statistics
    BandwidthStats statistics_;
    std::vector<float> latencyHistory_;
    
    // ID generation
    uint32_t nextTransmissionId_;
    uint32_t nextPacketId_;
    
    // Callbacks
    TransmissionCompleteCallback transmissionCompleteCallback_;
    TransmissionProgressCallback transmissionProgressCallback_;
    PacketReceivedCallback packetReceivedCallback_;
    TransmissionErrorCallback transmissionErrorCallback_;
    
    // Internal transmission methods
    uint32_t generateTransmissionId();
    uint32_t generatePacketId();
    
    bool queueTransmission(const TransmissionRequest& request);
    bool startTransmission(uint32_t transmissionId);
    bool transmitNextPacket(uint32_t transmissionId);
    bool transmitPacket(const ReliablePacket& packet);
    
    // Packet management
    std::vector<ReliablePacket> splitIntoPackets(const TransmissionRequest& request);
    bool validatePacket(const ReliablePacket& packet);
    uint32_t calculateCRC32(const std::vector<uint8_t>& data);
    
    // Acknowledgment methods
    bool sendAcknowledgment(uint32_t packetId, uint32_t transmissionId, uint32_t destination);
    bool processAcknowledgment(uint32_t transmissionId, uint32_t packetId);
    bool checkAckTimeout(uint32_t transmissionId);
    
    // Retry logic
    bool shouldRetryTransmission(uint32_t transmissionId);
    unsigned long calculateRetryDelay(uint32_t transmissionId, uint8_t retryCount);
    bool scheduleRetry(uint32_t transmissionId);
    
    // Bandwidth management methods
    bool canTransmitNow();
    void updateBandwidthUsage(uint32_t bytes);
    void calculateThroughput();
    bool exceedsQueueTimeout(const TransmissionRequest& request);
    
    // Statistics methods
    void updateStatistics(const ReliablePacket& packet, bool success);
    void updateLatency(uint32_t transmissionId, unsigned long latency);
    void calculatePacketLossRate();
    
    // Priority scheduling
    TransmissionRequest getNextTransmission();
    bool hasHigherPriorityTransmission(TransmissionPriority priority);
    
    // Cleanup methods
    void cleanupCompletedTransmissions();
    void cleanupExpiredTransmissions();
    void removeOldStatistics();
    
    // Utility methods
    TransmissionState determineTransmissionState(uint32_t transmissionId);
    float calculateProgress(uint32_t transmissionId);
    unsigned long estimateTimeRemaining(uint32_t transmissionId);
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function
ReliableTransmissionProtocol* createReliableTransmissionProtocol(MeshInterface* meshInterface);

// Configuration helpers
ReliableTransmissionConfig createDefaultRTPConfig();
ReliableTransmissionConfig createLowBandwidthConfig();
ReliableTransmissionConfig createHighReliabilityConfig();
ReliableTransmissionConfig createBestEffortConfig();

// Priority helpers
const char* priorityToString(TransmissionPriority priority);
TransmissionPriority stringToPriority(const char* str);

// State helpers
const char* stateToString(TransmissionState state);
const char* resultToString(TransmissionResult result);

// Validation functions
bool isValidTransmissionRequest(const TransmissionRequest& request);
bool isValidPacket(const ReliablePacket& packet);

#endif // RELIABLE_TRANSMISSION_PROTOCOL_H
