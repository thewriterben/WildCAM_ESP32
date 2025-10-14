/**
 * @file reliable_transmission_protocol.cpp
 * @brief Reliable Data Transmission Protocol Implementation for ESP32WildlifeCAM
 * 
 * Comprehensive implementation of reliable data transmission with error recovery,
 * retransmission, bandwidth management, and prioritization.
 */

#include "reliable_transmission_protocol.h"
#include "../debug_utils.h"
#include <esp_crc.h>
#include <algorithm>

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

ReliableTransmissionProtocol::ReliableTransmissionProtocol()
    : initialized_(false)
    , meshInterface_(nullptr)
    , bandwidthLimit_(RTP_MAX_BYTES_PER_SECOND)
    , lastTransmissionTime_(0)
    , currentBurstSize_(0)
    , nextTransmissionId_(1)
    , nextPacketId_(1)
    , transmissionCompleteCallback_(nullptr)
    , transmissionProgressCallback_(nullptr)
    , packetReceivedCallback_(nullptr)
    , transmissionErrorCallback_(nullptr)
{
    config_ = createDefaultRTPConfig();
    memset(&statistics_, 0, sizeof(BandwidthStats));
}

ReliableTransmissionProtocol::~ReliableTransmissionProtocol() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool ReliableTransmissionProtocol::init(MeshInterface* meshInterface) {
    DEBUG_PRINTLN("ReliableTransmissionProtocol: Initializing...");
    
    if (!meshInterface) {
        DEBUG_PRINTLN("ReliableTransmissionProtocol: Invalid mesh interface");
        return false;
    }
    
    meshInterface_ = meshInterface;
    
    // Initialize statistics
    statistics_.lastUpdateTime = millis();
    
    // Reserve space for tracking
    recentTransmissions_.reserve(config_.statsWindowSize);
    transmissionTimes_.reserve(config_.statsWindowSize);
    latencyHistory_.reserve(config_.statsWindowSize);
    
    initialized_ = true;
    
    DEBUG_PRINTLN("ReliableTransmissionProtocol: Initialized successfully");
    
    return true;
}

bool ReliableTransmissionProtocol::configure(const ReliableTransmissionConfig& config) {
    if (!initialized_) {
        return false;
    }
    
    config_ = config;
    bandwidthLimit_ = config.maxBytesPerSecond;
    
    DEBUG_PRINTLN("ReliableTransmissionProtocol: Configuration updated");
    
    return true;
}

void ReliableTransmissionProtocol::cleanup() {
    // Clear all queues and maps
    while (!transmissionQueue_.empty()) {
        transmissionQueue_.pop();
    }
    
    activeTransmissions_.clear();
    transmissionPackets_.clear();
    packetAckStatus_.clear();
    nextRetryTime_.clear();
    retryDelays_.clear();
    ackTimeouts_.clear();
    receivedAcks_.clear();
    recentTransmissions_.clear();
    transmissionTimes_.clear();
    latencyHistory_.clear();
    
    initialized_ = false;
}

// ===========================
// TRANSMISSION METHODS
// ===========================

uint32_t ReliableTransmissionProtocol::transmitData(
    uint32_t destination,
    MeshPacketType type,
    const std::vector<uint8_t>& data,
    TransmissionPriority priority,
    bool requiresAck)
{
    if (!initialized_ || data.empty()) {
        return 0;
    }
    
    TransmissionRequest request;
    request.transmissionId = generateTransmissionId();
    request.destination = destination;
    request.type = type;
    request.priority = priority;
    request.data = data;
    request.requiresAck = requiresAck;
    request.maxRetries = config_.maxRetries;
    request.retryDelay = config_.initialRetryDelay;
    request.splitIntoPackets = (data.size() > 200);  // Split if larger than 200 bytes
    request.queueTime = millis();
    
    if (queueTransmission(request)) {
        return request.transmissionId;
    }
    
    return 0;
}

uint32_t ReliableTransmissionProtocol::transmitDataAsync(const TransmissionRequest& request) {
    if (!initialized_ || !isValidTransmissionRequest(request)) {
        return 0;
    }
    
    if (queueTransmission(request)) {
        return request.transmissionId;
    }
    
    return 0;
}

bool ReliableTransmissionProtocol::cancelTransmission(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    it->second.state = TX_STATE_CANCELLED;
    it->second.result = TX_CANCELLED;
    
    if (transmissionCompleteCallback_) {
        transmissionCompleteCallback_(transmissionId, false);
    }
    
    DEBUG_PRINTF("ReliableTransmissionProtocol: Transmission %08X cancelled\n", transmissionId);
    
    return true;
}

bool ReliableTransmissionProtocol::retryTransmission(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    if (it->second.state == TX_STATE_FAILED || it->second.state == TX_STATE_CANCELLED) {
        it->second.retryCount = 0;
        it->second.state = TX_STATE_QUEUED;
        return scheduleRetry(transmissionId);
    }
    
    return false;
}

// ===========================
// QUEUE MANAGEMENT
// ===========================

bool ReliableTransmissionProtocol::queueTransmission(const TransmissionRequest& request) {
    if (transmissionQueue_.size() >= config_.maxQueueSize) {
        DEBUG_PRINTLN("ReliableTransmissionProtocol: Queue full");
        if (transmissionErrorCallback_) {
            transmissionErrorCallback_(request.transmissionId, TX_QUEUE_FULL);
        }
        return false;
    }
    
    transmissionQueue_.push(request);
    
    // Initialize transmission status
    TransmissionStatus status;
    status.transmissionId = request.transmissionId;
    status.state = TX_STATE_QUEUED;
    status.priority = request.priority;
    status.packetsTransmitted = 0;
    status.packetsAcknowledged = 0;
    status.totalPackets = 0;
    status.bytesTransmitted = 0;
    status.totalBytes = request.data.size();
    status.retryCount = 0;
    status.progressPercentage = 0.0;
    status.startTime = millis();
    status.lastActivityTime = millis();
    status.estimatedTimeRemaining = 0;
    status.result = TX_PENDING;
    
    activeTransmissions_[request.transmissionId] = status;
    
    DEBUG_PRINTF("ReliableTransmissionProtocol: Queued transmission %08X (priority: %d, size: %d bytes)\n",
                 request.transmissionId, request.priority, request.data.size());
    
    return true;
}

bool ReliableTransmissionProtocol::startTransmission(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    // Find corresponding request in queue
    TransmissionRequest request;
    bool found = false;
    std::queue<TransmissionRequest> tempQueue;
    
    while (!transmissionQueue_.empty()) {
        TransmissionRequest req = transmissionQueue_.front();
        transmissionQueue_.pop();
        
        if (req.transmissionId == transmissionId) {
            request = req;
            found = true;
            break;
        } else {
            tempQueue.push(req);
        }
    }
    
    // Restore queue
    while (!tempQueue.empty()) {
        transmissionQueue_.push(tempQueue.front());
        tempQueue.pop();
    }
    
    if (!found) {
        return false;
    }
    
    // Split into packets if needed
    std::vector<ReliablePacket> packets = splitIntoPackets(request);
    if (packets.empty()) {
        it->second.state = TX_STATE_FAILED;
        it->second.result = TX_INVALID_DATA;
        return false;
    }
    
    transmissionPackets_[transmissionId] = packets;
    it->second.totalPackets = packets.size();
    
    // Initialize ACK status
    std::vector<bool> ackStatus(packets.size(), false);
    packetAckStatus_[transmissionId] = ackStatus;
    
    // Update state
    it->second.state = TX_STATE_TRANSMITTING;
    it->second.startTime = millis();
    
    DEBUG_PRINTF("ReliableTransmissionProtocol: Starting transmission %08X (%d packets)\n",
                 transmissionId, packets.size());
    
    return true;
}

// ===========================
// PACKET TRANSMISSION
// ===========================

bool ReliableTransmissionProtocol::transmitNextPacket(uint32_t transmissionId) {
    auto packetsIt = transmissionPackets_.find(transmissionId);
    auto ackIt = packetAckStatus_.find(transmissionId);
    auto statusIt = activeTransmissions_.find(transmissionId);
    
    if (packetsIt == transmissionPackets_.end() || 
        ackIt == packetAckStatus_.end() ||
        statusIt == activeTransmissions_.end()) {
        return false;
    }
    
    // Find next packet to transmit (not yet acknowledged)
    for (size_t i = 0; i < packetsIt->second.size(); i++) {
        if (!ackIt->second[i]) {
            // Check bandwidth limits
            if (!canTransmitNow()) {
                return false;  // Wait for bandwidth availability
            }
            
            // Transmit packet
            if (transmitPacket(packetsIt->second[i])) {
                statusIt->second.packetsTransmitted++;
                statusIt->second.bytesTransmitted += packetsIt->second[i].payloadSize;
                statusIt->second.lastActivityTime = millis();
                
                // Update progress
                statusIt->second.progressPercentage = calculateProgress(transmissionId);
                
                if (transmissionProgressCallback_) {
                    transmissionProgressCallback_(transmissionId, statusIt->second.progressPercentage);
                }
                
                // Set ACK timeout if required
                if (packetsIt->second[i].requiresAck) {
                    ackTimeouts_[packetsIt->second[i].packetId] = millis() + config_.ackTimeout;
                }
                
                return true;
            } else {
                statusIt->second.state = TX_STATE_FAILED;
                statusIt->second.result = TX_NETWORK_ERROR;
                return false;
            }
        }
    }
    
    // All packets transmitted and acknowledged
    statusIt->second.state = TX_STATE_COMPLETED;
    statusIt->second.result = TX_SUCCESS;
    statusIt->second.progressPercentage = 100.0;
    
    if (transmissionCompleteCallback_) {
        transmissionCompleteCallback_(transmissionId, true);
    }
    
    DEBUG_PRINTF("ReliableTransmissionProtocol: Transmission %08X completed successfully\n", transmissionId);
    
    return true;
}

bool ReliableTransmissionProtocol::transmitPacket(const ReliablePacket& packet) {
    if (!meshInterface_ || !validatePacket(packet)) {
        return false;
    }
    
    // Serialize packet for transmission
    DynamicJsonDocument doc(512);
    doc["packetId"] = packet.packetId;
    doc["transmissionId"] = packet.transmissionId;
    doc["seqNum"] = packet.sequenceNumber;
    doc["totalPkts"] = packet.totalPackets;
    doc["crc32"] = packet.crc32;
    doc["requiresAck"] = packet.requiresAck;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Combine metadata and payload
    std::vector<uint8_t> fullPayload(jsonString.begin(), jsonString.end());
    fullPayload.push_back(0);  // Null terminator for JSON
    fullPayload.insert(fullPayload.end(), packet.payload.begin(), packet.payload.end());
    
    // Map priority to routing priority
    RoutingPriority routingPriority;
    switch (packet.priority) {
        case PRIORITY_CRITICAL:
            routingPriority = PRIORITY_EMERGENCY;
            break;
        case PRIORITY_HIGH:
            routingPriority = PRIORITY_HIGH;
            break;
        case PRIORITY_NORMAL:
            routingPriority = PRIORITY_NORMAL;
            break;
        case PRIORITY_LOW:
        case PRIORITY_BACKGROUND:
        default:
            routingPriority = PRIORITY_BACKGROUND;
            break;
    }
    
    bool success = meshInterface_->sendMessage(packet.destination, packet.type, 
                                               fullPayload, routingPriority, packet.requiresAck);
    
    if (success) {
        updateBandwidthUsage(fullPayload.size());
        statistics_.packetsTransmitted++;
        statistics_.bytesTransmitted += fullPayload.size();
    }
    
    return success;
}

std::vector<ReliablePacket> ReliableTransmissionProtocol::splitIntoPackets(const TransmissionRequest& request) {
    std::vector<ReliablePacket> packets;
    
    if (!request.splitIntoPackets || request.data.size() <= 200) {
        // Single packet transmission
        ReliablePacket packet;
        packet.packetId = generatePacketId();
        packet.transmissionId = request.transmissionId;
        packet.destination = request.destination;
        packet.type = request.type;
        packet.priority = request.priority;
        packet.payload = request.data;
        packet.payloadSize = request.data.size();
        packet.crc32 = calculateCRC32(request.data);
        packet.sequenceNumber = 0;
        packet.totalPackets = 1;
        packet.requiresAck = request.requiresAck;
        packet.timestamp = millis();
        
        packets.push_back(packet);
    } else {
        // Multi-packet transmission
        const size_t chunkSize = 200;  // Bytes per packet
        size_t offset = 0;
        uint16_t sequenceNumber = 0;
        uint16_t totalPackets = (request.data.size() + chunkSize - 1) / chunkSize;
        
        while (offset < request.data.size()) {
            size_t currentChunkSize = std::min(chunkSize, request.data.size() - offset);
            
            ReliablePacket packet;
            packet.packetId = generatePacketId();
            packet.transmissionId = request.transmissionId;
            packet.destination = request.destination;
            packet.type = request.type;
            packet.priority = request.priority;
            packet.payload = std::vector<uint8_t>(
                request.data.begin() + offset,
                request.data.begin() + offset + currentChunkSize
            );
            packet.payloadSize = currentChunkSize;
            packet.crc32 = calculateCRC32(packet.payload);
            packet.sequenceNumber = sequenceNumber++;
            packet.totalPackets = totalPackets;
            packet.requiresAck = request.requiresAck;
            packet.timestamp = millis();
            
            packets.push_back(packet);
            offset += currentChunkSize;
        }
    }
    
    return packets;
}

// ===========================
// ACKNOWLEDGMENT HANDLING
// ===========================

bool ReliableTransmissionProtocol::sendAcknowledgment(uint32_t packetId, uint32_t transmissionId, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    DynamicJsonDocument doc(256);
    doc["type"] = "ack";
    doc["packetId"] = packetId;
    doc["transmissionId"] = transmissionId;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    
    return meshInterface_->sendMessage(destination, PACKET_TYPE_DATA, payload, PRIORITY_HIGH);
}

bool ReliableTransmissionProtocol::processAcknowledgment(uint32_t transmissionId, uint32_t packetId) {
    auto packetsIt = transmissionPackets_.find(transmissionId);
    auto ackIt = packetAckStatus_.find(transmissionId);
    auto statusIt = activeTransmissions_.find(transmissionId);
    
    if (packetsIt == transmissionPackets_.end() || 
        ackIt == packetAckStatus_.end() ||
        statusIt == activeTransmissions_.end()) {
        return false;
    }
    
    // Find packet and mark as acknowledged
    for (size_t i = 0; i < packetsIt->second.size(); i++) {
        if (packetsIt->second[i].packetId == packetId && !ackIt->second[i]) {
            ackIt->second[i] = true;
            statusIt->second.packetsAcknowledged++;
            
            // Calculate and update latency
            unsigned long latency = millis() - packetsIt->second[i].timestamp;
            updateLatency(transmissionId, latency);
            
            // Remove ACK timeout
            ackTimeouts_.erase(packetId);
            
            DEBUG_PRINTF("ReliableTransmissionProtocol: ACK received for packet %08X (latency: %lu ms)\n",
                        packetId, latency);
            
            return true;
        }
    }
    
    return false;
}

bool ReliableTransmissionProtocol::handleReceivedAck(const PacketAck& ack) {
    if (!initialized_) {
        return false;
    }
    
    receivedAcks_[ack.packetId] = ack;
    
    return processAcknowledgment(ack.transmissionId, ack.packetId);
}

bool ReliableTransmissionProtocol::checkAckTimeout(uint32_t transmissionId) {
    auto statusIt = activeTransmissions_.find(transmissionId);
    if (statusIt == activeTransmissions_.end()) {
        return false;
    }
    
    unsigned long currentTime = millis();
    bool timeout = false;
    
    // Check all pending ACKs for this transmission
    for (auto it = ackTimeouts_.begin(); it != ackTimeouts_.end();) {
        if (currentTime > it->second) {
            DEBUG_PRINTF("ReliableTransmissionProtocol: ACK timeout for packet %08X\n", it->first);
            timeout = true;
            statistics_.packetsLost++;
            it = ackTimeouts_.erase(it);
        } else {
            ++it;
        }
    }
    
    if (timeout) {
        statusIt->second.state = TX_STATE_RETRYING;
        statusIt->second.result = TX_TIMEOUT;
        return true;
    }
    
    return false;
}

// ===========================
// RETRY LOGIC
// ===========================

bool ReliableTransmissionProtocol::shouldRetryTransmission(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    return config_.enableRetransmission && 
           it->second.retryCount < config_.maxRetries &&
           (it->second.state == TX_STATE_RETRYING || it->second.state == TX_STATE_FAILED);
}

unsigned long ReliableTransmissionProtocol::calculateRetryDelay(uint32_t transmissionId, uint8_t retryCount) {
    unsigned long delay;
    
    if (config_.adaptiveRetry) {
        // Adaptive retry based on network conditions
        float lossRate = getPacketLossRate();
        float multiplier = 1.0 + (lossRate * 2.0);  // Increase delay with higher loss rate
        delay = config_.initialRetryDelay * pow(config_.backoffMultiplier, retryCount) * multiplier;
    } else {
        // Standard exponential backoff
        delay = config_.initialRetryDelay * pow(config_.backoffMultiplier, retryCount);
    }
    
    // Cap at maximum retry delay
    delay = std::min(delay, config_.maxRetryDelay);
    
    return delay;
}

bool ReliableTransmissionProtocol::scheduleRetry(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    unsigned long retryDelay = calculateRetryDelay(transmissionId, it->second.retryCount);
    nextRetryTime_[transmissionId] = millis() + retryDelay;
    retryDelays_[transmissionId] = retryDelay;
    
    it->second.state = TX_STATE_RETRYING;
    it->second.retryCount++;
    
    DEBUG_PRINTF("ReliableTransmissionProtocol: Retry scheduled for transmission %08X (attempt %d, delay %lu ms)\n",
                 transmissionId, it->second.retryCount, retryDelay);
    
    return true;
}

// ===========================
// BANDWIDTH MANAGEMENT
// ===========================

bool ReliableTransmissionProtocol::canTransmitNow() {
    if (!config_.enableBandwidthManagement) {
        return true;
    }
    
    unsigned long currentTime = millis();
    
    // Check minimum packet interval
    if (currentTime - lastTransmissionTime_ < config_.minPacketInterval) {
        return false;
    }
    
    // Check burst size
    if (currentBurstSize_ >= config_.burstSize) {
        // Check if burst window has expired
        if (currentTime - lastTransmissionTime_ < 1000) {
            return false;
        }
        currentBurstSize_ = 0;  // Reset burst counter
    }
    
    return true;
}

void ReliableTransmissionProtocol::updateBandwidthUsage(uint32_t bytes) {
    lastTransmissionTime_ = millis();
    currentBurstSize_ += bytes;
    
    // Track for throughput calculation
    recentTransmissions_.push_back(bytes);
    transmissionTimes_.push_back(lastTransmissionTime_);
    
    // Limit history size
    if (recentTransmissions_.size() > config_.statsWindowSize) {
        recentTransmissions_.erase(recentTransmissions_.begin());
        transmissionTimes_.erase(transmissionTimes_.begin());
    }
}

bool ReliableTransmissionProtocol::setBandwidthLimit(uint32_t bytesPerSecond) {
    bandwidthLimit_ = bytesPerSecond;
    config_.maxBytesPerSecond = bytesPerSecond;
    return true;
}

uint32_t ReliableTransmissionProtocol::getBandwidthLimit() const {
    return bandwidthLimit_;
}

float ReliableTransmissionProtocol::getCurrentBandwidthUsage() const {
    return statistics_.throughput;
}

bool ReliableTransmissionProtocol::throttleTransmission() {
    // Simple throttling: delay if current usage exceeds limit
    if (statistics_.throughput > bandwidthLimit_) {
        delay(config_.minPacketInterval);
        return true;
    }
    return false;
}

// ===========================
// STATISTICS
// ===========================

void ReliableTransmissionProtocol::updateStatistics(const ReliablePacket& packet, bool success) {
    if (!config_.enableStatistics) {
        return;
    }
    
    if (success) {
        statistics_.packetsTransmitted++;
        statistics_.bytesTransmitted += packet.payloadSize;
    } else {
        statistics_.packetsLost++;
    }
    
    calculatePacketLossRate();
    calculateThroughput();
    
    statistics_.lastUpdateTime = millis();
}

void ReliableTransmissionProtocol::updateLatency(uint32_t transmissionId, unsigned long latency) {
    latencyHistory_.push_back(latency);
    
    if (latencyHistory_.size() > config_.statsWindowSize) {
        latencyHistory_.erase(latencyHistory_.begin());
    }
    
    // Calculate average latency
    if (!latencyHistory_.empty()) {
        float sum = 0;
        for (float lat : latencyHistory_) {
            sum += lat;
        }
        statistics_.averageLatency = sum / latencyHistory_.size();
    }
}

void ReliableTransmissionProtocol::calculatePacketLossRate() {
    uint32_t totalPackets = statistics_.packetsTransmitted + statistics_.packetsLost;
    if (totalPackets > 0) {
        statistics_.packetLossRate = (float)statistics_.packetsLost / totalPackets;
    } else {
        statistics_.packetLossRate = 0.0;
    }
}

void ReliableTransmissionProtocol::calculateThroughput() {
    if (transmissionTimes_.size() < 2) {
        statistics_.throughput = 0.0;
        return;
    }
    
    unsigned long timeSpan = transmissionTimes_.back() - transmissionTimes_.front();
    if (timeSpan == 0) {
        statistics_.throughput = 0.0;
        return;
    }
    
    uint32_t totalBytes = 0;
    for (uint32_t bytes : recentTransmissions_) {
        totalBytes += bytes;
    }
    
    statistics_.throughput = (float)totalBytes / (timeSpan / 1000.0);  // Bytes per second
}

BandwidthStats ReliableTransmissionProtocol::getStatistics() const {
    return statistics_;
}

void ReliableTransmissionProtocol::resetStatistics() {
    memset(&statistics_, 0, sizeof(BandwidthStats));
    statistics_.lastUpdateTime = millis();
    latencyHistory_.clear();
    recentTransmissions_.clear();
    transmissionTimes_.clear();
}

float ReliableTransmissionProtocol::getPacketLossRate() const {
    return statistics_.packetLossRate;
}

float ReliableTransmissionProtocol::getAverageLatency() const {
    return statistics_.averageLatency;
}

float ReliableTransmissionProtocol::getThroughput() const {
    return statistics_.throughput;
}

// ===========================
// STATUS AND MONITORING
// ===========================

TransmissionStatus ReliableTransmissionProtocol::getTransmissionStatus(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it != activeTransmissions_.end()) {
        return it->second;
    }
    
    TransmissionStatus emptyStatus;
    emptyStatus.transmissionId = 0;
    emptyStatus.state = TX_STATE_FAILED;
    emptyStatus.result = TX_INVALID_DATA;
    return emptyStatus;
}

std::vector<TransmissionStatus> ReliableTransmissionProtocol::getAllTransmissions() {
    std::vector<TransmissionStatus> statuses;
    
    for (const auto& pair : activeTransmissions_) {
        statuses.push_back(pair.second);
    }
    
    return statuses;
}

bool ReliableTransmissionProtocol::isTransmissionComplete(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    return it->second.state == TX_STATE_COMPLETED;
}

bool ReliableTransmissionProtocol::isTransmissionActive(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return false;
    }
    
    return it->second.state != TX_STATE_COMPLETED && 
           it->second.state != TX_STATE_FAILED &&
           it->second.state != TX_STATE_CANCELLED;
}

// ===========================
// PROCESSING
// ===========================

void ReliableTransmissionProtocol::process() {
    if (!initialized_) {
        return;
    }
    
    processTransmissionQueue();
    processRetries();
    processAcknowledgments();
    processBandwidthManagement();
    
    // Periodic cleanup
    static unsigned long lastCleanup = 0;
    if (millis() - lastCleanup > 60000) {  // Every minute
        cleanupCompletedTransmissions();
        cleanupExpiredTransmissions();
        removeOldStatistics();
        lastCleanup = millis();
    }
}

void ReliableTransmissionProtocol::processTransmissionQueue() {
    if (transmissionQueue_.empty()) {
        return;
    }
    
    // Process next transmission based on priority
    TransmissionRequest nextRequest = getNextTransmission();
    
    if (nextRequest.transmissionId != 0) {
        if (startTransmission(nextRequest.transmissionId)) {
            // Transmit first packet
            transmitNextPacket(nextRequest.transmissionId);
        }
    }
}

void ReliableTransmissionProtocol::processRetries() {
    unsigned long currentTime = millis();
    
    for (auto& pair : activeTransmissions_) {
        uint32_t transmissionId = pair.first;
        TransmissionStatus& status = pair.second;
        
        // Check for ACK timeouts
        if (status.state == TX_STATE_AWAITING_ACK) {
            if (checkAckTimeout(transmissionId)) {
                if (shouldRetryTransmission(transmissionId)) {
                    scheduleRetry(transmissionId);
                    statistics_.packetsRetransmitted++;
                } else {
                    status.state = TX_STATE_FAILED;
                    status.result = TX_MAX_RETRIES;
                    
                    if (transmissionCompleteCallback_) {
                        transmissionCompleteCallback_(transmissionId, false);
                    }
                    
                    if (transmissionErrorCallback_) {
                        transmissionErrorCallback_(transmissionId, TX_MAX_RETRIES);
                    }
                }
            }
        }
        
        // Process scheduled retries
        if (status.state == TX_STATE_RETRYING) {
            auto retryIt = nextRetryTime_.find(transmissionId);
            if (retryIt != nextRetryTime_.end() && currentTime >= retryIt->second) {
                // Time to retry
                status.state = TX_STATE_TRANSMITTING;
                transmitNextPacket(transmissionId);
                nextRetryTime_.erase(retryIt);
            }
        }
        
        // Continue transmitting packets
        if (status.state == TX_STATE_TRANSMITTING) {
            if (!transmitNextPacket(transmissionId)) {
                // Check if all packets are acknowledged
                if (status.packetsAcknowledged == status.totalPackets) {
                    status.state = TX_STATE_COMPLETED;
                    status.result = TX_SUCCESS;
                } else {
                    status.state = TX_STATE_AWAITING_ACK;
                }
            }
        }
    }
}

void ReliableTransmissionProtocol::processAcknowledgments() {
    // Process received ACKs
    for (auto it = receivedAcks_.begin(); it != receivedAcks_.end();) {
        if (processAcknowledgment(it->second.transmissionId, it->first)) {
            it = receivedAcks_.erase(it);
        } else {
            ++it;
        }
    }
}

void ReliableTransmissionProtocol::processBandwidthManagement() {
    if (config_.enableBandwidthManagement) {
        calculateThroughput();
        
        // Throttle if exceeding limits
        if (statistics_.throughput > bandwidthLimit_) {
            throttleTransmission();
        }
    }
}

// ===========================
// UTILITY METHODS
// ===========================

uint32_t ReliableTransmissionProtocol::generateTransmissionId() {
    return nextTransmissionId_++;
}

uint32_t ReliableTransmissionProtocol::generatePacketId() {
    return nextPacketId_++;
}

uint32_t ReliableTransmissionProtocol::calculateCRC32(const std::vector<uint8_t>& data) {
    return esp_crc32_le(0, data.data(), data.size());
}

bool ReliableTransmissionProtocol::validatePacket(const ReliablePacket& packet) {
    if (packet.payload.empty()) {
        return false;
    }
    
    uint32_t calculatedCRC = calculateCRC32(packet.payload);
    return calculatedCRC == packet.crc32;
}

TransmissionRequest ReliableTransmissionProtocol::getNextTransmission() {
    if (transmissionQueue_.empty()) {
        TransmissionRequest empty;
        empty.transmissionId = 0;
        return empty;
    }
    
    if (!config_.priorityBasedScheduling) {
        // FIFO
        TransmissionRequest request = transmissionQueue_.front();
        transmissionQueue_.pop();
        return request;
    }
    
    // Priority-based scheduling
    TransmissionRequest highestPriority;
    highestPriority.transmissionId = 0;
    highestPriority.priority = PRIORITY_BACKGROUND;
    
    std::queue<TransmissionRequest> tempQueue;
    TransmissionRequest selected;
    selected.transmissionId = 0;
    
    while (!transmissionQueue_.empty()) {
        TransmissionRequest request = transmissionQueue_.front();
        transmissionQueue_.pop();
        
        // Check for expired requests
        if (exceedsQueueTimeout(request)) {
            // Skip expired requests
            auto it = activeTransmissions_.find(request.transmissionId);
            if (it != activeTransmissions_.end()) {
                it->second.state = TX_STATE_FAILED;
                it->second.result = TX_TIMEOUT;
            }
            continue;
        }
        
        if (selected.transmissionId == 0 || request.priority < selected.priority) {
            if (selected.transmissionId != 0) {
                tempQueue.push(selected);
            }
            selected = request;
        } else {
            tempQueue.push(request);
        }
    }
    
    // Restore queue
    while (!tempQueue.empty()) {
        transmissionQueue_.push(tempQueue.front());
        tempQueue.pop();
    }
    
    return selected;
}

bool ReliableTransmissionProtocol::exceedsQueueTimeout(const TransmissionRequest& request) {
    return (millis() - request.queueTime) > config_.queueTimeout;
}

float ReliableTransmissionProtocol::calculateProgress(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end() || it->second.totalPackets == 0) {
        return 0.0;
    }
    
    return (float)it->second.packetsAcknowledged / it->second.totalPackets * 100.0;
}

unsigned long ReliableTransmissionProtocol::estimateTimeRemaining(uint32_t transmissionId) {
    auto it = activeTransmissions_.find(transmissionId);
    if (it == activeTransmissions_.end()) {
        return 0;
    }
    
    if (it->second.packetsAcknowledged == 0) {
        return 0;  // Not enough data yet
    }
    
    unsigned long elapsedTime = millis() - it->second.startTime;
    float completionRatio = (float)it->second.packetsAcknowledged / it->second.totalPackets;
    
    if (completionRatio > 0) {
        unsigned long estimatedTotal = elapsedTime / completionRatio;
        return estimatedTotal - elapsedTime;
    }
    
    return 0;
}

void ReliableTransmissionProtocol::cleanupCompletedTransmissions() {
    for (auto it = activeTransmissions_.begin(); it != activeTransmissions_.end();) {
        if (it->second.state == TX_STATE_COMPLETED) {
            // Clean up associated data
            transmissionPackets_.erase(it->first);
            packetAckStatus_.erase(it->first);
            nextRetryTime_.erase(it->first);
            retryDelays_.erase(it->first);
            
            it = activeTransmissions_.erase(it);
        } else {
            ++it;
        }
    }
}

void ReliableTransmissionProtocol::cleanupExpiredTransmissions() {
    unsigned long currentTime = millis();
    
    for (auto it = activeTransmissions_.begin(); it != activeTransmissions_.end();) {
        // Remove transmissions that have been inactive for too long
        if (currentTime - it->second.lastActivityTime > config_.queueTimeout) {
            it->second.state = TX_STATE_FAILED;
            it->second.result = TX_TIMEOUT;
            
            if (transmissionErrorCallback_) {
                transmissionErrorCallback_(it->first, TX_TIMEOUT);
            }
            
            ++it;
        } else {
            ++it;
        }
    }
}

void ReliableTransmissionProtocol::removeOldStatistics() {
    // Keep statistics within window size
    while (latencyHistory_.size() > config_.statsWindowSize) {
        latencyHistory_.erase(latencyHistory_.begin());
    }
    
    while (recentTransmissions_.size() > config_.statsWindowSize) {
        recentTransmissions_.erase(recentTransmissions_.begin());
        transmissionTimes_.erase(transmissionTimes_.begin());
    }
}

// ===========================
// CALLBACKS
// ===========================

void ReliableTransmissionProtocol::setTransmissionCompleteCallback(TransmissionCompleteCallback callback) {
    transmissionCompleteCallback_ = callback;
}

void ReliableTransmissionProtocol::setTransmissionProgressCallback(TransmissionProgressCallback callback) {
    transmissionProgressCallback_ = callback;
}

void ReliableTransmissionProtocol::setPacketReceivedCallback(PacketReceivedCallback callback) {
    packetReceivedCallback_ = callback;
}

void ReliableTransmissionProtocol::setTransmissionErrorCallback(TransmissionErrorCallback callback) {
    transmissionErrorCallback_ = callback;
}

bool ReliableTransmissionProtocol::handleReceivedPacket(const ReliablePacket& packet) {
    if (!initialized_ || !validatePacket(packet)) {
        return false;
    }
    
    statistics_.packetsReceived++;
    statistics_.bytesReceived += packet.payloadSize;
    
    // Send acknowledgment if required
    if (packet.requiresAck) {
        sendAcknowledgment(packet.packetId, packet.transmissionId, packet.destination);
    }
    
    // Call callback
    if (packetReceivedCallback_) {
        packetReceivedCallback_(packet);
    }
    
    return true;
}

// ===========================
// CONFIGURATION
// ===========================

ReliableTransmissionConfig ReliableTransmissionProtocol::getConfig() const {
    return config_;
}

bool ReliableTransmissionProtocol::setConfig(const ReliableTransmissionConfig& config) {
    return configure(config);
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

ReliableTransmissionProtocol* createReliableTransmissionProtocol(MeshInterface* meshInterface) {
    ReliableTransmissionProtocol* protocol = new ReliableTransmissionProtocol();
    if (protocol->init(meshInterface)) {
        return protocol;
    } else {
        delete protocol;
        return nullptr;
    }
}

ReliableTransmissionConfig createDefaultRTPConfig() {
    ReliableTransmissionConfig config;
    config.enableRetransmission = true;
    config.maxRetries = RTP_MAX_RETRIES;
    config.initialRetryDelay = RTP_INITIAL_RETRY_DELAY;
    config.maxRetryDelay = RTP_MAX_RETRY_DELAY;
    config.backoffMultiplier = RTP_BACKOFF_MULTIPLIER;
    
    config.enableAcknowledgments = true;
    config.ackTimeout = RTP_ACK_TIMEOUT;
    config.ackBatchSize = RTP_ACK_BATCH_SIZE;
    
    config.enableBandwidthManagement = true;
    config.maxBytesPerSecond = RTP_MAX_BYTES_PER_SECOND;
    config.burstSize = RTP_BURST_SIZE;
    config.minPacketInterval = RTP_MIN_PACKET_INTERVAL;
    
    config.maxQueueSize = RTP_MAX_QUEUE_SIZE;
    config.queueTimeout = RTP_QUEUE_TIMEOUT;
    
    config.enableStatistics = true;
    config.statsWindowSize = RTP_STATS_WINDOW_SIZE;
    
    config.priorityBasedScheduling = true;
    config.adaptiveRetry = true;
    
    return config;
}

ReliableTransmissionConfig createLowBandwidthConfig() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    
    // Reduce bandwidth usage
    config.maxBytesPerSecond = 512;      // 512 bytes/s
    config.burstSize = 2048;             // 2KB burst
    config.minPacketInterval = 200;      // 200ms between packets
    
    // More aggressive retries due to lossy environment
    config.maxRetries = 7;
    config.initialRetryDelay = 2000;     // 2 seconds
    config.backoffMultiplier = 2.5;
    config.adaptiveRetry = true;
    
    return config;
}

ReliableTransmissionConfig createHighReliabilityConfig() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    
    // Maximum reliability
    config.maxRetries = 10;
    config.initialRetryDelay = 1000;
    config.maxRetryDelay = 120000;       // 2 minutes max delay
    config.backoffMultiplier = 2.0;
    config.adaptiveRetry = true;
    
    config.ackTimeout = 10000;           // 10 second ACK timeout
    
    return config;
}

ReliableTransmissionConfig createBestEffortConfig() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    
    // Minimal reliability for best throughput
    config.enableRetransmission = false;
    config.enableAcknowledgments = false;
    config.enableBandwidthManagement = false;
    config.priorityBasedScheduling = false;
    
    return config;
}

const char* priorityToString(TransmissionPriority priority) {
    switch (priority) {
        case PRIORITY_CRITICAL:   return "CRITICAL";
        case PRIORITY_HIGH:       return "HIGH";
        case PRIORITY_NORMAL:     return "NORMAL";
        case PRIORITY_LOW:        return "LOW";
        case PRIORITY_BACKGROUND: return "BACKGROUND";
        default:                  return "UNKNOWN";
    }
}

TransmissionPriority stringToPriority(const char* str) {
    if (strcmp(str, "CRITICAL") == 0)   return PRIORITY_CRITICAL;
    if (strcmp(str, "HIGH") == 0)       return PRIORITY_HIGH;
    if (strcmp(str, "NORMAL") == 0)     return PRIORITY_NORMAL;
    if (strcmp(str, "LOW") == 0)        return PRIORITY_LOW;
    if (strcmp(str, "BACKGROUND") == 0) return PRIORITY_BACKGROUND;
    return PRIORITY_NORMAL;
}

const char* stateToString(TransmissionState state) {
    switch (state) {
        case TX_STATE_QUEUED:        return "QUEUED";
        case TX_STATE_TRANSMITTING:  return "TRANSMITTING";
        case TX_STATE_AWAITING_ACK:  return "AWAITING_ACK";
        case TX_STATE_RETRYING:      return "RETRYING";
        case TX_STATE_COMPLETED:     return "COMPLETED";
        case TX_STATE_FAILED:        return "FAILED";
        case TX_STATE_CANCELLED:     return "CANCELLED";
        default:                     return "UNKNOWN";
    }
}

const char* resultToString(TransmissionResult result) {
    switch (result) {
        case TX_SUCCESS:        return "SUCCESS";
        case TX_PENDING:        return "PENDING";
        case TX_TIMEOUT:        return "TIMEOUT";
        case TX_MAX_RETRIES:    return "MAX_RETRIES";
        case TX_QUEUE_FULL:     return "QUEUE_FULL";
        case TX_INVALID_DATA:   return "INVALID_DATA";
        case TX_NETWORK_ERROR:  return "NETWORK_ERROR";
        case TX_CANCELLED:      return "CANCELLED";
        default:                return "UNKNOWN";
    }
}

bool isValidTransmissionRequest(const TransmissionRequest& request) {
    return request.transmissionId > 0 && 
           !request.data.empty() &&
           request.maxRetries > 0 &&
           request.retryDelay > 0;
}

bool isValidPacket(const ReliablePacket& packet) {
    return packet.packetId > 0 &&
           packet.transmissionId > 0 &&
           !packet.payload.empty() &&
           packet.sequenceNumber < packet.totalPackets;
}
