/**
 * @file swarm_coordination_example.cpp
 * @brief Comprehensive example demonstrating swarm robotics coordination
 * 
 * This example shows how to use the swarm coordination system for:
 * - Multi-drone surveillance
 * - Bio-inspired algorithms
 * - Target tracking
 * - Anti-poaching operations
 * - Emergency response
 */

#include <Arduino.h>
#include "../firmware/src/swarm/swarm_coordinator.h"
#include "../firmware/src/multi_board/multi_board_system.h"

// Example configuration
const int SWARM_SIZE = 10;
const int SIMULATION_DURATION = 300000; // 5 minutes

// Global instances
SwarmCoordinator* swarm = nullptr;

// Simulation state
unsigned long lastUpdate = 0;
unsigned long simulationStart = 0;
int currentScenario = 0;

// Forward declarations
void setupSwarmConfiguration();
void registerSwarmMembers();
void simulateWildlifeSurveillance();
void simulateAntiPoaching();
void simulateSearchAndRescue();
void simulateTargetTracking();
void demonstrateBioInspiredAlgorithms();
void demonstrateFormationControl();
void demonstrateConsensusDecision();
void demonstrateEnergyManagement();
void updateMemberStates();
void printSwarmStatus();
void runScenario(int scenario);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n==============================================");
    Serial.println("   Swarm Robotics Coordination System Demo");
    Serial.println("==============================================\n");
    
    // Initialize swarm coordinator
    setupSwarmConfiguration();
    
    // Register swarm members
    registerSwarmMembers();
    
    // Display initial status
    printSwarmStatus();
    
    simulationStart = millis();
    
    Serial.println("\nStarting swarm coordination demonstration...\n");
}

void loop() {
    unsigned long now = millis();
    
    // Process swarm coordination every 100ms
    if (now - lastUpdate > 100) {
        lastUpdate = now;
        
        // Update member states
        updateMemberStates();
        
        // Process swarm coordination
        processSwarmCoordination();
        
        // Run different scenarios every 30 seconds
        if ((now - simulationStart) % 30000 < 100) {
            runScenario(currentScenario);
            currentScenario = (currentScenario + 1) % 8;
        }
        
        // Print status every 5 seconds
        if ((now - simulationStart) % 5000 < 100) {
            printSwarmStatus();
        }
    }
    
    // End simulation after duration
    if (now - simulationStart > SIMULATION_DURATION) {
        Serial.println("\n=== Simulation Complete ===");
        Serial.println("Swarm coordination demonstration finished.");
        
        // Final statistics
        printSwarmStatus();
        
        // Cleanup
        if (swarm) {
            swarm->cleanup();
            delete swarm;
            swarm = nullptr;
        }
        
        while (true) {
            delay(1000);
        }
    }
    
    delay(10);
}

// ===========================
// SETUP FUNCTIONS
// ===========================

void setupSwarmConfiguration() {
    Serial.println("Initializing swarm configuration...");
    
    SwarmConfig config;
    config.swarmSize = SWARM_SIZE;
    config.formation = FORMATION_GRID;
    config.mission = MISSION_SURVEILLANCE;
    config.behavior = BEHAVIOR_FLOCKING;
    config.separationDistance = 15.0f;    // 15 meters
    config.cohesionRadius = 50.0f;        // 50 meters
    config.alignmentRadius = 30.0f;       // 30 meters
    config.maxVelocity = 5.0f;            // 5 m/s
    config.maxAcceleration = 2.0f;        // 2 m/s²
    config.enableCollisionAvoidance = true;
    config.enableFaultTolerance = true;
    config.enableAdaptiveFormation = true;
    config.minBatteryThreshold = 20.0f;   // 20%
    
    swarm = new SwarmCoordinator();
    
    if (swarm->init(config)) {
        Serial.println("✓ Swarm coordinator initialized successfully");
    } else {
        Serial.println("✗ Failed to initialize swarm coordinator");
    }
}

void registerSwarmMembers() {
    Serial.println("\nRegistering swarm members...");
    
    // Register 5 aerial drones
    for (int i = 1; i <= 5; i++) {
        SwarmMember member;
        member.memberId = i;
        member.type = SWARM_DRONE_AERIAL;
        member.position = {(i-3) * 20.0f, 0.0f, 15.0f, 0.0f};
        member.velocity = {0.0f, 0.0f, 0.0f};
        member.batteryLevel = 85.0f + random(-10, 10);
        member.signalStrength = 90.0f + random(-5, 5);
        member.isActive = true;
        member.isHealthy = true;
        member.taskLoad = 0.0f;
        
        if (swarm->registerMember(member)) {
            Serial.printf("  ✓ Registered Aerial Drone #%d\n", i);
        }
    }
    
    // Register 2 ground robots
    for (int i = 6; i <= 7; i++) {
        SwarmMember member;
        member.memberId = i;
        member.type = SWARM_GROUND_ROBOT;
        member.position = {(i-6) * 30.0f, 50.0f, 0.0f, 0.0f};
        member.velocity = {0.0f, 0.0f, 0.0f};
        member.batteryLevel = 75.0f + random(-10, 10);
        member.signalStrength = 85.0f + random(-5, 5);
        member.isActive = true;
        member.isHealthy = true;
        member.taskLoad = 0.0f;
        
        if (swarm->registerMember(member)) {
            Serial.printf("  ✓ Registered Ground Robot #%d\n", i);
        }
    }
    
    // Register 2 camera traps
    for (int i = 8; i <= 9; i++) {
        SwarmMember member;
        member.memberId = i;
        member.type = SWARM_CAMERA_TRAP;
        member.position = {(i-8) * 100.0f, 100.0f, 2.0f, 0.0f};
        member.velocity = {0.0f, 0.0f, 0.0f};
        member.batteryLevel = 95.0f;
        member.signalStrength = 80.0f;
        member.isActive = true;
        member.isHealthy = true;
        member.taskLoad = 0.0f;
        
        if (swarm->registerMember(member)) {
            Serial.printf("  ✓ Registered Camera Trap #%d\n", i);
        }
    }
    
    // Register 1 charging station
    SwarmMember station;
    station.memberId = 10;
    station.type = SWARM_CHARGING_STATION;
    station.position = {0.0f, 0.0f, 0.0f, 0.0f};
    station.velocity = {0.0f, 0.0f, 0.0f};
    station.batteryLevel = 100.0f;
    station.signalStrength = 100.0f;
    station.isActive = true;
    station.isHealthy = true;
    station.taskLoad = 0.0f;
    
    if (swarm->registerMember(station)) {
        Serial.println("  ✓ Registered Charging Station");
    }
    
    Serial.printf("\n✓ Successfully registered %d swarm members\n", SWARM_SIZE);
}

// ===========================
// SCENARIO FUNCTIONS
// ===========================

void runScenario(int scenario) {
    Serial.println("\n==============================================");
    
    switch (scenario) {
        case 0:
            simulateWildlifeSurveillance();
            break;
        case 1:
            simulateAntiPoaching();
            break;
        case 2:
            simulateSearchAndRescue();
            break;
        case 3:
            simulateTargetTracking();
            break;
        case 4:
            demonstrateBioInspiredAlgorithms();
            break;
        case 5:
            demonstrateFormationControl();
            break;
        case 6:
            demonstrateConsensusDecision();
            break;
        case 7:
            demonstrateEnergyManagement();
            break;
    }
    
    Serial.println("==============================================\n");
}

void simulateWildlifeSurveillance() {
    Serial.println("SCENARIO: Wildlife Surveillance");
    Serial.println("----------------------------------------------");
    
    Position3D area = {500.0f, 500.0f, 0.0f, 0.0f};
    float radius = 200.0f;
    
    Serial.printf("Starting surveillance of area (%.1f, %.1f) with radius %.1f m\n",
                  area.x, area.y, radius);
    
    if (swarm->startWildlifeSurveillance(area, radius)) {
        Serial.println("✓ Wildlife surveillance mission initiated");
        
        // Set optimal formation for surveillance
        swarm->setFormation(FORMATION_GRID);
        Serial.println("✓ Grid formation established for systematic coverage");
        
        SwarmStats stats = swarm->getStatistics();
        Serial.printf("Coverage area: %.2f square meters\n", stats.coverageArea);
    }
}

void simulateAntiPoaching() {
    Serial.println("SCENARIO: Anti-Poaching Response");
    Serial.println("----------------------------------------------");
    
    Position3D threat = {1200.0f, 800.0f, 0.0f, 0.0f};
    
    Serial.printf("⚠ ALERT: Suspicious activity detected at (%.1f, %.1f)\n",
                  threat.x, threat.y);
    
    if (swarm->coordinateAntiPoaching(threat)) {
        Serial.println("✓ Anti-poaching response coordinated");
        
        // Select response team
        std::vector<int> team = swarm->selectMemberTeam(threat, 4);
        Serial.printf("✓ Response team of %d members deployed:\n", team.size());
        for (int memberId : team) {
            SwarmMember member = swarm->getMember(memberId);
            Serial.printf("  - Member %d (Battery: %.1f%%)\n", 
                         memberId, member.batteryLevel);
        }
        
        // Switch to converging behavior
        swarm->setBehavior(BEHAVIOR_CONVERGING);
        Serial.println("✓ Swarm behavior: CONVERGING on threat");
    }
}

void simulateSearchAndRescue() {
    Serial.println("SCENARIO: Search and Rescue");
    Serial.println("----------------------------------------------");
    
    Position3D searchArea = {-500.0f, 300.0f, 0.0f, 0.0f};
    
    Serial.printf("🆘 EMERGENCY: Search and rescue at (%.1f, %.1f)\n",
                  searchArea.x, searchArea.y);
    
    if (swarm->coordinateSearchAndRescue(searchArea)) {
        Serial.println("✓ Search and rescue mission initiated");
        
        // Deploy emergency relay
        Position3D relayPos = {-400.0f, 250.0f, 20.0f, 0.0f};
        if (swarm->deployEmergencyRelay(relayPos)) {
            Serial.println("✓ Emergency communication relay deployed");
        }
        
        // Optimal search pattern
        swarm->setFormation(FORMATION_GRID);
        Serial.println("✓ Grid search pattern established");
    }
}

void simulateTargetTracking() {
    Serial.println("SCENARIO: Wildlife Target Tracking");
    Serial.println("----------------------------------------------");
    
    TrackingTarget target;
    target.targetId = 1;
    target.position = {200.0f, 150.0f, 0.0f, 0.0f};
    target.velocity = {0.8f, 0.5f, 0.0f};
    target.species = "African Elephant";
    target.confidence = 0.95f;
    target.lastSeen = millis();
    
    Serial.printf("🐘 Target detected: %s (Confidence: %.0f%%)\n",
                  target.species.c_str(), target.confidence * 100);
    Serial.printf("   Position: (%.1f, %.1f)\n", 
                  target.position.x, target.position.y);
    
    if (swarm->trackTarget(target)) {
        Serial.println("✓ Target tracking initiated");
        
        // Assign tracking team
        std::vector<int> team = swarm->assignTrackingTeam(target.targetId);
        Serial.printf("✓ Tracking team assigned: %d members\n", team.size());
        
        // Switch to herding behavior
        swarm->setBehavior(BEHAVIOR_HERDING);
        Serial.println("✓ Swarm behavior: HERDING for tracking");
        
        // Show active targets
        std::vector<TrackingTarget> active = swarm->getActiveTargets();
        Serial.printf("Active targets being tracked: %d\n", active.size());
    }
}

void demonstrateBioInspiredAlgorithms() {
    Serial.println("SCENARIO: Bio-Inspired Algorithms");
    Serial.println("----------------------------------------------");
    
    // Demonstrate PSO
    Serial.println("\n[Particle Swarm Optimization]");
    PSOParams psoParams;
    psoParams.populationSize = 15;
    psoParams.maxIterations = 50;
    psoParams.inertiaWeight = 0.7f;
    psoParams.cognitiveWeight = 1.5f;
    psoParams.socialWeight = 1.5f;
    psoParams.minVelocity = -5.0f;
    psoParams.maxVelocity = 5.0f;
    
    if (swarm->initPSO(psoParams)) {
        Serial.println("✓ PSO initialized");
        
        // Run a few iterations
        for (int i = 0; i < 10; i++) {
            swarm->runPSOIteration();
        }
        
        Position3D best = swarm->getBestPSOPosition();
        Serial.printf("✓ Best position found: (%.2f, %.2f, %.2f)\n",
                     best.x, best.y, best.z);
    }
    
    // Demonstrate flocking
    Serial.println("\n[Flocking Behavior]");
    swarm->setBehavior(BEHAVIOR_FLOCKING);
    Serial.println("✓ Flocking behavior activated");
    
    // Calculate flocking forces for first member
    Velocity3D flocking = swarm->calculateFlockingVelocity(1);
    Serial.printf("✓ Flocking velocity: (%.2f, %.2f, %.2f) m/s\n",
                 flocking.vx, flocking.vy, flocking.vz);
    
    // Show individual forces
    Velocity3D sep = swarm->separationForce(1);
    Velocity3D coh = swarm->cohesionForce(1);
    Velocity3D ali = swarm->alignmentForce(1);
    
    Serial.println("  Forces breakdown:");
    Serial.printf("    - Separation: (%.2f, %.2f, %.2f)\n", sep.vx, sep.vy, sep.vz);
    Serial.printf("    - Cohesion:   (%.2f, %.2f, %.2f)\n", coh.vx, coh.vy, coh.vz);
    Serial.printf("    - Alignment:  (%.2f, %.2f, %.2f)\n", ali.vx, ali.vy, ali.vz);
}

void demonstrateFormationControl() {
    Serial.println("SCENARIO: Formation Control");
    Serial.println("----------------------------------------------");
    
    SwarmFormation formations[] = {
        FORMATION_LINE,
        FORMATION_CIRCLE,
        FORMATION_V_SHAPE,
        FORMATION_GRID
    };
    
    const char* formationNames[] = {
        "LINE",
        "CIRCLE",
        "V-SHAPE",
        "GRID"
    };
    
    // Cycle through formations
    static int formationIndex = 0;
    SwarmFormation formation = formations[formationIndex];
    
    Serial.printf("Setting formation: %s\n", formationNames[formationIndex]);
    
    if (swarm->setFormation(formation)) {
        Serial.println("✓ Formation established");
        
        // Show desired positions for first 3 members
        Serial.println("Desired positions:");
        for (int i = 1; i <= 3; i++) {
            Position3D desired = swarm->calculateDesiredPosition(i);
            Serial.printf("  Member %d: (%.1f, %.1f, %.1f)\n",
                         i, desired.x, desired.y, desired.z);
        }
    }
    
    formationIndex = (formationIndex + 1) % 4;
}

void demonstrateConsensusDecision() {
    Serial.println("SCENARIO: Consensus Decision Making");
    Serial.println("----------------------------------------------");
    
    std::vector<String> options = {
        "continue_patrol",
        "return_to_base",
        "investigate_area"
    };
    
    Serial.println("Initiating consensus vote: Next Action");
    
    if (swarm->initiateConsensus("next_action", options)) {
        Serial.println("✓ Consensus voting initiated");
        Serial.println("Options:");
        for (const auto& opt : options) {
            Serial.printf("  - %s\n", opt.c_str());
        }
        
        // Simulate votes from active members
        std::vector<SwarmMember> members = swarm->getActiveMembers();
        Serial.println("\nVoting:");
        
        for (const auto& member : members) {
            if (member.type == SWARM_DRONE_AERIAL || 
                member.type == SWARM_GROUND_ROBOT) {
                String vote = options[random(0, options.size())];
                swarm->submitVote(member.memberId, vote);
                Serial.printf("  Member %d voted: %s\n", 
                             member.memberId, vote.c_str());
            }
        }
        
        // Get result
        ConsensusDecision result = swarm->getConsensusResult();
        Serial.println("\n✓ Consensus reached:");
        Serial.printf("  Decision: %s\n", result.winner.c_str());
        Serial.printf("  Confidence: %.1f%%\n", result.confidence * 100);
    }
}

void demonstrateEnergyManagement() {
    Serial.println("SCENARIO: Energy Management");
    Serial.println("----------------------------------------------");
    
    // Check for low battery members
    std::vector<int> lowBattery = swarm->identifyLowBatteryMembers();
    
    Serial.printf("Members with low battery: %d\n", lowBattery.size());
    
    if (!lowBattery.empty()) {
        Serial.println("Low battery members:");
        for (int memberId : lowBattery) {
            SwarmMember member = swarm->getMember(memberId);
            Serial.printf("  Member %d: %.1f%%\n", 
                         memberId, member.batteryLevel);
        }
        
        // Coordinate charging
        if (swarm->coordinateChargingRotation()) {
            Serial.println("✓ Charging rotation coordinated");
        }
    } else {
        Serial.println("✓ All members have adequate battery levels");
    }
    
    // Optimize energy consumption
    Serial.println("\nOptimizing swarm energy consumption...");
    if (swarm->optimizeEnergyConsumption()) {
        Serial.println("✓ Energy optimization applied");
        
        SwarmConfig config = swarm->getConfig();
        Serial.printf("  Adjusted max velocity: %.2f m/s\n", config.maxVelocity);
        Serial.printf("  Adjusted separation: %.2f m\n", config.separationDistance);
    }
    
    SwarmStats stats = swarm->getStatistics();
    Serial.printf("\nAverage battery level: %.1f%%\n", stats.averageBattery);
}

// ===========================
// UPDATE & STATUS FUNCTIONS
// ===========================

void updateMemberStates() {
    // Simulate member state updates
    std::vector<SwarmMember> members = swarm->getAllMembers();
    
    for (auto& member : members) {
        if (member.type == SWARM_CHARGING_STATION || 
            member.type == SWARM_CAMERA_TRAP) {
            continue; // Static members
        }
        
        // Simulate movement
        Position3D newPos = member.position;
        Velocity3D newVel = member.velocity;
        
        // Add some random movement
        newPos.x += random(-5, 5) * 0.1f;
        newPos.y += random(-5, 5) * 0.1f;
        
        // Simulate battery drain
        float batteryDrain = 0.01f; // 0.01% per update
        float newBattery = member.batteryLevel - batteryDrain;
        
        swarm->updateMemberState(member.memberId, newPos, newVel, newBattery);
    }
}

void printSwarmStatus() {
    SwarmStats stats = swarm->getStatistics();
    
    Serial.println("\n--- Swarm Status ---");
    Serial.printf("Active Members:    %d/%d\n", 
                  stats.activeMembers, 
                  stats.activeMembers + stats.inactiveMembers + stats.failedMembers);
    Serial.printf("Coverage Area:     %.2f sq m\n", stats.coverageArea);
    Serial.printf("Network Stability: %.2f\n", stats.networkStability);
    Serial.printf("Average Battery:   %.1f%%\n", stats.averageBattery);
    Serial.printf("Efficiency:        %.2f\n", stats.efficiency);
    Serial.printf("Completed Tasks:   %d\n", stats.completedTasks);
    Serial.printf("Mission Time:      %lu s\n", stats.missionTime / 1000);
    Serial.println("-------------------");
}
