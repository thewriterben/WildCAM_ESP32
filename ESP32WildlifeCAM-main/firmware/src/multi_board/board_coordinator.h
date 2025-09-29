/**
 * @file board_coordinator.h
 * @brief Coordinator role implementation for multi-board systems
 * 
 * Implements the master board functionality for managing a network
 * of ESP32 camera boards in a coordinated wildlife monitoring system.
 */

#ifndef BOARD_COORDINATOR_H
#define BOARD_COORDINATOR_H

#include <Arduino.h>
#include <vector>
#include "message_protocol.h"
#include "discovery_protocol.h"

// Coordinator state
enum CoordinatorState {
    COORD_INACTIVE = 0,
    COORD_INITIALIZING = 1,
    COORD_DISCOVERING = 2,
    COORD_ACTIVE = 3,
    COORD_ELECTION = 4
};

// Task management
struct Task {
    int taskId;
    String taskType;
    int assignedNode;
    JsonObject parameters;
    unsigned long deadline;
    int priority;
    bool completed;
    unsigned long createdTime;
};

// Network management configuration
struct NetworkConfig {
    unsigned long heartbeatInterval;
    unsigned long taskTimeout;
    unsigned long coordinatorTimeout;
    int maxRetries;
    bool enableLoadBalancing;
    bool enableAutomaticRoleAssignment;
};

class BoardCoordinator {
public:
    BoardCoordinator();
    ~BoardCoordinator();
    
    /**
     * Initialize coordinator
     */
    bool init(int nodeId);
    
    /**
     * Start coordinator role
     */
    bool startCoordinator();
    
    /**
     * Stop coordinator role
     */
    void stopCoordinator();
    
    /**
     * Process coordinator tasks and messages
     */
    void process();
    
    /**
     * Handle incoming messages
     */
    void handleMessage(const MultiboardMessage& msg);
    
    /**
     * Assign roles to nodes based on capabilities
     */
    bool assignNodeRoles();
    
    /**
     * Assign task to specific node
     */
    bool assignTask(const String& taskType, int targetNode, const JsonObject& parameters, 
                   int priority = 1, unsigned long deadline = 0);
    
    /**
     * Broadcast configuration update
     */
    bool broadcastConfigUpdate(const JsonObject& config);
    
    /**
     * Get coordinator state
     */
    CoordinatorState getState() const;
    
    /**
     * Get managed nodes
     */
    const std::vector<NetworkNode>& getManagedNodes() const;
    
    /**
     * Get active tasks
     */
    const std::vector<Task>& getActiveTasks() const;
    
    /**
     * Get network statistics
     */
    struct CoordinatorStats {
        int managedNodes;
        int activeTasks;
        int completedTasks;
        int failedTasks;
        unsigned long uptimeMs;
        float networkEfficiency;
    };
    CoordinatorStats getStats() const;
    
    /**
     * Set network configuration
     */
    void setNetworkConfig(const NetworkConfig& config);
    
    /**
     * Get network configuration
     */
    const NetworkConfig& getNetworkConfig() const;
    
    /**
     * Force coordinator election
     */
    void triggerElection();
    
    /**
     * Check if this node should be coordinator
     */
    bool shouldBeCoordinator() const;
    
    /**
     * Handle coordinator failover
     */
    void handleCoordinatorFailover();
    
private:
    // Core state
    int nodeId_;
    CoordinatorState state_;
    BoardCapabilities capabilities_;
    DiscoveryProtocol* discoveryProtocol_;
    bool initialized_;
    
    // Network management
    std::vector<NetworkNode> managedNodes_;
    std::vector<Task> activeTasks_;
    std::vector<Task> completedTasks_;
    NetworkConfig networkConfig_;
    
    // Timing
    unsigned long startTime_;
    unsigned long lastHeartbeat_;
    unsigned long lastTaskCheck_;
    unsigned long lastElection_;
    
    // Task management
    int nextTaskId_;
    
    // Internal methods
    void processDiscovery();
    void processNodeManagement();
    void processTaskManagement();
    void processElection();
    
    void handleNodeDiscovery(const DiscoveryMessage& discovery);
    void handleTaskCompletion(const MultiboardMessage& msg);
    void handleElectionMessage(const MultiboardMessage& msg);
    
    bool sendRoleAssignment(int nodeId, BoardRole role);
    bool sendTaskAssignment(const Task& task);
    
    void cleanupCompletedTasks();
    void cleanupTimedOutTasks();
    void updateNodeStatus(int nodeId, const JsonObject& status);
    
    BoardRole determineOptimalRole(const BoardCapabilities& caps) const;
    int selectBestNodeForTask(const String& taskType) const;
    
    void broadcastHeartbeat();
    void sendNetworkTopology();
    
    // Election management
    void startElection();
    void processElectionVotes();
    bool isElectionWinner() const;
    
    // Default configuration
    static const NetworkConfig DEFAULT_CONFIG;
    static const unsigned long HEARTBEAT_INTERVAL = 30000;      // 30 seconds
    static const unsigned long TASK_CHECK_INTERVAL = 10000;    // 10 seconds
    static const unsigned long ELECTION_TIMEOUT = 60000;       // 1 minute
};

#endif // BOARD_COORDINATOR_H