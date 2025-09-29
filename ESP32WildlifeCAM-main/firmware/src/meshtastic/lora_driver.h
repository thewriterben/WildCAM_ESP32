/**
 * @file lora_driver.h
 * @brief LoRa Radio Driver Header for ESP32WildlifeCAM
 * 
 * Provides hardware abstraction for multiple LoRa modules:
 * - SX1276/RFM95/RFM96/RFM98 
 * - SX1262/SX1268
 * - SX1280 (2.4 GHz)
 * - E22-900M30S
 */

#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <Arduino.h>
#include <SPI.h>
#include "mesh_config.h"

// ===========================
// DRIVER STRUCTURES
// ===========================

// LoRa radio configuration
struct LoRaConfig {
    // Radio parameters
    uint32_t frequency;
    uint32_t bandwidth;
    uint8_t spreadingFactor;
    uint8_t codingRate;
    int8_t txPower;
    uint16_t preambleLength;
    uint8_t syncWord;
    bool crcEnabled;
    bool explicitHeader;
    
    // GPIO configuration
    int8_t csPin;
    int8_t resetPin;
    int8_t dio0Pin;
    int8_t dio1Pin;
    int8_t dio2Pin;
    
    // Module type
    LoRaModule moduleType;
    LoRaRegion region;
};

// Radio status information
struct LoRaStatus {
    bool initialized;
    bool transmitting;
    bool receiving;
    int16_t lastRssi;
    float lastSnr;
    uint32_t packetsTransmitted;
    uint32_t packetsReceived;
    uint32_t transmissionErrors;
    uint32_t receptionErrors;
    unsigned long lastActivity;
};

// Packet structure
struct LoRaPacket {
    uint8_t* data;
    size_t length;
    int16_t rssi;
    float snr;
    unsigned long timestamp;
};

// Callback function types
typedef void (*LoRaReceiveCallback)(LoRaPacket packet);
typedef void (*LoRaTransmitCallback)(bool success);

// ===========================
// DRIVER CLASS
// ===========================

class LoRaDriver {
public:
    LoRaDriver();
    ~LoRaDriver();
    
    // Initialization and configuration
    bool init(const LoRaConfig& config);
    bool reset();
    void cleanup();
    
    // Module detection and configuration
    LoRaModule detectModule();
    bool configureModule(LoRaModule moduleType);
    bool validateConfiguration();
    
    // Radio operations
    bool transmit(const uint8_t* data, size_t length);
    bool transmitAsync(const uint8_t* data, size_t length);
    bool receive(uint8_t* buffer, size_t bufferSize, size_t& receivedLength);
    bool startReceive();
    bool stopReceive();
    
    // Power and frequency management
    bool setTxPower(int8_t power);
    bool setFrequency(uint32_t frequency);
    bool setBandwidth(uint32_t bandwidth);
    bool setSpreadingFactor(uint8_t sf);
    bool setCodingRate(uint8_t cr);
    
    // Status and diagnostics
    LoRaStatus getStatus() const;
    int16_t getRssi();
    float getSnr();
    uint8_t getVersion();
    bool isTransmitting();
    bool isReceiving();
    
    // Callback management
    void setReceiveCallback(LoRaReceiveCallback callback);
    void setTransmitCallback(LoRaTransmitCallback callback);
    
    // Power management
    bool enterSleepMode();
    bool exitSleepMode();
    bool enterStandbyMode();
    
    // Advanced features
    bool setCAD(bool enabled);                    // Channel Activity Detection
    bool performCAD();                           // Perform single CAD
    bool setLowDataRateOptimize(bool enabled);   // Low data rate optimization
    bool setAGC(bool enabled);                   // Automatic Gain Control
    
    // Regulatory compliance
    bool checkDutyCycle();
    uint32_t getDutyCycleRemaining();
    bool isTransmissionAllowed();
    
    // Diagnostics and testing
    bool selfTest();
    bool calibrate();
    void printConfiguration();
    void printStatistics();

private:
    // Internal state
    LoRaConfig config_;
    LoRaStatus status_;
    SPIClass* spi_;
    
    // Callbacks
    LoRaReceiveCallback receiveCallback_;
    LoRaTransmitCallback transmitCallback_;
    
    // Duty cycle tracking
    unsigned long lastTransmissionTime_;
    uint32_t transmissionDuration_;
    uint32_t dutyCycleWindow_;
    
    // Module-specific implementations
    bool initSX1276();
    bool initSX1262();
    bool initSX1280();
    bool initE22_900M30S();
    
    // Register access functions
    uint8_t readRegister(uint8_t address);
    void writeRegister(uint8_t address, uint8_t value);
    void readBuffer(uint8_t address, uint8_t* buffer, size_t length);
    void writeBuffer(uint8_t address, const uint8_t* buffer, size_t length);
    
    // Module detection helpers
    bool probeSX1276();
    bool probeSX1262();
    bool probeSX1280();
    bool probeE22_900M30S();
    
    // Configuration helpers
    uint32_t calculateBandwidthValue(uint32_t bandwidth);
    uint8_t calculateCodingRateValue(uint8_t codingRate);
    bool validateFrequency(uint32_t frequency);
    bool validateTxPower(int8_t power);
    
    // Power management helpers
    void configurePowerPins();
    void enablePowerSaving();
    void disablePowerSaving();
    
    // Duty cycle management
    void updateDutyCycle(uint32_t transmissionTime);
    bool checkRegionalCompliance();
    
    // Interrupt handlers
    static void IRAM_ATTR onDio0Interrupt();
    static void IRAM_ATTR onDio1Interrupt();
    
    // Static instance for ISR access
    static LoRaDriver* instance_;
    
    // Module-specific register definitions
    static const uint8_t SX1276_REG_VERSION = 0x42;
    static const uint8_t SX1262_REG_VERSION = 0x0320;
    static const uint8_t SX1280_REG_VERSION = 0x0320;
    
    // Expected version values
    static const uint8_t SX1276_VERSION = 0x12;
    static const uint8_t SX1262_VERSION = 0x00;
    static const uint8_t SX1280_VERSION = 0x00;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function to create driver with auto-detection
LoRaDriver* createLoRaDriver(const LoRaConfig& config);

// Helper functions for configuration
LoRaConfig createDefaultConfig(LoRaRegion region = LORA_REGION);
LoRaConfig createWildlifeConfig(MeshChannelPreset preset = MESH_CHANNEL_PRESET);
bool validateLoRaConfig(const LoRaConfig& config);

// Regional frequency helpers
uint32_t getRegionalFrequency(LoRaRegion region);
int8_t getMaxTxPower(LoRaRegion region);
uint32_t getDutyCycleLimit(LoRaRegion region);

// Bandwidth conversion helpers
uint32_t bandwidthToHz(uint8_t bwIndex);
uint8_t bandwidthFromHz(uint32_t bandwidth);

// Spreading factor utilities
uint32_t getSymbolTime(uint8_t sf, uint32_t bandwidth);
uint32_t getTimeOnAir(size_t payloadLength, uint8_t sf, uint32_t bandwidth, uint8_t cr);

#endif // LORA_DRIVER_H