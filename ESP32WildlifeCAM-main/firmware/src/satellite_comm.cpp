/**
 * @file satellite_comm.cpp
 * @brief Satellite communication manager for ESP32 Wildlife Camera
 * 
 * Handles LEO satellite communication (Iridium, Swarm) with store-and-forward
 * messaging and satellite pass prediction.
 */

#include "satellite_comm.h"
#include "satellite_config.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <time.h>

SatelliteComm::SatelliteComm() :
    satSerial(SAT_RX_PIN, SAT_TX_PIN),
    isConnected(false),
    moduleType(MODULE_NONE),
    messagesSentToday(0),
    maxDailyMessages(DEFAULT_DAILY_MESSAGE_LIMIT),
    lastPassTime(0),
    todayCost(0.0),
    emergencyMode(false),
    lowPowerMode(false),
    lastTransmissionTime(0),
    lastPassPrediction(0) {
    
    // Initialize default configuration
    config.enabled = false;
    config.module = MODULE_NONE;
}

bool SatelliteComm::configure(const SatelliteConfig& newConfig) {
    config = newConfig;
    moduleType = config.module;
    maxDailyMessages = config.maxDailyMessages;
    emergencyMode = false;
    
    Serial.println("Satellite communication configured");
    Serial.printf("Module: %d, Max daily messages: %d, Cost limit: $%.2f\n", 
                  config.module, config.maxDailyMessages, config.maxDailyCost);
    
    return true;
}

void SatelliteComm::updateConfiguration(const SatelliteConfig& newConfig) {
    configure(newConfig);
}

SatelliteConfig SatelliteComm::getConfiguration() const {
    return config;
}
bool SatelliteComm::initialize(SatelliteModule module) {
    moduleType = module;
    config.module = module;
    
    if (!config.enabled) {
        Serial.println("Satellite communication disabled in configuration");
        return false;
    }
    
    // Initialize serial communication with configurable baud rate
    satSerial.begin(SATELLITE_BAUD_RATE);
    
    // Configure control pins
    pinMode(SAT_SLEEP_PIN, OUTPUT);
    pinMode(SAT_RING_PIN, INPUT);
    
    // Wake up the module
    wakeUpModule();
    
    // Test communication based on module type
    switch (moduleType) {
        case MODULE_IRIDIUM:
            return initializeIridium();
        case MODULE_SWARM:
            return initializeSwarm();
        case MODULE_ROCKBLOCK:
            return initializeRockBlock();
        default:
            Serial.println("Unknown satellite module type");
            return false;
    }
}

bool SatelliteComm::initializeIridium() {
    Serial.println("Initializing Iridium 9603N module");
    
    // Test AT communication with configurable retry count
    for (int i = 0; i < SATELLITE_RETRY_COUNT; i++) {
        satSerial.println("AT");
        delay(SERIAL_INIT_DELAY);
        if (satSerial.available()) {
            String response = satSerial.readString();
            if (response.indexOf("OK") != -1) {
                Serial.println("Iridium module responding");
                configureIridium();
                return true;
            }
        }
    }
    return false;
}

bool SatelliteComm::initializeSwarm() {
    Serial.println("Initializing Swarm M138 module");
    
    // Swarm uses different command structure
    satSerial.println("$CS*");
    delay(SATELLITE_RESPONSE_DELAY);
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        if (response.indexOf("$CS") != -1) {
            Serial.println("Swarm module responding");
            configureSwarm();
            return true;
        }
    }
    return false;
}

bool SatelliteComm::initializeRockBlock() {
    Serial.println("Initializing RockBLOCK module");
    
    // RockBLOCK uses Iridium protocol
    return initializeIridium();
}

void SatelliteComm::configureIridium() {
    // Enable network registration notifications
    satSerial.println("AT+CIER=1,1,1,1");
    delay(500);
    
    // Check signal quality
    checkSignalQuality();
}

void SatelliteComm::configureSwarm() {
    // Configure Swarm-specific settings
    satSerial.println("$RT RSSI*");
    delay(500);
    
    checkSignalQuality();
}

void SatelliteComm::wakeUpModule() {
    digitalWrite(SAT_SLEEP_PIN, HIGH);
    delay(SATELLITE_WAKEUP_DELAY);
    digitalWrite(SAT_SLEEP_PIN, LOW);
    delay(SATELLITE_RESPONSE_DELAY); // Wait for module to wake up
}

bool SatelliteComm::checkSatelliteAvailability() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CIER?");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            // Check for satellite network registration
            return response.indexOf("+CIEV:0,1") != -1;
        }
    } else if (moduleType == MODULE_SWARM) {
        // Swarm has different availability checking
        satSerial.println("$GS*");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            return response.indexOf("$GS 1") != -1;
        }
    }
    
    return false;
}

int SatelliteComm::checkSignalQuality() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CSQ");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            int csqIndex = response.indexOf("+CSQ:");
            if (csqIndex != -1) {
                int signalStrength = response.substring(csqIndex + 5).toInt();
                Serial.printf("Satellite signal strength: %d\n", signalStrength);
                return signalStrength;
            }
        }
    } else if (moduleType == MODULE_SWARM) {
        satSerial.println("$RT RSSI*");
        delay(2000);
        
        if (satSerial.available()) {
            String response = satSerial.readString();
            // Parse Swarm RSSI response
            int rssiIndex = response.indexOf("RSSI");
            if (rssiIndex != -1) {
                // Extract RSSI value
                return response.substring(rssiIndex + 5).toInt();
            }
        }
    }
    
    return -1;
}

bool SatelliteComm::sendMessage(const String& message, SatelliteMessagePriority priority) {
    if (!config.enabled) {
        Serial.println("Satellite communication disabled");
        return false;
    }
    
    // Check daily message limit
    if (messagesSentToday >= maxDailyMessages && priority != SAT_PRIORITY_EMERGENCY) {
        Serial.println("Daily message limit exceeded");
        return storeMessage(message, priority);
    }
    
    // Check cost limit
    if (!isWithinCostLimit() && priority != SAT_PRIORITY_EMERGENCY) {
        Serial.println("Daily cost limit reached");
        return storeMessage(message, priority);
    }
    
    // Check if we should transmit now based on priority and optimal windows
    if (!shouldTransmitNow(priority)) {
        return storeMessage(message, priority);
    }
    
    // Check satellite availability
    if (!checkSatelliteAvailability() && priority != SAT_PRIORITY_EMERGENCY) {
        return storeMessage(message, priority);
    }
    
    return sendWithRetry(message, MAX_RETRY_ATTEMPTS);
}

bool SatelliteComm::sendWildlifeAlert(const String& alertData) {
    String alertMessage = "WILDLIFE:" + alertData;
    return sendMessage(alertMessage, SAT_PRIORITY_HIGH);
}

bool SatelliteComm::sendThumbnail(const uint8_t* thumbnailData, size_t size) {
    if (size > SATELLITE_THUMBNAIL_MAX_SIZE) {
        Serial.println("Thumbnail too large for satellite transmission");
        return false;
    }
    
    String thumbnailMessage = "THUMB:";
    for (size_t i = 0; i < size; i++) {
        thumbnailMessage += String(thumbnailData[i], HEX);
        if (i < size - 1) thumbnailMessage += ",";
    }
    
    return sendMessage(thumbnailMessage, SAT_PRIORITY_NORMAL);
}

bool SatelliteComm::sendStatusReport() {
    String statusMessage = createStatusMessage();
    return sendMessage(statusMessage, SAT_PRIORITY_LOW);
}

bool SatelliteComm::receiveConfigUpdate() {
    // Check for incoming configuration updates
    if (satSerial.available()) {
        String incomingData = satSerial.readString();
        
        if (incomingData.indexOf("CONFIG:") != -1) {
            Serial.println("Configuration update received: " + incomingData);
            // Parse and apply configuration update
            // This would parse the incoming configuration and update settings
            return true;
        }
    }
    return false;
}

bool SatelliteComm::sendIridiumMessage(const String& message) {
    // Prepare message for SBD (Short Burst Data)
    String hexMessage = stringToHex(message);
    
    // Clear mobile originated buffer
    satSerial.println("AT+SBDD0");
    delay(1000);
    
    // Write message to buffer
    satSerial.println("AT+SBDWB=" + String(hexMessage.length() / 2));
    delay(1000);
    
    // Send hex data
    for (int i = 0; i < hexMessage.length(); i += 2) {
        String hexByte = hexMessage.substring(i, i + 2);
        satSerial.write(strtol(hexByte.c_str(), NULL, 16));
    }
    
    delay(1000);
    
    // Initiate SBD session
    satSerial.println("AT+SBDI");
    delay(30000); // SBD transmission can take up to 30 seconds
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        Serial.println("SBD Response: " + response);
        
        // Check for successful transmission (SBDI response format: +SBDI:0,x,y,z)
        return response.indexOf("+SBDI:0") != -1;
    }
    
    return false;
}

bool SatelliteComm::sendSwarmMessage(const String& message) {
    // Swarm message format
    String swarmCommand = "$TD \"" + message + "\"*";
    
    satSerial.println(swarmCommand);
    delay(10000); // Wait for transmission
    
    if (satSerial.available()) {
        String response = satSerial.readString();
        Serial.println("Swarm Response: " + response);
        
        // Check for successful transmission
        return response.indexOf("$TD OK") != -1;
    }
    
    return false;
}

bool SatelliteComm::storeMessage(const String& message, SatelliteMessagePriority priority) {
    // Store message in EEPROM or flash for later transmission
    storedMessages.push_back(message);
    storedPriorities.push_back(priority);
    Serial.printf("Message stored for later transmission (Priority: %d)\n", priority);
    return true;
}

bool SatelliteComm::sendStoredMessages() {
    if (storedMessages.empty()) {
        return true;
    }
    
    if (!checkSatelliteAvailability()) {
        Serial.println("No satellite connectivity for stored messages");
        return false;
    }
    
    int sentCount = 0;
    auto msgIt = storedMessages.begin();
    auto prioIt = storedPriorities.begin();
    
    while (msgIt != storedMessages.end() && 
           prioIt != storedPriorities.end() &&
           messagesSentToday < maxDailyMessages &&
           isWithinCostLimit()) {
        
        if (sendMessage(*msgIt, *prioIt)) {
            msgIt = storedMessages.erase(msgIt);
            prioIt = storedPriorities.erase(prioIt);
            sentCount++;
        } else {
            ++msgIt;
            ++prioIt;
        }
        
        delay(5000); // Wait between transmissions
    }
    
    Serial.printf("Sent %d stored messages\n", sentCount);
    return sentCount > 0;
}

String SatelliteComm::stringToHex(const String& input) {
    String hex = "";
    for (int i = 0; i < input.length(); i++) {
        char hexByte[3];
        sprintf(hexByte, "%02X", input.charAt(i));
        hex += hexByte;
    }
    return hex;
}

bool SatelliteComm::predictNextPass() {
    // Simplified satellite pass prediction
    // In a real implementation, this would use orbital mechanics
    // and current satellite constellation data
    
    time_t now = time(nullptr);
    
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        // Iridium satellites pass every 100-110 minutes
        lastPassTime = now + (100 * 60); // Next pass in ~100 minutes
    } else if (moduleType == MODULE_SWARM) {
        // Swarm satellites have different orbital patterns
        lastPassTime = now + (90 * 60); // Next pass in ~90 minutes
    }
    
    Serial.printf("Next satellite pass predicted at: %ld\n", lastPassTime);
    return true;
}

time_t SatelliteComm::getNextPassTime() {
    return lastPassTime;
}

void SatelliteComm::enterSleepMode() {
    if (moduleType == MODULE_IRIDIUM || moduleType == MODULE_ROCKBLOCK) {
        satSerial.println("AT+CPWROFF");
        delay(1000);
    }
    
    digitalWrite(SAT_SLEEP_PIN, HIGH);
}

void SatelliteComm::exitSleepMode() {
    wakeUpModule();
    
    // Re-establish communication
    delay(COMMUNICATION_SETUP_DELAY);
    
    switch (moduleType) {
        case MODULE_IRIDIUM:
        case MODULE_ROCKBLOCK:
            initializeIridium();
            break;
        case MODULE_SWARM:
            initializeSwarm();
            break;
    }
}

size_t SatelliteComm::getMessageCount() {
    return messagesSentToday;
}

void SatelliteComm::resetMessageCount() {
    messagesSentToday = 0;
}

void SatelliteComm::setMessageLimit(size_t limit) {
    maxDailyMessages = limit;
    config.maxDailyMessages = limit;
}

// New enhanced methods implementation

TransmissionWindow SatelliteComm::getOptimalTransmissionWindow() {
    return calculateOptimalWindow(SAT_PRIORITY_NORMAL);
}

bool SatelliteComm::isInLowPowerMode() const {
    return lowPowerMode;
}

void SatelliteComm::optimizePowerConsumption(uint8_t batteryLevel) {
    if (batteryLevel < LOW_BATTERY_SATELLITE_THRESHOLD) {
        if (!lowPowerMode) {
            Serial.println("Entering satellite low power mode");
            lowPowerMode = true;
            // Reduce transmission frequency in low power mode
            config.transmissionInterval *= 2;
        }
    } else if (batteryLevel > (LOW_BATTERY_SATELLITE_THRESHOLD + 10)) {
        if (lowPowerMode) {
            Serial.println("Exiting satellite low power mode");
            lowPowerMode = false;
            // Restore normal transmission frequency
            config.transmissionInterval /= 2;
        }
    }
}

size_t SatelliteComm::getStoredMessageCount() {
    return storedMessages.size();
}

float SatelliteComm::getTodayCost() const {
    return todayCost;
}

bool SatelliteComm::isWithinCostLimit() const {
    return todayCost < config.maxDailyCost;
}

bool SatelliteComm::sendEmergencyAlert(const String& alertData) {
    String emergencyMessage = "EMERGENCY:" + alertData;
    return sendMessage(emergencyMessage, SAT_PRIORITY_EMERGENCY);
}

void SatelliteComm::enableEmergencyMode() {
    emergencyMode = true;
    Serial.println("Satellite emergency mode enabled");
}

void SatelliteComm::disableEmergencyMode() {
    emergencyMode = false;
    Serial.println("Satellite emergency mode disabled");
}

bool SatelliteComm::isEmergencyMode() const {
    return emergencyMode;
}

bool SatelliteComm::sendWithRetry(const String& message, int maxAttempts) {
    String compressedMessage = config.enableMessageCompression ? compressMessage(message) : message;
    
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        bool success = false;
        float cost = 0.0;
        
        switch (moduleType) {
            case MODULE_IRIDIUM:
            case MODULE_ROCKBLOCK:
                success = sendIridiumMessage(compressedMessage);
                cost = IRIDIUM_COST_PER_MESSAGE;
                break;
            case MODULE_SWARM:
                success = sendSwarmMessage(compressedMessage);
                cost = SWARM_COST_PER_MESSAGE;
                break;
            default:
                return false;
        }
        
        if (success) {
            messagesSentToday++;
            updateCost(cost);
            lastTransmissionTime = millis();
            logTransmission(true, message, cost);
            Serial.printf("Message sent successfully on attempt %d. Daily count: %d/%d, Cost: $%.2f\n", 
                         attempt, messagesSentToday, maxDailyMessages, todayCost);
            return true;
        }
        
        logTransmission(false, message, 0.0);
        
        if (attempt < maxAttempts) {
            unsigned long retryDelay = calculateNextRetryDelay(attempt);
            Serial.printf("Transmission failed, retrying in %lu seconds (attempt %d/%d)\n", 
                         retryDelay / 1000, attempt, maxAttempts);
            delay(retryDelay);
        }
    }
    
    Serial.printf("Failed to send message after %d attempts\n", maxAttempts);
    return false;
}

void SatelliteComm::updateCost(float messageCost) {
    todayCost += messageCost;
}

bool SatelliteComm::shouldTransmitNow(SatelliteMessagePriority priority) {
    if (priority == SAT_PRIORITY_EMERGENCY) {
        return true;
    }
    
    if (emergencyMode && priority >= SAT_PRIORITY_HIGH) {
        return true;
    }
    
    if (!config.useScheduledTransmission) {
        return true;
    }
    
    TransmissionWindow window = calculateOptimalWindow(priority);
    return window == WINDOW_IMMEDIATE || window == WINDOW_EMERGENCY;
}

TransmissionWindow SatelliteComm::calculateOptimalWindow(SatelliteMessagePriority priority) {
    if (priority == SAT_PRIORITY_EMERGENCY || emergencyMode) {
        return WINDOW_EMERGENCY;
    }
    
    if (config.autoPassPrediction && isOptimalTransmissionTime()) {
        return WINDOW_IMMEDIATE;
    }
    
    if (priority == SAT_PRIORITY_HIGH) {
        return WINDOW_NEXT_PASS;
    }
    
    return WINDOW_SCHEDULED;
}

void SatelliteComm::updatePassPrediction() {
    time_t now = time(nullptr);
    
    if (now - lastPassPrediction > PASS_PREDICTION_WINDOW) {
        predictNextPass();
        lastPassPrediction = now;
    }
}

bool SatelliteComm::isOptimalTransmissionTime() {
    updatePassPrediction();
    time_t now = time(nullptr);
    
    // Check if we're within a good transmission window relative to satellite pass
    return (lastPassTime - now) < 300; // Within 5 minutes of predicted pass
}

unsigned long SatelliteComm::calculateNextRetryDelay(int attemptNumber) {
    unsigned long delay = RETRY_BASE_DELAY;
    
    // Exponential backoff with jitter
    for (int i = 1; i < attemptNumber; i++) {
        delay *= RETRY_MULTIPLIER;
    }
    
    // Cap at maximum delay
    if (delay > RETRY_MAX_DELAY) {
        delay = RETRY_MAX_DELAY;
    }
    
    // Add random jitter (±25%)
    unsigned long jitter = delay / 4;
    delay += random(-jitter, jitter);
    
    return delay;
}

String SatelliteComm::createStatusMessage() {
    String status = "STATUS:";
    status += "BAT=" + String(analogRead(A0) * 3.3 / 4095.0, 2) + "V,";
    status += "MSG=" + String(messagesSentToday) + "/" + String(maxDailyMessages) + ",";
    status += "COST=$" + String(todayCost, 2) + ",";
    status += "SIGNAL=" + String(checkSignalQuality()) + ",";
    status += "STORED=" + String(storedMessages.size());
    
    return status;
}

String SatelliteComm::compressMessage(const String& message) {
    // Simple compression: remove unnecessary spaces and use abbreviations
    String compressed = message;
    compressed.replace("WILDLIFE", "WL");
    compressed.replace("EMERGENCY", "EM");
    compressed.replace("STATUS", "ST");
    compressed.replace("THUMBNAIL", "TH");
    compressed.replace("  ", " ");
    compressed.trim();
    
    return compressed;
}

void SatelliteComm::logTransmission(bool success, const String& message, float cost) {
    Serial.printf("[SAT_LOG] %s: %s (Cost: $%.2f)\n", 
                  success ? "SUCCESS" : "FAILED", 
                  message.substring(0, 20).c_str(), 
                  cost);
}