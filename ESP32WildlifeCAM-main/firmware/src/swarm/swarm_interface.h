/**
 * @file swarm_interface.h
 * @brief Human-swarm interface for monitoring and control
 * 
 * Provides:
 * - Intuitive control interfaces
 * - Real-time monitoring
 * - Mission planning tools
 * - Emergency override systems
 * - Visualization and analytics
 * - Voice command integration
 * - Mobile application support
 */

#ifndef SWARM_INTERFACE_H
#define SWARM_INTERFACE_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "swarm_coordinator.h"

// Interface mode
enum InterfaceMode {
    MODE_MONITORING = 0,
    MODE_CONTROL = 1,
    MODE_PLANNING = 2,
    MODE_ANALYSIS = 3,
    MODE_EMERGENCY = 4
};

// Command type
enum CommandType {
    CMD_START_MISSION = 0,
    CMD_PAUSE_MISSION = 1,
    CMD_ABORT_MISSION = 2,
    CMD_CHANGE_FORMATION = 3,
    CMD_ASSIGN_TASK = 4,
    CMD_RETURN_HOME = 5,
    CMD_EMERGENCY_STOP = 6,
    CMD_RECHARGE = 7,
    CMD_UPDATE_CONFIG = 8
};

// Alert type
enum AlertType {
    ALERT_INFO = 0,
    ALERT_WARNING = 1,
    ALERT_ERROR = 2,
    ALERT_CRITICAL = 3,
    ALERT_EMERGENCY = 4
};

// Visualization type
enum VisualizationType {
    VIZ_MAP_2D = 0,
    VIZ_MAP_3D = 1,
    VIZ_HEATMAP = 2,
    VIZ_TRAJECTORY = 3,
    VIZ_NETWORK = 4,
    VIZ_STATISTICS = 5
};

// User role
enum UserRole {
    ROLE_OPERATOR = 0,
    ROLE_RESEARCHER = 1,
    ROLE_ADMINISTRATOR = 2,
    ROLE_EMERGENCY_RESPONDER = 3,
    ROLE_VIEWER = 4
};

// Command result
struct CommandResult {
    bool success;
    String message;
    int affectedMembers;
    unsigned long executionTime;
    String errorDetails;
};

// Mission plan
struct MissionPlan {
    int planId;
    String name;
    SwarmMission missionType;
    Position3D targetArea;
    float radius;
    std::vector<Waypoint> waypoints;
    SwarmFormation formation;
    int requiredMembers;
    unsigned long estimatedDuration;
    TaskPriority priority;
    String description;
};

// Alert message
struct AlertMessage {
    int alertId;
    AlertType type;
    String title;
    String message;
    Position3D location;
    int memberId;
    unsigned long timestamp;
    bool acknowledged;
    bool resolved;
};

// Visualization data
struct VisualizationData {
    VisualizationType type;
    std::vector<Position3D> positions;
    std::vector<int> memberIds;
    std::map<String, float> values;
    String title;
    String metadata;
    unsigned long timestamp;
};

// User session
struct UserSession {
    int sessionId;
    String username;
    UserRole role;
    bool isActive;
    unsigned long loginTime;
    unsigned long lastActivity;
    std::vector<String> permissions;
};

// Voice command
struct VoiceCommand {
    int commandId;
    String transcript;
    CommandType type;
    float confidence;
    std::map<String, String> parameters;
    unsigned long timestamp;
};

// Interface statistics
struct InterfaceStats {
    int totalCommands;
    int successfulCommands;
    int failedCommands;
    int activeAlerts;
    int activeUsers;
    float averageResponseTime;
    unsigned long uptime;
};

class SwarmInterface {
public:
    SwarmInterface();
    ~SwarmInterface();
    
    // ===========================
    // INITIALIZATION
    // ===========================
    bool init();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // ===========================
    // USER MANAGEMENT
    // ===========================
    int login(const String& username, const String& password, UserRole role);
    bool logout(int sessionId);
    bool hasPermission(int sessionId, const String& permission) const;
    UserSession getSession(int sessionId) const;
    std::vector<UserSession> getActiveSessions() const;
    
    // ===========================
    // COMMAND INTERFACE
    // ===========================
    CommandResult sendCommand(CommandType type, const std::map<String, String>& params);
    CommandResult startMission(const MissionPlan& plan);
    CommandResult pauseMission();
    CommandResult abortMission();
    CommandResult changeFormation(SwarmFormation formation);
    CommandResult emergencyStop();
    CommandResult returnAllToBase();
    
    // ===========================
    // MONITORING
    // ===========================
    SwarmStats getCurrentStatus() const;
    std::vector<SwarmMember> getSwarmMembers() const;
    Position3D getMemberPosition(int memberId) const;
    std::vector<TrackingTarget> getActiveTargets() const;
    std::vector<AlertMessage> getActiveAlerts() const;
    
    // ===========================
    // MISSION PLANNING
    // ===========================
    int createMissionPlan(const MissionPlan& plan);
    bool updateMissionPlan(int planId, const MissionPlan& plan);
    bool deleteMissionPlan(int planId);
    MissionPlan getMissionPlan(int planId) const;
    std::vector<MissionPlan> getAllMissionPlans() const;
    bool validateMissionPlan(const MissionPlan& plan) const;
    
    // ===========================
    // ALERT MANAGEMENT
    // ===========================
    int createAlert(AlertType type, const String& title, const String& message);
    bool acknowledgeAlert(int alertId, int sessionId);
    bool resolveAlert(int alertId);
    bool clearAlerts(AlertType type);
    AlertMessage getAlert(int alertId) const;
    std::vector<AlertMessage> getAlertsByType(AlertType type) const;
    
    // ===========================
    // VISUALIZATION
    // ===========================
    VisualizationData generateVisualization(VisualizationType type);
    VisualizationData generateMapView();
    VisualizationData generateHeatmap(const String& metric);
    VisualizationData generateTrajectoryView(int memberId);
    VisualizationData generateNetworkTopology();
    VisualizationData generateStatistics();
    
    // ===========================
    // VOICE COMMANDS
    // ===========================
    bool enableVoiceControl(bool enable);
    VoiceCommand processVoiceInput(const String& transcript);
    CommandResult executeVoiceCommand(const VoiceCommand& command);
    std::vector<String> getSupportedVoiceCommands() const;
    
    // ===========================
    // AUGMENTED REALITY
    // ===========================
    bool enableARMode(bool enable);
    VisualizationData getAROverlay();
    bool updateARMarker(int memberId, const Position3D& position);
    
    // ===========================
    // MOBILE APPLICATION
    // ===========================
    String getMobileStatus();  // JSON format
    bool handleMobileCommand(const String& command);
    bool streamVideoFeed(int memberId, const String& endpoint);
    bool sendPushNotification(const String& message, AlertType type);
    
    // ===========================
    // EMERGENCY OVERRIDE
    // ===========================
    bool activateEmergencyMode(int sessionId);
    bool deactivateEmergencyMode(int sessionId);
    bool isEmergencyMode() const { return emergencyMode_; }
    CommandResult emergencyOverride(const String& action);
    bool forceLanding(int memberId);
    bool forceReturnToBase();
    
    // ===========================
    // CONFIGURATION
    // ===========================
    bool updateSwarmConfig(const SwarmConfig& config);
    SwarmConfig getSwarmConfig() const;
    bool saveConfiguration(const String& name);
    bool loadConfiguration(const String& name);
    std::vector<String> getSavedConfigurations() const;
    
    // ===========================
    // DATA EXPORT
    // ===========================
    bool exportData(const String& format, const String& filename);
    bool exportMissionLog(unsigned long startTime, unsigned long endTime);
    bool exportDetections(const String& filename);
    bool exportTelemetry(int memberId, const String& filename);
    
    // ===========================
    // ANALYTICS
    // ===========================
    std::map<String, float> calculateMissionEfficiency() const;
    std::map<String, int> getDetectionStatistics() const;
    std::map<String, float> getMemberPerformance(int memberId) const;
    std::vector<Position3D> predictSwarmMovement(float timeSeconds) const;
    
    // ===========================
    // TRAINING & SIMULATION
    // ===========================
    bool enableSimulationMode(bool enable);
    bool runSimulation(const MissionPlan& plan);
    bool loadTrainingScenario(const String& scenarioName);
    float evaluateOperatorPerformance(int sessionId) const;
    
    // ===========================
    // NOTIFICATIONS
    // ===========================
    bool subscribeToAlerts(int sessionId, AlertType type);
    bool unsubscribeFromAlerts(int sessionId, AlertType type);
    bool sendNotification(int sessionId, const String& message);
    std::vector<AlertMessage> getNotifications(int sessionId) const;
    
    // ===========================
    // STATISTICS & MONITORING
    // ===========================
    InterfaceStats getStatistics() const;
    void logInterfaceEvent(const String& event, const String& details = "");
    
private:
    // State
    bool initialized_;
    bool emergencyMode_;
    bool voiceControlEnabled_;
    bool arModeEnabled_;
    bool simulationMode_;
    InterfaceMode currentMode_;
    
    // User sessions
    std::map<int, UserSession> sessions_;
    int nextSessionId_;
    
    // Mission plans
    std::map<int, MissionPlan> missionPlans_;
    int nextPlanId_;
    
    // Alerts
    std::vector<AlertMessage> alerts_;
    int nextAlertId_;
    
    // Voice commands
    std::vector<VoiceCommand> voiceHistory_;
    
    // Subscriptions
    std::map<int, std::vector<AlertType>> alertSubscriptions_;
    
    // Statistics
    InterfaceStats stats_;
    unsigned long startTime_;
    
    // Private helper methods
    bool validateSession(int sessionId) const;
    bool hasEmergencyPermission(int sessionId) const;
    String formatJSON(const std::map<String, String>& data) const;
    bool parseVoiceCommand(const String& transcript, VoiceCommand& command);
    void updateActivityTime(int sessionId);
    void broadcastAlert(const AlertMessage& alert);
};

// Global interface instance
extern SwarmInterface* g_swarmInterface;

// Utility functions
bool initializeSwarmInterface();
void processSwarmInterface();
InterfaceStats getInterfaceStatistics();
void cleanupSwarmInterface();

#endif // SWARM_INTERFACE_H
