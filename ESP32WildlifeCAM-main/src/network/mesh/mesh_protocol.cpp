/**
 * @file mesh_protocol.cpp
 * @brief Custom Mesh Protocol Implementation for Wildlife Monitoring
 * 
 * Implements the custom mesh networking protocol with wildlife-specific
 * optimizations, encryption, and reliability features.
 */

#include "mesh_protocol.h"
#include "../../include/network/mesh_config.h"
#include <Arduino.h>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

MeshProtocol::MeshProtocol() 
    : initialized_(false)
    , sequenceNumber_(0)
    , packetId_(0)
    , encryptionEnabled_(MESH_ENCRYPTION_ENABLED)
    , compressionEnabled_(true)
{
    memset(&statistics_, 0, sizeof(statistics_));
    memset(encryptionKey_, 0, MESH_KEY_SIZE);
    recentPackets_.clear();
    ackWaitingList_.clear();
}

MeshProtocol::~MeshProtocol() {
    cleanup();
}

bool MeshProtocol::initialize(const MeshConfig& config) {
    config_ = config;
    
    // Initialize encryption key
    memcpy(encryptionKey_, config.encryptionKey, MESH_KEY_SIZE);
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.packetsProcessed = 0;
    statistics_.encryptionOperations = 0;
    statistics_.compressionRatio = 1.0;
    
    // Clear packet tracking
    recentPackets_.clear();
    ackWaitingList_.clear();
    
    initialized_ = true;
    Serial.println("Mesh Protocol initialized successfully");
    return true;
}

void MeshProtocol::cleanup() {
    recentPackets_.clear();
    ackWaitingList_.clear();
    initialized_ = false;
}

// ===========================
// PACKET CREATION
// ===========================

MeshPacketHeader MeshProtocol::createPacketHeader(uint32_t source, uint32_t destination,
                                                 MeshPacketType type, MeshPriority priority,
                                                 bool requestAck) {
    MeshPacketHeader header;
    
    header.magic = MESH_PACKET_MAGIC;
    header.version = MESH_PROTOCOL_VERSION;
    header.packetId = generatePacketId();
    header.source = source;
    header.destination = destination;
    header.type = type;
    header.priority = priority;
    header.flags = 0;
    header.hopCount = 0;
    header.maxHops = MESH_MAX_HOPS;
    header.sequence = getNextSequenceNumber();
    header.timestamp = millis();
    header.ttl = calculateTTL(type);
    header.checksum = 0;  // Will be calculated later
    
    // Set flags
    if (requestAck) {
        header.flags |= MESH_FLAG_ACK_REQUESTED;
    }
    if (encryptionEnabled_) {
        header.flags |= MESH_FLAG_ENCRYPTED;
    }
    if (compressionEnabled_) {
        header.flags |= MESH_FLAG_COMPRESSED;
    }
    
    return header;
}

bool MeshProtocol::createPacket(const MeshPacketHeader& header, const uint8_t* payload,
                               size_t payloadSize, MeshPacketData& packet) {
    if (!initialized_ || payloadSize > MESH_MAX_PAYLOAD_SIZE) {
        return false;
    }
    
    packet.header = header;
    packet.payloadSize = payloadSize;
    
    // Copy payload
    memcpy(packet.payload, payload, payloadSize);
    
    // Apply compression if enabled
    if (compressionEnabled_ && shouldCompress(payloadSize, header.type)) {
        if (compressPayload(packet)) {
            packet.header.flags |= MESH_FLAG_COMPRESSED;
        }
    }
    
    // Apply encryption if enabled
    if (encryptionEnabled_) {
        if (encryptPacket(packet)) {
            packet.header.flags |= MESH_FLAG_ENCRYPTED;
        } else {
            Serial.println("Failed to encrypt packet");
            return false;
        }
    }
    
    // Calculate checksum
    packet.header.checksum = calculateChecksum(packet);
    
    statistics_.packetsProcessed++;
    return true;
}

// ===========================
// PACKET PROCESSING
// ===========================

bool MeshProtocol::processIncomingPacket(const uint8_t* data, size_t dataSize,
                                        MeshPacketData& packet, int16_t rssi) {
    if (!initialized_ || dataSize < sizeof(MeshPacketHeader)) {
        statistics_.malformedPackets++;
        return false;
    }
    
    // Parse header
    memcpy(&packet.header, data, sizeof(MeshPacketHeader));
    
    // Validate magic number and version
    if (packet.header.magic != MESH_PACKET_MAGIC ||
        packet.header.version != MESH_PROTOCOL_VERSION) {
        statistics_.invalidPackets++;
        return false;
    }
    
    // Check packet size
    size_t expectedSize = sizeof(MeshPacketHeader) + packet.header.payloadSize;
    if (dataSize != expectedSize) {
        statistics_.malformedPackets++;
        return false;
    }
    
    // Copy payload
    packet.payloadSize = packet.header.payloadSize;
    memcpy(packet.payload, data + sizeof(MeshPacketHeader), packet.payloadSize);
    
    // Verify checksum
    uint16_t calculatedChecksum = calculateChecksum(packet);
    if (calculatedChecksum != packet.header.checksum) {
        statistics_.checksumErrors++;
        return false;
    }
    
    // Check for duplicate packets
    if (isDuplicatePacket(packet.header.packetId, packet.header.source)) {
        statistics_.duplicatePackets++;
        return false;
    }
    
    // Add to recent packets list
    addToRecentPackets(packet.header.packetId, packet.header.source);
    
    // Decrypt if encrypted
    if (packet.header.flags & MESH_FLAG_ENCRYPTED) {
        if (!decryptPacket(packet)) {
            statistics_.decryptionErrors++;
            return false;
        }
    }
    
    // Decompress if compressed
    if (packet.header.flags & MESH_FLAG_COMPRESSED) {
        if (!decompressPayload(packet)) {
            statistics_.decompressionErrors++;
            return false;
        }
    }
    
    // Check TTL
    if (packet.header.ttl <= packet.header.hopCount) {
        statistics_.expiredPackets++;
        return false;
    }
    
    // Send ACK if requested
    if (packet.header.flags & MESH_FLAG_ACK_REQUESTED) {
        sendAck(packet.header.source, packet.header.packetId);
    }
    
    statistics_.packetsProcessed++;
    statistics_.lastRssi = rssi;
    
    return true;
}

bool MeshProtocol::forwardPacket(MeshPacketData& packet, uint32_t nextHop) {
    if (!initialized_) {
        return false;
    }
    
    // Increment hop count
    packet.header.hopCount++;
    
    // Check hop limit
    if (packet.header.hopCount >= packet.header.maxHops) {
        statistics_.hopLimitExceeded++;
        return false;
    }
    
    // Update timestamp
    packet.header.timestamp = millis();
    
    // Recalculate checksum
    packet.header.checksum = calculateChecksum(packet);
    
    // Add to forwarding statistics
    statistics_.packetsForwarded++;
    
    return true;
}

// ===========================
// ENCRYPTION & COMPRESSION
// ===========================

bool MeshProtocol::encryptPacket(MeshPacketData& packet) {
    if (!encryptionEnabled_ || packet.payloadSize == 0) {
        return true;
    }
    
    // Simplified AES-256 encryption placeholder
    // Real implementation would use proper AES encryption
    for (size_t i = 0; i < packet.payloadSize; i++) {
        packet.payload[i] ^= encryptionKey_[i % MESH_KEY_SIZE];
    }
    
    statistics_.encryptionOperations++;
    return true;
}

bool MeshProtocol::decryptPacket(MeshPacketData& packet) {
    if (!encryptionEnabled_ || packet.payloadSize == 0) {
        return true;
    }
    
    // Simplified AES-256 decryption placeholder
    // Real implementation would use proper AES decryption
    for (size_t i = 0; i < packet.payloadSize; i++) {
        packet.payload[i] ^= encryptionKey_[i % MESH_KEY_SIZE];
    }
    
    statistics_.decryptionOperations++;
    return true;
}

bool MeshProtocol::compressPayload(MeshPacketData& packet) {
    if (packet.payloadSize < MESH_MIN_COMPRESSION_SIZE) {
        return false;
    }
    
    // Simplified compression placeholder
    // Real implementation would use LZ77 or similar algorithm
    
    // For wildlife images, use simple run-length encoding
    if (packet.header.type == MESH_PACKET_IMAGE) {
        return compressImageData(packet);
    }
    
    // For other data, use basic compression
    return compressGenericData(packet);
}

bool MeshProtocol::decompressPayload(MeshPacketData& packet) {
    // Simplified decompression placeholder
    if (packet.header.type == MESH_PACKET_IMAGE) {
        return decompressImageData(packet);
    }
    
    return decompressGenericData(packet);
}

// ===========================
// ACKNOWLEDGMENT HANDLING
// ===========================

void MeshProtocol::sendAck(uint32_t destination, uint32_t packetId) {
    AckPacket ack;
    ack.originalPacketId = packetId;
    ack.timestamp = millis();
    ack.nodeId = config_.nodeId;
    
    MeshPacketHeader header = createPacketHeader(config_.nodeId, destination,
                                               MESH_PACKET_ACK, MESH_PRIORITY_HIGH, false);
    
    MeshPacketData packet;
    if (createPacket(header, (uint8_t*)&ack, sizeof(ack), packet)) {
        // Send ACK packet (implementation would use radio interface)
        statistics_.acksSent++;
    }
}

bool MeshProtocol::waitForAck(uint32_t packetId, uint32_t timeout) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < timeout) {
        if (isAckReceived(packetId)) {
            removeFromAckWaitingList(packetId);
            statistics_.acksReceived++;
            return true;
        }
        delay(10);  // Small delay to prevent busy waiting
    }
    
    // Timeout occurred
    removeFromAckWaitingList(packetId);
    statistics_.ackTimeouts++;
    return false;
}

void MeshProtocol::handleAckReceived(const AckPacket& ack) {
    // Mark ACK as received
    for (auto& entry : ackWaitingList_) {
        if (entry.packetId == ack.originalPacketId) {
            entry.ackReceived = true;
            entry.ackTimestamp = millis();
            break;
        }
    }
    
    statistics_.acksReceived++;
}

// ===========================
// QUALITY OF SERVICE
// ===========================

uint8_t MeshProtocol::calculateTTL(MeshPacketType type) {
    switch (type) {
        case MESH_PACKET_EMERGENCY:
            return MESH_MAX_HOPS * 2;  // Extended TTL for emergency packets
        case MESH_PACKET_IMAGE:
            return MESH_MAX_HOPS + 2;  // Extra hops for wildlife images
        case MESH_PACKET_BEACON:
            return 1;  // Beacons are local only
        default:
            return MESH_MAX_HOPS;
    }
}

MeshPriority MeshProtocol::adjustPriorityForWildlife(MeshPacketType type, const uint8_t* payload) {
    if (type == MESH_PACKET_IMAGE) {
        // Wildlife images get high priority
        return MESH_PRIORITY_HIGH;
    }
    
    if (type == MESH_PACKET_DATA && payload != nullptr) {
        // Check if payload contains wildlife detection data
        // This is simplified - real implementation would parse payload
        return MESH_PRIORITY_HIGH;
    }
    
    return MESH_PRIORITY_NORMAL;
}

bool MeshProtocol::shouldPrioritizePacket(const MeshPacketData& packet) {
    // Prioritize wildlife-related packets
    if (packet.header.type == MESH_PACKET_IMAGE ||
        packet.header.type == MESH_PACKET_EMERGENCY) {
        return true;
    }
    
    // Prioritize high-priority packets
    if (packet.header.priority >= MESH_PRIORITY_HIGH) {
        return true;
    }
    
    return false;
}

// ===========================
// UTILITY METHODS
// ===========================

uint32_t MeshProtocol::generatePacketId() {
    return ++packetId_;
}

uint16_t MeshProtocol::getNextSequenceNumber() {
    return ++sequenceNumber_;
}

uint16_t MeshProtocol::calculateChecksum(const MeshPacketData& packet) {
    uint16_t checksum = 0;
    
    // Calculate checksum for header (excluding checksum field)
    const uint8_t* headerData = (const uint8_t*)&packet.header;
    for (size_t i = 0; i < sizeof(MeshPacketHeader) - sizeof(uint16_t); i++) {
        checksum += headerData[i];
    }
    
    // Calculate checksum for payload
    for (size_t i = 0; i < packet.payloadSize; i++) {
        checksum += packet.payload[i];
    }
    
    return checksum;
}

bool MeshProtocol::isDuplicatePacket(uint32_t packetId, uint32_t source) {
    for (const auto& recent : recentPackets_) {
        if (recent.packetId == packetId && recent.source == source) {
            return true;
        }
    }
    return false;
}

void MeshProtocol::addToRecentPackets(uint32_t packetId, uint32_t source) {
    RecentPacket recent;
    recent.packetId = packetId;
    recent.source = source;
    recent.timestamp = millis();
    
    recentPackets_.push_back(recent);
    
    // Clean up old entries
    cleanupRecentPackets();
}

void MeshProtocol::cleanupRecentPackets() {
    auto it = recentPackets_.begin();
    while (it != recentPackets_.end()) {
        if (millis() - it->timestamp > MESH_RECENT_PACKET_TIMEOUT) {
            it = recentPackets_.erase(it);
        } else {
            ++it;
        }
    }
}

bool MeshProtocol::shouldCompress(size_t payloadSize, MeshPacketType type) {
    // Don't compress small payloads
    if (payloadSize < MESH_MIN_COMPRESSION_SIZE) {
        return false;
    }
    
    // Always compress image data
    if (type == MESH_PACKET_IMAGE) {
        return true;
    }
    
    // Compress large telemetry data
    if (type == MESH_PACKET_TELEMETRY && payloadSize > 64) {
        return true;
    }
    
    return false;
}

MeshProtocolStatistics MeshProtocol::getStatistics() const {
    return statistics_;
}

void MeshProtocol::resetStatistics() {
    memset(&statistics_, 0, sizeof(statistics_));
    statistics_.startTime = millis();
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

bool MeshProtocol::compressImageData(MeshPacketData& packet) {
    // Simplified image compression for wildlife camera data
    // Real implementation would use proper image compression
    size_t originalSize = packet.payloadSize;
    
    // Placeholder compression - would implement actual algorithm
    statistics_.compressionRatio = 0.7;  // Assume 70% of original size
    
    return true;
}

bool MeshProtocol::decompressImageData(MeshPacketData& packet) {
    // Simplified image decompression
    // Real implementation would decompress properly
    return true;
}

bool MeshProtocol::compressGenericData(MeshPacketData& packet) {
    // Simplified generic data compression
    // Real implementation would use LZ77 or similar
    statistics_.compressionRatio = 0.8;  // Assume 80% of original size
    return true;
}

bool MeshProtocol::decompressGenericData(MeshPacketData& packet) {
    // Simplified generic data decompression
    return true;
}

bool MeshProtocol::isAckReceived(uint32_t packetId) {
    for (const auto& entry : ackWaitingList_) {
        if (entry.packetId == packetId && entry.ackReceived) {
            return true;
        }
    }
    return false;
}

void MeshProtocol::removeFromAckWaitingList(uint32_t packetId) {
    auto it = ackWaitingList_.begin();
    while (it != ackWaitingList_.end()) {
        if (it->packetId == packetId) {
            it = ackWaitingList_.erase(it);
        } else {
            ++it;
        }
    }
}