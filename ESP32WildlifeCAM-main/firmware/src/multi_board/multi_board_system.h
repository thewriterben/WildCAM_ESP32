/**
 * @file multi_board_system.h
 * @brief Central manager for multi-board communication system
 * 
 * Provides a unified interface for managing multi-board coordination,
 * integrating coordinator and node roles with existing wildlife camera systems.
 */

#ifndef MULTI_BOARD_SYSTEM_H
#define MULTI_BOARD_SYSTEM_H

#include <Arduino.h>
#include "message_protocol.h"
#include "discovery_protocol.h"
#include "board_coordinator.h"
#include "board_node.h"
#include "../lora_mesh.h"
#include "../network_selector.h"

// System states
enum SystemState {
    SYSTEM_INACTIVE = 0,
    SYSTEM_INITIALIZING = 1,
    SYSTEM_DISCOVERING = 2,
    SYSTEM_COORDINATOR = 3,
    SYSTEM_NODE = 4,
    SYSTEM_STANDALONE = 5,
    SYSTEM_ERROR = 6
};

// System configuration
struct SystemConfig {
    int nodeId;
    BoardRole preferredRole;
    bool enableAutomaticRoleSelection;
    bool enableStandaloneFallback;
    unsigned long discoveryTimeout;
    unsigned long roleChangeTimeout;
};

// System statistics
struct SystemStats {
    SystemState currentState;
    BoardRole currentRole;
    unsigned long systemUptime;
    int networkedNodes;
    int activeTasks;
    int messagesProcessed;
    float networkEfficiency;
    unsigned long lastStateChange;
};

class MultiboardSystem {
public:
    MultiboardSystem();
    ~MultiboardSystem();
    
    /**
     * Initialize multi-board system
     */
    bool init(const SystemConfig& config);
    
    /**
     * Start multi-board system operation
     */
    bool start();
    
    /**
     * Stop multi-board system
     */
    void stop();
    
    /**
     * Process system tasks and messages
     */
    void process();
    
    /**
     * Handle incoming multi-board messages
     */
    void handleMultiboardMessage(const String& message);
    
    /**
     * Get current system state
     */
    SystemState getSystemState() const;
    
    /**
     * Get current role
     */
    BoardRole getCurrentRole() const;
    
    /**
     * Get system statistics
     */
    SystemStats getSystemStats() const;
    
    /**
     * Force role change
     */
    bool forceRole(BoardRole role);
    
    /**
     * Trigger network discovery
     */
    void triggerDiscovery();
    
    /**
     * Get discovered nodes
     */
    std::vector<NetworkNode> getDiscoveredNodes() const;
    
    /**
     * Get coordinator node ID
     */
    int getCoordinatorNode() const;
    
    /**
     * Send task to node (coordinator only)
     */
    bool sendTaskToNode(int nodeId, const String& taskType, 
                       const JsonObject& parameters, int priority = 1);
    
    /**
     * Broadcast configuration update (coordinator only)
     */
    bool broadcastConfiguration(const JsonObject& config);
    
    /**
     * Get active tasks (node only)
     */
    std::vector<NodeTask> getActiveTasks() const;
    
    /**
     * Set system configuration
     */
    void setSystemConfig(const SystemConfig& config);
    
    /**
     * Get system configuration
     */
    const SystemConfig& getSystemConfig() const;
    
    /**
     * Check if system is operational
     */
    bool isOperational() const;
    
    /**
     * Get last error message
     */
    String getLastError() const;
    
    /**
     * Integration with existing systems
     */
    void integrateWithLoraMesh(bool enable = true);
    void integrateWithNetworkSelector(NetworkSelector* networkSelector);
    
    /**
     * Get network topology information
     */
    struct TopologyInfo {
        int totalNodes;
        int activeNodes;
        int coordinatorNode;
        std::vector<NetworkNode> nodes;
        bool isStable;
        unsigned long lastUpdate;
    };
    TopologyInfo getNetworkTopology() const;
    
private:
    // Core components
    BoardCoordinator* coordinator_;
    BoardNode* node_;
    SystemConfig config_;
    SystemState state_;
    BoardRole currentRole_;
    bool initialized_;
    
    // Integration
    NetworkSelector* networkSelector_;
    bool loraMeshIntegrated_;
    
    // Timing and state management
    unsigned long startTime_;
    unsigned long lastStateChange_;
    unsigned long lastDiscovery_;
    unsigned long messagesProcessed_;
    String lastError_;
    
    // Internal state management
    void updateSystemState();
    void handleStateTransition(SystemState newState);
    void processSystemMessages();
    void checkRoleAssignment();
    
    // Role management
    bool becomeCoordinator();
    bool becomeNode();
    void enterStandaloneMode();
    
    // Message integration
    void setupLoraMeshIntegration();
    void processLoraMeshMessages();
    
    // Error handling
    void setError(const String& error);
    void clearError();
    
    // Default configuration
    static const SystemConfig DEFAULT_CONFIG;
    static const unsigned long STATE_CHECK_INTERVAL = 10000;    // 10 seconds
    static const unsigned long DISCOVERY_TIMEOUT = 120000;     // 2 minutes
    static const unsigned long ROLE_CHANGE_TIMEOUT = 60000;    // 1 minute
};

// Global multi-board system instance
extern MultiboardSystem* g_multiboardSystem;

// Utility functions for easy integration
bool initializeMultiboardSystem(int nodeId, BoardRole preferredRole = ROLE_NODE);
void processMultiboardSystem();
SystemStats getMultiboardStats();
bool sendMultiboardTask(int nodeId, const String& taskType, const JsonObject& parameters);
void cleanupMultiboardSystem();

#endif // MULTI_BOARD_SYSTEM_H