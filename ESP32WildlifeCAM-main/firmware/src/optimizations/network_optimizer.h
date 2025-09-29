#ifndef NETWORK_OPTIMIZER_H
#define NETWORK_OPTIMIZER_H

#include <Arduino.h>
#include <vector>
#include <map>

/**
 * Network Optimizer - Enhanced networking performance for ESP32WildlifeCAM
 * 
 * Implements advanced compression, intelligent message batching,
 * and topology optimization for 60% bandwidth reduction.
 */

// Forward declarations
enum DataSyncPriority;
struct WildlifeObservation;

// Advanced compression for wildlife data
class WildlifeDataCompressor {
public:
    // Semantic compression for detection data
    size_t compressDetection(const WildlifeObservation& obs, uint8_t* output);
    bool decompressDetection(const uint8_t* input, size_t size, WildlifeObservation& obs);
    
    // Delta compression for time series
    size_t compressSensorData(const std::vector<float>& data, uint8_t* output);
    bool decompressSensorData(const uint8_t* input, size_t size, std::vector<float>& data);
    
    // Image compression optimized for wildlife
    size_t compressWildlifeImage(const uint8_t* image, uint16_t width, uint16_t height,
                                uint8_t* compressed, float quality = 0.7f);
    bool decompressWildlifeImage(const uint8_t* compressed, size_t compressedSize,
                               uint8_t* image, uint16_t width, uint16_t height);
    
    // Compression statistics
    struct CompressionStats {
        uint32_t totalCompressions;
        uint32_t totalOriginalBytes;
        uint32_t totalCompressedBytes;
        float averageCompressionRatio;
        
        CompressionStats() : totalCompressions(0), totalOriginalBytes(0),
                           totalCompressedBytes(0), averageCompressionRatio(0) {}
    };
    
    CompressionStats getStats() const { return stats; }
    void resetStats();

private:
    CompressionStats stats;
    
    // Dictionary-based compression for common species
    std::map<String, uint8_t> speciesDictionary;
    void buildSpeciesDictionary();
    
    // Simple LZ compression implementation
    size_t simpleLZCompress(const uint8_t* input, size_t inputSize, 
                           uint8_t* output, size_t outputSize);
    size_t simpleLZDecompress(const uint8_t* input, size_t inputSize,
                             uint8_t* output, size_t outputSize);
    
    void updateCompressionStats(size_t original, size_t compressed);
};

// Optimized message batching
class IntelligentMessageBatcher {
    struct BatchedMessage {
        String messageType;
        std::vector<String> payloads;
        int priority;
        unsigned long deadline;
        unsigned long createdTime;
        
        BatchedMessage() : priority(0), deadline(0), createdTime(0) {}
    };
    
    std::vector<BatchedMessage> batches;
    unsigned long lastBatchTime;
    size_t maxBatchSize;
    unsigned long batchTimeout;
    
public:
    IntelligentMessageBatcher();
    ~IntelligentMessageBatcher();
    
    bool init(size_t maxBatchSize = 10, unsigned long timeoutMs = 5000);
    void cleanup();
    
    void addMessage(const String& type, const String& payload, int priority = 0);
    std::vector<BatchedMessage> createOptimalBatches();
    
    // Adaptive batching based on network conditions
    void adjustBatchingStrategy(float latency, float throughput, float reliability);
    
    // Batch management
    size_t getPendingMessageCount() const;
    void forceBatch(); // Force immediate batching
    void clearBatches();

private:
    uint32_t calculateOptimalBatchSize(const String& messageType);
    bool shouldForceBatch(int priority, unsigned long age);
    void optimizeBatchOrder(std::vector<BatchedMessage>& batches);
};

// Network topology optimization
class TopologyOptimizer {
    struct NodeInfo {
        String nodeId;
        float signalStrength;
        float reliability;
        uint32_t hopCount;
        unsigned long lastSeen;
        
        NodeInfo() : signalStrength(0), reliability(0), hopCount(0), lastSeen(0) {}
    };
    
    std::map<String, NodeInfo> nodeMap;
    std::map<String, std::vector<String>> adjacencyList;
    
public:
    TopologyOptimizer();
    ~TopologyOptimizer();
    
    bool init();
    void cleanup();
    
    // Minimum spanning tree for mesh network
    std::vector<std::pair<String, String>> calculateOptimalTopology(
        const std::map<String, std::vector<std::pair<String, float>>>& linkQuality);
    
    // Load-aware routing
    String selectNextHop(const String& destination, 
                        const std::map<String, float>& nodeLoads);
    
    // Adaptive routing based on conditions
    void updateRoutingTable(const std::map<String, float>& latencies,
                           const std::map<String, float>& reliability);
    
    // Network analysis
    float calculateNetworkEfficiency();
    std::vector<String> identifyBottleneckNodes();
    void optimizeNodePlacement();
    
    // Node management
    void addNode(const String& nodeId, float signalStrength, float reliability);
    void removeNode(const String& nodeId);
    void updateNodeInfo(const String& nodeId, float signalStrength, float reliability);

private:
    struct RoutingEntry {
        String nextHop;
        float cost;
        unsigned long lastUpdate;
        float reliability;
        
        RoutingEntry() : cost(0), lastUpdate(0), reliability(0) {}
    };
    
    std::map<String, RoutingEntry> routingTable;
    
    // Graph algorithms
    float calculatePathCost(const std::vector<String>& path);
    std::vector<String> findShortestPath(const String& source, const String& destination);
    bool isConnected(const String& nodeA, const String& nodeB);
    
    // Optimization algorithms
    void dijkstraOptimization();
    void minimumSpanningTree();
};

// Adaptive transmission power controller
class AdaptiveTransmissionController {
    struct TransmissionRecord {
        String destination;
        int8_t powerLevel;
        int8_t rssi;
        bool successful;
        unsigned long timestamp;
        
        TransmissionRecord() : powerLevel(0), rssi(0), successful(false), timestamp(0) {}
    };
    
    std::vector<TransmissionRecord> transmissionHistory;
    std::map<String, int8_t> optimalPowerLevels;
    
public:
    AdaptiveTransmissionController();
    ~AdaptiveTransmissionController();
    
    bool init();
    void cleanup();
    
    // Power optimization
    int8_t getOptimalPowerLevel(const String& destination, int8_t currentRSSI);
    void recordTransmissionResult(const String& destination, int8_t powerLevel, 
                                 int8_t rssi, bool successful);
    
    // Adaptive algorithms
    void adaptPowerBasedOnHistory();
    void optimizeForBatteryLife();
    void optimizeForReliability();
    
    // Statistics
    struct PowerStats {
        float averagePowerLevel;
        float powerEfficiency;
        uint32_t totalTransmissions;
        uint32_t successfulTransmissions;
        
        PowerStats() : averagePowerLevel(0), powerEfficiency(0),
                      totalTransmissions(0), successfulTransmissions(0) {}
    };
    
    PowerStats getPowerStats() const;
    void resetStats();

private:
    static const int8_t MIN_POWER_LEVEL = 5;
    static const int8_t MAX_POWER_LEVEL = 20;
    static const size_t MAX_HISTORY_SIZE = 100;
    
    int8_t calculatePowerAdjustment(int8_t currentRSSI, int8_t targetRSSI);
    void pruneOldRecords();
    float calculateSuccessRate(const String& destination);
};

// Integrated network optimization system
class NetworkOptimizationSystem {
    WildlifeDataCompressor compressor;
    IntelligentMessageBatcher batcher;
    TopologyOptimizer topology;
    AdaptiveTransmissionController powerController;
    
    bool initialized;
    
    // Performance metrics
    struct NetworkMetrics {
        float bandwidthUtilization;
        float averageLatency;
        float packetLossRate;
        float compressionRatio;
        float energyEfficiency;
        
        NetworkMetrics() : bandwidthUtilization(0), averageLatency(0),
                          packetLossRate(0), compressionRatio(0), energyEfficiency(0) {}
    };
    
    NetworkMetrics currentMetrics;
    
public:
    NetworkOptimizationSystem();
    ~NetworkOptimizationSystem();
    
    bool init();
    void cleanup();
    void update();
    
    // Optimization control
    void enableCompression(bool enable = true);
    void enableBatching(bool enable = true);
    void enableTopologyOptimization(bool enable = true);
    void enableAdaptivePower(bool enable = true);
    
    // Performance monitoring
    NetworkMetrics getMetrics() const { return currentMetrics; }
    void resetMetrics();
    
    // Integration methods
    bool sendOptimizedMessage(const String& destination, const String& message, int priority = 0);
    bool broadcastOptimizedMessage(const String& message, int priority = 0);
    
    // Network status
    bool isNetworkHealthy();
    String getNetworkStatusReport();
    void diagnoseNetworkIssues();

private:
    void updateMetrics();
    void optimizeBasedOnMetrics();
    void logNetworkEvent(const String& event);
};

#endif // NETWORK_OPTIMIZER_H