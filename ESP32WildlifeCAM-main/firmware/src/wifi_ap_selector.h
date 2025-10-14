/**
 * @file wifi_ap_selector.h
 * @brief Intelligent WiFi Access Point Selection and Optimization
 * 
 * Provides smart AP selection based on signal strength, network load,
 * and historical performance metrics for field-deployed cameras.
 */

#ifndef WIFI_AP_SELECTOR_H
#define WIFI_AP_SELECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

// Access point information structure
struct APInfo {
    String ssid;
    String bssid;
    int32_t rssi;              // Signal strength in dBm
    uint8_t channel;
    wifi_auth_mode_t encryption;
    uint32_t lastSeen;         // Timestamp of last scan
    uint32_t successfulConnections;
    uint32_t failedConnections;
    uint32_t totalUptime;      // Total connection time in ms
    float averageLatency;      // Average response time in ms
    bool isPreferred;          // User-marked preferred network
    
    APInfo() : rssi(-100), channel(0), encryption(WIFI_AUTH_OPEN),
               lastSeen(0), successfulConnections(0), failedConnections(0),
               totalUptime(0), averageLatency(0), isPreferred(false) {}
};

// AP selection criteria
struct SelectionCriteria {
    int minRSSI;               // Minimum acceptable signal strength
    bool preferStrongerSignal; // Prioritize signal strength
    bool considerHistory;      // Use historical performance data
    bool avoidCongestion;      // Prefer less congested channels
    uint32_t maxAge;           // Maximum age of scan data (ms)
    
    SelectionCriteria() : minRSSI(-75), preferStrongerSignal(true),
                         considerHistory(true), avoidCongestion(true),
                         maxAge(300000) {} // 5 minutes default
};

// Network performance metrics
struct NetworkMetrics {
    float signalQuality;       // 0.0 - 1.0
    float connectionReliability; // 0.0 - 1.0 based on history
    float performanceScore;    // Overall score 0.0 - 100.0
    uint32_t estimatedThroughput; // Kbps
    bool isStable;             // Connection stability indicator
};

/**
 * @class WiFiAPSelector
 * @brief Intelligent WiFi access point selection and management
 */
class WiFiAPSelector {
public:
    WiFiAPSelector();
    ~WiFiAPSelector();
    
    // Initialization
    bool init();
    void cleanup();
    
    // Network scanning
    bool scanNetworks(bool async = false);
    int getScannedNetworkCount() const;
    bool isScanComplete() const;
    
    // AP information retrieval
    std::vector<APInfo> getAvailableNetworks() const;
    APInfo getNetworkInfo(int index) const;
    APInfo getBestNetwork(const SelectionCriteria& criteria = SelectionCriteria()) const;
    
    // AP selection
    String selectBestSSID(const std::vector<String>& knownNetworks,
                         const SelectionCriteria& criteria = SelectionCriteria());
    String selectBestBSSID(const String& ssid);
    bool isNetworkAvailable(const String& ssid) const;
    
    // Network scoring and ranking
    float calculateNetworkScore(const APInfo& ap, const SelectionCriteria& criteria) const;
    NetworkMetrics calculateMetrics(const APInfo& ap) const;
    std::vector<APInfo> rankNetworks(const std::vector<String>& knownNetworks,
                                     const SelectionCriteria& criteria = SelectionCriteria()) const;
    
    // Historical data management
    void recordConnectionSuccess(const String& ssid, const String& bssid);
    void recordConnectionFailure(const String& ssid, const String& bssid);
    void updateConnectionUptime(const String& ssid, uint32_t uptime);
    void updateLatency(const String& ssid, float latency);
    void clearHistory();
    void saveHistory();
    bool loadHistory();
    
    // Preferred networks
    void setPreferredNetwork(const String& ssid, bool preferred = true);
    bool isPreferredNetwork(const String& ssid) const;
    
    // Configuration
    void setSelectionCriteria(const SelectionCriteria& criteria);
    SelectionCriteria getSelectionCriteria() const;
    void setMinimumRSSI(int rssi);
    
    // Channel analysis
    uint8_t findLeastCongestedChannel() const;
    std::vector<uint8_t> getCongestedChannels() const;
    int getChannelLoad(uint8_t channel) const;
    
    // Statistics and diagnostics
    void printAvailableNetworks() const;
    void printNetworkMetrics(const String& ssid) const;
    String getNetworkSummary() const;
    
private:
    // Internal state
    bool initialized_;
    bool scanInProgress_;
    std::vector<APInfo> networks_;
    SelectionCriteria criteria_;
    unsigned long lastScanTime_;
    
    // Historical data storage
    static const int MAX_HISTORY_ENTRIES = 10;
    std::vector<APInfo> networkHistory_;
    
    // Internal methods
    void updateNetworkHistory(const APInfo& ap);
    APInfo* findInHistory(const String& ssid, const String& bssid);
    float calculateSignalQuality(int32_t rssi) const;
    float calculateReliability(const APInfo& ap) const;
    bool isChannelCongested(uint8_t channel) const;
    void sortNetworksByScore(std::vector<APInfo>& networks, 
                            const SelectionCriteria& criteria) const;
    String bssidToString(const uint8_t* bssid) const;
    
    // Constants
    static const int SCAN_TIMEOUT_MS = 10000;
    static const int RSSI_EXCELLENT = -50;
    static const int RSSI_GOOD = -60;
    static const int RSSI_FAIR = -70;
    static const int RSSI_POOR = -80;
};

#endif // WIFI_AP_SELECTOR_H
