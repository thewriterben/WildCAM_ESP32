#ifndef BLOCKCHAIN_INTEGRATION_H
#define BLOCKCHAIN_INTEGRATION_H

#include <Arduino.h>
#include "BlockchainManager.h"
#include "../data/storage_manager.h"

/**
 * @brief Integration layer for blockchain with existing ESP32 Wildlife CAM components
 * 
 * Provides seamless integration points for blockchain functionality with
 * existing camera, AI, storage, and networking systems.
 */
class BlockchainIntegration {
public:
    /**
     * @brief Integration configuration
     */
    struct IntegrationConfig {
        bool integrateWithStorageManager;    // Integrate with StorageManager
        bool integrateWithAIProcessor;       // Integrate with AI detection
        bool integrateWithCameraCapture;     // Integrate with camera capture
        bool integrateWithNetworking;        // Integrate with networking
        bool integrateWithWebInterface;      // Integrate with web interface
        bool enableAutomaticHashing;         // Enable automatic data hashing
        uint8_t hashFrequency;              // Hashing frequency (1=every, 2=every other, etc.)
        
        IntegrationConfig() : integrateWithStorageManager(true), integrateWithAIProcessor(true),
            integrateWithCameraCapture(true), integrateWithNetworking(false), 
            integrateWithWebInterface(true), enableAutomaticHashing(true), hashFrequency(1) {}
    };

public:
    BlockchainIntegration();
    ~BlockchainIntegration();

    /**
     * @brief Initialize blockchain integration
     * @param blockchainManager Pointer to blockchain manager
     * @param config Integration configuration
     * @return true if initialization successful
     */
    bool initialize(BlockchainManager* blockchainManager, const IntegrationConfig& config);

    /**
     * @brief Handle image capture event
     * @param imageFilename Captured image filename
     * @param metadata Image metadata JSON
     * @return true if handled successfully
     */
    bool handleImageCapture(const String& imageFilename, const String& metadata);

    /**
     * @brief Handle AI detection event
     * @param detectionResults AI detection results JSON
     * @param confidence Detection confidence score
     * @return true if handled successfully
     */
    bool handleAIDetection(const String& detectionResults, float confidence);

    /**
     * @brief Handle sensor data event
     * @param sensorType Sensor type identifier
     * @param sensorData Sensor readings JSON
     * @return true if handled successfully
     */
    bool handleSensorData(const String& sensorType, const String& sensorData);

    /**
     * @brief Handle system event
     * @param eventType System event type
     * @param eventData Event data JSON
     * @return true if handled successfully
     */
    bool handleSystemEvent(const String& eventType, const String& eventData);

    /**
     * @brief Update blockchain integration (call from main loop)
     */
    void update();

    /**
     * @brief Get integration status
     * @return JSON status string
     */
    String getStatus() const;

    /**
     * @brief Enhanced storage operations with blockchain
     */
    class EnhancedStorage {
    public:
        /**
         * @brief Save file with automatic blockchain hashing
         * @param filename File path
         * @param data File data
         * @param metadata File metadata
         * @return true if saved with blockchain hash
         */
        static bool saveFileWithBlockchain(const String& filename, const String& data, const String& metadata);

        /**
         * @brief Verify file against blockchain
         * @param filename File to verify
         * @return true if file is verified in blockchain
         */
        static bool verifyFileWithBlockchain(const String& filename);

        /**
         * @brief Get enhanced storage statistics with blockchain info
         * @return Enhanced storage stats
         */
        static String getEnhancedStorageStats();
    };

    /**
     * @brief AI processor integration hooks
     */
    class AIProcessorHooks {
    public:
        /**
         * @brief Process AI detection with blockchain logging
         * @param imageData Image data for analysis
         * @param detectionResults Output detection results
         * @return true if processed with blockchain logging
         */
        static bool processDetectionWithBlockchain(const String& imageData, String& detectionResults);

        /**
         * @brief Verify AI detection against blockchain
         * @param detectionHash Detection result hash
         * @return true if detection is verified in blockchain
         */
        static bool verifyDetectionWithBlockchain(const String& detectionHash);
    };

    /**
     * @brief Camera capture integration hooks
     */
    class CameraCaptureHooks {
    public:
        /**
         * @brief Capture image with automatic blockchain hashing
         * @param filename Output filename
         * @param metadata Capture metadata
         * @return true if captured with blockchain hash
         */
        static bool captureImageWithBlockchain(const String& filename, const String& metadata);

        /**
         * @brief Enhanced metadata creation with blockchain fields
         * @param basicMetadata Basic capture metadata
         * @return Enhanced metadata with blockchain information
         */
        static String createEnhancedMetadata(const String& basicMetadata);
    };

    /**
     * @brief Web interface integration
     */
    class WebInterfaceHooks {
    public:
        /**
         * @brief Get blockchain status for web interface
         * @return JSON status for web display
         */
        static String getBlockchainStatusForWeb();

        /**
         * @brief Get blockchain verification results for web
         * @param filename File to check
         * @return JSON verification results
         */
        static String getVerificationStatusForWeb(const String& filename);

        /**
         * @brief Handle blockchain configuration from web interface
         * @param configJSON Configuration JSON from web
         * @return true if configuration applied
         */
        static bool handleWebConfiguration(const String& configJSON);
    };

private:
    BlockchainManager* blockchainManager_;
    IntegrationConfig config_;
    bool initialized_;
    uint32_t operationCounter_;
    uint32_t lastUpdateTime_;

    /**
     * @brief Check if operation should be hashed based on frequency
     * @return true if this operation should be hashed
     */
    bool shouldHashOperation();

    /**
     * @brief Log integration event
     * @param event Event description
     */
    void logIntegrationEvent(const String& event);
};

// Global integration instance
extern BlockchainIntegration* g_blockchainIntegration;

// Utility functions for easy integration
namespace BlockchainUtils {
    /**
     * @brief Initialize blockchain system with default configuration
     * @return true if initialization successful
     */
    bool initializeBlockchainSystem();

    /**
     * @brief Quick hash image file
     * @param imageFilename Image file to hash
     * @return Hash string, empty on error
     */
    String quickHashImage(const String& imageFilename);

    /**
     * @brief Quick verify image file
     * @param imageFilename Image file to verify
     * @return true if verified in blockchain
     */
    bool quickVerifyImage(const String& imageFilename);

    /**
     * @brief Get blockchain summary for display
     * @return Summary string
     */
    String getBlockchainSummary();

    /**
     * @brief Check if blockchain is healthy
     * @return true if blockchain system is healthy
     */
    bool isBlockchainHealthy();
}

// Enhanced metadata structure for integration
struct IntegratedMetadata {
    // Basic metadata
    String filename;
    uint32_t timestamp;
    size_t fileSize;
    String mimeType;
    
    // Camera metadata
    String cameraSettings;
    float exposureTime;
    int isoValue;
    String focusMode;
    
    // Environmental metadata
    float temperature;
    float humidity;
    float lightLevel;
    String weatherConditions;
    
    // AI metadata
    String detectedSpecies;
    float confidence;
    String behaviorObserved;
    String aiModelVersion;
    
    // Blockchain metadata
    String dataHash;
    String blockHash;
    uint32_t blockNumber;
    String transactionHash;
    bool blockchainVerified;
    String verificationTime;
    
    IntegratedMetadata() : filename(""), timestamp(0), fileSize(0), mimeType(""),
        cameraSettings(""), exposureTime(0.0), isoValue(0), focusMode(""),
        temperature(0.0), humidity(0.0), lightLevel(0.0), weatherConditions(""),
        detectedSpecies(""), confidence(0.0), behaviorObserved(""), aiModelVersion(""),
        dataHash(""), blockHash(""), blockNumber(0), transactionHash(""),
        blockchainVerified(false), verificationTime("") {}
    
    String toJSON() const;
    bool fromJSON(const String& json);
};

#endif // BLOCKCHAIN_INTEGRATION_H