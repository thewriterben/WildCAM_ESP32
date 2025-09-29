/**
 * @file federated_power_manager.h
 * @brief Power Management Integration for Federated Learning
 * 
 * Integrates federated learning with the existing power management system
 * to optimize training schedules based on solar power availability and
 * battery levels.
 */

#ifndef FEDERATED_POWER_MANAGER_H
#define FEDERATED_POWER_MANAGER_H

#include "../power/power_manager.h"
#include "federated_learning/federated_learning_system.h"
#include <Arduino.h>

/**
 * Power Management Modes for Federated Learning
 */
enum class FLPowerMode {
    HIGH_PERFORMANCE,      // Full federated learning capabilities
    BALANCED,             // Balanced performance and power
    POWER_SAVING,         // Limited federated learning
    EMERGENCY,            // Only critical operations
    SOLAR_OPTIMIZED       // Optimized for solar charging cycles
};

/**
 * Training Schedule Based on Power Availability
 */
struct TrainingSchedule {
    uint32_t startTime;        // Start time (timestamp)
    uint32_t endTime;          // End time (timestamp)
    uint32_t duration;         // Duration in milliseconds
    float minBatteryLevel;     // Minimum battery level required
    float maxPowerConsumption; // Maximum power consumption allowed
    bool solarOptimized;       // Whether to wait for solar charging
    TrainingMode mode;         // Training mode for this schedule
    
    TrainingSchedule() : startTime(0), endTime(0), duration(0),
                        minBatteryLevel(0.3f), maxPowerConsumption(500.0f),
                        solarOptimized(true), mode(TrainingMode::INCREMENTAL) {}
};

/**
 * Power Optimization Configuration
 */
struct FLPowerConfig {
    FLPowerMode defaultMode;
    
    // Battery thresholds
    float criticalBatteryLevel;     // Stop all FL operations
    float lowBatteryLevel;          // Reduce FL operations
    float optimalBatteryLevel;      // Full FL operations
    
    // Power consumption limits
    float maxTrainingPower;         // Max power for training (mW)
    float maxCommunicationPower;    // Max power for communication (mW)
    float maxIdlePower;            // Max power when idle (mW)
    
    // Solar charging integration
    bool enableSolarOptimization;
    uint32_t solarPeakStartHour;   // Hour when solar charging typically starts
    uint32_t solarPeakEndHour;     // Hour when solar charging typically ends
    float minSolarPower;           // Minimum solar power for FL operations
    
    // Scheduling constraints
    uint32_t maxTrainingDuration;  // Maximum training duration per session
    uint32_t minIdleTime;          // Minimum idle time between training
    bool enableNightMode;          // Allow FL operations at night
    
    FLPowerConfig() : defaultMode(FLPowerMode::BALANCED),
                     criticalBatteryLevel(0.15f), lowBatteryLevel(0.3f),
                     optimalBatteryLevel(0.7f), maxTrainingPower(800.0f),
                     maxCommunicationPower(200.0f), maxIdlePower(50.0f),
                     enableSolarOptimization(true), solarPeakStartHour(8),
                     solarPeakEndHour(16), minSolarPower(100.0f),
                     maxTrainingDuration(300000), // 5 minutes
                     minIdleTime(600000), // 10 minutes
                     enableNightMode(false) {}
};

/**
 * Power Monitoring Data
 */
struct FLPowerMetrics {
    float currentBatteryLevel;
    float currentPowerConsumption;
    float currentSolarPower;
    float averageTrainingPower;
    float averageCommunicationPower;
    uint32_t totalTrainingTime;
    uint32_t totalEnergyConsumed;
    float powerEfficiency;         // FL operations per mWh
    uint32_t batteryOptimizations; // Number of power optimizations applied
    
    FLPowerMetrics() : currentBatteryLevel(0.0f), currentPowerConsumption(0.0f),
                      currentSolarPower(0.0f), averageTrainingPower(0.0f),
                      averageCommunicationPower(0.0f), totalTrainingTime(0),
                      totalEnergyConsumed(0), powerEfficiency(0.0f),
                      batteryOptimizations(0) {}
};

/**
 * Federated Learning Power Manager
 * 
 * Manages power consumption for federated learning operations:
 * - Battery level monitoring and optimization
 * - Solar charging integration
 * - Training schedule optimization
 * - Power-aware model selection
 * - Emergency power management
 */
class FederatedPowerManager {
public:
    FederatedPowerManager();
    ~FederatedPowerManager();
    
    // Initialization
    bool init(const FLPowerConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Power manager integration
    bool attachToPowerManager(PowerManager* powerManager);
    void detachFromPowerManager();
    bool attachToFederatedSystem(FederatedLearningSystem* federatedSystem);
    
    // Configuration
    void setPowerConfig(const FLPowerConfig& config);
    FLPowerConfig getPowerConfig() const { return config_; }
    void setPowerMode(FLPowerMode mode);
    FLPowerMode getCurrentPowerMode() const { return currentMode_; }
    
    // Power monitoring and control
    bool isPowerAvailableForTraining() const;
    bool isPowerAvailableForCommunication() const;
    float getAvailablePowerBudget() const;
    bool requestPowerForOperation(float powerRequired, uint32_t durationMs);
    
    // Training schedule optimization
    bool scheduleTraining(const TrainingSchedule& schedule);
    std::vector<TrainingSchedule> getOptimalTrainingWindows(uint32_t lookaheadHours = 24);
    TrainingSchedule getNextTrainingWindow() const;
    bool cancelScheduledTraining(uint32_t startTime);
    
    // Solar charging integration
    bool isSolarChargingActive() const;
    float getCurrentSolarPower() const;
    bool waitForSolarCharging(uint32_t maxWaitTimeMs);
    std::vector<uint32_t> predictSolarChargingWindows(uint32_t lookaheadHours = 24);
    
    // Power optimization strategies
    bool optimizeForBatteryLife();
    bool optimizeForPerformance();
    bool optimizeForSolarCharging();
    ModelType selectPowerOptimalModel(WildlifeModelType baseType) const;
    uint32_t calculateOptimalBatchSize(float availablePower) const;
    
    // Emergency power management
    bool handleCriticalBatteryLevel();
    bool handlePowerEmergency();
    void enableEmergencyMode(bool enable);
    bool isEmergencyModeActive() const { return emergencyMode_; }
    
    // Adaptive power management
    bool enableAdaptivePowerManagement(bool enable);
    void updatePowerProfile();
    bool learnFromPowerPatterns();
    float predictPowerConsumption(TrainingMode mode, uint32_t duration) const;
    
    // Power metrics and monitoring
    FLPowerMetrics getPowerMetrics() const { return metrics_; }
    void resetPowerMetrics();
    float calculatePowerEfficiency() const;
    String generatePowerReport() const;
    
    // Event callbacks
    typedef std::function<void(FLPowerMode, FLPowerMode)> PowerModeChangeCallback;
    typedef std::function<void(float)> BatteryLevelCallback;
    typedef std::function<void(bool)> SolarChargingCallback;
    typedef std::function<void()> EmergencyCallback;
    
    void setPowerModeChangeCallback(PowerModeChangeCallback callback) { 
        powerModeChangeCallback_ = callback; 
    }
    void setBatteryLevelCallback(BatteryLevelCallback callback) { 
        batteryLevelCallback_ = callback; 
    }
    void setSolarChargingCallback(SolarChargingCallback callback) { 
        solarChargingCallback_ = callback; 
    }
    void setEmergencyCallback(EmergencyCallback callback) { 
        emergencyCallback_ = callback; 
    }
    
    // Utility methods
    bool validatePowerConfiguration() const;
    uint32_t estimateTrainingDuration(TrainingMode mode, uint32_t samples) const;
    float estimateEnergyConsumption(TrainingMode mode, uint32_t duration) const;
    bool isOptimalTimeForTraining() const;

private:
    // Configuration
    FLPowerConfig config_;
    FLPowerMode currentMode_;
    bool initialized_;
    bool emergencyMode_;
    bool adaptiveModeEnabled_;
    
    // Component integration
    PowerManager* powerManager_;
    FederatedLearningSystem* federatedSystem_;
    
    // Power monitoring
    FLPowerMetrics metrics_;
    float lastBatteryLevel_;
    float lastSolarPower_;
    uint32_t lastPowerUpdate_;
    
    // Training scheduling
    std::vector<TrainingSchedule> scheduledTraining_;
    uint32_t lastTrainingTime_;
    
    // Power patterns learning
    struct PowerPattern {
        uint32_t hour;
        float averageBatteryLevel;
        float averageSolarPower;
        float averageConsumption;
        uint32_t sampleCount;
    };
    std::vector<PowerPattern> powerPatterns_;
    
    // Callbacks
    PowerModeChangeCallback powerModeChangeCallback_;
    BatteryLevelCallback batteryLevelCallback_;
    SolarChargingCallback solarChargingCallback_;
    EmergencyCallback emergencyCallback_;
    
    // Private methods - Power monitoring
    void updatePowerMetrics();
    void checkBatteryThresholds();
    void checkSolarCharging();
    void handlePowerModeTransition(FLPowerMode newMode);
    
    // Private methods - Schedule optimization
    bool isTimeInSolarWindow(uint32_t timestamp) const;
    float calculateTrainingPowerRequirement(const TrainingSchedule& schedule) const;
    bool validateTrainingSchedule(const TrainingSchedule& schedule) const;
    void optimizeScheduleForPower();
    
    // Private methods - Adaptive learning
    void recordPowerPattern();
    PowerPattern* getPowerPatternForHour(uint32_t hour);
    float predictBatteryLevel(uint32_t futureTime) const;
    float predictSolarPower(uint32_t futureTime) const;
    
    // Private methods - Emergency handling
    void triggerEmergencyShutdown();
    void suspendNonCriticalOperations();
    void restoreNormalOperations();
    
    // Private methods - Utilities
    uint32_t getCurrentHour() const;
    uint32_t getCurrentTimestamp() const;
    void logPowerEvent(const String& event, const String& details = "");
    bool isNightTime() const;
    float interpolateValue(float value1, float value2, float ratio) const;
};

// Global instance
extern FederatedPowerManager* g_federatedPowerManager;

// Utility functions
bool initializeFederatedPowerManager(const FLPowerConfig& config);
void cleanupFederatedPowerManager();
bool isPowerAvailableForFederatedLearning();
FLPowerMode getCurrentFederatedPowerMode();
TrainingSchedule getNextOptimalTrainingWindow();
bool requestFederatedLearningPower(float powerRequired, uint32_t durationMs);

#endif // FEDERATED_POWER_MANAGER_H