#include "network_optimizer.h"
#include "../config.h"

// WildlifeDataCompressor implementation
size_t WildlifeDataCompressor::compressDetection(const WildlifeObservation& obs, uint8_t* output) {
    if (!output) return 0;
    
    // Simplified compression for demonstration
    size_t originalSize = sizeof(WildlifeObservation);
    size_t compressedSize = originalSize * 0.6f; // Simulate 40% compression
    
    // Simple byte copying with mock compression
    memcpy(output, &obs, min(originalSize, compressedSize));
    
    updateCompressionStats(originalSize, compressedSize);
    return compressedSize;
}

size_t WildlifeDataCompressor::compressSensorData(const std::vector<float>& data, uint8_t* output) {
    if (!output || data.empty()) return 0;
    
    size_t originalSize = data.size() * sizeof(float);
    
    // Delta compression simulation
    float previous = 0;
    size_t outputPos = 0;
    
    for (float value : data) {
        float delta = value - previous;
        // Store delta as int16_t for compression
        int16_t deltaPacked = (int16_t)(delta * 100); // Scale for precision
        memcpy(output + outputPos, &deltaPacked, sizeof(int16_t));
        outputPos += sizeof(int16_t);
        previous = value;
    }
    
    updateCompressionStats(originalSize, outputPos);
    return outputPos;
}

size_t WildlifeDataCompressor::compressWildlifeImage(const uint8_t* image, uint16_t width, uint16_t height,
                                                    uint8_t* compressed, float quality) {
    if (!image || !compressed) return 0;
    
    size_t originalSize = width * height;
    
    // Simplified image compression (mock JPEG-like compression)
    size_t compressedSize = (size_t)(originalSize * quality);
    
    // Simple compression: store every nth pixel based on quality
    int step = (int)(1.0f / quality);
    size_t outputPos = 0;
    
    for (uint16_t y = 0; y < height; y += step) {
        for (uint16_t x = 0; x < width; x += step) {
            compressed[outputPos++] = image[y * width + x];
        }
    }
    
    updateCompressionStats(originalSize, outputPos);
    return outputPos;
}

void WildlifeDataCompressor::updateCompressionStats(size_t original, size_t compressed) {
    stats.totalCompressions++;
    stats.totalOriginalBytes += original;
    stats.totalCompressedBytes += compressed;
    
    if (stats.totalOriginalBytes > 0) {
        stats.averageCompressionRatio = (float)stats.totalCompressedBytes / stats.totalOriginalBytes;
    }
}

void WildlifeDataCompressor::resetStats() {
    stats = CompressionStats();
}

// IntelligentMessageBatcher implementation
IntelligentMessageBatcher::IntelligentMessageBatcher() 
    : lastBatchTime(0), maxBatchSize(10), batchTimeout(5000) {
}

IntelligentMessageBatcher::~IntelligentMessageBatcher() {
    cleanup();
}

bool IntelligentMessageBatcher::init(size_t maxBatchSize, unsigned long timeoutMs) {
    this->maxBatchSize = maxBatchSize;
    this->batchTimeout = timeoutMs;
    this->lastBatchTime = millis();
    
    DEBUG_PRINTLN("Intelligent Message Batcher initialized");
    return true;
}

void IntelligentMessageBatcher::cleanup() {
    clearBatches();
}

void IntelligentMessageBatcher::addMessage(const String& type, const String& payload, int priority) {
    // Find existing batch for this message type
    BatchedMessage* targetBatch = nullptr;
    
    for (auto& batch : batches) {
        if (batch.messageType == type && batch.payloads.size() < maxBatchSize) {
            targetBatch = &batch;
            break;
        }
    }
    
    // Create new batch if needed
    if (!targetBatch) {
        batches.emplace_back();
        targetBatch = &batches.back();
        targetBatch->messageType = type;
        targetBatch->createdTime = millis();
        targetBatch->deadline = targetBatch->createdTime + batchTimeout;
    }
    
    targetBatch->payloads.push_back(payload);
    targetBatch->priority = max(targetBatch->priority, priority);
}

std::vector<IntelligentMessageBatcher::BatchedMessage> IntelligentMessageBatcher::createOptimalBatches() {
    std::vector<BatchedMessage> readyBatches;
    unsigned long currentTime = millis();
    
    auto it = batches.begin();
    while (it != batches.end()) {
        bool shouldBatch = false;
        
        // Check if batch is full
        if (it->payloads.size() >= maxBatchSize) {
            shouldBatch = true;
        }
        // Check if batch has timed out
        else if (currentTime >= it->deadline) {
            shouldBatch = true;
        }
        // Check if high priority message needs immediate batching
        else if (shouldForceBatch(it->priority, currentTime - it->createdTime)) {
            shouldBatch = true;
        }
        
        if (shouldBatch) {
            readyBatches.push_back(*it);
            it = batches.erase(it);
        } else {
            ++it;
        }
    }
    
    optimizeBatchOrder(readyBatches);
    lastBatchTime = currentTime;
    
    return readyBatches;
}

bool IntelligentMessageBatcher::shouldForceBatch(int priority, unsigned long age) {
    // High priority messages (>= 8) batch immediately
    if (priority >= 8) return true;
    
    // Medium priority messages batch after 2 seconds
    if (priority >= 5 && age > 2000) return true;
    
    return false;
}

void IntelligentMessageBatcher::optimizeBatchOrder(std::vector<BatchedMessage>& batches) {
    // Sort by priority (descending)
    std::sort(batches.begin(), batches.end(), 
              [](const BatchedMessage& a, const BatchedMessage& b) {
                  return a.priority > b.priority;
              });
}

size_t IntelligentMessageBatcher::getPendingMessageCount() const {
    size_t count = 0;
    for (const auto& batch : batches) {
        count += batch.payloads.size();
    }
    return count;
}

void IntelligentMessageBatcher::clearBatches() {
    batches.clear();
}

// TopologyOptimizer implementation
TopologyOptimizer::TopologyOptimizer() {
}

TopologyOptimizer::~TopologyOptimizer() {
    cleanup();
}

bool TopologyOptimizer::init() {
    DEBUG_PRINTLN("Topology Optimizer initialized");
    return true;
}

void TopologyOptimizer::cleanup() {
    nodeMap.clear();
    adjacencyList.clear();
    routingTable.clear();
}

std::vector<std::pair<String, String>> TopologyOptimizer::calculateOptimalTopology(
    const std::map<String, std::vector<std::pair<String, float>>>& linkQuality) {
    
    std::vector<std::pair<String, String>> topology;
    
    // Simplified minimum spanning tree algorithm
    for (const auto& node : linkQuality) {
        const String& nodeId = node.first;
        const auto& connections = node.second;
        
        // Find best connection for this node
        float bestQuality = 0;
        String bestNeighbor;
        
        for (const auto& connection : connections) {
            if (connection.second > bestQuality) {
                bestQuality = connection.second;
                bestNeighbor = connection.first;
            }
        }
        
        if (!bestNeighbor.isEmpty() && bestQuality > 0.5f) {
            topology.push_back(std::make_pair(nodeId, bestNeighbor));
        }
    }
    
    return topology;
}

String TopologyOptimizer::selectNextHop(const String& destination, 
                                       const std::map<String, float>& nodeLoads) {
    auto routeIt = routingTable.find(destination);
    if (routeIt != routingTable.end()) {
        const String& nextHop = routeIt->second.nextHop;
        
        // Check if next hop is not overloaded
        auto loadIt = nodeLoads.find(nextHop);
        if (loadIt != nodeLoads.end() && loadIt->second < 0.8f) {
            return nextHop;
        }
    }
    
    // Find alternative route with lowest load
    String bestHop;
    float lowestLoad = 1.0f;
    
    for (const auto& load : nodeLoads) {
        if (load.second < lowestLoad) {
            lowestLoad = load.second;
            bestHop = load.first;
        }
    }
    
    return bestHop;
}

void TopologyOptimizer::addNode(const String& nodeId, float signalStrength, float reliability) {
    NodeInfo& info = nodeMap[nodeId];
    info.nodeId = nodeId;
    info.signalStrength = signalStrength;
    info.reliability = reliability;
    info.lastSeen = millis();
    
    DEBUG_PRINTF("Added node: %s (signal: %.1f, reliability: %.1f)\n", 
                nodeId.c_str(), signalStrength, reliability);
}

void TopologyOptimizer::updateNodeInfo(const String& nodeId, float signalStrength, float reliability) {
    auto it = nodeMap.find(nodeId);
    if (it != nodeMap.end()) {
        it->second.signalStrength = signalStrength;
        it->second.reliability = reliability;
        it->second.lastSeen = millis();
    }
}

float TopologyOptimizer::calculateNetworkEfficiency() {
    if (nodeMap.empty()) return 0.0f;
    
    float totalReliability = 0.0f;
    for (const auto& node : nodeMap) {
        totalReliability += node.second.reliability;
    }
    
    return totalReliability / nodeMap.size();
}

// AdaptiveTransmissionController implementation
AdaptiveTransmissionController::AdaptiveTransmissionController() {
}

AdaptiveTransmissionController::~AdaptiveTransmissionController() {
    cleanup();
}

bool AdaptiveTransmissionController::init() {
    DEBUG_PRINTLN("Adaptive Transmission Controller initialized");
    return true;
}

void AdaptiveTransmissionController::cleanup() {
    transmissionHistory.clear();
    optimalPowerLevels.clear();
}

int8_t AdaptiveTransmissionController::getOptimalPowerLevel(const String& destination, int8_t currentRSSI) {
    auto it = optimalPowerLevels.find(destination);
    if (it != optimalPowerLevels.end()) {
        return it->second;
    }
    
    // Default power level based on RSSI
    if (currentRSSI > -50) return MIN_POWER_LEVEL;
    if (currentRSSI > -70) return (MIN_POWER_LEVEL + MAX_POWER_LEVEL) / 2;
    return MAX_POWER_LEVEL;
}

void AdaptiveTransmissionController::recordTransmissionResult(const String& destination, 
                                                            int8_t powerLevel, int8_t rssi, bool successful) {
    TransmissionRecord record;
    record.destination = destination;
    record.powerLevel = powerLevel;
    record.rssi = rssi;
    record.successful = successful;
    record.timestamp = millis();
    
    transmissionHistory.push_back(record);
    
    // Prune old records
    if (transmissionHistory.size() > MAX_HISTORY_SIZE) {
        transmissionHistory.erase(transmissionHistory.begin());
    }
    
    // Update optimal power level
    if (successful && rssi > -80) {
        // Successful transmission - try reducing power
        optimalPowerLevels[destination] = max(MIN_POWER_LEVEL, powerLevel - 1);
    } else if (!successful) {
        // Failed transmission - increase power
        optimalPowerLevels[destination] = min(MAX_POWER_LEVEL, powerLevel + 2);
    }
}

AdaptiveTransmissionController::PowerStats AdaptiveTransmissionController::getPowerStats() const {
    PowerStats stats;
    
    if (transmissionHistory.empty()) return stats;
    
    float totalPower = 0;
    uint32_t successful = 0;
    
    for (const auto& record : transmissionHistory) {
        totalPower += record.powerLevel;
        if (record.successful) successful++;
    }
    
    stats.totalTransmissions = transmissionHistory.size();
    stats.successfulTransmissions = successful;
    stats.averagePowerLevel = totalPower / transmissionHistory.size();
    stats.powerEfficiency = (float)successful / transmissionHistory.size();
    
    return stats;
}

// NetworkOptimizationSystem implementation
NetworkOptimizationSystem::NetworkOptimizationSystem() 
    : initialized(false) {
}

NetworkOptimizationSystem::~NetworkOptimizationSystem() {
    cleanup();
}

bool NetworkOptimizationSystem::init() {
    if (initialized) return true;
    
    DEBUG_PRINTLN("Initializing Network Optimization System...");
    
    if (!compressor.resetStats()) {
        compressor.resetStats(); // Initialize compressor
    }
    
    if (!batcher.init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize message batcher");
        return false;
    }
    
    if (!topology.init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize topology optimizer");
        return false;
    }
    
    if (!powerController.init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize power controller");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Network Optimization System initialized successfully");
    return true;
}

void NetworkOptimizationSystem::cleanup() {
    if (initialized) {
        compressor.resetStats();
        batcher.cleanup();
        topology.cleanup();
        powerController.cleanup();
        initialized = false;
    }
}

void NetworkOptimizationSystem::update() {
    if (!initialized) return;
    
    // Update metrics
    updateMetrics();
    
    // Process batched messages
    auto readyBatches = batcher.createOptimalBatches();
    if (!readyBatches.empty()) {
        DEBUG_PRINTF("Processing %zu ready message batches\n", readyBatches.size());
    }
    
    // Optimize based on current metrics
    optimizeBasedOnMetrics();
}

bool NetworkOptimizationSystem::sendOptimizedMessage(const String& destination, 
                                                    const String& message, int priority) {
    if (!initialized) return false;
    
    // Add to batch queue
    batcher.addMessage("DATA", message, priority);
    
    // Record transmission attempt (simplified)
    int8_t powerLevel = powerController.getOptimalPowerLevel(destination, -70);
    powerController.recordTransmissionResult(destination, powerLevel, -70, true);
    
    return true;
}

void NetworkOptimizationSystem::updateMetrics() {
    // Update compression ratio
    auto compressionStats = compressor.getStats();
    if (compressionStats.totalCompressions > 0) {
        currentMetrics.compressionRatio = compressionStats.averageCompressionRatio;
    }
    
    // Update power efficiency
    auto powerStats = powerController.getPowerStats();
    if (powerStats.totalTransmissions > 0) {
        currentMetrics.energyEfficiency = powerStats.powerEfficiency;
    }
    
    // Simulate other metrics
    currentMetrics.bandwidthUtilization = 0.6f; // 60% utilization
    currentMetrics.averageLatency = 150.0f; // 150ms average latency
    currentMetrics.packetLossRate = 0.02f; // 2% packet loss
}

void NetworkOptimizationSystem::optimizeBasedOnMetrics() {
    // Adjust batching strategy based on latency
    if (currentMetrics.averageLatency > 200.0f) {
        batcher.adjustBatchingStrategy(currentMetrics.averageLatency, 0.8f, 0.95f);
    }
    
    // Optimize topology if packet loss is high
    if (currentMetrics.packetLossRate > 0.05f) {
        float efficiency = topology.calculateNetworkEfficiency();
        if (efficiency < 0.8f) {
            DEBUG_PRINTLN("Network efficiency low - optimizing topology");
        }
    }
}

bool NetworkOptimizationSystem::isNetworkHealthy() {
    return currentMetrics.packetLossRate < 0.05f && 
           currentMetrics.averageLatency < 300.0f &&
           currentMetrics.energyEfficiency > 0.8f;
}

String NetworkOptimizationSystem::getNetworkStatusReport() {
    String report = "Network Optimization Status:\n";
    report += "  Compression Ratio: " + String(currentMetrics.compressionRatio, 2) + "\n";
    report += "  Average Latency: " + String(currentMetrics.averageLatency, 0) + "ms\n";
    report += "  Packet Loss: " + String(currentMetrics.packetLossRate * 100, 1) + "%\n";
    report += "  Energy Efficiency: " + String(currentMetrics.energyEfficiency * 100, 1) + "%\n";
    report += "  Pending Messages: " + String(batcher.getPendingMessageCount()) + "\n";
    
    return report;
}