/**
 * @file swarm_distributed.h
 * @brief Distributed computing framework for swarm operations
 * 
 * Provides:
 * - Edge computing on swarm members
 * - Distributed consensus protocols
 * - Mesh networking with dynamic routing
 * - Load balancing across members
 * - Fault-tolerant distributed databases
 * - Blockchain-based coordination
 * - Federated learning integration
 */

#ifndef SWARM_DISTRIBUTED_H
#define SWARM_DISTRIBUTED_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <queue>
#include "swarm_coordinator.h"

// Compute task type
enum ComputeTaskType {
    TASK_IMAGE_PROCESSING = 0,
    TASK_AI_INFERENCE = 1,
    TASK_DATA_ANALYSIS = 2,
    TASK_PATH_PLANNING = 3,
    TASK_SENSOR_FUSION = 4,
    TASK_COMMUNICATION = 5,
    TASK_COORDINATION = 6
};

// Task priority
enum TaskPriority {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 1,
    PRIORITY_HIGH = 2,
    PRIORITY_CRITICAL = 3,
    PRIORITY_EMERGENCY = 4
};

// Task status
enum TaskStatus {
    TASK_PENDING = 0,
    TASK_ASSIGNED = 1,
    TASK_RUNNING = 2,
    TASK_COMPLETED = 3,
    TASK_FAILED = 4,
    TASK_CANCELLED = 5
};

// Network topology
enum NetworkTopology {
    TOPOLOGY_STAR = 0,
    TOPOLOGY_MESH = 1,
    TOPOLOGY_TREE = 2,
    TOPOLOGY_RING = 3,
    TOPOLOGY_HYBRID = 4
};

// Consensus algorithm
enum ConsensusAlgorithm {
    CONSENSUS_RAFT = 0,
    CONSENSUS_PAXOS = 1,
    CONSENSUS_PBFT = 2,      // Practical Byzantine Fault Tolerance
    CONSENSUS_VOTING = 3,
    CONSENSUS_LEADER_ELECTION = 4
};

// Compute task
struct ComputeTask {
    int taskId;
    ComputeTaskType type;
    TaskPriority priority;
    TaskStatus status;
    int assignedMember;
    int requestingMember;
    std::vector<uint8_t> inputData;
    std::vector<uint8_t> outputData;
    unsigned long createdTime;
    unsigned long startTime;
    unsigned long completionTime;
    unsigned long deadline;
    float cpuLoad;
    float memoryRequired;
    String errorMessage;
};

// Compute node capacity
struct ComputeCapacity {
    int memberId;
    float cpuUtilization;      // Percentage
    float memoryUtilization;   // Percentage
    float availableCPU;        // MIPS or similar
    float availableMemory;     // MB
    int activeTasks;
    int queuedTasks;
    bool isAvailable;
};

// Network route
struct NetworkRoute {
    int sourceId;
    int destinationId;
    std::vector<int> hops;
    float latency;            // milliseconds
    float bandwidth;          // kbps
    float reliability;        // 0-1
    unsigned long lastUpdate;
};

// Message packet
struct MessagePacket {
    int packetId;
    int sourceId;
    int destinationId;
    int hopCount;
    int maxHops;
    TaskPriority priority;
    std::vector<uint8_t> data;
    unsigned long timestamp;
    bool requiresAck;
    bool isAck;
};

// Distributed database entry
struct DatabaseEntry {
    String key;
    String value;
    unsigned long version;
    int ownerId;
    std::vector<int> replicas;
    unsigned long timestamp;
    bool isDeleted;
};

// Blockchain block
struct BlockchainBlock {
    int blockId;
    String previousHash;
    String hash;
    std::vector<String> transactions;
    int validatorId;
    unsigned long timestamp;
    int nonce;
};

// Distributed statistics
struct DistributedStats {
    int totalTasks;
    int completedTasks;
    int failedTasks;
    float averageLatency;
    float averageCPUUtilization;
    float averageMemoryUtilization;
    int activeRoutes;
    float networkThroughput;
    int consensusRounds;
    int blockchainLength;
};

class SwarmDistributed {
public:
    SwarmDistributed();
    ~SwarmDistributed();
    
    // ===========================
    // INITIALIZATION
    // ===========================
    bool init(NetworkTopology topology);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // ===========================
    // EDGE COMPUTING
    // ===========================
    int submitTask(const ComputeTask& task);
    bool assignTask(int taskId, int memberId);
    bool cancelTask(int taskId);
    TaskStatus getTaskStatus(int taskId) const;
    ComputeTask getTask(int taskId) const;
    std::vector<ComputeTask> getActiveTasks() const;
    
    // ===========================
    // LOAD BALANCING
    // ===========================
    int selectOptimalNode(const ComputeTask& task);
    bool balanceLoad();
    ComputeCapacity getNodeCapacity(int memberId) const;
    std::vector<int> getAvailableNodes() const;
    bool redistributeTasks();
    
    // ===========================
    // DISTRIBUTED CONSENSUS
    // ===========================
    bool initConsensus(ConsensusAlgorithm algorithm);
    bool proposeConsensus(const String& proposal);
    bool voteOnProposal(int memberId, bool approve);
    bool reachConsensus(String& result);
    bool electLeader(int& leaderId);
    
    // ===========================
    // MESH NETWORKING
    // ===========================
    bool buildNetworkTopology();
    bool updateTopology();
    std::vector<NetworkRoute> getAllRoutes() const;
    NetworkRoute findBestRoute(int sourceId, int destinationId);
    bool sendMessage(const MessagePacket& packet);
    bool broadcastMessage(const std::vector<uint8_t>& data, TaskPriority priority);
    
    // ===========================
    // DYNAMIC ROUTING
    // ===========================
    bool updateRoutes();
    std::vector<int> calculatePath(int sourceId, int destinationId);
    float calculateRouteCost(const std::vector<int>& path) const;
    bool handleRouteFailure(int sourceId, int destinationId);
    bool optimizeRouting();
    
    // ===========================
    // FAULT-TOLERANT DATABASE
    // ===========================
    bool storeData(const String& key, const String& value);
    String retrieveData(const String& key);
    bool replicateData(const String& key, const std::vector<int>& replicas);
    bool synchronizeDatabase();
    bool verifyDataIntegrity();
    std::vector<DatabaseEntry> getAllEntries() const;
    
    // ===========================
    // BLOCKCHAIN COORDINATION
    // ===========================
    bool initBlockchain();
    bool addTransaction(const String& transaction);
    bool mineBlock(int validatorId);
    bool validateBlock(const BlockchainBlock& block);
    bool synchronizeBlockchain();
    BlockchainBlock getLatestBlock() const;
    std::vector<BlockchainBlock> getBlockchain() const;
    
    // ===========================
    // COMMUNICATION PROTOCOLS
    // ===========================
    bool enableUWB(bool enable);              // Ultra-wideband
    bool enableLoRaWAN(bool enable);          // Long range
    bool enableSDN(bool enable);              // Software-defined networking
    bool enableCognitiveRadio(bool enable);   // Adaptive frequency
    bool configureCommunication(const String& protocol, const String& params);
    
    // ===========================
    // FEDERATED LEARNING INTEGRATION
    // ===========================
    bool enableFederatedLearning(bool enable);
    bool contributToFederatedRound(int memberId);
    bool aggregateModels(const std::vector<int>& contributors);
    bool distributeGlobalModel();
    
    // ===========================
    // DISTRIBUTED MEMORY
    // ===========================
    bool shareMemory(int memberId, const String& key, const String& value);
    String accessMemory(const String& key);
    bool synchronizeMemory();
    bool clearDistributedMemory();
    
    // ===========================
    // EMERGENCY COMMUNICATION
    // ===========================
    bool sendEmergencyBroadcast(const String& message);
    bool establishEmergencyRelay(const Position3D& location);
    bool prioritizeEmergencyTraffic(bool enable);
    
    // ===========================
    // NETWORK OPTIMIZATION
    // ===========================
    bool optimizeNetworkTopology();
    bool reduceLatency();
    bool increaseReliability();
    float calculateNetworkEfficiency() const;
    
    // ===========================
    // STATISTICS & MONITORING
    // ===========================
    DistributedStats getStatistics() const;
    float getNetworkLoad() const;
    float getAverageLatency() const;
    void logDistributedEvent(const String& event, const String& details = "");
    
private:
    // State
    bool initialized_;
    NetworkTopology topology_;
    ConsensusAlgorithm consensusAlgorithm_;
    
    // Tasks
    std::map<int, ComputeTask> tasks_;
    std::queue<int> taskQueue_;
    int nextTaskId_;
    
    // Compute nodes
    std::map<int, ComputeCapacity> nodeCapacity_;
    
    // Networking
    std::vector<NetworkRoute> routes_;
    std::map<int, MessagePacket> messageQueue_;
    int nextPacketId_;
    
    // Database
    std::map<String, DatabaseEntry> database_;
    
    // Blockchain
    std::vector<BlockchainBlock> blockchain_;
    std::vector<String> pendingTransactions_;
    
    // Consensus
    std::map<int, bool> consensusVotes_;
    String currentProposal_;
    
    // Communication protocols
    bool uwbEnabled_;
    bool loraEnabled_;
    bool sdnEnabled_;
    bool cognitiveRadioEnabled_;
    
    // Statistics
    DistributedStats stats_;
    
    // Private helper methods
    void updateTaskStatus(int taskId, TaskStatus status);
    float calculateNodeScore(int memberId, const ComputeTask& task) const;
    void updateNodeCapacity(int memberId);
    String hashData(const String& data) const;
    bool verifyRoute(const NetworkRoute& route) const;
    void cleanupCompletedTasks();
    int findNextHop(int currentNode, int destination) const;
};

// Global distributed computing instance
extern SwarmDistributed* g_swarmDistributed;

// Utility functions
bool initializeSwarmDistributed(NetworkTopology topology);
void processSwarmDistributed();
DistributedStats getDistributedStatistics();
void cleanupSwarmDistributed();

#endif // SWARM_DISTRIBUTED_H
