#ifndef CRITICAL_BUG_FIXES_H
#define CRITICAL_BUG_FIXES_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <WiFi.h>
#include <memory>

/**
 * Critical Bug Fixes - Addresses race conditions, memory leaks, and stability issues
 * 
 * Implements thread-safe coordinator election, robust WiFi management,
 * leak-free analytics, and safe time management for ESP32WildlifeCAM.
 */

// Forward declarations
class CoordinatorElection;
class WildlifeAnalyticsEngine;
struct WildlifeObservation;

// BUG FIX #1: Thread-safe coordinator election to prevent race conditions
class SafeCoordinatorElection {
    mutable SemaphoreHandle_t electionMutex;
    String nodeId;
    uint32_t currentTerm;
    String votedFor;
    bool isCandidate;
    bool isLeader;
    unsigned long lastHeartbeat;
    
public:
    SafeCoordinatorElection(const String& nodeId);
    ~SafeCoordinatorElection();
    
    bool init();
    void cleanup();
    
    // Thread-safe election methods
    bool startElection();
    bool requestVote(const String& candidateId, uint32_t term, uint32_t lastLogIndex);
    void sendHeartbeat();
    bool isLeaderActive();
    String getCurrentLeader();
    
    // State management
    void becomeFollower(uint32_t term);
    void becomeCandidate();
    void becomeLeader();
    
private:
    bool validateTerm(uint32_t term);
    void resetElectionTimeout();
    void updateLastHeartbeat();
};

// BUG FIX #2: Memory pool for analytics to prevent leaks
class MemoryPool {
    struct Block {
        void* ptr;
        bool inUse;
        size_t size;
        
        Block() : ptr(nullptr), inUse(false), size(0) {}
    };
    
    std::vector<Block> blocks;
    size_t blockSize;
    size_t blockCount;
    mutable SemaphoreHandle_t poolMutex;
    
public:
    MemoryPool(size_t blockSize, size_t blockCount);
    ~MemoryPool();
    
    bool init();
    void cleanup();
    void* acquire();
    void release(void* ptr);
    size_t getAvailableBlocks() const;
    float getUtilization() const;
    
private:
    int findFreeBlock();
    int findBlockByPtr(void* ptr);
};

class LeakFreeAnalyticsEngine {
    std::unique_ptr<MemoryPool> observationPool;
    mutable SemaphoreHandle_t analyticsMutex;
    bool initialized;
    
public:
    LeakFreeAnalyticsEngine();
    ~LeakFreeAnalyticsEngine();
    
    bool init();
    void cleanup();
    void recordObservation(const WildlifeObservation& observation);
    void processQueuedObservations();
    size_t getQueueSize() const;

private:
    void processObservationInternal(const WildlifeObservation& obs);
    void validateObservation(const WildlifeObservation& obs);
};

// BUG FIX #3: Robust WiFi connection management
class RobustWiFiManager {
    static constexpr uint32_t WIFI_TIMEOUT_MS = 30000;
    static constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
    static constexpr uint32_t MAX_RECONNECT_ATTEMPTS = 5;
    
    unsigned long lastConnectionAttempt;
    uint32_t reconnectAttempts;
    bool connectionLost;
    bool initialized;
    String lastSSID;
    String lastPassword;
    mutable SemaphoreHandle_t wifiMutex;
    
    // Connection statistics
    uint32_t totalConnections;
    uint32_t totalDisconnections;
    uint32_t totalReconnectAttempts;
    unsigned long totalConnectedTime;
    unsigned long lastConnectedTime;
    
public:
    RobustWiFiManager();
    ~RobustWiFiManager();
    
    bool init();
    void cleanup();
    
    // Connection management
    bool connect(const String& ssid, const String& password);
    void disconnect();
    bool isConnected();
    bool maintainConnection();
    void forceReconnect();
    
    // Event handling
    void handleWiFiEvents();
    static void WiFiEvent(WiFiEvent_t event);
    
    // Statistics
    struct WiFiStats {
        uint32_t connections;
        uint32_t disconnections;
        uint32_t reconnectAttempts;
        float uptime; // percentage
        int8_t signalStrength;
        
        WiFiStats() : connections(0), disconnections(0), reconnectAttempts(0), 
                     uptime(0.0f), signalStrength(0) {}
    };
    
    WiFiStats getStats() const;
    void resetStats();
    
private:
    static RobustWiFiManager* instance;
    void resetWiFiStack();
    bool attemptConnection();
    void updateConnectionStats();
    void logConnectionEvent(const String& event);
};

// BUG FIX #4: Safe time management to prevent overflow
class SafeTimeManager {
    unsigned long bootTime;
    unsigned long lastNTPSync;
    bool ntpInitialized;
    bool timeValid;
    mutable SemaphoreHandle_t timeMutex;
    
    // Overflow detection
    unsigned long lastTimestamp;
    uint32_t overflowCount;
    
public:
    SafeTimeManager();
    ~SafeTimeManager();
    
    bool init();
    void cleanup();
    
    // Overflow-safe time calculations
    unsigned long getSecureTimestamp();
    bool isTimestampValid(unsigned long timestamp);
    int32_t calculateTimeDifference(unsigned long t1, unsigned long t2);
    unsigned long addTime(unsigned long base, unsigned long offset);
    
    // NTP synchronization with fallback
    bool synchronizeWithNTP();
    void validateSystemTime();
    bool isNTPSynchronized() const;
    unsigned long getLastNTPSync() const;
    
    // Time conversion utilities
    String timestampToString(unsigned long timestamp);
    unsigned long stringToTimestamp(const String& timeStr);
    
private:
    bool detectTimeOverflow(unsigned long timestamp);
    void handleTimeOverflow();
    unsigned long adjustForOverflow(unsigned long timestamp);
    bool connectToNTPServer();
    void updateBootTime();
};

// BUG FIX #5: Safe message handling to prevent buffer overruns
class SafeMessageHandler {
    static constexpr size_t MAX_MESSAGE_SIZE = 1024;
    static constexpr size_t MESSAGE_QUEUE_SIZE = 20;
    
    struct SafeMessage {
        char data[MAX_MESSAGE_SIZE];
        size_t length;
        unsigned long timestamp;
        String messageType;
        
        SafeMessage() : length(0), timestamp(0), messageType("") {
            memset(data, 0, MAX_MESSAGE_SIZE);
        }
    };
    
    SafeMessage messageQueue[MESSAGE_QUEUE_SIZE];
    size_t queueHead;
    size_t queueTail;
    size_t queueCount;
    mutable SemaphoreHandle_t queueMutex;
    bool initialized;
    
public:
    SafeMessageHandler();
    ~SafeMessageHandler();
    
    bool init();
    void cleanup();
    
    // Safe message operations
    bool enqueueMessage(const String& message, const String& type);
    bool dequeueMessage(String& message, String& type);
    size_t getQueueSize() const;
    bool isQueueFull() const;
    void clearQueue();
    
    // Message validation
    bool validateMessage(const String& message);
    String sanitizeMessage(const String& message);
    
private:
    size_t nextIndex(size_t index);
    bool isValidMessageType(const String& type);
};

#endif // CRITICAL_BUG_FIXES_H