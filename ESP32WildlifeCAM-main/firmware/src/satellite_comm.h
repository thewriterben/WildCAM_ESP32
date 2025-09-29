/**
 * @file satellite_comm.h
 * @brief Header file for satellite communication manager
 */

#ifndef SATELLITE_COMM_H
#define SATELLITE_COMM_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <vector>
#include <time.h>
#include "satellite_config.h"

enum SatelliteModule {
    MODULE_NONE = 0,
    MODULE_IRIDIUM,
    MODULE_SWARM,
    MODULE_ROCKBLOCK
};

class SatelliteComm {
public:
    SatelliteComm();
    
    // Initialization and configuration
    bool initialize(SatelliteModule module);
    bool configure(const SatelliteConfig& config);
    void updateConfiguration(const SatelliteConfig& config);
    SatelliteConfig getConfiguration() const;
    
    // Communication
    bool sendMessage(const String& message, SatelliteMessagePriority priority = SAT_PRIORITY_NORMAL);
    bool sendWildlifeAlert(const String& alertData);
    bool sendThumbnail(const uint8_t* thumbnailData, size_t size);
    bool sendStatusReport();
    bool sendStoredMessages();
    bool receiveConfigUpdate();
    
    // Satellite status
    bool checkSatelliteAvailability();
    int checkSignalQuality();
    bool predictNextPass();
    time_t getNextPassTime();
    TransmissionWindow getOptimalTransmissionWindow();
    
    // Power management
    void enterSleepMode();
    void exitSleepMode();
    bool isInLowPowerMode() const;
    void optimizePowerConsumption(uint8_t batteryLevel);
    
    // Message management
    size_t getMessageCount();
    size_t getStoredMessageCount();
    void resetMessageCount();
    void setMessageLimit(size_t limit);
    float getTodayCost() const;
    bool isWithinCostLimit() const;
    
    // Emergency handling
    bool sendEmergencyAlert(const String& alertData);
    void enableEmergencyMode();
    void disableEmergencyMode();
    bool isEmergencyMode() const;

private:
    SoftwareSerial satSerial;
    bool isConnected;
    SatelliteModule moduleType;
    SatelliteConfig config;
    size_t messagesSentToday;
    size_t maxDailyMessages;
    time_t lastPassTime;
    float todayCost;
    bool emergencyMode;
    bool lowPowerMode;
    unsigned long lastTransmissionTime;
    unsigned long lastPassPrediction;
    std::vector<String> storedMessages;
    std::vector<SatelliteMessagePriority> storedPriorities;
    
    // Module-specific initialization
    bool initializeIridium();
    bool initializeSwarm();
    bool initializeRockBlock();
    
    // Configuration
    void configureIridium();
    void configureSwarm();
    void wakeUpModule();
    
    // Message transmission with retry logic
    bool sendIridiumMessage(const String& message);
    bool sendSwarmMessage(const String& message);
    bool storeMessage(const String& message, SatelliteMessagePriority priority = SAT_PRIORITY_NORMAL);
    bool sendWithRetry(const String& message, int maxAttempts = MAX_RETRY_ATTEMPTS);
    
    // Cost and optimization
    void updateCost(float messageCost);
    bool shouldTransmitNow(SatelliteMessagePriority priority);
    TransmissionWindow calculateOptimalWindow(SatelliteMessagePriority priority);
    
    // Pass prediction and timing
    void updatePassPrediction();
    bool isOptimalTransmissionTime();
    unsigned long calculateNextRetryDelay(int attemptNumber);
    
    // Utility functions
    String stringToHex(const String& input);
    String createStatusMessage();
    String compressMessage(const String& message);
    void logTransmission(bool success, const String& message, float cost);
};

#endif // SATELLITE_COMM_H