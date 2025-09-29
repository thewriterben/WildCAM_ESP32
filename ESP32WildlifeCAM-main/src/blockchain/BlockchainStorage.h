#ifndef BLOCKCHAIN_STORAGE_H
#define BLOCKCHAIN_STORAGE_H

#include <Arduino.h>
#include "../data/storage_manager.h"
#include "BlockchainManager.h"

/**
 * @brief Blockchain storage integration with existing StorageManager
 * 
 * Provides seamless integration between blockchain operations and
 * the existing storage management system for the ESP32 Wildlife CAM.
 */
class BlockchainStorage {
public:
    /**
     * @brief Blockchain storage statistics
     */
    struct BlockchainStorageStats {
        uint64_t blockchainSize;           // Total blockchain storage size
        uint32_t blockCount;               // Number of blocks stored
        uint32_t blockFileCount;           // Individual block files
        uint64_t averageBlockSize;         // Average block size
        float storagePercentage;           // Percentage of total storage used
        bool warningThreshold;             // Storage warning threshold exceeded
        String oldestBlock;                // Oldest block timestamp
        String newestBlock;                // Newest block timestamp
        
        BlockchainStorageStats() : blockchainSize(0), blockCount(0), blockFileCount(0),
            averageBlockSize(0), storagePercentage(0.0), warningThreshold(false),
            oldestBlock(""), newestBlock("") {}
    };

public:
    BlockchainStorage();
    ~BlockchainStorage();

    /**
     * @brief Initialize blockchain storage integration
     * @param blockchainManager Pointer to blockchain manager
     * @return true if initialization successful
     */
    bool initialize(BlockchainManager* blockchainManager);

    /**
     * @brief Check storage space before blockchain operations
     * @param requiredSpace Required space in bytes
     * @return true if adequate space available
     */
    bool checkStorageSpace(size_t requiredSpace = 0);

    /**
     * @brief Store image with blockchain integration
     * @param imageData Image data buffer
     * @param dataSize Size of image data
     * @param filename Output filename
     * @param metadata JSON metadata
     * @return true if stored successfully with blockchain hash
     */
    bool storeImageWithBlockchain(const uint8_t* imageData, size_t dataSize, 
                                 const String& filename, const String& metadata);

    /**
     * @brief Verify stored image against blockchain
     * @param filename Image filename
     * @return true if image is verified against blockchain
     */
    bool verifyImageWithBlockchain(const String& filename);

    /**
     * @brief Get blockchain storage statistics
     * @return Storage statistics
     */
    BlockchainStorageStats getBlockchainStorageStats();

    /**
     * @brief Clean up old blockchain files
     * @param maxAgeDays Maximum age in days
     * @return Number of files cleaned up
     */
    uint32_t cleanupOldBlockchainFiles(uint8_t maxAgeDays = 30);

    /**
     * @brief Archive blockchain data to external storage
     * @param archivePath Archive destination path
     * @return true if archive successful
     */
    bool archiveBlockchainData(const String& archivePath);

    /**
     * @brief Restore blockchain data from archive
     * @param archivePath Archive source path
     * @return true if restore successful
     */
    bool restoreBlockchainData(const String& archivePath);

    /**
     * @brief Optimize blockchain storage
     * @return Number of bytes freed
     */
    uint64_t optimizeBlockchainStorage();

    /**
     * @brief Check if blockchain storage is healthy
     * @return true if storage is healthy
     */
    bool isStorageHealthy();

    /**
     * @brief Update storage manager with blockchain awareness
     * Extends existing StorageManager functionality with blockchain considerations
     */
    static bool updateStorageManagerForBlockchain();

    /**
     * @brief Enhanced cleanup with blockchain preservation
     * @param aggressiveCleanup Enable aggressive cleanup
     * @return Enhanced cleanup result
     */
    static StorageManager::CleanupResult enhancedCleanup(bool aggressiveCleanup = false);

    /**
     * @brief Get comprehensive storage statistics including blockchain
     * @return Enhanced storage statistics
     */
    static StorageManager::StorageStats getEnhancedStorageStats();

private:
    BlockchainManager* blockchainManager_;
    bool initialized_;
    
    /**
     * @brief Calculate total blockchain storage usage
     * @return Total bytes used by blockchain
     */
    uint64_t calculateBlockchainStorageUsage();

    /**
     * @brief Validate blockchain storage integrity
     * @return true if storage integrity is valid
     */
    bool validateStorageIntegrity();

    /**
     * @brief Get blockchain storage directory size
     * @return Directory size in bytes
     */
    uint64_t getBlockchainDirectorySize();

    /**
     * @brief Count blockchain files
     * @param blockFiles Number of block files
     * @param chainFiles Number of chain files
     * @param totalFiles Total blockchain-related files
     */
    void countBlockchainFiles(uint32_t& blockFiles, uint32_t& chainFiles, uint32_t& totalFiles);
};

/**
 * @brief Enhanced metadata structure with blockchain integration
 */
struct EnhancedMetadata {
    // Original metadata fields
    String filename;
    uint32_t timestamp;
    size_t fileSize;
    String imageHash;
    
    // Blockchain-specific fields
    String blockHash;                    // Hash of block containing this data
    uint32_t blockNumber;               // Block number
    String transactionHash;             // Transaction hash
    bool blockchainVerified;            // Verification status
    String merkleProof;                 // Merkle proof for verification
    uint32_t blockTimestamp;            // Block creation timestamp
    
    EnhancedMetadata() : filename(""), timestamp(0), fileSize(0), imageHash(""),
        blockHash(""), blockNumber(0), transactionHash(""), blockchainVerified(false),
        merkleProof(""), blockTimestamp(0) {}
};

/**
 * @brief Utility functions for blockchain storage integration
 */
namespace BlockchainStorageUtils {
    /**
     * @brief Create enhanced metadata JSON with blockchain fields
     * @param metadata Enhanced metadata structure
     * @return JSON string
     */
    String createEnhancedMetadataJSON(const EnhancedMetadata& metadata);

    /**
     * @brief Parse enhanced metadata JSON
     * @param json JSON string
     * @return Enhanced metadata structure
     */
    EnhancedMetadata parseEnhancedMetadataJSON(const String& json);

    /**
     * @brief Migrate existing metadata to enhanced format
     * @param oldMetadataFile Existing metadata file path
     * @param newMetadataFile Enhanced metadata file path
     * @return true if migration successful
     */
    bool migrateMetadataToEnhanced(const String& oldMetadataFile, const String& newMetadataFile);

    /**
     * @brief Verify file against enhanced metadata
     * @param filename File to verify
     * @param metadata Enhanced metadata
     * @return true if verification successful
     */
    bool verifyFileWithEnhancedMetadata(const String& filename, const EnhancedMetadata& metadata);
}

#endif // BLOCKCHAIN_STORAGE_H