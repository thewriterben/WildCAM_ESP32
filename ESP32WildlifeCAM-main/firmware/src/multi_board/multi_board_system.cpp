/**
 * @file multi_board_system.cpp
 * @brief Implementation of central multi-board communication system manager
 */

#include "multi_board_system.h"
#include "../config.h"

// Global instance
MultiboardSystem* g_multiboardSystem = nullptr;

// Default system configuration
const SystemConfig MultiboardSystem::DEFAULT_CONFIG = {
    .nodeId = 1,
    .preferredRole = ROLE_NODE,
    .enableAutomaticRoleSelection = true,
    .enableStandaloneFallback = true,
    .discoveryTimeout = 120000,    // 2 minutes
    .roleChangeTimeout = 60000     // 1 minute
};

MultiboardSystem::MultiboardSystem() :
    coordinator_(nullptr),
    node_(nullptr),
    config_(DEFAULT_CONFIG),
    state_(SYSTEM_INACTIVE),
    currentRole_(ROLE_UNKNOWN),
    initialized_(false),
    networkSelector_(nullptr),
    loraMeshIntegrated_(false),
    startTime_(0),
    lastStateChange_(0),
    lastDiscovery_(0),
    messagesProcessed_(0) {
}

MultiboardSystem::~MultiboardSystem() {
    stop();
    if (coordinator_) delete coordinator_;
    if (node_) delete node_;
}

bool MultiboardSystem::init(const SystemConfig& config) {
    config_ = config;
    
    // Initialize message protocol
    if (!MessageProtocol::init(config.nodeId, BOARD_AI_THINKER_ESP32_CAM, SENSOR_OV2640)) {
        setError("Failed to initialize message protocol");
        return false;
    }
    
    state_ = SYSTEM_INACTIVE;
    currentRole_ = ROLE_UNKNOWN;
    initialized_ = true;
    
    Serial.printf("Multi-board system initialized: Node %d, Preferred role: %s\n",
                  config.nodeId, MessageProtocol::roleToString(config.preferredRole));
    
    clearError();
    return true;
}

bool MultiboardSystem::start() {
    if (!initialized_) {
        setError("System not initialized");
        return false;
    }
    
    state_ = SYSTEM_INITIALIZING;
    startTime_ = millis();
    lastStateChange_ = millis();
    
    Serial.println("Starting multi-board system...");
    
    // Setup LoRa mesh integration if available
    if (loraMeshIntegrated_) {
        setupLoraMeshIntegration();
    }
    
    // Start discovery process
    state_ = SYSTEM_DISCOVERING;
    lastDiscovery_ = millis();
    
    Serial.println("Multi-board system discovery started");
    return true;
}

void MultiboardSystem::stop() {
    if (coordinator_) {
        coordinator_->stopCoordinator();
        delete coordinator_;
        coordinator_ = nullptr;
    }
    
    if (node_) {
        node_->stopNode();
        delete node_;
        node_ = nullptr;
    }
    
    state_ = SYSTEM_INACTIVE;
    currentRole_ = ROLE_UNKNOWN;
    
    Serial.println("Multi-board system stopped");
}

void MultiboardSystem::process() {
    if (!initialized_ || state_ == SYSTEM_INACTIVE) {
        return;
    }
    
    // Process LoRa mesh messages if integrated
    if (loraMeshIntegrated_) {
        processLoraMeshMessages();
    }
    
    // Process current role
    if (coordinator_) {
        coordinator_->process();
    }
    
    if (node_) {
        node_->process();
    }
    
    // Update system state
    updateSystemState();
    
    // Process system-level tasks
    processSystemMessages();
}

void MultiboardSystem::handleMultiboardMessage(const String& message) {
    MultiboardMessage msg;
    if (!MessageProtocol::parseMessage(message, msg)) {
        Serial.println("Failed to parse multi-board message");
        return;
    }
    
    messagesProcessed_++;
    
    // Route message to appropriate handler
    if (coordinator_ && (state_ == SYSTEM_COORDINATOR)) {
        coordinator_->handleMessage(msg);
    }
    
    if (node_ && (state_ == SYSTEM_NODE)) {
        node_->handleMessage(msg);
    }
    
    // Handle system-level messages
    if (msg.type == MSG_ELECTION) {
        // Process coordinator election
        checkRoleAssignment();
    }
}

SystemState MultiboardSystem::getSystemState() const {
    return state_;
}

BoardRole MultiboardSystem::getCurrentRole() const {
    return currentRole_;
}

SystemStats MultiboardSystem::getSystemStats() const {
    SystemStats stats = {};
    
    stats.currentState = state_;
    stats.currentRole = currentRole_;
    stats.systemUptime = millis() - startTime_;
    stats.lastStateChange = lastStateChange_;
    stats.messagesProcessed = messagesProcessed_;
    
    if (coordinator_) {
        auto coordStats = coordinator_->getStats();
        stats.networkedNodes = coordStats.managedNodes;
        stats.activeTasks = coordStats.activeTasks;
        stats.networkEfficiency = coordStats.networkEfficiency;
    } else if (node_) {
        auto nodeStats = node_->getStats();
        stats.activeTasks = node_->getActiveTasks().size();
        stats.networkEfficiency = nodeStats.taskSuccessRate;
    }
    
    return stats;
}

bool MultiboardSystem::forceRole(BoardRole role) {
    if (role == currentRole_) {
        return true;
    }
    
    Serial.printf("Forcing role change to: %s\n", MessageProtocol::roleToString(role));
    
    // Clean up current role
    if (coordinator_) {
        coordinator_->stopCoordinator();
        delete coordinator_;
        coordinator_ = nullptr;
    }
    
    if (node_) {
        node_->stopNode();
        delete node_;
        node_ = nullptr;
    }
    
    // Set new role
    if (role == ROLE_COORDINATOR) {
        return becomeCoordinator();
    } else {
        return becomeNode();
    }
}

void MultiboardSystem::triggerDiscovery() {
    state_ = SYSTEM_DISCOVERING;
    lastDiscovery_ = millis();
    
    if (coordinator_) {
        // Coordinator triggers network-wide discovery
        Serial.println("Coordinator triggering network-wide discovery");
        
        // Get discovery protocol from coordinator
        auto discoveryProtocol = coordinator_->getDiscoveryProtocol();
        if (discoveryProtocol) {
            // Broadcast topology update to inform all nodes
            discoveryProtocol->sendTopologyUpdate();
            
            // Send discovery advertisement to trigger responses from nodes
            discoveryProtocol->sendDiscoveryAdvertisement();
        }
        
        // Trigger coordinator election to ensure network stability
        coordinator_->triggerElection();
        
        Serial.println("Network-wide discovery initiated by coordinator");
    }
    
    if (node_) {
        node_->seekCoordinator();
    }
    
    Serial.println("Discovery triggered");
}

std::vector<NetworkNode> MultiboardSystem::getDiscoveredNodes() const {
    if (coordinator_) {
        return coordinator_->getManagedNodes();
    }
    
    if (node_) {
        // Node has limited visibility
        return std::vector<NetworkNode>();
    }
    
    return std::vector<NetworkNode>();
}

int MultiboardSystem::getCoordinatorNode() const {
    if (coordinator_) {
        return config_.nodeId; // We are the coordinator
    }
    
    if (node_) {
        return node_->getCoordinatorNode();
    }
    
    return -1;
}

bool MultiboardSystem::sendTaskToNode(int nodeId, const String& taskType, 
                                     const JsonObject& parameters, int priority) {
    if (!coordinator_ || state_ != SYSTEM_COORDINATOR) {
        setError("Not operating as coordinator");
        return false;
    }
    
    return coordinator_->assignTask(taskType, nodeId, parameters, priority);
}

bool MultiboardSystem::broadcastConfiguration(const JsonObject& config) {
    if (!coordinator_ || state_ != SYSTEM_COORDINATOR) {
        setError("Not operating as coordinator");
        return false;
    }
    
    return coordinator_->broadcastConfigUpdate(config);
}

std::vector<NodeTask> MultiboardSystem::getActiveTasks() const {
    if (node_) {
        return node_->getActiveTasks();
    }
    
    return std::vector<NodeTask>();
}

void MultiboardSystem::setSystemConfig(const SystemConfig& config) {
    config_ = config;
}

const SystemConfig& MultiboardSystem::getSystemConfig() const {
    return config_;
}

bool MultiboardSystem::isOperational() const {
    return initialized_ && (state_ == SYSTEM_COORDINATOR || 
                           state_ == SYSTEM_NODE || 
                           state_ == SYSTEM_STANDALONE);
}

String MultiboardSystem::getLastError() const {
    return lastError_;
}

void MultiboardSystem::integrateWithLoraMesh(bool enable) {
    loraMeshIntegrated_ = enable;
    
    if (enable) {
        Serial.println("LoRa mesh integration enabled");
    } else {
        Serial.println("LoRa mesh integration disabled");
    }
}

void MultiboardSystem::integrateWithNetworkSelector(NetworkSelector* networkSelector) {
    networkSelector_ = networkSelector;
    
    if (networkSelector_) {
        Serial.println("Network selector integration enabled");
    }
}

MultiboardSystem::TopologyInfo MultiboardSystem::getNetworkTopology() const {
    TopologyInfo topology = {};
    
    if (coordinator_) {
        const auto& nodes = coordinator_->getManagedNodes();
        topology.nodes = nodes;
        topology.totalNodes = nodes.size();
        topology.coordinatorNode = config_.nodeId;
        
        for (const auto& node : nodes) {
            if (node.isActive) {
                topology.activeNodes++;
            }
        }
        
        topology.isStable = true; // TODO: Implement stability check
        topology.lastUpdate = millis();
    }
    
    return topology;
}

void MultiboardSystem::updateSystemState() {
    unsigned long now = millis();
    
    // Check for state transitions based on timing
    switch (state_) {
        case SYSTEM_DISCOVERING:
            if (now - lastDiscovery_ > config_.discoveryTimeout) {
                if (config_.enableAutomaticRoleSelection) {
                    // Determine role based on discovery results
                    checkRoleAssignment();
                } else {
                    // Use preferred role
                    if (config_.preferredRole == ROLE_COORDINATOR) {
                        becomeCoordinator();
                    } else {
                        becomeNode();
                    }
                }
            }
            break;
            
        case SYSTEM_COORDINATOR:
            if (coordinator_ && !coordinator_->shouldBeCoordinator()) {
                // Should step down as coordinator
                becomeNode();
            }
            break;
            
        case SYSTEM_NODE:
            if (node_ && !node_->isCoordinatorAvailable() && 
                config_.enableStandaloneFallback) {
                enterStandaloneMode();
            }
            break;
            
        default:
            break;
    }
}

void MultiboardSystem::handleStateTransition(SystemState newState) {
    if (newState != state_) {
        Serial.printf("State transition: %d -> %d\n", state_, newState);
        state_ = newState;
        lastStateChange_ = millis();
    }
}

void MultiboardSystem::processSystemMessages() {
    // Process any system-level message handling
    static unsigned long lastCheck = 0;
    unsigned long now = millis();
    
    if (now - lastCheck > STATE_CHECK_INTERVAL) {
        // Periodic system checks
        lastCheck = now;
    }
}

void MultiboardSystem::checkRoleAssignment() {
    // TODO: Implement intelligent role assignment based on discovery results
    // For now, default to becoming a node
    becomeNode();
}

bool MultiboardSystem::becomeCoordinator() {
    if (coordinator_) {
        return true; // Already coordinator
    }
    
    coordinator_ = new BoardCoordinator();
    if (!coordinator_->init(config_.nodeId)) {
        delete coordinator_;
        coordinator_ = nullptr;
        setError("Failed to initialize coordinator");
        return false;
    }
    
    if (!coordinator_->startCoordinator()) {
        delete coordinator_;
        coordinator_ = nullptr;
        setError("Failed to start coordinator");
        return false;
    }
    
    currentRole_ = ROLE_COORDINATOR;
    handleStateTransition(SYSTEM_COORDINATOR);
    
    Serial.println("Became coordinator");
    return true;
}

bool MultiboardSystem::becomeNode() {
    if (node_) {
        return true; // Already node
    }
    
    node_ = new BoardNode();
    if (!node_->init(config_.nodeId, config_.preferredRole)) {
        delete node_;
        node_ = nullptr;
        setError("Failed to initialize node");
        return false;
    }
    
    if (!node_->startNode()) {
        delete node_;
        node_ = nullptr;
        setError("Failed to start node");
        return false;
    }
    
    currentRole_ = node_->getCurrentRole();
    handleStateTransition(SYSTEM_NODE);
    
    Serial.println("Became node");
    return true;
}

void MultiboardSystem::enterStandaloneMode() {
    currentRole_ = ROLE_NODE; // Maintain node role but standalone
    handleStateTransition(SYSTEM_STANDALONE);
    
    Serial.println("Entered standalone mode");
}

void MultiboardSystem::setupLoraMeshIntegration() {
    // Initialize LoRa mesh if not already done
    if (!LoraMesh::init()) {
        Serial.println("Warning: LoRa mesh initialization failed");
        loraMeshIntegrated_ = false;
    } else {
        LoraMesh::setNodeId(config_.nodeId);
        Serial.println("LoRa mesh integration setup complete");
    }
}

void MultiboardSystem::processLoraMeshMessages() {
    // Process LoRa mesh messages and forward multi-board messages
    LoraMesh::processMessages();
    
    // TODO: Integrate with LoRa mesh message processing
    // This would involve extending the existing LoRa mesh to handle
    // multi-board message types
}

void MultiboardSystem::setError(const String& error) {
    lastError_ = error;
    Serial.printf("Multi-board system error: %s\n", error.c_str());
}

void MultiboardSystem::clearError() {
    lastError_ = "";
}

// Global utility functions
bool initializeMultiboardSystem(int nodeId, BoardRole preferredRole) {
    if (g_multiboardSystem) {
        delete g_multiboardSystem;
    }
    
    g_multiboardSystem = new MultiboardSystem();
    
    SystemConfig config = MultiboardSystem::DEFAULT_CONFIG;
    config.nodeId = nodeId;
    config.preferredRole = preferredRole;
    
    if (!g_multiboardSystem->init(config)) {
        delete g_multiboardSystem;
        g_multiboardSystem = nullptr;
        return false;
    }
    
    return g_multiboardSystem->start();
}

void processMultiboardSystem() {
    if (g_multiboardSystem) {
        g_multiboardSystem->process();
    }
}

SystemStats getMultiboardStats() {
    if (g_multiboardSystem) {
        return g_multiboardSystem->getSystemStats();
    }
    
    return SystemStats{};
}

bool sendMultiboardTask(int nodeId, const String& taskType, const JsonObject& parameters) {
    if (g_multiboardSystem) {
        return g_multiboardSystem->sendTaskToNode(nodeId, taskType, parameters);
    }
    
    return false;
}

void cleanupMultiboardSystem() {
    if (g_multiboardSystem) {
        delete g_multiboardSystem;
        g_multiboardSystem = nullptr;
    }
}