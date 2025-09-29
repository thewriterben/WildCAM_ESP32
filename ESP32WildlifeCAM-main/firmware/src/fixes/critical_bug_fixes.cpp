#include "critical_bug_fixes.h"
#include "../config.h"
#include <WiFi.h>

// SafeCoordinatorElection implementation
SafeCoordinatorElection::SafeCoordinatorElection(const String& nodeId) 
    : nodeId(nodeId), currentTerm(0), votedFor(""), isCandidate(false), 
      isLeader(false), lastHeartbeat(0) {
    electionMutex = xSemaphoreCreateMutex();
}

SafeCoordinatorElection::~SafeCoordinatorElection() {
    cleanup();
}

bool SafeCoordinatorElection::init() {
    if (!electionMutex) {
        DEBUG_PRINTLN("ERROR: Failed to create election mutex");
        return false;
    }
    
    DEBUG_PRINTLN("Safe Coordinator Election initialized");
    return true;
}

void SafeCoordinatorElection::cleanup() {
    if (electionMutex) {
        vSemaphoreDelete(electionMutex);
        electionMutex = nullptr;
    }
}

bool SafeCoordinatorElection::startElection() {
    if (!electionMutex) return false;
    
    if (xSemaphoreTake(electionMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        currentTerm++;
        votedFor = nodeId;
        isCandidate = true;
        isLeader = false;
        
        DEBUG_PRINTF("Starting election for term %u\n", currentTerm);
        
        xSemaphoreGive(electionMutex);
        return true;
    }
    
    DEBUG_PRINTLN("ERROR: Failed to acquire election mutex");
    return false;
}

bool SafeCoordinatorElection::requestVote(const String& candidateId, uint32_t term, uint32_t lastLogIndex) {
    if (!electionMutex) return false;
    
    if (xSemaphoreTake(electionMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        bool voteGranted = false;
        
        if (validateTerm(term)) {
            if (votedFor.isEmpty() || votedFor == candidateId) {
                votedFor = candidateId;
                voteGranted = true;
                DEBUG_PRINTF("Vote granted to %s for term %u\n", candidateId.c_str(), term);
            }
        }
        
        xSemaphoreGive(electionMutex);
        return voteGranted;
    }
    
    return false;
}

bool SafeCoordinatorElection::validateTerm(uint32_t term) {
    if (term > currentTerm) {
        currentTerm = term;
        votedFor = "";
        isCandidate = false;
        isLeader = false;
        return true;
    }
    return term == currentTerm;
}

void SafeCoordinatorElection::sendHeartbeat() {
    if (isLeader) {
        updateLastHeartbeat();
        // Send heartbeat message to network
        DEBUG_PRINTF("Heartbeat sent for term %u\n", currentTerm);
    }
}

void SafeCoordinatorElection::updateLastHeartbeat() {
    lastHeartbeat = millis();
}

// MemoryPool implementation
MemoryPool::MemoryPool(size_t blockSize, size_t blockCount) 
    : blockSize(blockSize), blockCount(blockCount) {
    poolMutex = xSemaphoreCreateMutex();
    blocks.resize(blockCount);
}

MemoryPool::~MemoryPool() {
    cleanup();
}

bool MemoryPool::init() {
    if (!poolMutex) return false;
    
    for (size_t i = 0; i < blockCount; i++) {
        blocks[i].ptr = malloc(blockSize);
        if (!blocks[i].ptr) {
            DEBUG_PRINTF("ERROR: Failed to allocate block %zu\n", i);
            return false;
        }
        blocks[i].size = blockSize;
        blocks[i].inUse = false;
    }
    
    DEBUG_PRINTF("Memory pool initialized: %zu blocks of %zu bytes\n", blockCount, blockSize);
    return true;
}

void MemoryPool::cleanup() {
    if (poolMutex && xSemaphoreTake(poolMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        for (auto& block : blocks) {
            if (block.ptr) {
                free(block.ptr);
                block.ptr = nullptr;
            }
        }
        xSemaphoreGive(poolMutex);
        vSemaphoreDelete(poolMutex);
        poolMutex = nullptr;
    }
}

void* MemoryPool::acquire() {
    if (!poolMutex) return nullptr;
    
    if (xSemaphoreTake(poolMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        int index = findFreeBlock();
        if (index >= 0) {
            blocks[index].inUse = true;
            void* ptr = blocks[index].ptr;
            xSemaphoreGive(poolMutex);
            return ptr;
        }
        xSemaphoreGive(poolMutex);
    }
    
    return nullptr;
}

void MemoryPool::release(void* ptr) {
    if (!ptr || !poolMutex) return;
    
    if (xSemaphoreTake(poolMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        int index = findBlockByPtr(ptr);
        if (index >= 0) {
            blocks[index].inUse = false;
        }
        xSemaphoreGive(poolMutex);
    }
}

int MemoryPool::findFreeBlock() {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (!blocks[i].inUse) {
            return i;
        }
    }
    return -1;
}

int MemoryPool::findBlockByPtr(void* ptr) {
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i].ptr == ptr) {
            return i;
        }
    }
    return -1;
}

// LeakFreeAnalyticsEngine implementation
LeakFreeAnalyticsEngine::LeakFreeAnalyticsEngine() 
    : initialized(false) {
    analyticsMutex = xSemaphoreCreateMutex();
}

LeakFreeAnalyticsEngine::~LeakFreeAnalyticsEngine() {
    cleanup();
}

bool LeakFreeAnalyticsEngine::init() {
    if (!analyticsMutex) return false;
    
    observationPool = std::make_unique<MemoryPool>(sizeof(WildlifeObservation), 50);
    if (!observationPool->init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize observation pool");
        return false;
    }
    
    initialized = true;
    DEBUG_PRINTLN("Leak-free Analytics Engine initialized");
    return true;
}

void LeakFreeAnalyticsEngine::cleanup() {
    if (analyticsMutex) {
        if (xSemaphoreTake(analyticsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            observationPool.reset();
            xSemaphoreGive(analyticsMutex);
        }
        vSemaphoreDelete(analyticsMutex);
        analyticsMutex = nullptr;
    }
    initialized = false;
}

void LeakFreeAnalyticsEngine::recordObservation(const WildlifeObservation& observation) {
    if (!initialized || !analyticsMutex) return;
    
    if (xSemaphoreTake(analyticsMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // Use object pool instead of dynamic allocation
        auto* pooledObs = (WildlifeObservation*)observationPool->acquire();
        if (pooledObs) {
            *pooledObs = observation;
            
            // Process and release immediately
            processObservationInternal(*pooledObs);
            observationPool->release(pooledObs);
        } else {
            DEBUG_PRINTLN("WARNING: No available observation pool blocks");
        }
        
        xSemaphoreGive(analyticsMutex);
    }
}

void LeakFreeAnalyticsEngine::processObservationInternal(const WildlifeObservation& obs) {
    // Placeholder for observation processing
    DEBUG_PRINTLN("Processing wildlife observation");
}

// RobustWiFiManager implementation
RobustWiFiManager* RobustWiFiManager::instance = nullptr;

RobustWiFiManager::RobustWiFiManager() 
    : lastConnectionAttempt(0), reconnectAttempts(0), connectionLost(false),
      initialized(false), totalConnections(0), totalDisconnections(0),
      totalReconnectAttempts(0), totalConnectedTime(0), lastConnectedTime(0) {
    wifiMutex = xSemaphoreCreateMutex();
    instance = this;
}

RobustWiFiManager::~RobustWiFiManager() {
    cleanup();
    instance = nullptr;
}

bool RobustWiFiManager::init() {
    if (!wifiMutex) return false;
    
    WiFi.onEvent(WiFiEvent);
    initialized = true;
    
    DEBUG_PRINTLN("Robust WiFi Manager initialized");
    return true;
}

void RobustWiFiManager::cleanup() {
    if (wifiMutex) {
        disconnect();
        vSemaphoreDelete(wifiMutex);
        wifiMutex = nullptr;
    }
    initialized = false;
}

bool RobustWiFiManager::connect(const String& ssid, const String& password) {
    if (!initialized || !wifiMutex) return false;
    
    if (xSemaphoreTake(wifiMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        lastSSID = ssid;
        lastPassword = password;
        reconnectAttempts = 0;
        
        bool result = attemptConnection();
        
        xSemaphoreGive(wifiMutex);
        return result;
    }
    
    return false;
}

bool RobustWiFiManager::attemptConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    DEBUG_PRINTF("Attempting WiFi connection to %s\n", lastSSID.c_str());
    
    WiFi.begin(lastSSID.c_str(), lastPassword.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           (millis() - startTime) < WIFI_TIMEOUT_MS) {
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        totalConnections++;
        lastConnectedTime = millis();
        DEBUG_PRINTF("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        DEBUG_PRINTLN("WiFi connection failed");
        return false;
    }
}

void RobustWiFiManager::WiFiEvent(WiFiEvent_t event) {
    if (!instance) return;
    
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            DEBUG_PRINTLN("WiFi connected event");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            DEBUG_PRINTLN("WiFi disconnected event");
            instance->connectionLost = true;
            instance->totalDisconnections++;
            break;
        default:
            break;
    }
}

bool RobustWiFiManager::maintainConnection() {
    if (!initialized || !wifiMutex) return false;
    
    if (connectionLost && 
        (millis() - lastConnectionAttempt) > RECONNECT_INTERVAL_MS &&
        reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        
        if (xSemaphoreTake(wifiMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            DEBUG_PRINTF("Attempting reconnection %u/%u\n", 
                        reconnectAttempts + 1, MAX_RECONNECT_ATTEMPTS);
            
            lastConnectionAttempt = millis();
            reconnectAttempts++;
            totalReconnectAttempts++;
            
            if (attemptConnection()) {
                connectionLost = false;
                reconnectAttempts = 0;
            }
            
            xSemaphoreGive(wifiMutex);
        }
    }
    
    return WiFi.status() == WL_CONNECTED;
}

// SafeTimeManager implementation
SafeTimeManager::SafeTimeManager() 
    : bootTime(0), lastNTPSync(0), ntpInitialized(false), 
      timeValid(false), lastTimestamp(0), overflowCount(0) {
    timeMutex = xSemaphoreCreateMutex();
}

SafeTimeManager::~SafeTimeManager() {
    cleanup();
}

bool SafeTimeManager::init() {
    if (!timeMutex) return false;
    
    updateBootTime();
    timeValid = true;
    
    DEBUG_PRINTLN("Safe Time Manager initialized");
    return true;
}

void SafeTimeManager::cleanup() {
    if (timeMutex) {
        vSemaphoreDelete(timeMutex);
        timeMutex = nullptr;
    }
}

unsigned long SafeTimeManager::getSecureTimestamp() {
    if (!timeMutex) return millis();
    
    if (xSemaphoreTake(timeMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        unsigned long current = millis();
        
        // Check for overflow
        if (detectTimeOverflow(current)) {
            handleTimeOverflow();
        }
        
        lastTimestamp = current;
        xSemaphoreGive(timeMutex);
        
        return adjustForOverflow(current);
    }
    
    return millis();
}

bool SafeTimeManager::detectTimeOverflow(unsigned long timestamp) {
    return timestamp < lastTimestamp; // Overflow detected
}

void SafeTimeManager::handleTimeOverflow() {
    overflowCount++;
    DEBUG_PRINTF("Time overflow detected (count: %u)\n", overflowCount);
}

unsigned long SafeTimeManager::adjustForOverflow(unsigned long timestamp) {
    // Adjust timestamp accounting for overflows
    return timestamp + (overflowCount * ULONG_MAX);
}

int32_t SafeTimeManager::calculateTimeDifference(unsigned long t1, unsigned long t2) {
    // Safe time difference calculation handling overflow
    if (t2 >= t1) {
        return t2 - t1;
    } else {
        // Handle overflow case
        return (ULONG_MAX - t1) + t2 + 1;
    }
}

void SafeTimeManager::updateBootTime() {
    bootTime = millis();
}

// SafeMessageHandler implementation
SafeMessageHandler::SafeMessageHandler() 
    : queueHead(0), queueTail(0), queueCount(0), initialized(false) {
    queueMutex = xSemaphoreCreateMutex();
}

SafeMessageHandler::~SafeMessageHandler() {
    cleanup();
}

bool SafeMessageHandler::init() {
    if (!queueMutex) return false;
    
    // Initialize message queue
    for (size_t i = 0; i < MESSAGE_QUEUE_SIZE; i++) {
        messageQueue[i] = SafeMessage();
    }
    
    initialized = true;
    DEBUG_PRINTLN("Safe Message Handler initialized");
    return true;
}

void SafeMessageHandler::cleanup() {
    if (queueMutex) {
        clearQueue();
        vSemaphoreDelete(queueMutex);
        queueMutex = nullptr;
    }
    initialized = false;
}

bool SafeMessageHandler::enqueueMessage(const String& message, const String& type) {
    if (!initialized || !queueMutex || isQueueFull()) return false;
    
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if (!validateMessage(message)) {
            xSemaphoreGive(queueMutex);
            return false;
        }
        
        SafeMessage& msg = messageQueue[queueTail];
        size_t copyLen = min(message.length(), MAX_MESSAGE_SIZE - 1);
        
        strncpy(msg.data, message.c_str(), copyLen);
        msg.data[copyLen] = '\0';
        msg.length = copyLen;
        msg.timestamp = millis();
        msg.messageType = type;
        
        queueTail = nextIndex(queueTail);
        queueCount++;
        
        xSemaphoreGive(queueMutex);
        return true;
    }
    
    return false;
}

bool SafeMessageHandler::dequeueMessage(String& message, String& type) {
    if (!initialized || !queueMutex || queueCount == 0) return false;
    
    if (xSemaphoreTake(queueMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        SafeMessage& msg = messageQueue[queueHead];
        
        message = String(msg.data);
        type = msg.messageType;
        
        queueHead = nextIndex(queueHead);
        queueCount--;
        
        xSemaphoreGive(queueMutex);
        return true;
    }
    
    return false;
}

bool SafeMessageHandler::validateMessage(const String& message) {
    // Basic message validation
    return !message.isEmpty() && message.length() < MAX_MESSAGE_SIZE;
}

size_t SafeMessageHandler::nextIndex(size_t index) {
    return (index + 1) % MESSAGE_QUEUE_SIZE;
}

bool SafeMessageHandler::isQueueFull() const {
    return queueCount >= MESSAGE_QUEUE_SIZE;
}

void SafeMessageHandler::clearQueue() {
    if (queueMutex && xSemaphoreTake(queueMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        queueHead = 0;
        queueTail = 0;
        queueCount = 0;
        xSemaphoreGive(queueMutex);
    }
}