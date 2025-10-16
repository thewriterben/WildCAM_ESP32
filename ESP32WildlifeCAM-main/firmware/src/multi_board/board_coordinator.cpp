/**
 * @file board_coordinator.cpp
 * @brief Implementation of coordinator role for multi-board systems
 */

#include "board_coordinator.h"
#include "../lora_mesh.h"
#include "../config.h"
#include <map>

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
    // If no specific target node, use load balancing to select best node
    if (targetNode <= 0 && networkConfig_.enableLoadBalancing) {
        targetNode = selectBestNodeForTask(taskType);
        if (targetNode <= 0) {
            Serial.printf("Failed to find suitable node for task '%s'\n", taskType.c_str());
            return false;
        }
        Serial.printf("Load balancing: Assigned task '%s' to node %d\n", taskType.c_str(), targetNode);
    }
    
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
        
        // Track nodes that joined or left
        std::vector<int> joinedNodes;
        std::vector<int> leftNodes;
        
        // Find newly joined nodes
        for (const auto& node : discoveredNodes) {
            auto it = std::find_if(managedNodes_.begin(), managedNodes_.end(),
                                  [&node](const NetworkNode& n) { return n.nodeId == node.nodeId; });
            
            if (it == managedNodes_.end()) {
                // New node joined
                joinedNodes.push_back(node.nodeId);
                Serial.printf("⚡ Node %d joined the network (Role: %s)\n",
                             node.nodeId, MessageProtocol::roleToString(node.role));
            } else {
                // Check if node capabilities have changed significantly
                bool capabilitiesChanged = 
                    (it->capabilities.batteryLevel != node.capabilities.batteryLevel && 
                     abs((int)it->capabilities.batteryLevel - (int)node.capabilities.batteryLevel) > 20) ||
                    (it->capabilities.hasCamera != node.capabilities.hasCamera) ||
                    (it->capabilities.hasLoRa != node.capabilities.hasLoRa) ||
                    (it->capabilities.hasAI != node.capabilities.hasAI) ||
                    (it->capabilities.hasPSRAM != node.capabilities.hasPSRAM);
                
                if (capabilitiesChanged && networkConfig_.enableAutomaticRoleAssignment) {
                    Serial.printf("⚠ Node %d capabilities changed, reassessing role\n", node.nodeId);
                    Serial.printf("  - Camera: %d, LoRa: %d, AI: %d, Battery: %d%%\n",
                                 node.capabilities.hasCamera, node.capabilities.hasLoRa,
                                 node.capabilities.hasAI, node.capabilities.batteryLevel);
                    
                    BoardRole optimalRole = determineOptimalRole(node.capabilities);
                    if (optimalRole != node.role) {
                        Serial.printf("  - Reassigning from %s to %s\n",
                                     MessageProtocol::roleToString(node.role),
                                     MessageProtocol::roleToString(optimalRole));
                        sendRoleAssignment(node.nodeId, optimalRole);
                    }
                }
            }
        }
        
        // Find nodes that left
        for (const auto& oldNode : managedNodes_) {
            auto it = std::find_if(discoveredNodes.begin(), discoveredNodes.end(),
                                  [&oldNode](const NetworkNode& n) { return n.nodeId == oldNode.nodeId; });
            
            if (it == discoveredNodes.end()) {
                leftNodes.push_back(oldNode.nodeId);
                Serial.printf("⚠ Node %d left the network\n", oldNode.nodeId);
            }
        }
        
        // Update managed nodes list
        managedNodes_ = discoveredNodes;
        
        // Handle newly joined nodes - assign roles
        if (!joinedNodes.empty() && networkConfig_.enableAutomaticRoleAssignment) {
            Serial.println("=== Assigning roles to new nodes ===");
            assignNodeRoles();
            
            // Balance load across all nodes including new ones
            if (networkConfig_.enableLoadBalancing) {
                Serial.println("=== Rebalancing load after node join ===");
                rebalanceTaskLoad();
            }
        }
        
        // Handle node failures - reassign their tasks
        if (!leftNodes.empty()) {
            Serial.println("=== Handling node failures and task reassignment ===");
            for (int failedNodeId : leftNodes) {
                reassignTasksFromFailedNode(failedNodeId);
            }
        }
    }

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

void BoardCoordinator::reassignTasksFromFailedNode(int failedNodeId) {
    Serial.printf("Reassigning tasks from failed node %d\n", failedNodeId);
    
    // Find all active tasks assigned to failed node
    for (auto& task : activeTasks_) {
        if (task.assignedNode == failedNodeId && !task.completed) {
            // Select a new node for this task
            int newNodeId = selectBestNodeForTask(task.taskType);
            
            if (newNodeId > 0) {
                Serial.printf("  - Reassigning task %d (%s) to node %d\n",
                             task.taskId, task.taskType.c_str(), newNodeId);
                task.assignedNode = newNodeId;
                task.deadline = millis() + networkConfig_.taskTimeout; // Reset deadline
                sendTaskAssignment(task);
            } else {
                Serial.printf("  - No suitable node found for task %d, marking as failed\n", task.taskId);
                task.completed = true; // Mark as completed (failed)
            }
        }
    }
}

void BoardCoordinator::rebalanceTaskLoad() {
    Serial.println("Rebalancing task load across nodes...");
    
    // Count tasks per node
    std::map<int, int> nodeTaskCount;
    std::map<int, std::vector<Task*>> nodeTasks;
    
    for (auto& task : activeTasks_) {
        if (!task.completed) {
            nodeTaskCount[task.assignedNode]++;
            nodeTasks[task.assignedNode].push_back(&task);
        }
    }
    
    // Find average and identify overloaded nodes
    int totalTasks = 0;
    for (const auto& pair : nodeTaskCount) {
        totalTasks += pair.second;
    }
    
    if (managedNodes_.empty() || totalTasks == 0) {
        return;
    }
    
    float avgLoad = (float)totalTasks / managedNodes_.size();
    Serial.printf("Average task load: %.1f tasks per node\n", avgLoad);
    
    // Rebalance overloaded nodes
    for (const auto& pair : nodeTaskCount) {
        int nodeId = pair.first;
        int taskCount = pair.second;
        
        if (taskCount > avgLoad * 1.5) { // Node is overloaded (>150% of average)
            Serial.printf("Node %d is overloaded with %d tasks (%.0f%% above average)\n",
                         nodeId, taskCount, (taskCount - avgLoad) / avgLoad * 100);
            
            // Try to reassign some tasks to less loaded nodes
            auto& tasks = nodeTasks[nodeId];
            int tasksToMove = taskCount - (int)avgLoad;
            
            for (int i = 0; i < tasksToMove && i < tasks.size(); i++) {
                Task* task = tasks[i];
                if (task->priority < 3) { // Only move non-critical tasks
                    int newNodeId = selectBestNodeForTask(task->taskType);
                    if (newNodeId > 0 && newNodeId != nodeId) {
                        Serial.printf("  - Moving task %d to node %d for load balancing\n",
                                     task->taskId, newNodeId);
                        task->assignedNode = newNodeId;
                        sendTaskAssignment(*task);
                    }
                }
            }
        }
    }
}

BoardRole BoardCoordinator::determineOptimalRole(const BoardCapabilities& caps) const {
    // Dynamic capability-based role assignment based on hardware capabilities
    // Priority: Camera > LoRa > AI > Storage > Power considerations
    
    // Consider battery level for role assignment
    bool lowBattery = (caps.batteryLevel < 30);
    bool hasSolarPower = (caps.solarVoltage > 4.0);
    
    // Nodes with camera become capture nodes (highest priority for wildlife monitoring)
    if (caps.hasCamera && !lowBattery) {
        // High-resolution camera with AI - best for capture and processing
        if (caps.hasAI && caps.hasPSRAM && caps.maxResolution >= 1920 * 1080) {
            Serial.printf("Role: AI_PROCESSOR - Camera=%d, AI=%d, PSRAM=%d, Res=%d, Bat=%d%%\n",
                         caps.hasCamera, caps.hasAI, caps.hasPSRAM, caps.maxResolution, caps.batteryLevel);
            return ROLE_AI_PROCESSOR;
        }
        
        // High-resolution camera with good storage - hub for data collection
        if (caps.maxResolution >= 1600 * 1200 && caps.availableStorage > 1024 * 1024 && caps.hasSD) {
            Serial.printf("Role: HUB - Camera=%d, Res=%d, Storage=%d MB, SD=%d\n",
                         caps.hasCamera, caps.maxResolution, caps.availableStorage / (1024*1024), caps.hasSD);
            return ROLE_HUB;
        }
        
        // Standard camera node for wildlife capture
        Serial.printf("Role: NODE (capture) - Camera=%d, Res=%d, Bat=%d%%\n",
                     caps.hasCamera, caps.maxResolution, caps.batteryLevel);
        return ROLE_NODE;
    }
    
    // Nodes with LoRa become relay nodes (for network extension)
    if (caps.hasLoRa) {
        // LoRa with solar power - excellent relay for remote areas
        if (hasSolarPower && caps.batteryLevel > 50) {
            Serial.printf("Role: RELAY - LoRa=%d, Solar=%.1fV, Battery=%d%%\n",
                         caps.hasLoRa, caps.solarVoltage, caps.batteryLevel);
            return ROLE_RELAY;
        }
        
        // LoRa with good battery - relay node
        if (caps.batteryLevel >= 50) {
            Serial.printf("Role: RELAY - LoRa=%d, Battery=%d%%\n",
                         caps.hasLoRa, caps.batteryLevel);
            return ROLE_RELAY;
        }
    }
    
    // Low-power boards or low battery - assign stealth role for energy conservation
    if (caps.powerProfile <= 1 || lowBattery) {
        Serial.printf("Role: STEALTH - Power=%d, Battery=%d%%\n",
                     caps.powerProfile, caps.batteryLevel);
        return ROLE_STEALTH;
    }
    
    // Boards with cellular/satellite - portable monitoring stations
    if (caps.hasCellular || caps.hasSatellite) {
        Serial.printf("Role: PORTABLE - Cellular=%d, Satellite=%d\n",
                     caps.hasCellular, caps.hasSatellite);
        return ROLE_PORTABLE;
    }
    
    // Edge sensors without camera - minimal processing edge nodes
    if (!caps.hasCamera && !caps.hasLoRa) {
        Serial.printf("Role: EDGE_SENSOR - Basic sensor node\n");
        return ROLE_EDGE_SENSOR;
    }
    
    // Default to node role for standard operation
    Serial.printf("Role: NODE (default) - fallback assignment\n");
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


            bestNodeId = node.nodeId;
        }
    }
    
}

DiscoveryProtocol* BoardCoordinator::getDiscoveryProtocol() const {
    return discoveryProtocol_;
}

void BoardCoordinator::checkNodeHealth() {
    unsigned long now = millis();
    
    for (auto& node : managedNodes_) {
        // Skip checking the coordinator node itself
        if (node.nodeId == nodeId_) {
            continue;
        }
        
        // Check if node has missed heartbeat for more than NODE_FAILURE_TIMEOUT
        if (node.isActive && (now - node.lastSeen > NODE_FAILURE_TIMEOUT)) {
            Serial.printf("⚠ Node %d failure detected: no heartbeat for %lu ms\n", 
                         node.nodeId, now - node.lastSeen);
            markNodeAsFailed(node.nodeId);
        }
    }
}

void BoardCoordinator::markNodeAsFailed(int nodeId) {
    // Find the node and mark it as inactive
    for (auto& node : managedNodes_) {
        if (node.nodeId == nodeId && node.isActive) {
            node.isActive = false;
            logNodeFailure(nodeId, "Heartbeat timeout");
            
            // Reassign tasks from this failed node
            reassignTasksFromFailedNode(nodeId);
            
            Serial.printf("✗ Node %d marked as FAILED\n", nodeId);
            break;
        }
    }
}

void BoardCoordinator::reassignTasksFromFailedNode(int failedNodeId) {
    int reassignedCount = 0;
    
    // Find all tasks assigned to the failed node
    for (auto& task : activeTasks_) {
        if (task.assignedNode == failedNodeId && !task.completed) {
            // Select a healthy node for this task
            int newNodeId = selectHealthyNodeForTask(task.taskType);
            
            if (newNodeId > 0) {
                Serial.printf("↻ Reassigning task %d from node %d to node %d\n", 
                             task.taskId, failedNodeId, newNodeId);
                
                // Update task assignment
                task.assignedNode = newNodeId;
                task.deadline = millis() + networkConfig_.taskTimeout; // Reset deadline
                
                // Send new task assignment
                if (sendTaskAssignment(task)) {
                    reassignedCount++;
                } else {
                    Serial.printf("⚠ Failed to send reassigned task %d to node %d\n", 
                                 task.taskId, newNodeId);
                }
            } else {
                Serial.printf("⚠ No healthy node available to reassign task %d\n", task.taskId);
            }
        }
    }
    
    if (reassignedCount > 0) {
        Serial.printf("✓ Reassigned %d tasks from failed node %d\n", 
                     reassignedCount, failedNodeId);
    }
}

int BoardCoordinator::selectHealthyNodeForTask(const String& taskType) {
    int bestNodeId = -1;
    int bestScore = -1;
    
    for (const auto& node : managedNodes_) {
        // Skip inactive nodes and the coordinator itself
        if (!node.isActive || node.nodeId == nodeId_) {
            continue;
        }
        
        // Calculate node score based on capabilities and load
        int score = 0;
        
        // Prefer nodes with AI capabilities for processing tasks
        if (node.capabilities.hasAI && (taskType.indexOf("process") >= 0 || 
                                        taskType.indexOf("detect") >= 0 || 
                                        taskType.indexOf("analyze") >= 0)) {
            score += 50;
        }
        
        // Consider battery level (prefer higher battery)
        score += node.capabilities.batteryLevel / 2;
        
        // Consider signal strength (prefer stronger signals)
        score += (100 + node.signalStrength) / 2; // RSSI typically -100 to 0
        
        // Consider current load (count active tasks for this node)
        int nodeTaskCount = 0;
        for (const auto& task : activeTasks_) {
            if (task.assignedNode == node.nodeId && !task.completed) {
                nodeTaskCount++;
            }
        }
        score -= nodeTaskCount * 10; // Penalize busy nodes
        
        // Select node with best score
        if (score > bestScore) {
            bestScore = score;
            bestNodeId = node.nodeId;
        }
    }
    
    return bestNodeId;
}

void BoardCoordinator::logNodeFailure(int nodeId, const char* reason) {
    unsigned long now = millis();
    
    Serial.println("========================================");
    Serial.println("NODE FAILURE EVENT");
    Serial.println("========================================");
    Serial.printf("Timestamp: %lu ms\n", now);
    Serial.printf("Node ID: %d\n", nodeId);
    Serial.printf("Reason: %s\n", reason);
    Serial.printf("Uptime at failure: %lu ms\n", now - startTime_);
    Serial.printf("Active nodes remaining: %d\n", countActiveNodes());
    Serial.printf("Tasks to reassign: %d\n", countTasksForNode(nodeId));
    Serial.println("========================================");
}

int BoardCoordinator::countActiveNodes() const {
    int count = 0;
    for (const auto& node : managedNodes_) {
        if (node.isActive) {
            count++;
        }
    }
    return count;
}

int BoardCoordinator::countTasksForNode(int nodeId) const {
    int count = 0;
    for (const auto& task : activeTasks_) {
        if (task.assignedNode == nodeId && !task.completed) {
            count++;
        }
    }
    return count;
}

void BoardCoordinator::sendNetworkTopology() {
    String message = MessageProtocol::createTopologyMessage(managedNodes_);
    LoraMesh::queueMessage(message);
}