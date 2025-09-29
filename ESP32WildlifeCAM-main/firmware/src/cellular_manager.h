/**
 * @file cellular_manager.h
 * @brief Header file for cellular communication manager
 */

#ifndef CELLULAR_MANAGER_H
#define CELLULAR_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>

enum NetworkType {
    NETWORK_NONE = 0,
    NETWORK_2G,
    NETWORK_3G,
    NETWORK_LTE,
    NETWORK_CATM1,
    NETWORK_NBIOT,
    NETWORK_UNKNOWN
};

class CellularManager {
public:
    CellularManager();
    
    // Initialization and setup
    bool initialize();
    bool connectToNetwork();
    
    // Data transmission
    bool sendData(const uint8_t* data, size_t length);
    bool sendSMS(const String& phoneNumber, const String& message);
    
    // Network status
    int getSignalStrength();
    NetworkType getNetworkType();
    bool isNetworkConnected();
    
    // Power management
    void enterSleepMode();
    void exitSleepMode();
    
    // Data usage tracking
    size_t getDataUsage();
    void resetDataUsage();
    void setDataLimit(size_t limitMB);

private:
    HardwareSerial modemSerial;
    bool isConnected;
    NetworkType networkType;
    size_t dataUsedMB;
    size_t maxDailyDataMB;
    
    // Internal methods
    void powerOnModem();
    bool testATCommands();
    void configureNetworkPreferences();
    bool checkSIMStatus();
    bool waitForNetworkRegistration();
    bool attemptFallbackConnection();
    bool establishDataConnection();
    void detectNetworkType();
};

#endif // CELLULAR_MANAGER_H