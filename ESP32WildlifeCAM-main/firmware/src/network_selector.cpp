/**
 * @file network_selector.cpp
 * @brief Network selection and management for ESP32 Wildlife Camera
 * 
 * Handles automatic network selection with priority hierarchy:
 * LoRa mesh → Cellular → Satellite
 */

#include "network_selector.h"
#include "cellular_manager.h"
#include "satellite_comm.h"
#include <Arduino.h>
#include <WiFi.h>

NetworkSelector::NetworkSelector() :
    currentNetwork(NETWORK_TYPE_NONE),
    lastNetworkCheck(0),
    networkCheckInterval(30000), // Check every 30 seconds
    cellularManager(nullptr),
    satelliteComm(nullptr),
    costOptimizationEnabled(true) {
}

bool NetworkSelector::initialize(CellularManager* cellular, SatelliteComm* satellite) {
    cellularManager = cellular;
    satelliteComm = satellite;
    
    Serial.println("Network selector initialized");
    
    // Initial network scan
    return scanAvailableNetworks();
}

bool NetworkSelector::scanAvailableNetworks() {
    Serial.println("Scanning available networks...");
    
    availableNetworks.clear();
    
    // Check LoRa mesh (placeholder - would integrate with actual LoRa implementation)
    if (checkLoRaMeshAvailability()) {
        NetworkInfo loraInfo;
        loraInfo.type = NETWORK_TYPE_LORA;
        loraInfo.signalStrength = getLoRaSignalStrength();
        loraInfo.estimatedCost = 0; // Free
        loraInfo.powerConsumption = POWER_LOW;
        loraInfo.available = true;
        availableNetworks.push_back(loraInfo);
    }
    
    // Check cellular
    if (cellularManager && cellularManager->isNetworkConnected()) {
        NetworkInfo cellularInfo;
        cellularInfo.type = NETWORK_TYPE_CELLULAR;
        cellularInfo.signalStrength = cellularManager->getSignalStrength();
        cellularInfo.estimatedCost = calculateCellularCost();
        cellularInfo.powerConsumption = POWER_MEDIUM;
        cellularInfo.available = true;
        availableNetworks.push_back(cellularInfo);
    } else if (cellularManager) {
        // Try to connect
        if (cellularManager->connectToNetwork()) {
            NetworkInfo cellularInfo;
            cellularInfo.type = NETWORK_TYPE_CELLULAR;
            cellularInfo.signalStrength = cellularManager->getSignalStrength();
            cellularInfo.estimatedCost = calculateCellularCost();
            cellularInfo.powerConsumption = POWER_MEDIUM;
            cellularInfo.available = true;
            availableNetworks.push_back(cellularInfo);
        }
    }
    
    // Check satellite
    if (satelliteComm && satelliteComm->checkSatelliteAvailability()) {
        NetworkInfo satelliteInfo;
        satelliteInfo.type = NETWORK_TYPE_SATELLITE;
        satelliteInfo.signalStrength = satelliteComm->checkSignalQuality();
        satelliteInfo.estimatedCost = calculateSatelliteCost();
        satelliteInfo.powerConsumption = POWER_HIGH;
        satelliteInfo.available = true;
        availableNetworks.push_back(satelliteInfo);
    }
    
    Serial.printf("Found %d available networks\n", availableNetworks.size());
    return !availableNetworks.empty();
}

NetworkType NetworkSelector::selectOptimalNetwork(size_t dataSize, MessagePriority priority) {
    if (availableNetworks.empty()) {
        scanAvailableNetworks();
    }
    
    NetworkType selectedNetwork = NETWORK_TYPE_NONE;
    int bestScore = -1;
    
    for (const auto& network : availableNetworks) {
        if (!network.available) continue;
        
        int score = calculateNetworkScore(network, dataSize, priority);
        
        if (score > bestScore) {
            bestScore = score;
            selectedNetwork = network.type;
        }
    }
    
    if (selectedNetwork != NETWORK_TYPE_NONE) {
        Serial.printf("Selected network: %s (score: %d)\n", 
                     getNetworkTypeName(selectedNetwork), bestScore);
        currentNetwork = selectedNetwork;
    }
    
    return selectedNetwork;
}

int NetworkSelector::calculateNetworkScore(const NetworkInfo& network, 
                                         size_t dataSize, 
                                         MessagePriority priority) {
    int score = 0;
    
    // Base priority scores
    switch (network.type) {
        case NETWORK_TYPE_LORA:
            score = NETWORK_SCORE_EXCELLENT; // Highest priority
            break;
        case NETWORK_TYPE_CELLULAR:
            score = 70;
            break;
        case NETWORK_TYPE_SATELLITE:
            score = 40;
            break;
        default:
            return 0;
    }
    
    // Adjust for signal strength
    if (network.signalStrength > 80) {
        score += 20;
    } else if (network.signalStrength > 60) {
        score += 10;
    } else if (network.signalStrength < 30) {
        score -= 20;
    }
    
    // Cost optimization
    if (costOptimizationEnabled) {
        if (network.estimatedCost == 0) {
            score += 15; // Prefer free networks
        } else if (network.estimatedCost > 10) {
            score -= 15; // Penalize expensive networks
        }
    }
    
    // Power consumption consideration
    switch (network.powerConsumption) {
        case POWER_LOW:
            score += 10;
            break;
        case POWER_MEDIUM:
            score += 5;
            break;
        case POWER_HIGH:
            score -= 5;
            break;
    }
    
    // Priority-based adjustments
    if (priority == PRIORITY_EMERGENCY) {
        // For emergency, prefer reliability over cost
        if (network.type == NETWORK_TYPE_SATELLITE) {
            score += 30; // Satellite is most reliable in remote areas
        }
    } else if (priority == PRIORITY_LOW) {
        // For low priority, prefer cost efficiency
        if (network.type == NETWORK_TYPE_LORA) {
            score += 20;
        }
    }
    
    // Data size considerations
    if (dataSize > 1024 * 1024) { // > 1MB
        if (network.type == NETWORK_TYPE_SATELLITE) {
            score -= 30; // Satellite not good for large data
        }
    }
    
    return score;
}

bool NetworkSelector::sendData(const uint8_t* data, size_t length, MessagePriority priority) {
    NetworkType selectedNetwork = selectOptimalNetwork(length, priority);
    
    if (selectedNetwork == NETWORK_TYPE_NONE) {
        Serial.println("No available networks for data transmission");
        return false;
    }
    
    bool success = false;
    
    switch (selectedNetwork) {
        case NETWORK_TYPE_LORA:
            success = sendViaLoRa(data, length);
            break;
        case NETWORK_TYPE_CELLULAR:
            success = sendViaCellular(data, length);
            break;
        case NETWORK_TYPE_SATELLITE:
            success = sendViaSatellite(data, length);
            break;
        default:
            return false;
    }
    
    if (!success && priority == PRIORITY_EMERGENCY) {
        // Try fallback networks for emergency messages
        return attemptFallbackTransmission(data, length);
    }
    
    return success;
}

bool NetworkSelector::sendViaLoRa(const uint8_t* data, size_t length) {
    // Placeholder for LoRa mesh implementation
    Serial.printf("Sending %d bytes via LoRa mesh\n", length);
    
    // Simulate transmission time based on network type
    delay(LORA_TRANSMISSION_DELAY);
    return true;
}

bool NetworkSelector::sendViaCellular(const uint8_t* data, size_t length) {
    if (!cellularManager) {
        return false;
    }
    
    return cellularManager->sendData(data, length);
}

bool NetworkSelector::sendViaSatellite(const uint8_t* data, size_t length) {
    if (!satelliteComm) {
        return false;
    }
    
    // Convert binary data to text for satellite transmission
    String message = "DATA:";
    for (size_t i = 0; i < min(length, (size_t)SATELLITE_MESSAGE_MAX_LENGTH); i++) { // Limit for satellite
        message += String(data[i], HEX);
        if (i < length - 1) message += ",";
    }
    
    return satelliteComm->sendMessage(message);
}

bool NetworkSelector::attemptFallbackTransmission(const uint8_t* data, size_t length) {
    Serial.println("Attempting fallback transmission for emergency message");
    
    // Try all available networks in order of reliability
    for (const auto& network : availableNetworks) {
        if (!network.available) continue;
        
        bool success = false;
        switch (network.type) {
            case NETWORK_TYPE_SATELLITE:
                success = sendViaSatellite(data, length);
                break;
            case NETWORK_TYPE_CELLULAR:
                success = sendViaCellular(data, length);
                break;
            case NETWORK_TYPE_LORA:
                success = sendViaLoRa(data, length);
                break;
        }
        
        if (success) {
            Serial.printf("Emergency message sent via %s\n", 
                         getNetworkTypeName(network.type));
            return true;
        }
        
        delay(NETWORK_RETRY_DELAY); // Wait between attempts
    }
    
    return false;
}

bool NetworkSelector::checkLoRaMeshAvailability() {
    // Placeholder for LoRa mesh availability check
    // In a real implementation, this would scan for LoRa mesh nodes
    return true; // Simulate available mesh
}

int NetworkSelector::getLoRaSignalStrength() {
    // Placeholder for LoRa signal strength
    return 85; // Simulate good signal
}

float NetworkSelector::calculateCellularCost() {
    if (!cellularManager) return 0;
    
    // Estimate cost based on data usage
    size_t dataUsage = cellularManager->getDataUsage();
    return dataUsage * 0.1; // $0.10 per MB
}

float NetworkSelector::calculateSatelliteCost() {
    if (!satelliteComm) return 0;
    
    // Estimate cost based on message count
    size_t messageCount = satelliteComm->getMessageCount();
    return messageCount * 0.5; // $0.50 per message
}

void NetworkSelector::periodicNetworkCheck() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastNetworkCheck >= networkCheckInterval) {
        scanAvailableNetworks();
        lastNetworkCheck = currentTime;
    }
}

NetworkType NetworkSelector::getCurrentNetwork() {
    return currentNetwork;
}

const char* NetworkSelector::getNetworkTypeName(NetworkType type) {
    switch (type) {
        case NETWORK_TYPE_LORA: return "LoRa Mesh";
        case NETWORK_TYPE_CELLULAR: return "Cellular";
        case NETWORK_TYPE_SATELLITE: return "Satellite";
        default: return "None";
    }
}

void NetworkSelector::setCostOptimization(bool enabled) {
    costOptimizationEnabled = enabled;
}

std::vector<NetworkInfo> NetworkSelector::getAvailableNetworks() {
    return availableNetworks;
}