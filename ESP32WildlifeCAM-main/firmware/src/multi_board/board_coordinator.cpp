/**
 * @file board_coordinator.cpp
 * @brief Implementation of coordinator role for multi-board systems
 */

#include "board_coordinator.h"
#include "../lora_mesh.h"
#include "../config.h"

// Default network configuration
const NetworkConfig BoardCoordinator::DEFAULT_CONFIG = {
    .heartbeatInterval = 30000,      // 30 seconds
    .taskTimeout = 300000,           // 5 minutes
    .coordinatorTimeout = 120000,    // 2 minutes
    .maxRetries = 3,
    .enableLoadBalancing = true,
    .enableAutomaticRoleAssignment = true
};

BoardCoordinator::BoardCoordinator() :
    nodeId_(0),
    state_(COORD_INACTIVE),
    discoveryProtocol_(nullptr),
    initialized_(false),
    networkConfig_(DEFAULT_CONFIG),
    startTime_(0),
    lastHeartbeat_(0),
    lastTaskCheck_(0),
    lastElection_(0),
    nextTaskId_(1) {
}

BoardCoordinator::~BoardCoordinator() {
    stopCoordinator();
    if (discoveryProtocol_) {
        delete discoveryProtocol_;
    }
}

bool BoardCoordinator::init(int nodeId) {
    nodeId_ = nodeId;
    capabilities_ = MessageProtocol::getCurrentCapabilities();
    
    // Initialize discovery protocol
    discoveryProtocol_ = new DiscoveryProtocol();
    if (!discoveryProtocol_->init(nodeId, ROLE_COORDINATOR)) {
        Serial.println("Failed to initialize discovery protocol");
        delete discoveryProtocol_;
        discoveryProtocol_ = nullptr;
        return false;
    }
    
    state_ = COORD_INACTIVE;
    initialized_ = true;
    
    Serial.printf("Board coordinator initialized: Node %d\n", nodeId);
    return true;
}

bool BoardCoordinator::startCoordinator() {
    if (!initialized_) {
        Serial.println("Coordinator not initialized");
        return false;
    }
    
    state_ = COORD_INITIALIZING;
    startTime_ = millis();
    lastHeartbeat_ = millis();
    lastTaskCheck_ = millis();
    
    Serial.println("Starting coordinator role...");
    
    // Start discovery to find existing network
    if (discoveryProtocol_->startDiscovery()) {
        state_ = COORD_DISCOVERING;
        Serial.println("Coordinator discovery started");
        return true;
    }
    
    return false;
}

void BoardCoordinator::stopCoordinator() {
    if (discoveryProtocol_) {
        discoveryProtocol_->stopDiscovery();
    }
    
    state_ = COORD_INACTIVE;
    managedNodes_.clear();
    activeTasks_.clear();
    
    Serial.println("Coordinator stopped");
}

void BoardCoordinator::process() {
    if (!initialized_ || state_ == COORD_INACTIVE) {
        return;
    }
    
    unsigned long now = millis();
    
    // Process discovery protocol
    if (discoveryProtocol_) {
        discoveryProtocol_->processMessages();
    }
    
    switch (state_) {
        case COORD_DISCOVERING:
            processDiscovery();
            break;
            
        case COORD_ACTIVE:
            processNodeManagement();
            processTaskManagement();
            break;
            
        case COORD_ELECTION:
            processElection();
            break;
            
        default:
            break;
    }
    
    // Send periodic heartbeat
    if (now - lastHeartbeat_ >= networkConfig_.heartbeatInterval) {
        broadcastHeartbeat();
        lastHeartbeat_ = now;
    }
    
    // Check tasks periodically
    if (now - lastTaskCheck_ >= TASK_CHECK_INTERVAL) {
        cleanupCompletedTasks();
        cleanupTimedOutTasks();
        lastTaskCheck_ = now;
    }
}

void BoardCoordinator::handleMessage(const MultiboardMessage& msg) {
    switch (msg.type) {
        case MSG_DISCOVERY:
            if (discoveryProtocol_) {
                discoveryProtocol_->handleDiscoveryMessage(msg);
            }
            break;
            
        case MSG_STATUS:
            updateNodeStatus(msg.sourceNode, msg.data);
            break;
            
        case MSG_DATA:
            handleTaskCompletion(msg);
            break;
            
        case MSG_ELECTION:
            handleElectionMessage(msg);
            break;
            
        default:
            Serial.printf("Coordinator received unhandled message type: %s\n",
                         MessageProtocol::messageTypeToString(msg.type));
            break;
    }
}

bool BoardCoordinator::assignNodeRoles() {
    if (!networkConfig_.enableAutomaticRoleAssignment) {
        return false;
    }
    
    const auto& nodes = discoveryProtocol_->getDiscoveredNodes();
    int assignedRoles = 0;
    
    for (const auto& node : nodes) {
        BoardRole optimalRole = determineOptimalRole(node.capabilities);
        if (optimalRole != node.role) {
            if (sendRoleAssignment(node.nodeId, optimalRole)) {
                assignedRoles++;
                Serial.printf("Assigned role %s to node %d\n",
                             MessageProtocol::roleToString(optimalRole), node.nodeId);
            }
        }
    }
    
    Serial.printf("Assigned roles to %d nodes\n", assignedRoles);
    return assignedRoles > 0;
}

bool BoardCoordinator::assignTask(const String& taskType, int targetNode, 
                                 const JsonObject& parameters, int priority, 
                                 unsigned long deadline) {
    Task task;
    task.taskId = nextTaskId_++;
    task.taskType = taskType;
    task.assignedNode = targetNode;
    task.parameters = parameters;
    task.priority = priority;
    task.deadline = deadline > 0 ? deadline : millis() + networkConfig_.taskTimeout;
    task.completed = false;
    task.createdTime = millis();
    
    activeTasks_.push_back(task);
    
    return sendTaskAssignment(task);
}

bool BoardCoordinator::broadcastConfigUpdate(const JsonObject& config) {
    DynamicJsonDocument doc(1024);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = nodeId_;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    for (JsonPair kv : config) {
        data[kv.key()] = kv.value();
    }
    
    String message;
    serializeJson(doc, message);
    
    return LoraMesh::queueMessage(message);
}

CoordinatorState BoardCoordinator::getState() const {
    return state_;
}

const std::vector<NetworkNode>& BoardCoordinator::getManagedNodes() const {
    return managedNodes_;
}

const std::vector<Task>& BoardCoordinator::getActiveTasks() const {
    return activeTasks_;
}

BoardCoordinator::CoordinatorStats BoardCoordinator::getStats() const {
    CoordinatorStats stats = {};
    
    if (discoveryProtocol_) {
        stats.managedNodes = discoveryProtocol_->getDiscoveredNodes().size();
    }
    
    stats.activeTasks = activeTasks_.size();
    stats.completedTasks = completedTasks_.size();
    stats.uptimeMs = millis() - startTime_;
    
    // Calculate network efficiency (placeholder)
    stats.networkEfficiency = stats.managedNodes > 0 ? 
        (float)stats.completedTasks / (stats.completedTasks + stats.failedTasks + 1) : 0.0;
    
    return stats;
}

void BoardCoordinator::setNetworkConfig(const NetworkConfig& config) {
    networkConfig_ = config;
}

const NetworkConfig& BoardCoordinator::getNetworkConfig() const {
    return networkConfig_;
}

void BoardCoordinator::triggerElection() {
    startElection();
}

bool BoardCoordinator::shouldBeCoordinator() const {
    if (!discoveryProtocol_) return false;
    
    return discoveryProtocol_->getCoordinatorNode() == nodeId_;
}

void BoardCoordinator::processDiscovery() {
    if (!discoveryProtocol_) return;
    
    // Update managed nodes continuously for real-time coordination
    const auto& currentNodes = discoveryProtocol_->getDiscoveredNodes();
    bool nodesChanged = (currentNodes.size() != managedNodes_.size());
    
    if (discoveryProtocol_->isDiscoveryComplete()) {
        managedNodes_ = currentNodes;
        
        if (shouldBeCoordinator()) {
            if (state_ != COORD_ACTIVE) {
                state_ = COORD_ACTIVE;
                Serial.printf("✓ Coordinator active with %d managed nodes\n", managedNodes_.size());
                
                // Perform initial role assignments
                assignNodeRoles();
                
                // Send network topology to all nodes
                sendNetworkTopology();
            } else if (nodesChanged) {
                // Dynamic topology update - new device joined or left
                Serial.printf("⚡ Topology changed: %d managed nodes\n", managedNodes_.size());
                
                // Reassign roles based on new topology
                if (networkConfig_.enableAutomaticRoleAssignment) {
                    assignNodeRoles();
                }
                
                // Broadcast updated topology
                sendNetworkTopology();
            }
        } else {
            // Another node is coordinator, step down
            Serial.printf("Node %d is coordinator, stepping down\n", 
                         discoveryProtocol_->getCoordinatorNode());
            stopCoordinator();
        }
    } else if (state_ == COORD_ACTIVE && nodesChanged) {
        // Real-time mesh formation: even during discovery, coordinate new nodes
        managedNodes_ = currentNodes;
        Serial.printf("⚡ Real-time coordination: %d nodes\n", managedNodes_.size());
        sendNetworkTopology();
    }
}

void BoardCoordinator::processNodeManagement() {
    // Update managed nodes from discovery
    if (discoveryProtocol_) {
        const auto& discoveredNodes = discoveryProtocol_->getDiscoveredNodes();
        
        // Check for capability changes that might require role reassignment
        for (const auto& node : discoveredNodes) {
            auto it = std::find_if(managedNodes_.begin(), managedNodes_.end(),
                                  [&node](const NetworkNode& n) { return n.nodeId == node.nodeId; });
            
            if (it != managedNodes_.end()) {
                // Check if node capabilities have changed significantly
                bool capabilitiesChanged = 
                    (it->capabilities.batteryLevel != node.capabilities.batteryLevel) ||
                    (it->capabilities.hasAI != node.capabilities.hasAI) ||
                    (it->capabilities.hasPSRAM != node.capabilities.hasPSRAM);
                
                if (capabilitiesChanged && networkConfig_.enableAutomaticRoleAssignment) {
                    Serial.printf("Node %d capabilities changed, reassessing role\n", node.nodeId);
                    BoardRole optimalRole = determineOptimalRole(node.capabilities);
                    if (optimalRole != node.role) {
                        sendRoleAssignment(node.nodeId, optimalRole);
                    }
                }
            }
        }
        
        managedNodes_ = discoveredNodes;
    }
    
    // Check for failed nodes and reassign tasks if needed
    checkFailedNodes();
}

void BoardCoordinator::processTaskManagement() {
    // Process task completion and timeouts
    // This is handled in the periodic task check
}

void BoardCoordinator::processElection() {
    unsigned long now = millis();
    
    if (now - lastElection_ > ELECTION_TIMEOUT) {
        if (isElectionWinner()) {
            state_ = COORD_ACTIVE;
            Serial.println("Election won, resuming coordinator role");
        } else {
            state_ = COORD_INACTIVE;
            Serial.println("Election lost, stepping down");
        }
    }
}

BoardRole BoardCoordinator::determineOptimalRole(const BoardCapabilities& caps) const {
    // Dynamic capability-based role assignment
    
    // Consider battery level for role assignment
    bool lowBattery = (caps.batteryLevel < 30);
    bool hasSolarPower = (caps.solarVoltage > 4.0);
    
    // AI-capable boards with high resolution and sufficient power
    if (caps.hasAI && caps.hasPSRAM && caps.maxResolution >= 1920 * 1080 && !lowBattery) {
        Serial.printf("Node capabilities: AI processor (AI=%d, PSRAM=%d, Res=%d, Bat=%d%%)\n",
                     caps.hasAI, caps.hasPSRAM, caps.maxResolution, caps.batteryLevel);
        return ROLE_AI_PROCESSOR;
    }
    
    // High-resolution boards with good storage
    if (caps.maxResolution >= 1600 * 1200 && caps.availableStorage > 1024 * 1024 && caps.hasSD) {
        Serial.printf("Node capabilities: Hub (Res=%d, Storage=%d MB, SD=%d)\n",
                     caps.maxResolution, caps.availableStorage / (1024*1024), caps.hasSD);
        return ROLE_HUB;
    }
    
    // Low-power boards or low battery - assign stealth role
    if (caps.powerProfile <= 1 || lowBattery) {
        Serial.printf("Node capabilities: Stealth (Power=%d, Battery=%d%%)\n",
                     caps.powerProfile, caps.batteryLevel);
        return ROLE_STEALTH;
    }
    
    // Boards with cellular capability - good for remote monitoring
    if (caps.hasCellular || caps.hasSatellite) {
        Serial.printf("Node capabilities: Portable (Cellular=%d, Satellite=%d)\n",
                     caps.hasCellular, caps.hasSatellite);
        return ROLE_PORTABLE;
    }
    
    // Boards with solar power and good connectivity - can be relay
    if (hasSolarPower && caps.batteryLevel > 50) {
        Serial.printf("Node capabilities: Relay (Solar=%.1fV, Battery=%d%%)\n",
                     caps.solarVoltage, caps.batteryLevel);
        return ROLE_RELAY;
    }
    
    // Default to node role for standard camera operation
    Serial.printf("Node capabilities: Standard node (defaults)\n");
    return ROLE_NODE;
}

bool BoardCoordinator::sendRoleAssignment(int nodeId, BoardRole role) {
    String message = MessageProtocol::createRoleAssignmentMessage(nodeId, role);
    return LoraMesh::queueMessage(message);
}

bool BoardCoordinator::sendTaskAssignment(const Task& task) {
    TaskAssignment assignment;
    assignment.taskId = task.taskId;
    assignment.assignedNode = task.assignedNode;
    assignment.taskType = task.taskType;
    assignment.parameters = task.parameters;
    assignment.deadline = task.deadline;
    assignment.priority = task.priority;
    
    String message = MessageProtocol::createTaskAssignmentMessage(assignment);
    return LoraMesh::queueMessage(message);
}

void BoardCoordinator::broadcastHeartbeat() {
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_HEARTBEAT;
    doc["source_node"] = nodeId_;
    doc["target_node"] = 0; // Broadcast
    doc["timestamp"] = millis();
    doc["hop_count"] = 0;
    
    JsonObject data = doc.createNestedObject("data");
    data["role"] = ROLE_COORDINATOR;
    data["managed_nodes"] = managedNodes_.size();
    data["active_tasks"] = activeTasks_.size();
    data["uptime"] = millis() - startTime_;
    
    String message;
    serializeJson(doc, message);
    LoraMesh::queueMessage(message);
}

void BoardCoordinator::cleanupCompletedTasks() {
    activeTasks_.erase(
        std::remove_if(activeTasks_.begin(), activeTasks_.end(),
                      [this](const Task& task) {
                          if (task.completed) {
                              completedTasks_.push_back(task);
                              return true;
                          }
                          return false;
                      }),
        activeTasks_.end()
    );
}

void BoardCoordinator::cleanupTimedOutTasks() {
    unsigned long now = millis();
    
    activeTasks_.erase(
        std::remove_if(activeTasks_.begin(), activeTasks_.end(),
                      [now](const Task& task) {
                          return now > task.deadline;
                      }),
        activeTasks_.end()
    );
}

void BoardCoordinator::updateNodeStatus(int nodeId, const JsonObject& status) {
    // Update node status in managed nodes list
    for (auto& node : managedNodes_) {
        if (node.nodeId == nodeId) {
            node.lastSeen = millis();
            node.isActive = true;
            // Update other status fields as needed
            break;
        }
    }
}

void BoardCoordinator::handleTaskCompletion(const MultiboardMessage& msg) {
    int taskId = msg.data["task_id"];
    
    for (auto& task : activeTasks_) {
        if (task.taskId == taskId) {
            task.completed = true;
            Serial.printf("Task %d completed by node %d\n", taskId, msg.sourceNode);
            break;
        }
    }
}

void BoardCoordinator::handleElectionMessage(const MultiboardMessage& msg) {
    // Process election messages
    state_ = COORD_ELECTION;
    lastElection_ = millis();
    
    float theirScore = msg.data["coordinator_score"];
    float ourScore = MessageProtocol::calculateCoordinatorScore(capabilities_);
    
    Serial.printf("Election message from node %d (score: %.1f vs our %.1f)\n",
                  msg.sourceNode, theirScore, ourScore);
}

void BoardCoordinator::startElection() {
    state_ = COORD_ELECTION;
    lastElection_ = millis();
    
    float ourScore = MessageProtocol::calculateCoordinatorScore(capabilities_);
    String message = MessageProtocol::createElectionMessage(ourScore, true);
    LoraMesh::queueMessage(message);
    
    Serial.printf("Started coordinator election with score %.1f\n", ourScore);
}

bool BoardCoordinator::isElectionWinner() const {
    if (!discoveryProtocol_) return false;
    
    float ourScore = MessageProtocol::calculateCoordinatorScore(capabilities_);
    const auto& nodes = discoveryProtocol_->getDiscoveredNodes();
    
    for (const auto& node : nodes) {
        if (node.coordinatorScore > ourScore) {
            return false;
        }
    }
    
    return true;
}

void BoardCoordinator::checkFailedNodes() {
    unsigned long now = millis();
    std::vector<int> failedNodeIds;
    
    // Check each managed node for heartbeat timeout
    for (auto& node : managedNodes_) {
        // Skip the coordinator itself
        if (node.nodeId == nodeId_) {
            continue;
        }
        
        // Check if node has exceeded failure timeout
        if (node.isActive && (now - node.lastSeen) > NODE_FAILURE_TIMEOUT) {
            // Mark node as failed
            node.isActive = false;
            failedNodeIds.push_back(node.nodeId);
            
            // Log node failure event
            Serial.printf("❌ NODE FAILURE DETECTED: Node %d - Last seen %lu ms ago (timeout: %lu ms)\n",
                         node.nodeId, 
                         now - node.lastSeen,
                         NODE_FAILURE_TIMEOUT);
            
            Serial.printf("   Node details: Role=%s, Signal=%d dBm, HopCount=%d\n",
                         MessageProtocol::roleToString(node.role),
                         node.signalStrength,
                         node.hopCount);
        }
    }
    
    // Reassign tasks from failed nodes
    for (int failedNodeId : failedNodeIds) {
        reassignTasksFromFailedNode(failedNodeId);
    }
    
    // Update network topology if any nodes failed
    if (!failedNodeIds.empty()) {
        Serial.printf("⚠️  Network degradation: %d node(s) failed, %d active nodes remaining\n",
                     failedNodeIds.size(),
                     std::count_if(managedNodes_.begin(), managedNodes_.end(),
                                  [](const NetworkNode& n) { return n.isActive; }));
        
        // Broadcast updated topology to inform remaining nodes
        sendNetworkTopology();
    }
}

void BoardCoordinator::reassignTasksFromFailedNode(int failedNodeId) {
    std::vector<Task> tasksToReassign;
    
    // Find all tasks assigned to the failed node
    for (auto& task : activeTasks_) {
        if (task.assignedNode == failedNodeId && !task.completed) {
            tasksToReassign.push_back(task);
        }
    }
    
    if (tasksToReassign.empty()) {
        Serial.printf("   No active tasks to reassign from failed node %d\n", failedNodeId);
        return;
    }
    
    Serial.printf("   Reassigning %d task(s) from failed node %d\n", 
                 tasksToReassign.size(), failedNodeId);
    
    // Attempt to reassign each task
    for (auto& task : tasksToReassign) {
        int newNodeId = selectHealthyNodeForTask(task.taskType);
        
        if (newNodeId > 0) {
            // Update task assignment
            task.assignedNode = newNodeId;
            
            // Send new task assignment
            if (sendTaskAssignment(task)) {
                Serial.printf("   ✓ Task %d (%s) reassigned: %d -> %d (Priority: %d)\n",
                             task.taskId,
                             task.taskType.c_str(),
                             failedNodeId,
                             newNodeId,
                             task.priority);
            } else {
                Serial.printf("   ✗ Failed to send reassignment for task %d to node %d\n",
                             task.taskId, newNodeId);
            }
        } else {
            Serial.printf("   ⚠️  No healthy node available to reassign task %d (%s)\n",
                         task.taskId,
                         task.taskType.c_str());
            
            // Mark task as failed if we can't reassign it
            task.completed = false;
            // Could extend deadline to give more time when network recovers
            task.deadline = millis() + networkConfig_.taskTimeout;
        }
    }
}

int BoardCoordinator::selectHealthyNodeForTask(const String& taskType) const {
    int bestNodeId = -1;
    int highestScore = -1;
    
    // Select the best healthy node based on capabilities and load
    for (const auto& node : managedNodes_) {
        // Skip inactive nodes and the coordinator itself
        if (!node.isActive || node.nodeId == nodeId_) {
            continue;
        }
        
        // Calculate node score based on capabilities and current load
        int score = 0;
        
        // Task type specific scoring
        if (taskType == "AI_INFERENCE" && node.capabilities.hasAI) {
            score += 50;
        }
        if (taskType == "IMAGE_STORAGE" && node.capabilities.hasSD) {
            score += 40;
        }
        if (taskType == "RELAY" && node.role == ROLE_RELAY) {
            score += 30;
        }
        
        // Battery level consideration (higher is better)
        score += node.capabilities.batteryLevel / 4;
        
        // Signal strength consideration (closer/stronger signal is better)
        // Convert signal strength from negative dBm to positive score
        score += (node.signalStrength + 120) / 2;
        
        // Hop count consideration (fewer hops is better)
        score -= node.hopCount * 5;
        
        // Load balancing: count how many tasks are already assigned to this node
        int nodeTaskCount = std::count_if(activeTasks_.begin(), activeTasks_.end(),
                                         [&node](const Task& t) { 
                                             return t.assignedNode == node.nodeId && !t.completed; 
                                         });
        score -= nodeTaskCount * 10;
        
        // Select node with highest score
        if (score > highestScore) {
            highestScore = score;
            bestNodeId = node.nodeId;
        }
    }
    
    return bestNodeId;
}

void BoardCoordinator::sendNetworkTopology() {
    String message = MessageProtocol::createTopologyMessage(managedNodes_);
    
    if (LoraMesh::queueMessage(message)) {
        Serial.printf("📡 Network topology broadcast: %d nodes\n", managedNodes_.size());
    } else {
        Serial.println("⚠️  Failed to broadcast network topology");
    }
}

DiscoveryProtocol* BoardCoordinator::getDiscoveryProtocol() const {
    return discoveryProtocol_;
}