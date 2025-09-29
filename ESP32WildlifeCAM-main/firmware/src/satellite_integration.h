/**
 * @file satellite_integration.h
 * @brief Integration example for satellite communication in wildlife camera system
 */

#ifndef SATELLITE_INTEGRATION_H
#define SATELLITE_INTEGRATION_H

#include "satellite_comm.h"
#include "satellite_config.h"
#include "data_compression.h"

class WildlifeCameraSatelliteIntegration {
public:
    WildlifeCameraSatelliteIntegration();
    
    // Initialization
    bool initializeSatelliteComm();
    void configureForEnvironment(bool remoteLocation = true);
    
    // Wildlife monitoring integration
    void handleWildlifeDetection(const String& species, float confidence, const uint8_t* imageData, size_t imageSize);
    void sendPeriodicStatusUpdate();
    void handleEmergencyCondition(const String& condition);
    
    // Power management integration  
    void updatePowerStatus(uint8_t batteryLevel, bool isCharging);
    void enterLowPowerMode();
    void exitLowPowerMode();
    
    // Configuration management
    void updateSatelliteConfig(const SatelliteConfig& newConfig);
    void checkForRemoteConfigUpdates();
    
    // Status and monitoring
    bool isSatelliteEnabled() const;
    float getTodaySatelliteCost() const;
    size_t getPendingMessageCount() const;

private:
    SatelliteComm satelliteComm;
    SatelliteConfig currentConfig;
    DataCompression dataCompression;
    
    unsigned long lastStatusUpdate;
    unsigned long lastConfigCheck;
    bool emergencyMode;
    bool lowPowerMode;
    
    // Helper methods
    void createDefaultConfig();
    String createWildlifeAlertMessage(const String& species, float confidence);
    bool shouldSendImmediately(const String& messageType);
    void logSatelliteActivity(const String& activity);
};

// Global instance for easy integration
extern WildlifeCameraSatelliteIntegration satelliteIntegration;

// Integration macros for easy use in main code
#define SATELLITE_INIT() satelliteIntegration.initializeSatelliteComm()
#define SATELLITE_WILDLIFE_ALERT(species, confidence, image, size) \
    satelliteIntegration.handleWildlifeDetection(species, confidence, image, size)
#define SATELLITE_EMERGENCY(condition) \
    satelliteIntegration.handleEmergencyCondition(condition)
#define SATELLITE_POWER_UPDATE(battery, charging) \
    satelliteIntegration.updatePowerStatus(battery, charging)

#endif // SATELLITE_INTEGRATION_H