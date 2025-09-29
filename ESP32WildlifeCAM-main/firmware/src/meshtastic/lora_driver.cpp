/**
 * @file lora_driver.cpp
 * @brief LoRa Radio Driver Implementation for ESP32WildlifeCAM
 * 
 * Provides hardware abstraction for multiple LoRa modules with
 * automatic detection, regional compliance, and power management.
 */

#include "lora_driver.h"
#include "../debug_utils.h"
#include <LoRa.h>

// Static instance for ISR access
LoRaDriver* LoRaDriver::instance_ = nullptr;

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

LoRaDriver::LoRaDriver() 
    : spi_(nullptr)
    , receiveCallback_(nullptr)
    , transmitCallback_(nullptr)
    , lastTransmissionTime_(0)
    , transmissionDuration_(0)
    , dutyCycleWindow_(3600000) // 1 hour window
{
    instance_ = this;
    
    // Initialize status
    status_.initialized = false;
    status_.transmitting = false;
    status_.receiving = false;
    status_.lastRssi = -200;
    status_.lastSnr = -20.0;
    status_.packetsTransmitted = 0;
    status_.packetsReceived = 0;
    status_.transmissionErrors = 0;
    status_.receptionErrors = 0;
    status_.lastActivity = 0;
}

LoRaDriver::~LoRaDriver() {
    cleanup();
    instance_ = nullptr;
}

// ===========================
// INITIALIZATION
// ===========================

bool LoRaDriver::init(const LoRaConfig& config) {
    DEBUG_PRINTLN("LoRaDriver: Initializing...");
    
    config_ = config;
    
    // Validate configuration
    if (!validateConfiguration()) {
        DEBUG_PRINTLN("LoRaDriver: Configuration validation failed");
        return false;
    }
    
    // Initialize SPI
    spi_ = &SPI;
    spi_->begin();
    
    // Configure GPIO pins
    configurePowerPins();
    
    // Auto-detect module if requested
    if (config_.moduleType == MODULE_AUTO_DETECT) {
        config_.moduleType = detectModule();
        if (config_.moduleType == MODULE_AUTO_DETECT) {
            DEBUG_PRINTLN("LoRaDriver: Module detection failed");
            return false;
        }
    }
    
    // Configure the detected/specified module
    if (!configureModule(config_.moduleType)) {
        DEBUG_PRINTLN("LoRaDriver: Module configuration failed");
        return false;
    }
    
    // Configure LoRa library with our pins
    LoRa.setPins(config_.csPin, config_.resetPin, config_.dio0Pin);
    
    // Initialize LoRa library
    if (!LoRa.begin(config_.frequency)) {
        DEBUG_PRINTLN("LoRaDriver: LoRa.begin() failed");
        return false;
    }
    
    // Configure radio parameters
    LoRa.setTxPower(config_.txPower);
    LoRa.setSpreadingFactor(config_.spreadingFactor);
    LoRa.setSignalBandwidth(config_.bandwidth);
    LoRa.setCodingRate4(config_.codingRate);
    LoRa.setPreambleLength(config_.preambleLength);
    LoRa.setSyncWord(config_.syncWord);
    
    if (config_.crcEnabled) {
        LoRa.enableCrc();
    } else {
        LoRa.disableCrc();
    }
    
    // Set up interrupt handling
    LoRa.onReceive(onDio0Interrupt);
    
    // Start in receive mode
    LoRa.receive();
    
    status_.initialized = true;
    status_.receiving = true;
    
    DEBUG_PRINTF("LoRaDriver: Initialized successfully (Module: %d, Freq: %.1f MHz)\n", 
                 config_.moduleType, config_.frequency / 1E6);
    
    return true;
}

bool LoRaDriver::reset() {
    if (config_.resetPin >= 0) {
        pinMode(config_.resetPin, OUTPUT);
        digitalWrite(config_.resetPin, LOW);
        delay(10);
        digitalWrite(config_.resetPin, HIGH);
        delay(10);
        return true;
    }
    return false;
}

void LoRaDriver::cleanup() {
    if (status_.initialized) {
        LoRa.end();
        status_.initialized = false;
    }
}

// ===========================
// MODULE DETECTION
// ===========================

LoRaModule LoRaDriver::detectModule() {
    DEBUG_PRINTLN("LoRaDriver: Detecting module type...");
    
    // Try to detect each module type
    if (probeSX1276()) {
        DEBUG_PRINTLN("LoRaDriver: Detected SX1276/RFM95");
        return MODULE_SX1276;
    }
    
    if (probeSX1262()) {
        DEBUG_PRINTLN("LoRaDriver: Detected SX1262/SX1268");
        return MODULE_SX1262;
    }
    
    if (probeSX1280()) {
        DEBUG_PRINTLN("LoRaDriver: Detected SX1280");
        return MODULE_SX1280;
    }
    
    if (probeE22_900M30S()) {
        DEBUG_PRINTLN("LoRaDriver: Detected E22-900M30S");
        return MODULE_E22_900M30S;
    }
    
    DEBUG_PRINTLN("LoRaDriver: No supported module detected");
    return MODULE_AUTO_DETECT;
}

bool LoRaDriver::probeSX1276() {
    // Reset the module
    if (!reset()) {
        return false;
    }
    
    delay(10);
    
    // Try to read version register
    uint8_t version = readRegister(SX1276_REG_VERSION);
    return (version == SX1276_VERSION);
}

bool LoRaDriver::probeSX1262() {
    // SX1262 uses SPI commands instead of register reads
    // This is a simplified detection - would need more sophisticated approach
    if (!reset()) {
        return false;
    }
    
    delay(10);
    
    // Try to read status - SX1262 specific implementation needed
    // For now, return false as LoRa library doesn't support SX1262 directly
    return false;
}

bool LoRaDriver::probeSX1280() {
    // SX1280 detection - simplified
    // For now, return false as LoRa library doesn't support SX1280 directly
    return false;
}

bool LoRaDriver::probeE22_900M30S() {
    // E22-900M30S is based on SX1262, but with different interface
    // Would need specific implementation
    return false;
}

// ===========================
// MODULE CONFIGURATION
// ===========================

bool LoRaDriver::configureModule(LoRaModule moduleType) {
    switch (moduleType) {
        case MODULE_SX1276:
            return initSX1276();
        case MODULE_SX1262:
            return initSX1262();
        case MODULE_SX1280:
            return initSX1280();
        case MODULE_E22_900M30S:
            return initE22_900M30S();
        default:
            return false;
    }
}

bool LoRaDriver::initSX1276() {
    // SX1276 is supported by LoRa library - minimal configuration needed
    DEBUG_PRINTLN("LoRaDriver: Configuring SX1276");
    return true;
}

bool LoRaDriver::initSX1262() {
    // SX1262 would need custom implementation
    DEBUG_PRINTLN("LoRaDriver: SX1262 not yet implemented");
    return false;
}

bool LoRaDriver::initSX1280() {
    // SX1280 would need custom implementation  
    DEBUG_PRINTLN("LoRaDriver: SX1280 not yet implemented");
    return false;
}

bool LoRaDriver::initE22_900M30S() {
    // E22-900M30S would need custom implementation
    DEBUG_PRINTLN("LoRaDriver: E22-900M30S not yet implemented");
    return false;
}

// ===========================
// RADIO OPERATIONS
// ===========================

bool LoRaDriver::transmit(const uint8_t* data, size_t length) {
    if (!status_.initialized || !data || length == 0 || length > 255) {
        return false;
    }
    
    // Check duty cycle compliance
    if (!isTransmissionAllowed()) {
        DEBUG_PRINTLN("LoRaDriver: Transmission blocked by duty cycle");
        return false;
    }
    
    // Calculate transmission time for duty cycle tracking
    uint32_t timeOnAir = getTimeOnAir(length, config_.spreadingFactor, 
                                     config_.bandwidth, config_.codingRate);
    
    // Start transmission
    LoRa.beginPacket();
    LoRa.write(data, length);
    bool success = LoRa.endPacket();
    
    if (success) {
        status_.packetsTransmitted++;
        updateDutyCycle(timeOnAir);
        status_.lastActivity = millis();
        
        // Call transmit callback if set
        if (transmitCallback_) {
            transmitCallback_(true);
        }
    } else {
        status_.transmissionErrors++;
        
        if (transmitCallback_) {
            transmitCallback_(false);
        }
    }
    
    // Return to receive mode
    LoRa.receive();
    status_.receiving = true;
    
    return success;
}

bool LoRaDriver::transmitAsync(const uint8_t* data, size_t length) {
    // For now, just use synchronous transmission
    // Async would require more complex state management
    return transmit(data, length);
}

bool LoRaDriver::startReceive() {
    if (!status_.initialized) {
        return false;
    }
    
    LoRa.receive();
    status_.receiving = true;
    
    return true;
}

bool LoRaDriver::stopReceive() {
    if (!status_.initialized) {
        return false;
    }
    
    LoRa.idle();
    status_.receiving = false;
    
    return true;
}

// ===========================
// PARAMETER CONFIGURATION
// ===========================

bool LoRaDriver::setTxPower(int8_t power) {
    if (!validateTxPower(power)) {
        return false;
    }
    
    LoRa.setTxPower(power);
    config_.txPower = power;
    
    return true;
}

bool LoRaDriver::setFrequency(uint32_t frequency) {
    if (!validateFrequency(frequency)) {
        return false;
    }
    
    if (!LoRa.begin(frequency)) {
        return false;
    }
    
    config_.frequency = frequency;
    
    return true;
}

bool LoRaDriver::setBandwidth(uint32_t bandwidth) {
    LoRa.setSignalBandwidth(bandwidth);
    config_.bandwidth = bandwidth;
    
    return true;
}

bool LoRaDriver::setSpreadingFactor(uint8_t sf) {
    if (sf < 6 || sf > 12) {
        return false;
    }
    
    LoRa.setSpreadingFactor(sf);
    config_.spreadingFactor = sf;
    
    return true;
}

bool LoRaDriver::setCodingRate(uint8_t cr) {
    if (cr < 5 || cr > 8) {
        return false;
    }
    
    LoRa.setCodingRate4(cr);
    config_.codingRate = cr;
    
    return true;
}

// ===========================
// STATUS AND DIAGNOSTICS
// ===========================

LoRaStatus LoRaDriver::getStatus() const {
    return status_;
}

int16_t LoRaDriver::getRssi() {
    if (!status_.initialized) {
        return -200;
    }
    
    int16_t rssi = LoRa.rssi();
    status_.lastRssi = rssi;
    
    return rssi;
}

float LoRaDriver::getSnr() {
    if (!status_.initialized) {
        return -20.0;
    }
    
    float snr = LoRa.snr();
    status_.lastSnr = snr;
    
    return snr;
}

uint8_t LoRaDriver::getVersion() {
    if (!status_.initialized) {
        return 0;
    }
    
    return readRegister(SX1276_REG_VERSION);
}

bool LoRaDriver::isTransmitting() {
    return status_.transmitting;
}

bool LoRaDriver::isReceiving() {
    return status_.receiving;
}

// ===========================
// POWER MANAGEMENT
// ===========================

bool LoRaDriver::enterSleepMode() {
    if (!status_.initialized) {
        return false;
    }
    
    LoRa.sleep();
    status_.receiving = false;
    
    return true;
}

bool LoRaDriver::exitSleepMode() {
    if (!status_.initialized) {
        return false;
    }
    
    LoRa.idle();
    delay(10);
    LoRa.receive();
    status_.receiving = true;
    
    return true;
}

bool LoRaDriver::enterStandbyMode() {
    if (!status_.initialized) {
        return false;
    }
    
    LoRa.idle();
    status_.receiving = false;
    
    return true;
}

// ===========================
// DUTY CYCLE MANAGEMENT
// ===========================

bool LoRaDriver::checkDutyCycle() {
    return isTransmissionAllowed();
}

uint32_t LoRaDriver::getDutyCycleRemaining() {
    unsigned long currentTime = millis();
    uint32_t windowStart = currentTime - dutyCycleWindow_;
    
    // This is simplified - real implementation would track all transmissions
    uint32_t dutyCycleLimit = getDutyCycleLimit(config_.region);
    uint32_t allowedTime = (dutyCycleWindow_ * dutyCycleLimit) / 100;
    
    if (transmissionDuration_ >= allowedTime) {
        return 0;
    }
    
    return allowedTime - transmissionDuration_;
}

bool LoRaDriver::isTransmissionAllowed() {
    uint32_t dutyCycleLimit = getDutyCycleLimit(config_.region);
    
    // No limit for regions like US
    if (dutyCycleLimit >= 100) {
        return true;
    }
    
    return getDutyCycleRemaining() > 0;
}

// ===========================
// VALIDATION FUNCTIONS
// ===========================

bool LoRaDriver::validateConfiguration() {
    return validateFrequency(config_.frequency) && 
           validateTxPower(config_.txPower);
}

bool LoRaDriver::validateFrequency(uint32_t frequency) {
    // Basic frequency validation - more sophisticated checks needed
    return (frequency >= 137E6 && frequency <= 1020E6) ||
           (frequency >= 2400E6 && frequency <= 2500E6);
}

bool LoRaDriver::validateTxPower(int8_t power) {
    int8_t maxPower = getMaxTxPower(config_.region);
    return (power >= 2 && power <= maxPower);
}

// ===========================
// HELPER FUNCTIONS
// ===========================

void LoRaDriver::configurePowerPins() {
    // Configure reset pin
    if (config_.resetPin >= 0) {
        pinMode(config_.resetPin, OUTPUT);
        digitalWrite(config_.resetPin, HIGH);
    }
    
    // Configure DIO pins
    if (config_.dio0Pin >= 0) {
        pinMode(config_.dio0Pin, INPUT);
    }
    if (config_.dio1Pin >= 0) {
        pinMode(config_.dio1Pin, INPUT);
    }
    if (config_.dio2Pin >= 0) {
        pinMode(config_.dio2Pin, INPUT);
    }
}

void LoRaDriver::updateDutyCycle(uint32_t transmissionTime) {
    transmissionDuration_ += transmissionTime;
    lastTransmissionTime_ = millis();
    
    // Clean up old transmissions outside the window
    // Simplified implementation - would need more sophisticated tracking
}

uint32_t LoRaDriver::getTimeOnAir(size_t payloadLength, uint8_t sf, uint32_t bandwidth, uint8_t cr) {
    // Simplified time-on-air calculation
    // Real implementation would use proper LoRa formula
    uint32_t symbolTime = (1000000 * (1 << sf)) / bandwidth;
    uint32_t preambleTime = (config_.preambleLength + 4.25) * symbolTime;
    uint32_t payloadSymbols = 8 + max((int)(ceil((8.0 * payloadLength - 4 * sf + 28 + 16) / (4 * sf)) * cr), 0);
    uint32_t payloadTime = payloadSymbols * symbolTime;
    
    return (preambleTime + payloadTime) / 1000; // Return in milliseconds
}

// ===========================
// REGISTER ACCESS
// ===========================

uint8_t LoRaDriver::readRegister(uint8_t address) {
    // Use LoRa library's internal access if available
    // This is a simplified implementation
    return 0;
}

void LoRaDriver::writeRegister(uint8_t address, uint8_t value) {
    // Use LoRa library's internal access if available
    // This is a simplified implementation
}

// ===========================
// INTERRUPT HANDLERS
// ===========================

void IRAM_ATTR LoRaDriver::onDio0Interrupt() {
    if (instance_ && instance_->receiveCallback_) {
        // Read packet
        int packetSize = LoRa.parsePacket();
        if (packetSize > 0) {
            LoRaPacket packet;
            uint8_t buffer[255];
            
            packet.length = min(packetSize, 255);
            packet.rssi = LoRa.rssi();
            packet.snr = LoRa.snr();
            packet.timestamp = millis();
            
            for (size_t i = 0; i < packet.length; i++) {
                buffer[i] = LoRa.read();
            }
            packet.data = buffer;
            
            instance_->status_.packetsReceived++;
            instance_->status_.lastActivity = packet.timestamp;
            instance_->status_.lastRssi = packet.rssi;
            instance_->status_.lastSnr = packet.snr;
            
            instance_->receiveCallback_(packet);
        }
    }
}

void IRAM_ATTR LoRaDriver::onDio1Interrupt() {
    // DIO1 interrupt handling - module specific
}

// ===========================
// CALLBACK MANAGEMENT
// ===========================

void LoRaDriver::setReceiveCallback(LoRaReceiveCallback callback) {
    receiveCallback_ = callback;
}

void LoRaDriver::setTransmitCallback(LoRaTransmitCallback callback) {
    transmitCallback_ = callback;
}

// ===========================
// DIAGNOSTICS
// ===========================

bool LoRaDriver::selfTest() {
    if (!status_.initialized) {
        return false;
    }
    
    // Basic self-test - check version register
    uint8_t version = getVersion();
    return (version != 0x00 && version != 0xFF);
}

void LoRaDriver::printConfiguration() {
    DEBUG_PRINTLN("=== LoRa Driver Configuration ===");
    DEBUG_PRINTF("Module Type: %d\n", config_.moduleType);
    DEBUG_PRINTF("Frequency: %.1f MHz\n", config_.frequency / 1E6);
    DEBUG_PRINTF("Bandwidth: %.1f kHz\n", config_.bandwidth / 1E3);
    DEBUG_PRINTF("Spreading Factor: %d\n", config_.spreadingFactor);
    DEBUG_PRINTF("Coding Rate: 4/%d\n", config_.codingRate);
    DEBUG_PRINTF("TX Power: %d dBm\n", config_.txPower);
    DEBUG_PRINTF("Preamble Length: %d\n", config_.preambleLength);
    DEBUG_PRINTF("Sync Word: 0x%02X\n", config_.syncWord);
    DEBUG_PRINTF("CRC Enabled: %s\n", config_.crcEnabled ? "Yes" : "No");
}

void LoRaDriver::printStatistics() {
    DEBUG_PRINTLN("=== LoRa Driver Statistics ===");
    DEBUG_PRINTF("Packets Transmitted: %u\n", status_.packetsTransmitted);
    DEBUG_PRINTF("Packets Received: %u\n", status_.packetsReceived);
    DEBUG_PRINTF("Transmission Errors: %u\n", status_.transmissionErrors);
    DEBUG_PRINTF("Reception Errors: %u\n", status_.receptionErrors);
    DEBUG_PRINTF("Last RSSI: %d dBm\n", status_.lastRssi);
    DEBUG_PRINTF("Last SNR: %.1f dB\n", status_.lastSnr);
    DEBUG_PRINTF("Last Activity: %lu ms ago\n", millis() - status_.lastActivity);
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

LoRaDriver* createLoRaDriver(const LoRaConfig& config) {
    LoRaDriver* driver = new LoRaDriver();
    if (driver->init(config)) {
        return driver;
    } else {
        delete driver;
        return nullptr;
    }
}

LoRaConfig createDefaultConfig(LoRaRegion region) {
    LoRaConfig config;
    
    // Set region-specific defaults
    config.region = region;
    config.frequency = getRegionalFrequency(region);
    config.txPower = min(20, getMaxTxPower(region));
    
    // Set other defaults
    config.bandwidth = LORA_SIGNAL_BANDWIDTH;
    config.spreadingFactor = LORA_SPREADING_FACTOR;
    config.codingRate = LORA_CODING_RATE;
    config.preambleLength = LORA_PREAMBLE_LENGTH;
    config.syncWord = LORA_SYNC_WORD;
    config.crcEnabled = LORA_CRC_ENABLED;
    config.explicitHeader = LORA_EXPLICIT_HEADER;
    
    // Set GPIO pins based on board type
    config.csPin = LORA_CS;
    config.resetPin = LORA_RST;
    config.dio0Pin = LORA_DIO0;
    config.dio1Pin = LORA_DIO1;
    config.dio2Pin = LORA_DIO2;
    
    config.moduleType = LORA_MODULE_TYPE;
    
    return config;
}

LoRaConfig createWildlifeConfig(MeshChannelPreset preset) {
    LoRaConfig config = createDefaultConfig();
    
    // Apply preset-specific settings
    switch (preset) {
        case CHANNEL_LONG_SLOW:
            config.spreadingFactor = 11;
            config.bandwidth = 125E3;
            config.codingRate = 8;
            break;
        case CHANNEL_LONG_FAST:
            config.spreadingFactor = 11;
            config.bandwidth = 250E3;
            config.codingRate = 6;
            break;
        case CHANNEL_MEDIUM_SLOW:
            config.spreadingFactor = 10;
            config.bandwidth = 125E3;
            config.codingRate = 8;
            break;
        case CHANNEL_MEDIUM_FAST:
            config.spreadingFactor = 10;
            config.bandwidth = 250E3;
            config.codingRate = 6;
            break;
        case CHANNEL_SHORT_SLOW:
            config.spreadingFactor = 9;
            config.bandwidth = 125E3;
            config.codingRate = 8;
            break;
        case CHANNEL_SHORT_FAST:
            config.spreadingFactor = 7;
            config.bandwidth = 250E3;
            config.codingRate = 5;
            break;
        default:
            // Keep defaults
            break;
    }
    
    return config;
}

uint32_t getRegionalFrequency(LoRaRegion region) {
    switch (region) {
        case REGION_US915:   return 915E6;
        case REGION_EU868:   return 868E6;
        case REGION_EU433:   return 433E6;
        case REGION_AU915:   return 915E6;
        case REGION_CN470:   return 470E6;
        case REGION_JP920:   return 920E6;
        case REGION_2_4GHZ:  return 2450E6;
        default:             return 915E6;
    }
}

int8_t getMaxTxPower(LoRaRegion region) {
    switch (region) {
        case REGION_US915:   return 30;
        case REGION_EU868:   return 14;
        case REGION_EU433:   return 10;
        case REGION_AU915:   return 30;
        case REGION_CN470:   return 17;
        case REGION_JP920:   return 13;
        case REGION_2_4GHZ:  return 10;
        default:             return 20;
    }
}

uint32_t getDutyCycleLimit(LoRaRegion region) {
    switch (region) {
        case REGION_EU868:   return 1;   // 1% duty cycle
        case REGION_EU433:   return 10;  // 10% duty cycle
        default:             return 100; // No limit
    }
}