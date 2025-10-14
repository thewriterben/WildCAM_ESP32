/**
 * @file wifi_ap_selector.cpp
 * @brief Implementation of intelligent WiFi access point selection
 */

#include "wifi_ap_selector.h"
#include "debug_utils.h"
#include <algorithm>
#include <Preferences.h>

// Constructor
WiFiAPSelector::WiFiAPSelector() 
    : initialized_(false), scanInProgress_(false), lastScanTime_(0) {
}

// Destructor
WiFiAPSelector::~WiFiAPSelector() {
    cleanup();
}

// Initialize the AP selector
bool WiFiAPSelector::init() {
    if (initialized_) {
        return true;
    }
    
    DEBUG_PRINTLN("Initializing WiFi AP Selector...");
    
    // Load historical data
    loadHistory();
    
    initialized_ = true;
    DEBUG_PRINTLN("WiFi AP Selector initialized");
    return true;
}

// Cleanup resources
void WiFiAPSelector::cleanup() {
    if (initialized_) {
        saveHistory();
        networks_.clear();
        networkHistory_.clear();
        initialized_ = false;
    }
}

// Scan for available networks
bool WiFiAPSelector::scanNetworks(bool async) {
    if (!initialized_) {
        DEBUG_PRINTLN("WiFi AP Selector not initialized");
        return false;
    }
    
    if (scanInProgress_) {
        DEBUG_PRINTLN("Scan already in progress");
        return false;
    }
    
    DEBUG_PRINTLN("Scanning for WiFi networks...");
    scanInProgress_ = true;
    
    int numNetworks = WiFi.scanNetworks(async);
    
    if (numNetworks == WIFI_SCAN_RUNNING) {
        return true; // Async scan started
    }
    
    scanInProgress_ = false;
    
    if (numNetworks < 0) {
        DEBUG_PRINTLN("Network scan failed");
        return false;
    }
    
    // Clear previous scan results
    networks_.clear();
    lastScanTime_ = millis();
    
    // Process scan results
    for (int i = 0; i < numNetworks; i++) {
        APInfo ap;
        ap.ssid = WiFi.SSID(i);
        ap.bssid = WiFi.BSSIDstr(i);
        ap.rssi = WiFi.RSSI(i);
        ap.channel = WiFi.channel(i);
        ap.encryption = WiFi.encryptionType(i);
        ap.lastSeen = lastScanTime_;
        
        // Merge with historical data
        APInfo* histAP = findInHistory(ap.ssid, ap.bssid);
        if (histAP) {
            ap.successfulConnections = histAP->successfulConnections;
            ap.failedConnections = histAP->failedConnections;
            ap.totalUptime = histAP->totalUptime;
            ap.averageLatency = histAP->averageLatency;
            ap.isPreferred = histAP->isPreferred;
        }
        
        networks_.push_back(ap);
    }
    
    DEBUG_PRINTF("Found %d networks\n", numNetworks);
    WiFi.scanDelete();
    return true;
}

// Check if scan is complete
bool WiFiAPSelector::isScanComplete() const {
    return !scanInProgress_;
}

// Get number of scanned networks
int WiFiAPSelector::getScannedNetworkCount() const {
    return networks_.size();
}

// Get all available networks
std::vector<APInfo> WiFiAPSelector::getAvailableNetworks() const {
    return networks_;
}

// Get network info by index
APInfo WiFiAPSelector::getNetworkInfo(int index) const {
    if (index >= 0 && index < static_cast<int>(networks_.size())) {
        return networks_[index];
    }
    return APInfo();
}

// Get the best network based on criteria
APInfo WiFiAPSelector::getBestNetwork(const SelectionCriteria& criteria) const {
    if (networks_.empty()) {
        return APInfo();
    }
    
    APInfo bestAP;
    float bestScore = -1.0f;
    
    for (const auto& ap : networks_) {
        if (ap.rssi < criteria.minRSSI) {
            continue; // Skip weak signals
        }
        
        float score = calculateNetworkScore(ap, criteria);
        if (score > bestScore) {
            bestScore = score;
            bestAP = ap;
        }
    }
    
    return bestAP;
}

// Select best SSID from known networks
String WiFiAPSelector::selectBestSSID(const std::vector<String>& knownNetworks,
                                     const SelectionCriteria& criteria) {
    if (networks_.empty() || knownNetworks.empty()) {
        return "";
    }
    
    String bestSSID = "";
    float bestScore = -1.0f;
    
    for (const auto& ap : networks_) {
        // Check if this AP is in the known networks list
        bool isKnown = false;
        for (const auto& known : knownNetworks) {
            if (ap.ssid == known) {
                isKnown = true;
                break;
            }
        }
        
        if (!isKnown) {
            continue;
        }
        
        if (ap.rssi < criteria.minRSSI) {
            continue;
        }
        
        float score = calculateNetworkScore(ap, criteria);
        if (score > bestScore) {
            bestScore = score;
            bestSSID = ap.ssid;
        }
    }
    
    return bestSSID;
}

// Select best BSSID for a given SSID
String WiFiAPSelector::selectBestBSSID(const String& ssid) {
    String bestBSSID = "";
    int32_t bestRSSI = -100;
    
    for (const auto& ap : networks_) {
        if (ap.ssid == ssid && ap.rssi > bestRSSI) {
            bestRSSI = ap.rssi;
            bestBSSID = ap.bssid;
        }
    }
    
    return bestBSSID;
}

// Check if network is available
bool WiFiAPSelector::isNetworkAvailable(const String& ssid) const {
    for (const auto& ap : networks_) {
        if (ap.ssid == ssid) {
            return true;
        }
    }
    return false;
}

// Calculate network score
float WiFiAPSelector::calculateNetworkScore(const APInfo& ap, 
                                           const SelectionCriteria& criteria) const {
    float score = 0.0f;
    
    // Signal strength component (0-40 points)
    float signalScore = calculateSignalQuality(ap.rssi) * 40.0f;
    score += signalScore;
    
    // Historical reliability component (0-30 points)
    if (criteria.considerHistory) {
        float reliability = calculateReliability(ap);
        score += reliability * 30.0f;
    }
    
    // Preferred network bonus (0-20 points)
    if (ap.isPreferred) {
        score += 20.0f;
    }
    
    // Channel congestion penalty (0-10 points deduction)
    if (criteria.avoidCongestion && isChannelCongested(ap.channel)) {
        score -= 10.0f;
    }
    
    return score;
}

// Calculate network metrics
NetworkMetrics WiFiAPSelector::calculateMetrics(const APInfo& ap) const {
    NetworkMetrics metrics;
    
    metrics.signalQuality = calculateSignalQuality(ap.rssi);
    metrics.connectionReliability = calculateReliability(ap);
    metrics.performanceScore = calculateNetworkScore(ap, criteria_);
    
    // Estimate throughput based on signal strength
    if (ap.rssi >= RSSI_EXCELLENT) {
        metrics.estimatedThroughput = 5000; // 5 Mbps
    } else if (ap.rssi >= RSSI_GOOD) {
        metrics.estimatedThroughput = 3000; // 3 Mbps
    } else if (ap.rssi >= RSSI_FAIR) {
        metrics.estimatedThroughput = 1000; // 1 Mbps
    } else {
        metrics.estimatedThroughput = 500;  // 500 Kbps
    }
    
    metrics.isStable = (metrics.connectionReliability > 0.8f);
    
    return metrics;
}

// Rank networks by score
std::vector<APInfo> WiFiAPSelector::rankNetworks(const std::vector<String>& knownNetworks,
                                                const SelectionCriteria& criteria) const {
    std::vector<APInfo> ranked;
    
    // Filter known networks
    for (const auto& ap : networks_) {
        bool isKnown = false;
        for (const auto& known : knownNetworks) {
            if (ap.ssid == known) {
                isKnown = true;
                break;
            }
        }
        
        if (isKnown && ap.rssi >= criteria.minRSSI) {
            ranked.push_back(ap);
        }
    }
    
    // Sort by score
    std::sort(ranked.begin(), ranked.end(),
        [this, &criteria](const APInfo& a, const APInfo& b) {
            return calculateNetworkScore(a, criteria) > calculateNetworkScore(b, criteria);
        });
    
    return ranked;
}

// Record successful connection
void WiFiAPSelector::recordConnectionSuccess(const String& ssid, const String& bssid) {
    APInfo* ap = findInHistory(ssid, bssid);
    if (ap) {
        ap->successfulConnections++;
    } else {
        APInfo newAP;
        newAP.ssid = ssid;
        newAP.bssid = bssid;
        newAP.successfulConnections = 1;
        updateNetworkHistory(newAP);
    }
    DEBUG_PRINTF("Recorded successful connection to %s\n", ssid.c_str());
}

// Record failed connection
void WiFiAPSelector::recordConnectionFailure(const String& ssid, const String& bssid) {
    APInfo* ap = findInHistory(ssid, bssid);
    if (ap) {
        ap->failedConnections++;
    } else {
        APInfo newAP;
        newAP.ssid = ssid;
        newAP.bssid = bssid;
        newAP.failedConnections = 1;
        updateNetworkHistory(newAP);
    }
    DEBUG_PRINTF("Recorded failed connection to %s\n", ssid.c_str());
}

// Update connection uptime
void WiFiAPSelector::updateConnectionUptime(const String& ssid, uint32_t uptime) {
    for (auto& ap : networkHistory_) {
        if (ap.ssid == ssid) {
            ap.totalUptime += uptime;
            break;
        }
    }
}

// Update latency
void WiFiAPSelector::updateLatency(const String& ssid, float latency) {
    for (auto& ap : networkHistory_) {
        if (ap.ssid == ssid) {
            // Exponential moving average
            if (ap.averageLatency == 0) {
                ap.averageLatency = latency;
            } else {
                ap.averageLatency = ap.averageLatency * 0.8f + latency * 0.2f;
            }
            break;
        }
    }
}

// Clear historical data
void WiFiAPSelector::clearHistory() {
    networkHistory_.clear();
    DEBUG_PRINTLN("Network history cleared");
}

// Save historical data to persistent storage
void WiFiAPSelector::saveHistory() {
    // Placeholder for persistent storage implementation
    DEBUG_PRINTLN("Saving network history...");
    // TODO: Implement using Preferences or SPIFFS
}

// Load historical data from persistent storage
bool WiFiAPSelector::loadHistory() {
    // Placeholder for persistent storage implementation
    DEBUG_PRINTLN("Loading network history...");
    // TODO: Implement using Preferences or SPIFFS
    return true;
}

// Set preferred network
void WiFiAPSelector::setPreferredNetwork(const String& ssid, bool preferred) {
    for (auto& ap : networkHistory_) {
        if (ap.ssid == ssid) {
            ap.isPreferred = preferred;
            DEBUG_PRINTF("Set %s as %s network\n", 
                        ssid.c_str(), preferred ? "preferred" : "normal");
            return;
        }
    }
    
    // Add to history if not found
    APInfo newAP;
    newAP.ssid = ssid;
    newAP.isPreferred = preferred;
    updateNetworkHistory(newAP);
}

// Check if network is preferred
bool WiFiAPSelector::isPreferredNetwork(const String& ssid) const {
    for (const auto& ap : networkHistory_) {
        if (ap.ssid == ssid) {
            return ap.isPreferred;
        }
    }
    return false;
}

// Set selection criteria
void WiFiAPSelector::setSelectionCriteria(const SelectionCriteria& criteria) {
    criteria_ = criteria;
}

// Get selection criteria
SelectionCriteria WiFiAPSelector::getSelectionCriteria() const {
    return criteria_;
}

// Set minimum RSSI
void WiFiAPSelector::setMinimumRSSI(int rssi) {
    criteria_.minRSSI = rssi;
}

// Find least congested channel
uint8_t WiFiAPSelector::findLeastCongestedChannel() const {
    int channelCount[14] = {0}; // Channels 1-13 (0 unused)
    
    for (const auto& ap : networks_) {
        if (ap.channel > 0 && ap.channel < 14) {
            channelCount[ap.channel]++;
        }
    }
    
    uint8_t leastCongested = 1;
    int minCount = channelCount[1];
    
    for (int i = 2; i < 14; i++) {
        if (channelCount[i] < minCount) {
            minCount = channelCount[i];
            leastCongested = i;
        }
    }
    
    return leastCongested;
}

// Get congested channels
std::vector<uint8_t> WiFiAPSelector::getCongestedChannels() const {
    std::vector<uint8_t> congested;
    int channelCount[14] = {0};
    
    for (const auto& ap : networks_) {
        if (ap.channel > 0 && ap.channel < 14) {
            channelCount[ap.channel]++;
        }
    }
    
    // Channels with more than 3 APs are considered congested
    for (int i = 1; i < 14; i++) {
        if (channelCount[i] > 3) {
            congested.push_back(i);
        }
    }
    
    return congested;
}

// Get channel load
int WiFiAPSelector::getChannelLoad(uint8_t channel) const {
    int count = 0;
    for (const auto& ap : networks_) {
        if (ap.channel == channel) {
            count++;
        }
    }
    return count;
}

// Print available networks
void WiFiAPSelector::printAvailableNetworks() const {
    DEBUG_PRINTLN("\n=== Available WiFi Networks ===");
    for (size_t i = 0; i < networks_.size(); i++) {
        const auto& ap = networks_[i];
        DEBUG_PRINTF("%d: %s (%s) - RSSI: %d dBm - Ch: %d\n",
                    i, ap.ssid.c_str(), ap.bssid.c_str(), ap.rssi, ap.channel);
    }
    DEBUG_PRINTLN("==============================\n");
}

// Print network metrics
void WiFiAPSelector::printNetworkMetrics(const String& ssid) const {
    for (const auto& ap : networks_) {
        if (ap.ssid == ssid) {
            NetworkMetrics metrics = calculateMetrics(ap);
            DEBUG_PRINTF("\n=== Network Metrics: %s ===\n", ssid.c_str());
            DEBUG_PRINTF("Signal Quality: %.2f\n", metrics.signalQuality);
            DEBUG_PRINTF("Reliability: %.2f\n", metrics.connectionReliability);
            DEBUG_PRINTF("Performance Score: %.2f\n", metrics.performanceScore);
            DEBUG_PRINTF("Est. Throughput: %d Kbps\n", metrics.estimatedThroughput);
            DEBUG_PRINTF("Stable: %s\n", metrics.isStable ? "Yes" : "No");
            DEBUG_PRINTLN("==========================\n");
            return;
        }
    }
    DEBUG_PRINTF("Network %s not found\n", ssid.c_str());
}

// Get network summary
String WiFiAPSelector::getNetworkSummary() const {
    String summary = "WiFi Networks: " + String(networks_.size()) + "\n";
    summary += "Scan Age: " + String((millis() - lastScanTime_) / 1000) + "s\n";
    summary += "History Entries: " + String(networkHistory_.size()) + "\n";
    return summary;
}

// Update network history
void WiFiAPSelector::updateNetworkHistory(const APInfo& ap) {
    // Check if already in history
    for (auto& hist : networkHistory_) {
        if (hist.ssid == ap.ssid && hist.bssid == ap.bssid) {
            hist = ap;
            return;
        }
    }
    
    // Add new entry
    if (networkHistory_.size() >= MAX_HISTORY_ENTRIES) {
        networkHistory_.erase(networkHistory_.begin()); // Remove oldest
    }
    networkHistory_.push_back(ap);
}

// Find network in history
APInfo* WiFiAPSelector::findInHistory(const String& ssid, const String& bssid) {
    for (auto& ap : networkHistory_) {
        if (ap.ssid == ssid && (bssid.isEmpty() || ap.bssid == bssid)) {
            return &ap;
        }
    }
    return nullptr;
}

// Calculate signal quality (0.0 - 1.0)
float WiFiAPSelector::calculateSignalQuality(int32_t rssi) const {
    if (rssi >= RSSI_EXCELLENT) {
        return 1.0f;
    } else if (rssi >= RSSI_GOOD) {
        return 0.8f;
    } else if (rssi >= RSSI_FAIR) {
        return 0.6f;
    } else if (rssi >= RSSI_POOR) {
        return 0.4f;
    } else {
        return 0.2f;
    }
}

// Calculate reliability based on history (0.0 - 1.0)
float WiFiAPSelector::calculateReliability(const APInfo& ap) const {
    uint32_t totalAttempts = ap.successfulConnections + ap.failedConnections;
    
    if (totalAttempts == 0) {
        return 0.5f; // Neutral score for unknown networks
    }
    
    return static_cast<float>(ap.successfulConnections) / totalAttempts;
}

// Check if channel is congested
bool WiFiAPSelector::isChannelCongested(uint8_t channel) const {
    return getChannelLoad(channel) > 3;
}

// Convert BSSID to string
String WiFiAPSelector::bssidToString(const uint8_t* bssid) const {
    char bssidStr[18];
    snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
            bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    return String(bssidStr);
}
