/**
 * @file cellular_manager.cpp
 * @brief Cellular communication manager for ESP32 Wildlife Camera
 * 
 * Handles LTE Cat-M1/NB-IoT communication with fallback to 2G/3G/4G networks.
 * Includes SMS backup functionality and automatic carrier switching.
 */

#include "cellular_manager.h"
#include "config.h"
#include <Arduino.h>
#include <HardwareSerial.h>

// Pin definitions for SIM7600/SIM7000 modules
#define SIM_RX_PIN 16
#define SIM_TX_PIN 17
#define SIM_POWER_PIN 18
#define SIM_RST_PIN 19

CellularManager::CellularManager() : 
    modemSerial(2),
    isConnected(false),
    networkType(NETWORK_NONE),
    dataUsedMB(0),
    maxDailyDataMB(100) {
}

bool CellularManager::initialize() {
    // Initialize serial communication with modem
    modemSerial.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    
    // Power on the modem
    pinMode(SIM_POWER_PIN, OUTPUT);
    pinMode(SIM_RST_PIN, OUTPUT);
    
    powerOnModem();
    
    // Test AT communication
    if (!testATCommands()) {
        Serial.println("Failed to communicate with cellular modem");
        return false;
    }
    
    // Configure network preferences
    configureNetworkPreferences();
    
    Serial.println("Cellular manager initialized successfully");
    return true;
}

void CellularManager::powerOnModem() {
    digitalWrite(SIM_POWER_PIN, HIGH);
    delay(100);
    digitalWrite(SIM_POWER_PIN, LOW);
    delay(1000);
    digitalWrite(SIM_POWER_PIN, HIGH);
    delay(3000); // Wait for modem to boot
}

bool CellularManager::testATCommands() {
    for (int i = 0; i < 5; i++) {
        modemSerial.println("AT");
        delay(1000);
        if (modemSerial.available()) {
            String response = modemSerial.readString();
            if (response.indexOf("OK") != -1) {
                return true;
            }
        }
    }
    return false;
}

void CellularManager::configureNetworkPreferences() {
    // Set preferred network modes: LTE Cat-M1 > NB-IoT > 2G/3G/4G
    modemSerial.println("AT+CNMP=38"); // LTE only
    delay(1000);
    modemSerial.println("AT+CMNB=1");  // Cat-M1
    delay(1000);
}

bool CellularManager::connectToNetwork() {
    // Check SIM card status
    if (!checkSIMStatus()) {
        Serial.println("SIM card not ready");
        return false;
    }
    
    // Wait for network registration
    if (!waitForNetworkRegistration()) {
        // Try fallback networks
        return attemptFallbackConnection();
    }
    
    // Establish data connection
    if (establishDataConnection()) {
        isConnected = true;
        detectNetworkType();
        Serial.println("Connected to cellular network");
        return true;
    }
    
    return false;
}

bool CellularManager::checkSIMStatus() {
    modemSerial.println("AT+CPIN?");
    delay(1000);
    
    if (modemSerial.available()) {
        String response = modemSerial.readString();
        return response.indexOf("READY") != -1;
    }
    return false;
}

bool CellularManager::waitForNetworkRegistration() {
    for (int i = 0; i < 30; i++) { // Wait up to 30 seconds
        modemSerial.println("AT+CREG?");
        delay(1000);
        
        if (modemSerial.available()) {
            String response = modemSerial.readString();
            if (response.indexOf(",1") != -1 || response.indexOf(",5") != -1) {
                return true; // Registered
            }
        }
    }
    return false;
}

bool CellularManager::attemptFallbackConnection() {
    Serial.println("Attempting fallback to 2G/3G/4G");
    
    // Switch to 2G/3G/4G mode
    modemSerial.println("AT+CNMP=13"); // GSM/WCDMA/LTE auto
    delay(2000);
    
    return waitForNetworkRegistration() && establishDataConnection();
}

bool CellularManager::establishDataConnection() {
    // Configure APN (this should be configurable for different carriers)
    modemSerial.println("AT+CGDCONT=1,\"IP\",\"iot.provider.com\"");
    delay(1000);
    
    // Activate context
    modemSerial.println("AT+CGACT=1,1");
    delay(COMMUNICATION_SETUP_DELAY);
    
    // Check if context is active
    modemSerial.println("AT+CGACT?");
    delay(1000);
    
    if (modemSerial.available()) {
        String response = modemSerial.readString();
        return response.indexOf("1,1") != -1;
    }
    return false;
}

void CellularManager::detectNetworkType() {
    modemSerial.println("AT+COPS?");
    delay(1000);
    
    if (modemSerial.available()) {
        String response = modemSerial.readString();
        if (response.indexOf("LTE") != -1) {
            networkType = NETWORK_LTE;
        } else if (response.indexOf("GSM") != -1) {
            networkType = NETWORK_2G;
        } else if (response.indexOf("WCDMA") != -1) {
            networkType = NETWORK_3G;
        } else {
            networkType = NETWORK_UNKNOWN;
        }
    }
}

bool CellularManager::sendData(const uint8_t* data, size_t length) {
    if (!isConnected) {
        Serial.println("Not connected to cellular network");
        return false;
    }
    
    // Check data usage limits
    if (dataUsedMB >= maxDailyDataMB) {
        Serial.println("Daily data limit exceeded");
        return false;
    }
    
    // Implement HTTP/MQTT data transmission here
    // This is a simplified implementation
    size_t dataSizeMB = length / (1024 * 1024);
    dataUsedMB += dataSizeMB;
    
    Serial.printf("Sent %d bytes via cellular (Total: %d MB)\n", length, dataUsedMB);
    return true;
}

bool CellularManager::sendSMS(const String& phoneNumber, const String& message) {
    if (message.length() > 160) {
        Serial.println("SMS message too long");
        return false;
    }
    
    // Set SMS text mode
    modemSerial.println("AT+CMGF=1");
    delay(1000);
    
    // Set phone number
    modemSerial.println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    
    // Send message
    modemSerial.print(message);
    modemSerial.write(26); // Ctrl+Z to send
    delay(COMMUNICATION_SETUP_DELAY);
    
    if (modemSerial.available()) {
        String response = modemSerial.readString();
        return response.indexOf("OK") != -1;
    }
    
    return false;
}

int CellularManager::getSignalStrength() {
    modemSerial.println("AT+CSQ");
    delay(1000);
    
    if (modemSerial.available()) {
        String response = modemSerial.readString();
        int csqIndex = response.indexOf("+CSQ: ");
        if (csqIndex != -1) {
            int rssi = response.substring(csqIndex + 6, response.indexOf(",", csqIndex)).toInt();
            return rssi;
        }
    }
    return -1;
}

NetworkType CellularManager::getNetworkType() {
    return networkType;
}

bool CellularManager::isNetworkConnected() {
    return isConnected;
}

void CellularManager::enterSleepMode() {
    if (isConnected) {
        modemSerial.println("AT+CFUN=0"); // Minimum functionality mode
        delay(1000);
    }
}

void CellularManager::exitSleepMode() {
    modemSerial.println("AT+CFUN=1"); // Full functionality mode
    delay(2000);
    connectToNetwork();
}

size_t CellularManager::getDataUsage() {
    return dataUsedMB;
}

void CellularManager::resetDataUsage() {
    dataUsedMB = 0;
}

void CellularManager::setDataLimit(size_t limitMB) {
    maxDailyDataMB = limitMB;
}