/**
 * @file network_selector.h
 * @brief Header file for network selection and management
 */

#ifndef NETWORK_SELECTOR_H
#define NETWORK_SELECTOR_H

#include <Arduino.h>
#include <vector>

// Forward declarations
class CellularManager;
class SatelliteComm;

enum NetworkType {
    NETWORK_TYPE_NONE = 0,
    NETWORK_TYPE_LORA,
    NETWORK_TYPE_CELLULAR,
    NETWORK_TYPE_SATELLITE
};

enum MessagePriority {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
    PRIORITY_EMERGENCY
};

enum PowerConsumption {
    POWER_LOW = 0,
    POWER_MEDIUM,
    POWER_HIGH
};

struct NetworkInfo {
    NetworkType type;
    int signalStrength;
    float estimatedCost;
    PowerConsumption powerConsumption;
    bool available;
};

class NetworkSelector {
public:
    NetworkSelector();
    
    // Initialization
    bool initialize(CellularManager* cellular, SatelliteComm* satellite);
    
    // Network management
    bool scanAvailableNetworks();
    NetworkType selectOptimalNetwork(size_t dataSize, MessagePriority priority = PRIORITY_NORMAL);
    
    // Data transmission
    bool sendData(const uint8_t* data, size_t length, MessagePriority priority = PRIORITY_NORMAL);
    
    // Network monitoring
    void periodicNetworkCheck();
    NetworkType getCurrentNetwork();
    std::vector<NetworkInfo> getAvailableNetworks();
    
    // Configuration
    void setCostOptimization(bool enabled);
    
    // Utility
    const char* getNetworkTypeName(NetworkType type);

private:
    NetworkType currentNetwork;
    unsigned long lastNetworkCheck;
    unsigned long networkCheckInterval;
    std::vector<NetworkInfo> availableNetworks;
    
    CellularManager* cellularManager;
    SatelliteComm* satelliteComm;
    bool costOptimizationEnabled;
    
    // Network scoring
    int calculateNetworkScore(const NetworkInfo& network, size_t dataSize, MessagePriority priority);
    
    // Transmission methods
    bool sendViaLoRa(const uint8_t* data, size_t length);
    bool sendViaCellular(const uint8_t* data, size_t length);
    bool sendViaSatellite(const uint8_t* data, size_t length);
    bool attemptFallbackTransmission(const uint8_t* data, size_t length);
    
    // Network availability checks
    bool checkLoRaMeshAvailability();
    int getLoRaSignalStrength();
    
    // Cost calculations
    float calculateCellularCost();
    float calculateSatelliteCost();
};

#endif // NETWORK_SELECTOR_H