/**
 * @file swarm_coordinator.cpp
 * @brief Implementation of swarm robotics coordination system
 */

#include "swarm_coordinator.h"
#include <cmath>
#include <algorithm>

// Global instance
SwarmCoordinator* g_swarmCoordinator = nullptr;

// ===========================
// CONSTRUCTOR / DESTRUCTOR
// ===========================

SwarmCoordinator::SwarmCoordinator() 
    : initialized_(false)
    , missionActive_(false)
    , currentMission_(MISSION_SURVEILLANCE)
    , currentBehavior_(BEHAVIOR_FLOCKING)
    , missionStartTime_(0)
    , taskCounter_(0)
{
    memset(&stats_, 0, sizeof(stats_));
    globalBestPosition_ = {0, 0, 0, 0};
}

SwarmCoordinator::~SwarmCoordinator() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool SwarmCoordinator::init(const SwarmConfig& config) {
    if (initialized_) {
        return true;
    }
    
    config_ = config;
    members_.clear();
    targets_.clear();
    
    // Initialize default PSO parameters
    psoParams_.populationSize = 20;
    psoParams_.maxIterations = 100;
    psoParams_.inertiaWeight = 0.7f;
    psoParams_.cognitiveWeight = 1.5f;
    psoParams_.socialWeight = 1.5f;
    psoParams_.minVelocity = -config.maxVelocity;
    psoParams_.maxVelocity = config.maxVelocity;
    
    // Initialize default ACO parameters
    acoParams_.antCount = 20;
    acoParams_.maxIterations = 100;
    acoParams_.pheromoneEvaporation = 0.1f;
    acoParams_.pheromoneDeposit = 1.0f;
    acoParams_.alpha = 1.0f;
    acoParams_.beta = 2.0f;
    
    // Initialize default Bee parameters
    beeParams_.scoutBees = 10;
    beeParams_.selectedSites = 3;
    beeParams_.eliteSites = 1;
    beeParams_.recruitedBees = 20;
    beeParams_.maxIterations = 100;
    beeParams_.searchRadius = 50.0f;
    
    initialized_ = true;
    logSwarmEvent("Swarm Coordinator Initialized", String("Size: ") + config.swarmSize);
    
    return true;
}

void SwarmCoordinator::cleanup() {
    members_.clear();
    targets_.clear();
    psoPositions_.clear();
    psoVelocities_.clear();
    psoFitness_.clear();
    memberVotes_.clear();
    initialized_ = false;
    missionActive_ = false;
}

// ===========================
// SWARM MANAGEMENT
// ===========================

bool SwarmCoordinator::registerMember(const SwarmMember& member) {
    if (!validateMember(member)) {
        return false;
    }
    
    members_[member.memberId] = member;
    members_[member.memberId].isActive = true;
    members_[member.memberId].isHealthy = true;
    members_[member.memberId].lastUpdate = millis();
    
    updateStatistics();
    logSwarmEvent("Member Registered", String("ID: ") + member.memberId);
    
    return true;
}

bool SwarmCoordinator::unregisterMember(int memberId) {
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return false;
    }
    
    members_.erase(it);
    updateStatistics();
    logSwarmEvent("Member Unregistered", String("ID: ") + memberId);
    
    return true;
}

bool SwarmCoordinator::updateMemberState(int memberId, const Position3D& position, 
                                        const Velocity3D& velocity, float battery) {
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return false;
    }
    
    it->second.position = position;
    it->second.velocity = velocity;
    it->second.batteryLevel = battery;
    it->second.lastUpdate = millis();
    it->second.isActive = true;
    
    return true;
}

SwarmMember SwarmCoordinator::getMember(int memberId) const {
    auto it = members_.find(memberId);
    if (it != members_.end()) {
        return it->second;
    }
    return SwarmMember{};
}

std::vector<SwarmMember> SwarmCoordinator::getAllMembers() const {
    std::vector<SwarmMember> result;
    for (const auto& pair : members_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<SwarmMember> SwarmCoordinator::getActiveMembers() const {
    std::vector<SwarmMember> result;
    for (const auto& pair : members_) {
        if (pair.second.isActive && pair.second.isHealthy) {
            result.push_back(pair.second);
        }
    }
    return result;
}

// ===========================
// MISSION COORDINATION
// ===========================

bool SwarmCoordinator::startMission(SwarmMission mission, const Position3D& target) {
    if (!initialized_) {
        return false;
    }
    
    currentMission_ = mission;
    missionActive_ = true;
    missionStartTime_ = millis();
    
    logSwarmEvent("Mission Started", String("Type: ") + mission);
    
    return true;
}

bool SwarmCoordinator::pauseMission() {
    if (!missionActive_) {
        return false;
    }
    
    missionActive_ = false;
    logSwarmEvent("Mission Paused", "");
    return true;
}

bool SwarmCoordinator::resumeMission() {
    if (missionActive_) {
        return false;
    }
    
    missionActive_ = true;
    logSwarmEvent("Mission Resumed", "");
    return true;
}

bool SwarmCoordinator::abortMission() {
    missionActive_ = false;
    currentMission_ = MISSION_SURVEILLANCE;
    logSwarmEvent("Mission Aborted", "");
    return true;
}

// ===========================
// FORMATION CONTROL
// ===========================

bool SwarmCoordinator::setFormation(SwarmFormation formation) {
    config_.formation = formation;
    logSwarmEvent("Formation Changed", String("Type: ") + formation);
    return updateFormation();
}

bool SwarmCoordinator::updateFormation() {
    if (members_.empty()) {
        return false;
    }
    
    // Update each member's desired position based on formation
    for (auto& pair : members_) {
        if (pair.second.isActive) {
            Position3D desired = calculateDesiredPosition(pair.first);
            // Member should move toward desired position
        }
    }
    
    return true;
}

bool SwarmCoordinator::maintainFormation() {
    return updateFormation();
}

Position3D SwarmCoordinator::calculateDesiredPosition(int memberId) {
    Position3D desired = {0, 0, 0, 0};
    
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return desired;
    }
    
    int index = std::distance(members_.begin(), it);
    int totalMembers = members_.size();
    
    switch (config_.formation) {
        case FORMATION_GRID: {
            int cols = static_cast<int>(sqrt(totalMembers));
            int row = index / cols;
            int col = index % cols;
            desired.x = col * config_.separationDistance;
            desired.y = row * config_.separationDistance;
            desired.z = 10.0f; // Default altitude
            break;
        }
        
        case FORMATION_LINE: {
            desired.x = index * config_.separationDistance;
            desired.y = 0;
            desired.z = 10.0f;
            break;
        }
        
        case FORMATION_CIRCLE: {
            float angle = (2.0f * PI * index) / totalMembers;
            float radius = (totalMembers * config_.separationDistance) / (2.0f * PI);
            desired.x = radius * cos(angle);
            desired.y = radius * sin(angle);
            desired.z = 10.0f;
            break;
        }
        
        case FORMATION_V_SHAPE: {
            int side = (index % 2 == 0) ? 1 : -1;
            int row = index / 2;
            desired.x = row * config_.separationDistance;
            desired.y = side * row * config_.separationDistance * 0.5f;
            desired.z = 10.0f;
            break;
        }
        
        default:
            desired = it->second.position;
            break;
    }
    
    return desired;
}

// ===========================
// BIO-INSPIRED ALGORITHMS - FLOCKING
// ===========================

Velocity3D SwarmCoordinator::calculateFlockingVelocity(int memberId) {
    Velocity3D separation = separationForce(memberId);
    Velocity3D cohesion = cohesionForce(memberId);
    Velocity3D alignment = alignmentForce(memberId);
    
    // Weighted combination of forces
    Velocity3D result;
    result.vx = separation.vx * 1.5f + cohesion.vx * 1.0f + alignment.vx * 1.0f;
    result.vy = separation.vy * 1.5f + cohesion.vy * 1.0f + alignment.vy * 1.0f;
    result.vz = separation.vz * 1.5f + cohesion.vz * 1.0f + alignment.vz * 1.0f;
    
    return limitVelocity(result);
}

Velocity3D SwarmCoordinator::separationForce(int memberId) {
    Velocity3D force = {0, 0, 0};
    
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return force;
    }
    
    std::vector<int> neighbors = findNeighbors(memberId, config_.separationDistance);
    
    for (int neighborId : neighbors) {
        auto neighbor = members_.find(neighborId);
        if (neighbor != members_.end()) {
            float dx = it->second.position.x - neighbor->second.position.x;
            float dy = it->second.position.y - neighbor->second.position.y;
            float dz = it->second.position.z - neighbor->second.position.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            
            if (dist > 0 && dist < config_.separationDistance) {
                force.vx += dx / dist;
                force.vy += dy / dist;
                force.vz += dz / dist;
            }
        }
    }
    
    return force;
}

Velocity3D SwarmCoordinator::cohesionForce(int memberId) {
    Velocity3D force = {0, 0, 0};
    
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return force;
    }
    
    std::vector<int> neighbors = findNeighbors(memberId, config_.cohesionRadius);
    
    if (neighbors.empty()) {
        return force;
    }
    
    // Calculate center of mass
    float centerX = 0, centerY = 0, centerZ = 0;
    for (int neighborId : neighbors) {
        auto neighbor = members_.find(neighborId);
        if (neighbor != members_.end()) {
            centerX += neighbor->second.position.x;
            centerY += neighbor->second.position.y;
            centerZ += neighbor->second.position.z;
        }
    }
    
    centerX /= neighbors.size();
    centerY /= neighbors.size();
    centerZ /= neighbors.size();
    
    // Move toward center
    force.vx = (centerX - it->second.position.x) * 0.01f;
    force.vy = (centerY - it->second.position.y) * 0.01f;
    force.vz = (centerZ - it->second.position.z) * 0.01f;
    
    return force;
}

Velocity3D SwarmCoordinator::alignmentForce(int memberId) {
    Velocity3D force = {0, 0, 0};
    
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return force;
    }
    
    std::vector<int> neighbors = findNeighbors(memberId, config_.alignmentRadius);
    
    if (neighbors.empty()) {
        return force;
    }
    
    // Calculate average velocity
    float avgVx = 0, avgVy = 0, avgVz = 0;
    for (int neighborId : neighbors) {
        auto neighbor = members_.find(neighborId);
        if (neighbor != members_.end()) {
            avgVx += neighbor->second.velocity.vx;
            avgVy += neighbor->second.velocity.vy;
            avgVz += neighbor->second.velocity.vz;
        }
    }
    
    avgVx /= neighbors.size();
    avgVy /= neighbors.size();
    avgVz /= neighbors.size();
    
    force.vx = (avgVx - it->second.velocity.vx) * 0.1f;
    force.vy = (avgVy - it->second.velocity.vy) * 0.1f;
    force.vz = (avgVz - it->second.velocity.vz) * 0.1f;
    
    return force;
}

// ===========================
// PARTICLE SWARM OPTIMIZATION
// ===========================

bool SwarmCoordinator::initPSO(const PSOParams& params) {
    psoParams_ = params;
    
    psoPositions_.clear();
    psoVelocities_.clear();
    psoFitness_.clear();
    
    // Initialize random positions and velocities
    for (int i = 0; i < params.populationSize; i++) {
        Position3D pos;
        pos.x = random(-1000, 1000);
        pos.y = random(-1000, 1000);
        pos.z = random(5, 100);
        pos.heading = random(0, 360);
        psoPositions_.push_back(pos);
        
        Velocity3D vel;
        vel.vx = random(-10, 10) * 0.1f;
        vel.vy = random(-10, 10) * 0.1f;
        vel.vz = random(-5, 5) * 0.1f;
        psoVelocities_.push_back(vel);
        
        psoFitness_.push_back(0);
    }
    
    globalBestPosition_ = psoPositions_[0];
    
    return true;
}

bool SwarmCoordinator::runPSOIteration() {
    if (psoPositions_.empty()) {
        return false;
    }
    
    // Evaluate fitness
    float globalBestFitness = -1e9;
    for (size_t i = 0; i < psoPositions_.size(); i++) {
        psoFitness_[i] = calculateFitness(psoPositions_[i]);
        if (psoFitness_[i] > globalBestFitness) {
            globalBestFitness = psoFitness_[i];
            globalBestPosition_ = psoPositions_[i];
        }
    }
    
    // Update velocities and positions
    for (size_t i = 0; i < psoPositions_.size(); i++) {
        float r1 = random(0, 100) / 100.0f;
        float r2 = random(0, 100) / 100.0f;
        
        psoVelocities_[i].vx = psoParams_.inertiaWeight * psoVelocities_[i].vx +
                                psoParams_.cognitiveWeight * r1 * (psoPositions_[i].x - psoPositions_[i].x) +
                                psoParams_.socialWeight * r2 * (globalBestPosition_.x - psoPositions_[i].x);
        
        psoVelocities_[i].vy = psoParams_.inertiaWeight * psoVelocities_[i].vy +
                                psoParams_.cognitiveWeight * r1 * (psoPositions_[i].y - psoPositions_[i].y) +
                                psoParams_.socialWeight * r2 * (globalBestPosition_.y - psoPositions_[i].y);
        
        psoVelocities_[i].vz = psoParams_.inertiaWeight * psoVelocities_[i].vz +
                                psoParams_.cognitiveWeight * r1 * (psoPositions_[i].z - psoPositions_[i].z) +
                                psoParams_.socialWeight * r2 * (globalBestPosition_.z - psoPositions_[i].z);
        
        // Update positions
        psoPositions_[i].x += psoVelocities_[i].vx;
        psoPositions_[i].y += psoVelocities_[i].vy;
        psoPositions_[i].z += psoVelocities_[i].vz;
    }
    
    return true;
}

Position3D SwarmCoordinator::getBestPSOPosition() const {
    return globalBestPosition_;
}

// ===========================
// ANT COLONY OPTIMIZATION
// ===========================

bool SwarmCoordinator::initACO(const ACOParams& params) {
    acoParams_ = params;
    // ACO initialization would involve pheromone matrix setup
    return true;
}

bool SwarmCoordinator::runACOIteration() {
    // Simplified ACO implementation
    // In practice, this would construct paths based on pheromone trails
    return true;
}

std::vector<Position3D> SwarmCoordinator::getBestACOPath() const {
    return std::vector<Position3D>();
}

// ===========================
// BEE ALGORITHM
// ===========================

bool SwarmCoordinator::initBeeAlgorithm(const BeeParams& params) {
    beeParams_ = params;
    return true;
}

bool SwarmCoordinator::runBeeIteration() {
    // Simplified Bee Algorithm implementation
    return true;
}

std::vector<Position3D> SwarmCoordinator::getBestSearchAreas() const {
    return std::vector<Position3D>();
}

// ===========================
// GENETIC ALGORITHM
// ===========================

bool SwarmCoordinator::runGeneticOptimization(int generations) {
    // Simplified genetic algorithm for swarm configuration optimization
    return true;
}

SwarmConfig SwarmCoordinator::getOptimalConfiguration() const {
    return config_;
}

// ===========================
// CONSENSUS ALGORITHMS
// ===========================

bool SwarmCoordinator::initiateConsensus(const String& decisionType, 
                                        const std::vector<String>& options) {
    currentConsensus_.decisionType = decisionType;
    currentConsensus_.votes.clear();
    currentConsensus_.timestamp = millis();
    
    for (const auto& option : options) {
        currentConsensus_.votes[option] = 0;
    }
    
    memberVotes_.clear();
    
    return true;
}

bool SwarmCoordinator::submitVote(int memberId, const String& option) {
    auto it = members_.find(memberId);
    if (it == members_.end() || !it->second.isActive) {
        return false;
    }
    
    if (currentConsensus_.votes.find(option) == currentConsensus_.votes.end()) {
        return false;
    }
    
    memberVotes_[memberId] = option;
    currentConsensus_.votes[option] += 1.0f;
    
    return true;
}

ConsensusDecision SwarmCoordinator::getConsensusResult() {
    // Apply Byzantine fault tolerance filtering
    applyByzantineFilter(currentConsensus_.votes);
    
    // Find winner
    String winner;
    float maxVotes = 0;
    float totalVotes = 0;
    
    for (const auto& pair : currentConsensus_.votes) {
        totalVotes += pair.second;
        if (pair.second > maxVotes) {
            maxVotes = pair.second;
            winner = pair.first;
        }
    }
    
    currentConsensus_.winner = winner;
    currentConsensus_.confidence = totalVotes > 0 ? maxVotes / totalVotes : 0;
    
    return currentConsensus_;
}

bool SwarmCoordinator::hasByzantineFault() const {
    // Simplified Byzantine fault detection
    return false;
}

// ===========================
// TASK ALLOCATION
// ===========================

bool SwarmCoordinator::assignTask(int memberId, const String& taskType, 
                                  const Position3D& location) {
    auto it = members_.find(memberId);
    if (it == members_.end() || !it->second.isActive) {
        return false;
    }
    
    taskCounter_++;
    logSwarmEvent("Task Assigned", String("Member: ") + memberId + " Task: " + taskType);
    
    return true;
}

int SwarmCoordinator::selectOptimalMember(const Position3D& taskLocation, 
                                         SwarmMemberType preferredType) {
    int bestMember = -1;
    float bestScore = -1e9;
    
    for (const auto& pair : members_) {
        if (!pair.second.isActive || !pair.second.isHealthy) {
            continue;
        }
        
        float distance = calculateDistance(pair.second.position, taskLocation);
        float batteryScore = pair.second.batteryLevel;
        float typeScore = (pair.second.type == preferredType) ? 100.0f : 0.0f;
        
        float score = -distance * 0.5f + batteryScore * 2.0f + typeScore;
        
        if (score > bestScore) {
            bestScore = score;
            bestMember = pair.first;
        }
    }
    
    return bestMember;
}

std::vector<int> SwarmCoordinator::selectMemberTeam(const Position3D& target, int teamSize) {
    std::vector<int> team;
    std::vector<std::pair<int, float>> scores;
    
    for (const auto& pair : members_) {
        if (!pair.second.isActive || !pair.second.isHealthy) {
            continue;
        }
        
        float distance = calculateDistance(pair.second.position, target);
        float score = -distance + pair.second.batteryLevel * 10.0f;
        scores.push_back({pair.first, score});
    }
    
    std::sort(scores.begin(), scores.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (size_t i = 0; i < std::min(static_cast<size_t>(teamSize), scores.size()); i++) {
        team.push_back(scores[i].first);
    }
    
    return team;
}

// ===========================
// TARGET TRACKING
// ===========================

bool SwarmCoordinator::trackTarget(const TrackingTarget& target) {
    targets_.push_back(target);
    
    // Assign tracking team
    std::vector<int> team = assignTrackingTeam(target.targetId);
    
    logSwarmEvent("Target Tracking", String("ID: ") + target.targetId + 
                  " Species: " + target.species);
    
    return true;
}

bool SwarmCoordinator::updateTargetPosition(int targetId, const Position3D& position) {
    for (auto& target : targets_) {
        if (target.targetId == targetId) {
            target.position = position;
            target.lastSeen = millis();
            return true;
        }
    }
    return false;
}

std::vector<TrackingTarget> SwarmCoordinator::getActiveTargets() const {
    std::vector<TrackingTarget> active;
    unsigned long now = millis();
    
    for (const auto& target : targets_) {
        if (now - target.lastSeen < 60000) { // Active within last minute
            active.push_back(target);
        }
    }
    
    return active;
}

std::vector<int> SwarmCoordinator::assignTrackingTeam(int targetId) {
    for (const auto& target : targets_) {
        if (target.targetId == targetId) {
            return selectMemberTeam(target.position, 3);
        }
    }
    return std::vector<int>();
}

// ===========================
// PATH PLANNING
// ===========================

std::vector<Position3D> SwarmCoordinator::planOptimalPath(const Position3D& start, 
                                                          const Position3D& goal) {
    std::vector<Position3D> path;
    
    // Simplified A* path planning
    path.push_back(start);
    path.push_back(goal);
    
    return path;
}

bool SwarmCoordinator::avoidObstacle(int memberId, const Position3D& obstacle) {
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return false;
    }
    
    float distance = calculateDistance(it->second.position, obstacle);
    
    if (distance < config_.separationDistance * 2.0f) {
        // Calculate avoidance vector
        return true;
    }
    
    return false;
}

// ===========================
// WILDLIFE MONITORING
// ===========================

bool SwarmCoordinator::startWildlifeSurveillance(const Position3D& area, float radius) {
    return startMission(MISSION_SURVEILLANCE, area);
}

bool SwarmCoordinator::detectWildlifeIntrusion(const Position3D& location) {
    logSwarmEvent("Wildlife Intrusion", String("X: ") + location.x + " Y: " + location.y);
    return true;
}

bool SwarmCoordinator::coordinateAntiPoaching(const Position3D& threatLocation) {
    startMission(MISSION_ANTI_POACHING, threatLocation);
    
    // Assign closest members to investigate
    std::vector<int> team = selectMemberTeam(threatLocation, 5);
    
    logSwarmEvent("Anti-Poaching Alert", String("Team Size: ") + team.size());
    
    return true;
}

bool SwarmCoordinator::trackMigrationPattern(const std::vector<Position3D>& waypoints) {
    logSwarmEvent("Migration Tracking", String("Waypoints: ") + waypoints.size());
    return true;
}

// ===========================
// EMERGENCY RESPONSE
// ===========================

bool SwarmCoordinator::triggerEmergencyResponse(const String& emergencyType, 
                                               const Position3D& location) {
    startMission(MISSION_EMERGENCY_RESPONSE, location);
    
    logSwarmEvent("Emergency Response", emergencyType);
    
    return true;
}

bool SwarmCoordinator::coordinateSearchAndRescue(const Position3D& searchArea) {
    setFormation(FORMATION_GRID);
    return startMission(MISSION_SEARCH, searchArea);
}

bool SwarmCoordinator::deployEmergencyRelay(const Position3D& location) {
    int relayMember = selectOptimalMember(location, SWARM_RELAY_NODE);
    
    if (relayMember >= 0) {
        assignTask(relayMember, "deploy_relay", location);
        return true;
    }
    
    return false;
}

// ===========================
// SELF-HEALING & FAULT TOLERANCE
// ===========================

bool SwarmCoordinator::detectFailedMembers() {
    unsigned long now = millis();
    bool foundFailures = false;
    
    for (auto& pair : members_) {
        if (now - pair.second.lastUpdate > 30000) { // 30 seconds timeout
            pair.second.isHealthy = false;
            pair.second.isActive = false;
            foundFailures = true;
            
            logSwarmEvent("Member Failed", String("ID: ") + pair.first);
        }
    }
    
    if (foundFailures) {
        updateStatistics();
    }
    
    return foundFailures;
}

bool SwarmCoordinator::replaceFailedMember(int failedId, int replacementId) {
    auto failed = members_.find(failedId);
    auto replacement = members_.find(replacementId);
    
    if (failed == members_.end() || replacement == members_.end()) {
        return false;
    }
    
    // Transfer tasks from failed to replacement
    logSwarmEvent("Member Replaced", String("Failed: ") + failedId + 
                  " Replacement: " + replacementId);
    
    return true;
}

bool SwarmCoordinator::rebalanceSwarm() {
    // Redistribute tasks among healthy members
    updateStatistics();
    return true;
}

bool SwarmCoordinator::enterDegradedMode() {
    logSwarmEvent("Degraded Mode", "Insufficient active members");
    return true;
}

// ===========================
// ENERGY MANAGEMENT
// ===========================

bool SwarmCoordinator::optimizeEnergyConsumption() {
    // Reduce velocity and formation density to save energy
    config_.maxVelocity *= 0.8f;
    config_.separationDistance *= 1.2f;
    
    return true;
}

std::vector<int> SwarmCoordinator::identifyLowBatteryMembers() {
    std::vector<int> lowBattery;
    
    for (const auto& pair : members_) {
        if (pair.second.batteryLevel < config_.minBatteryThreshold) {
            lowBattery.push_back(pair.first);
        }
    }
    
    return lowBattery;
}

bool SwarmCoordinator::coordinateChargingRotation() {
    std::vector<int> lowBattery = identifyLowBatteryMembers();
    
    for (int memberId : lowBattery) {
        auto member = members_.find(memberId);
        if (member != members_.end()) {
            Position3D station = findNearestChargingStation(member->second.position);
            assignTask(memberId, "charge", station);
        }
    }
    
    return !lowBattery.empty();
}

Position3D SwarmCoordinator::findNearestChargingStation(const Position3D& position) {
    Position3D nearest = {0, 0, 0, 0};
    float minDistance = 1e9;
    
    for (const auto& pair : members_) {
        if (pair.second.type == SWARM_CHARGING_STATION) {
            float dist = calculateDistance(position, pair.second.position);
            if (dist < minDistance) {
                minDistance = dist;
                nearest = pair.second.position;
            }
        }
    }
    
    return nearest;
}

// ===========================
// STATISTICS & MONITORING
// ===========================

SwarmStats SwarmCoordinator::getStatistics() const {
    return stats_;
}

float SwarmCoordinator::calculateSwarmEfficiency() const {
    if (stats_.activeMembers == 0) {
        return 0.0f;
    }
    
    return (float)stats_.completedTasks / (stats_.completedTasks + stats_.activeMissions + 1);
}

float SwarmCoordinator::calculateCoverageArea() const {
    if (members_.empty()) {
        return 0.0f;
    }
    
    // Calculate convex hull area (simplified)
    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    
    for (const auto& pair : members_) {
        if (pair.second.isActive) {
            minX = std::min(minX, pair.second.position.x);
            maxX = std::max(maxX, pair.second.position.x);
            minY = std::min(minY, pair.second.position.y);
            maxY = std::max(maxY, pair.second.position.y);
        }
    }
    
    return (maxX - minX) * (maxY - minY);
}

float SwarmCoordinator::calculateNetworkStability() const {
    if (stats_.activeMembers + stats_.inactiveMembers == 0) {
        return 0.0f;
    }
    
    return (float)stats_.activeMembers / (stats_.activeMembers + stats_.inactiveMembers);
}

void SwarmCoordinator::logSwarmEvent(const String& event, const String& details) {
    Serial.print("[SwarmCoordinator] ");
    Serial.print(event);
    if (details.length() > 0) {
        Serial.print(" - ");
        Serial.print(details);
    }
    Serial.println();
}

// ===========================
// CONFIGURATION
// ===========================

void SwarmCoordinator::setConfig(const SwarmConfig& config) {
    config_ = config;
}

void SwarmCoordinator::setBehavior(SwarmBehavior behavior) {
    currentBehavior_ = behavior;
    logSwarmEvent("Behavior Changed", String("Type: ") + behavior);
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

std::vector<int> SwarmCoordinator::findNeighbors(int memberId, float radius) {
    std::vector<int> neighbors;
    
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return neighbors;
    }
    
    for (const auto& pair : members_) {
        if (pair.first != memberId && pair.second.isActive) {
            float dist = calculateDistance(it->second.position, pair.second.position);
            if (dist <= radius) {
                neighbors.push_back(pair.first);
            }
        }
    }
    
    return neighbors;
}

float SwarmCoordinator::calculateDistance(const Position3D& a, const Position3D& b) const {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

float SwarmCoordinator::calculateFitness(const Position3D& position) {
    // Example fitness function - maximize coverage while minimizing distance
    return -calculateDistance(position, {0, 0, 0, 0});
}

bool SwarmCoordinator::isWithinBounds(const Position3D& position) const {
    // Define operational bounds
    return (position.x >= -10000 && position.x <= 10000 &&
            position.y >= -10000 && position.y <= 10000 &&
            position.z >= 0 && position.z <= 500);
}

void SwarmCoordinator::updateStatistics() {
    stats_.activeMembers = 0;
    stats_.inactiveMembers = 0;
    stats_.failedMembers = 0;
    float totalBattery = 0;
    
    for (const auto& pair : members_) {
        if (pair.second.isActive && pair.second.isHealthy) {
            stats_.activeMembers++;
            totalBattery += pair.second.batteryLevel;
        } else if (pair.second.isActive && !pair.second.isHealthy) {
            stats_.inactiveMembers++;
        } else {
            stats_.failedMembers++;
        }
    }
    
    stats_.averageBattery = stats_.activeMembers > 0 ? 
                           totalBattery / stats_.activeMembers : 0;
    stats_.coverageArea = calculateCoverageArea();
    stats_.networkStability = calculateNetworkStability();
    stats_.efficiency = calculateSwarmEfficiency();
    
    if (missionActive_) {
        stats_.missionTime = millis() - missionStartTime_;
    }
}

void SwarmCoordinator::broadcastToSwarm(const String& message) {
    // Broadcast message to all active members
    Serial.println("[Swarm Broadcast] " + message);
}

bool SwarmCoordinator::validateMember(const SwarmMember& member) const {
    return member.memberId >= 0 && 
           member.batteryLevel >= 0 && member.batteryLevel <= 100;
}

Velocity3D SwarmCoordinator::limitVelocity(const Velocity3D& velocity) const {
    Velocity3D limited = velocity;
    
    float magnitude = sqrt(velocity.vx*velocity.vx + 
                          velocity.vy*velocity.vy + 
                          velocity.vz*velocity.vz);
    
    if (magnitude > config_.maxVelocity) {
        float scale = config_.maxVelocity / magnitude;
        limited.vx *= scale;
        limited.vy *= scale;
        limited.vz *= scale;
    }
    
    return limited;
}

Position3D SwarmCoordinator::predictPosition(int memberId, float timeSeconds) const {
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return {0, 0, 0, 0};
    }
    
    Position3D predicted;
    predicted.x = it->second.position.x + it->second.velocity.vx * timeSeconds;
    predicted.y = it->second.position.y + it->second.velocity.vy * timeSeconds;
    predicted.z = it->second.position.z + it->second.velocity.vz * timeSeconds;
    predicted.heading = it->second.position.heading;
    
    return predicted;
}

bool SwarmCoordinator::detectCollisionRisk(int memberId) const {
    auto it = members_.find(memberId);
    if (it == members_.end()) {
        return false;
    }
    
    Position3D predicted = predictPosition(memberId, 5.0f); // 5 seconds ahead
    
    for (const auto& pair : members_) {
        if (pair.first != memberId && pair.second.isActive) {
            Position3D otherPredicted = predictPosition(pair.first, 5.0f);
            float distance = calculateDistance(predicted, otherPredicted);
            
            if (distance < config_.separationDistance) {
                return true;
            }
        }
    }
    
    return false;
}

void SwarmCoordinator::applyByzantineFilter(std::map<String, float>& votes) {
    // Simple Byzantine fault tolerance - remove outliers
    // In practice, this would implement more sophisticated BFT algorithms
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

bool initializeSwarmCoordination(const SwarmConfig& config) {
    if (g_swarmCoordinator == nullptr) {
        g_swarmCoordinator = new SwarmCoordinator();
    }
    return g_swarmCoordinator->init(config);
}

void processSwarmCoordination() {
    if (g_swarmCoordinator != nullptr && g_swarmCoordinator->isInitialized()) {
        g_swarmCoordinator->detectFailedMembers();
        g_swarmCoordinator->maintainFormation();
    }
}

SwarmStats getSwarmStatistics() {
    if (g_swarmCoordinator != nullptr) {
        return g_swarmCoordinator->getStatistics();
    }
    return SwarmStats{};
}

bool registerSwarmMember(int memberId, SwarmMemberType type) {
    if (g_swarmCoordinator == nullptr) {
        return false;
    }
    
    SwarmMember member;
    member.memberId = memberId;
    member.type = type;
    member.position = {0, 0, 10, 0};
    member.velocity = {0, 0, 0};
    member.batteryLevel = 100.0f;
    member.signalStrength = 100.0f;
    member.taskLoad = 0.0f;
    
    return g_swarmCoordinator->registerMember(member);
}

void cleanupSwarmCoordination() {
    if (g_swarmCoordinator != nullptr) {
        delete g_swarmCoordinator;
        g_swarmCoordinator = nullptr;
    }
}
