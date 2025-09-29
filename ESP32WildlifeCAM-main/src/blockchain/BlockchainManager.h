#ifndef BLOCKCHAIN_MANAGER_H
#define BLOCKCHAIN_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "Block.h"
#include "HashingService.h"
#include "MerkleTree.h"

/**
 * @brief Main blockchain orchestration and management system
 * 
 * Coordinates all blockchain operations for the ESP32 Wildlife CAM system,
 * integrating with existing storage, networking, and monitoring components.
 */
class BlockchainManager {
public:
    /**
     * @brief Blockchain configuration structure
     */
    struct BlockchainConfig {
        bool enabled;                           // Enable/disable blockchain
        uint8_t maxTransactionsPerBlock;        // Maximum transactions per block
        uint32_t blockCreationInterval;         // Block creation interval (seconds)
        bool enableImageHashing;                // Hash all captured images
        bool enableAIHashing;                   // Hash AI detection results
        bool enableSensorHashing;               // Hash sensor data
        bool enableSystemEventHashing;          // Hash system events
        uint8_t hashFrequency;                  // Hash frequency (1=every capture, 2=every other, etc.)
        bool enableNetworkSync;                 // Enable blockchain sync with other nodes
        String nodeId;                          // Unique node identifier
        uint32_t maxBlocksInMemory;            // Maximum blocks to keep in memory
        bool enableDigitalSignatures;           // Enable transaction signing
        
        BlockchainConfig() : 
            enabled(true), maxTransactionsPerBlock(10), blockCreationInterval(300),
            enableImageHashing(true), enableAIHashing(true), enableSensorHashing(false),
            enableSystemEventHashing(true), hashFrequency(1), enableNetworkSync(true),
            nodeId("ESP32CAM_001"), maxBlocksInMemory(5), enableDigitalSignatures(false) {}
    };

    /**
     * @brief Blockchain status information
     */
    struct BlockchainStatus {
        bool initialized;                       // System initialized
        bool healthy;                          // System health status
        uint32_t totalBlocks;                  // Total blocks in chain
        uint32_t totalTransactions;            // Total transactions processed
        uint32_t currentBlockTransactions;     // Transactions in current block
        uint32_t lastBlockTime;               // Timestamp of last block
        String lastBlockHash;                  // Hash of last block
        float averageBlockTime;                // Average time between blocks
        uint32_t hashOperationsCount;          // Total hash operations performed
        float hashPerformance;                 // Hashes per second
        size_t memoryUsage;                    // Current memory usage (bytes)
        bool syncStatus;                       // Network sync status
        String lastError;                      // Last error message
        
        BlockchainStatus() : 
            initialized(false), healthy(false), totalBlocks(0), totalTransactions(0),
            currentBlockTransactions(0), lastBlockTime(0), lastBlockHash(""),
            averageBlockTime(0.0), hashOperationsCount(0), hashPerformance(0.0),
            memoryUsage(0), syncStatus(false), lastError("") {}
    };

    /**
     * @brief Integrity verification result
     */
    struct IntegrityResult {
        bool isValid;                          // Overall validity
        uint32_t blocksChecked;               // Number of blocks verified
        uint32_t transactionsChecked;         // Number of transactions verified
        uint32_t hashMismatches;              // Number of hash mismatches found
        uint32_t signatureFailures;           // Number of signature failures
        std::vector<String> invalidBlocks;    // List of invalid block hashes
        String verificationTime;               // Time taken for verification
        
        IntegrityResult() : 
            isValid(true), blocksChecked(0), transactionsChecked(0),
            hashMismatches(0), signatureFailures(0), verificationTime("") {}
    };

public:
    BlockchainManager();
    ~BlockchainManager();

    /**
     * @brief Initialize blockchain system
     * @param config Blockchain configuration
     * @return true if initialization successful
     */
    bool initialize(const BlockchainConfig& config);

    /**
     * @brief Shutdown blockchain system
     */
    void shutdown();

    /**
     * @brief Add image capture transaction to blockchain
     * @param imageFilename Image file path
     * @param metadata JSON metadata string
     * @return true if transaction added successfully
     */
    bool addImageTransaction(const String& imageFilename, const String& metadata);

    /**
     * @brief Add AI detection result transaction
     * @param detectionData JSON detection data
     * @param confidence AI confidence score (0.0-1.0)
     * @return true if transaction added successfully
     */
    bool addAIDetectionTransaction(const String& detectionData, float confidence);

    /**
     * @brief Add sensor data transaction
     * @param sensorData JSON sensor readings
     * @return true if transaction added successfully
     */
    bool addSensorTransaction(const String& sensorData);

    /**
     * @brief Add system event transaction
     * @param eventType Event type string
     * @param eventData Event details
     * @return true if transaction added successfully
     */
    bool addSystemEventTransaction(const String& eventType, const String& eventData);

    /**
     * @brief Force creation of new block
     * @return true if block created successfully
     */
    bool createBlock();

    /**
     * @brief Verify integrity of entire blockchain
     * @return Integrity verification result
     */
    IntegrityResult verifyIntegrity();

    /**
     * @brief Verify specific file against blockchain
     * @param filename File to verify
     * @param expectedHash Expected hash value
     * @return true if file is verified
     */
    bool verifyFile(const String& filename, const String& expectedHash = "");

    /**
     * @brief Get current blockchain status
     * @return Status information
     */
    BlockchainStatus getStatus() const;

    /**
     * @brief Get blockchain configuration
     * @return Current configuration
     */
    BlockchainConfig getConfig() const { return config_; }

    /**
     * @brief Update blockchain configuration
     * @param config New configuration
     * @return true if update successful
     */
    bool updateConfig(const BlockchainConfig& config);

    /**
     * @brief Process pending blockchain operations
     * Call regularly from main loop
     */
    void update();

    /**
     * @brief Export blockchain to JSON format
     * @param startBlock Starting block number (0 = from beginning)
     * @param endBlock Ending block number (0 = to end)
     * @return JSON representation of blockchain
     */
    String exportToJSON(uint32_t startBlock = 0, uint32_t endBlock = 0) const;

    /**
     * @brief Import blockchain from JSON format
     * @param json JSON blockchain data
     * @return true if import successful
     */
    bool importFromJSON(const String& json);

    /**
     * @brief Save blockchain to storage
     * @return true if save successful
     */
    bool saveToStorage();

    /**
     * @brief Load blockchain from storage
     * @return true if load successful
     */
    bool loadFromStorage();

    /**
     * @brief Clear all blockchain data
     */
    void clearBlockchain();

    /**
     * @brief Get block by number
     * @param blockNumber Block number to retrieve
     * @return Pointer to block, nullptr if not found
     */
    const Block* getBlock(uint32_t blockNumber) const;

    /**
     * @brief Get block by hash
     * @param blockHash Block hash to find
     * @return Pointer to block, nullptr if not found
     */
    const Block* getBlockByHash(const String& blockHash) const;

    /**
     * @brief Check if blockchain is enabled
     * @return true if blockchain operations are enabled
     */
    bool isEnabled() const { return config_.enabled && initialized_; }

    /**
     * @brief Get memory usage statistics
     * @param totalMemory Total memory used by blockchain
     * @param blockMemory Memory used by blocks
     * @param hashMemory Memory used by hashing operations
     */
    void getMemoryStats(size_t& totalMemory, size_t& blockMemory, size_t& hashMemory) const;

private:
    BlockchainConfig config_;
    BlockchainStatus status_;
    HashingService hashingService_;
    std::vector<Block> blockchain_;
    Block currentBlock_;
    bool initialized_;
    uint32_t lastBlockTime_;
    uint32_t blockCounter_;

    /**
     * @brief Initialize genesis block
     * @return true if genesis block created
     */
    bool initializeGenesisBlock();

    /**
     * @brief Check if new block should be created
     * @return true if block creation is needed
     */
    bool shouldCreateBlock() const;

    /**
     * @brief Finalize current block and add to chain
     * @return true if block finalized successfully
     */
    bool finalizeCurrentBlock();

    /**
     * @brief Update blockchain status information
     */
    void updateStatus();

    /**
     * @brief Calculate average block creation time
     * @return Average time in seconds
     */
    float calculateAverageBlockTime() const;

    /**
     * @brief Generate unique node ID if not configured
     * @return Generated node ID
     */
    String generateNodeId() const;

    /**
     * @brief Validate blockchain configuration
     * @param config Configuration to validate
     * @return true if configuration is valid
     */
    bool validateConfig(const BlockchainConfig& config) const;

    /**
     * @brief Get blockchain storage directory
     * @return Storage directory path
     */
    String getStorageDirectory() const;

    /**
     * @brief Save block to file
     * @param block Block to save
     * @param filename Output filename
     * @return true if save successful
     */
    bool saveBlockToFile(const Block& block, const String& filename) const;

    /**
     * @brief Load block from file
     * @param filename Input filename
     * @param block Block to load into
     * @return true if load successful
     */
    bool loadBlockFromFile(const String& filename, Block& block) const;

    /**
     * @brief Clean up old blocks from memory
     */
    void cleanupOldBlocks();

    /**
     * @brief Log blockchain event
     * @param event Event description
     * @param level Log level
     */
    void logEvent(const String& event, int level = 0) const;
};

#endif // BLOCKCHAIN_MANAGER_H