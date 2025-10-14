/**
 * @file swarm_coordinator.h
 * @brief Core swarm robotics coordination system for autonomous wildlife monitoring
 * 
 * Implements comprehensive swarm intelligence algorithms including:
 * - Bio-inspired coordination (flocking, herding, schooling)
 * - Particle Swarm Optimization (PSO)
 * - Ant Colony Optimization (ACO)
 * - Bee Algorithm
 * - Genetic algorithms
 * - Consensus algorithms
 * - Byzantine fault tolerance
 */

#ifndef SWARM_COORDINATOR_H
#define SWARM_COORDINATOR_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "../multi_board/message_protocol.h"
#include "../multi_board/multi_board_system.h"

// Swarm member types
enum SwarmMemberType {
    SWARM_DRONE_AERIAL = 0,
    SWARM_DRONE_AQUATIC = 1,
    SWARM_GROUND_ROBOT = 2,
    SWARM_SENSOR_STATIC = 3,
    SWARM_CAMERA_TRAP = 4,
    SWARM_BASE_STATION = 5,
    SWARM_CHARGING_STATION = 6,
    SWARM_RELAY_NODE = 7
};

// Swarm formation types
enum SwarmFormation {
    FORMATION_RANDOM = 0,
    FORMATION_GRID = 1,
    FORMATION_LINE = 2,
    FORMATION_CIRCLE = 3,
    FORMATION_V_SHAPE = 4,
    FORMATION_CLUSTER = 5,
    FORMATION_ADAPTIVE = 6
};

// Swarm mission types
enum SwarmMission {
    MISSION_SURVEILLANCE = 0,
    MISSION_TRACKING = 1,
    MISSION_SEARCH = 2,
    MISSION_MAPPING = 3,
    MISSION_ANTI_POACHING = 4,
    MISSION_EMERGENCY_RESPONSE = 5,
    MISSION_DATA_COLLECTION = 6,
    MISSION_HABITAT_MONITORING = 7
};

// Swarm behavior modes
enum SwarmBehavior {
    BEHAVIOR_FLOCKING = 0,
    BEHAVIOR_HERDING = 1,
    BEHAVIOR_SCHOOLING = 2,
    BEHAVIOR_FORAGING = 3,
    BEHAVIOR_PATROLLING = 4,
    BEHAVIOR_CONVERGING = 5,
    BEHAVIOR_DISPERSING = 6
};

// Position in 3D space
struct Position3D {
    float x;        // meters
    float y;        // meters
    float z;        // meters (altitude)
    float heading;  // degrees (0-360)
};

// Velocity vector
struct Velocity3D {
    float vx;       // m/s
    float vy;       // m/s
    float vz;       // m/s
};

// Swarm member state
struct SwarmMember {
    int memberId;
    SwarmMemberType type;
    Position3D position;
    Velocity3D velocity;
    float batteryLevel;
    float signalStrength;
    bool isActive;
    bool isHealthy;
    unsigned long lastUpdate;
    std::vector<int> neighbors;
    float taskLoad;
    BoardCapabilities capabilities;
};

// Swarm configuration
struct SwarmConfig {
    int swarmSize;
    SwarmFormation formation;
    SwarmMission mission;
    SwarmBehavior behavior;
    float separationDistance;    // meters
    float cohesionRadius;        // meters
    float alignmentRadius;       // meters
    float maxVelocity;          // m/s
    float maxAcceleration;      // m/s²
    bool enableCollisionAvoidance;
    bool enableFaultTolerance;
    bool enableAdaptiveFormation;
    float minBatteryThreshold;
};

// PSO parameters
struct PSOParams {
    int populationSize;
    int maxIterations;
    float inertiaWeight;
    float cognitiveWeight;
    float socialWeight;
    float minVelocity;
    float maxVelocity;
};

// ACO parameters
struct ACOParams {
    int antCount;
    int maxIterations;
    float pheromoneEvaporation;
    float pheromoneDeposit;
    float alpha;  // pheromone importance
    float beta;   // heuristic importance
};

// Bee Algorithm parameters
struct BeeParams {
    int scoutBees;
    int selectedSites;
    int eliteSites;
    int recruitedBees;
    int maxIterations;
    float searchRadius;
};

// Target tracking information
struct TrackingTarget {
    int targetId;
    Position3D position;
    Velocity3D velocity;
    String species;
    float confidence;
    unsigned long lastSeen;
    std::vector<int> trackingMembers;
};

// Consensus decision
struct ConsensusDecision {
    String decisionType;
    std::map<String, float> votes;
    String winner;
    float confidence;
    unsigned long timestamp;
};

// Swarm statistics
struct SwarmStats {
    int activeMembers;
    int inactiveMembers;
    int failedMembers;
    float averageBattery;
    float coverageArea;        // square meters
    float networkStability;
    int completedTasks;
    int activeMissions;
    unsigned long missionTime;
    float efficiency;
};

class SwarmCoordinator {
public:
    SwarmCoordinator();
    ~SwarmCoordinator();
    
    // ===========================
    // INITIALIZATION
    // ===========================
    bool init(const SwarmConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // ===========================
    // SWARM MANAGEMENT
    // ===========================
    bool registerMember(const SwarmMember& member);
    bool unregisterMember(int memberId);
    bool updateMemberState(int memberId, const Position3D& position, 
                          const Velocity3D& velocity, float battery);
    SwarmMember getMember(int memberId) const;
    std::vector<SwarmMember> getAllMembers() const;
    std::vector<SwarmMember> getActiveMembers() const;
    
    // ===========================
    // MISSION COORDINATION
    // ===========================
    bool startMission(SwarmMission mission, const Position3D& target);
    bool pauseMission();
    bool resumeMission();
    bool abortMission();
    SwarmMission getCurrentMission() const { return currentMission_; }
    
    // ===========================
    // FORMATION CONTROL
    // ===========================
    bool setFormation(SwarmFormation formation);
    bool updateFormation();
    bool maintainFormation();
    Position3D calculateDesiredPosition(int memberId);
    
    // ===========================
    // BIO-INSPIRED ALGORITHMS
    // ===========================
    
    // Flocking behavior (Craig Reynolds)
    Velocity3D calculateFlockingVelocity(int memberId);
    Velocity3D separationForce(int memberId);
    Velocity3D cohesionForce(int memberId);
    Velocity3D alignmentForce(int memberId);
    
    // Particle Swarm Optimization
    bool initPSO(const PSOParams& params);
    bool runPSOIteration();
    Position3D getBestPSOPosition() const;
    
    // Ant Colony Optimization
    bool initACO(const ACOParams& params);
    bool runACOIteration();
    std::vector<Position3D> getBestACOPath() const;
    
    // Bee Algorithm
    bool initBeeAlgorithm(const BeeParams& params);
    bool runBeeIteration();
    std::vector<Position3D> getBestSearchAreas() const;
    
    // Genetic Algorithm
    bool runGeneticOptimization(int generations);
    SwarmConfig getOptimalConfiguration() const;
    
    // ===========================
    // CONSENSUS ALGORITHMS
    // ===========================
    bool initiateConsensus(const String& decisionType, 
                          const std::vector<String>& options);
    bool submitVote(int memberId, const String& option);
    ConsensusDecision getConsensusResult();
    bool hasByzantineFault() const;
    
    // ===========================
    // INTELLIGENT COORDINATION
    // ===========================
    
    // Task allocation
    bool assignTask(int memberId, const String& taskType, const Position3D& location);
    int selectOptimalMember(const Position3D& taskLocation, SwarmMemberType preferredType);
    std::vector<int> selectMemberTeam(const Position3D& target, int teamSize);
    
    // Target tracking
    bool trackTarget(const TrackingTarget& target);
    bool updateTargetPosition(int targetId, const Position3D& position);
    std::vector<TrackingTarget> getActiveTargets() const;
    std::vector<int> assignTrackingTeam(int targetId);
    
    // Path planning
    std::vector<Position3D> planOptimalPath(const Position3D& start, 
                                           const Position3D& goal);
    bool avoidObstacle(int memberId, const Position3D& obstacle);
    
    // ===========================
    // WILDLIFE MONITORING
    // ===========================
    bool startWildlifeSurveillance(const Position3D& area, float radius);
    bool detectWildlifeIntrusion(const Position3D& location);
    bool coordinateAntiPoaching(const Position3D& threatLocation);
    bool trackMigrationPattern(const std::vector<Position3D>& waypoints);
    
    // ===========================
    // EMERGENCY RESPONSE
    // ===========================
    bool triggerEmergencyResponse(const String& emergencyType, 
                                 const Position3D& location);
    bool coordinateSearchAndRescue(const Position3D& searchArea);
    bool deployEmergencyRelay(const Position3D& location);
    
    // ===========================
    // SELF-HEALING & FAULT TOLERANCE
    // ===========================
    bool detectFailedMembers();
    bool replaceFailedMember(int failedId, int replacementId);
    bool rebalanceSwarm();
    bool enterDegradedMode();
    
    // ===========================
    // ENERGY MANAGEMENT
    // ===========================
    bool optimizeEnergyConsumption();
    std::vector<int> identifyLowBatteryMembers();
    bool coordinateChargingRotation();
    Position3D findNearestChargingStation(const Position3D& position);
    
    // ===========================
    // STATISTICS & MONITORING
    // ===========================
    SwarmStats getStatistics() const;
    float calculateSwarmEfficiency() const;
    float calculateCoverageArea() const;
    float calculateNetworkStability() const;
    void logSwarmEvent(const String& event, const String& details = "");
    
    // ===========================
    // CONFIGURATION
    // ===========================
    void setConfig(const SwarmConfig& config);
    SwarmConfig getConfig() const { return config_; }
    void setBehavior(SwarmBehavior behavior);
    
private:
    // Member management
    std::map<int, SwarmMember> members_;
    std::vector<TrackingTarget> targets_;
    SwarmConfig config_;
    SwarmMission currentMission_;
    SwarmBehavior currentBehavior_;
    bool initialized_;
    bool missionActive_;
    
    // Algorithm states
    PSOParams psoParams_;
    ACOParams acoParams_;
    BeeParams beeParams_;
    std::vector<Position3D> psoPositions_;
    std::vector<Velocity3D> psoVelocities_;
    std::vector<float> psoFitness_;
    Position3D globalBestPosition_;
    
    // Consensus state
    ConsensusDecision currentConsensus_;
    std::map<int, String> memberVotes_;
    
    // Statistics
    SwarmStats stats_;
    unsigned long missionStartTime_;
    int taskCounter_;
    
    // Private helper methods
    std::vector<int> findNeighbors(int memberId, float radius);
    float calculateDistance(const Position3D& a, const Position3D& b) const;
    float calculateFitness(const Position3D& position);
    bool isWithinBounds(const Position3D& position) const;
    void updateStatistics();
    void broadcastToSwarm(const String& message);
    bool validateMember(const SwarmMember& member) const;
    Velocity3D limitVelocity(const Velocity3D& velocity) const;
    Position3D predictPosition(int memberId, float timeSeconds) const;
    bool detectCollisionRisk(int memberId) const;
    void applyByzantineFilter(std::map<String, float>& votes);
};

// Global swarm coordinator instance
extern SwarmCoordinator* g_swarmCoordinator;

// Utility functions
bool initializeSwarmCoordination(const SwarmConfig& config);
void processSwarmCoordination();
SwarmStats getSwarmStatistics();
bool registerSwarmMember(int memberId, SwarmMemberType type);
void cleanupSwarmCoordination();

#endif // SWARM_COORDINATOR_H
