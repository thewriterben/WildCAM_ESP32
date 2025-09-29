#ifndef ENHANCED_STORAGE_MANAGER_H
#define ENHANCED_STORAGE_MANAGER_H

#include "storage_manager.h"
#include "../blockchain/BlockchainManager.h"
#include <Arduino.h>

/**
 * @brief Enhanced storage manager with blockchain integration
 * 
 * Extends existing StorageManager functionality with automatic
 * blockchain data integrity verification and tamper-proof storage.
 */
class EnhancedStorageManager : public StorageManager {
public:
    /**
     * @brief Enhanced storage statistics including blockchain metrics
     */
    struct EnhancedStorageStats : public StorageStats {
        uint64_t blockchainSize;           // Blockchain storage usage
        uint32_t blockchainFiles;          // Number of blockchain files  
        uint32_t verifiedFiles;            // Files verified in blockchain
        uint32_t unverifiedFiles;          // Files not in blockchain
        float integrityScore;              // Overall integrity score (0-1)
        bool blockchainHealthy;            // Blockchain system health
        String lastVerificationTime;       // Last integrity check
        
        EnhancedStorageStats() : blockchainSize(0), blockchainFiles(0), 
            verifiedFiles(0), unverifiedFiles(0), integrityScore(1.0),
            blockchainHealthy(true), lastVerificationTime("") {}
    };

    /**
     * @brief Enhanced cleanup result with blockchain preservation
     */
    struct EnhancedCleanupResult : public CleanupResult {
        uint32_t blockchainFilesPreserved; // Blockchain files kept
        uint32_t verifiedFilesPreserved;   // Verified files kept
        bool blockchainIntegrityMaintained; // Blockchain integrity preserved
        
        EnhancedCleanupResult() : blockchainFilesPreserved(0), 
            verifiedFilesPreserved(0), blockchainIntegrityMaintained(true) {}
    };

public:
    /**
     * @brief Initialize enhanced storage manager with blockchain
     * @param blockchainManager Pointer to blockchain manager instance
     * @return true if initialization successful
     */
    static bool initializeWithBlockchain(BlockchainManager* blockchainManager);

    /**
     * @brief Save image file with automatic blockchain hashing
     * @param filename Image file path
     * @param imageData Image data buffer
     * @param dataSize Size of image data
     * @param metadata Image metadata JSON
     * @return true if saved successfully with blockchain hash
     */
    static bool saveImageWithBlockchain(const String& filename, const uint8_t* imageData, 
                                       size_t dataSize, const String& metadata);

    /**
     * @brief Save data file with blockchain integration
     * @param filename File path
     * @param data Data to save
     * @param metadata File metadata
     * @return true if saved with blockchain hash
     */
    static bool saveDataWithBlockchain(const String& filename, const String& data, 
                                      const String& metadata);

    /**
     * @brief Verify file against blockchain
     * @param filename File to verify
     * @return true if file is verified in blockchain
     */
    static bool verifyFileIntegrity(const String& filename);

    /**
     * @brief Get enhanced storage statistics including blockchain metrics
     * @return Enhanced storage statistics
     */
    static EnhancedStorageStats getEnhancedStatistics();

    /**
     * @brief Perform enhanced cleanup preserving blockchain integrity
     * @param aggressiveCleanup Enable aggressive cleanup
     * @return Enhanced cleanup result
     */
    static EnhancedCleanupResult performEnhancedCleanup(bool aggressiveCleanup = false);

    /**
     * @brief Verify storage integrity including blockchain
     * @return true if all storage is verified
     */
    static bool verifyStorageIntegrity();

    /**
     * @brief Create backup with blockchain verification
     * @param backupPath Backup destination
     * @return true if backup created with verification
     */
    static bool createVerifiedBackup(const String& backupPath);

    /**
     * @brief Restore from verified backup
     * @param backupPath Backup source
     * @return true if restore successful and verified
     */
    static bool restoreFromVerifiedBackup(const String& backupPath);

    /**
     * @brief Get file metadata with blockchain information
     * @param filename File to get metadata for
     * @return JSON metadata including blockchain info
     */
    static String getFileMetadataWithBlockchain(const String& filename);

    /**
     * @brief List files with verification status
     * @param directory Directory to list
     * @param fileList Output file list with verification info
     * @param maxFiles Maximum files to list
     * @return Number of files found
     */
    static uint16_t listFilesWithVerification(const String& directory, 
                                             String* fileList, uint16_t maxFiles);

    /**
     * @brief Delete file with blockchain consideration
     * @param filename File to delete
     * @param preserveBlockchainRecord Keep blockchain record
     * @return true if deleted successfully
     */
    static bool deleteFileWithBlockchain(const String& filename, 
                                        bool preserveBlockchainRecord = true);

    /**
     * @brief Migrate existing files to blockchain
     * @param directory Directory to migrate
     * @param createHashes Create hashes for existing files
     * @return Number of files migrated
     */
    static uint32_t migrateToBlockchain(const String& directory = "/", 
                                       bool createHashes = true);

    /**
     * @brief Auto-repair storage integrity issues
     * @return Number of issues repaired
     */
    static uint32_t autoRepairIntegrity();

    /**
     * @brief Export storage manifest with blockchain verification
     * @param manifestFile Output manifest file
     * @return true if manifest created successfully
     */
    static bool exportStorageManifest(const String& manifestFile);

    /**
     * @brief Import and verify storage manifest
     * @param manifestFile Manifest file to import
     * @return true if manifest verified successfully
     */
    static bool importStorageManifest(const String& manifestFile);

private:
    static BlockchainManager* blockchainManager_;
    static bool blockchainEnabled_;
    static uint32_t lastIntegrityCheck_;

    /**
     * @brief Calculate storage integrity score
     * @return Integrity score between 0.0 and 1.0
     */
    static float calculateIntegrityScore();

    /**
     * @brief Update blockchain statistics
     * @param stats Storage statistics to update
     */
    static void updateBlockchainStats(EnhancedStorageStats& stats);

    /**
     * @brief Check if file should be preserved during cleanup
     * @param filename File to check
     * @param isVerified Whether file is blockchain verified
     * @return true if file should be preserved
     */
    static bool shouldPreserveFile(const String& filename, bool isVerified);

    /**
     * @brief Create enhanced metadata JSON
     * @param filename File name
     * @param basicMetadata Basic metadata
     * @param blockchainHash Blockchain hash (if available)
     * @return Enhanced metadata JSON
     */
    static String createEnhancedMetadata(const String& filename, 
                                        const String& basicMetadata,
                                        const String& blockchainHash = "");
};

// Enhanced file info structure
struct EnhancedFileInfo {
    String filename;
    size_t fileSize;
    uint32_t timestamp;
    String hash;
    bool blockchainVerified;
    String blockHash;
    uint32_t blockNumber;
    String verificationStatus;
    
    EnhancedFileInfo() : filename(""), fileSize(0), timestamp(0), hash(""),
        blockchainVerified(false), blockHash(""), blockNumber(0), 
        verificationStatus("unknown") {}
        
    String toJSON() const {
        return "{\"filename\":\"" + filename + 
               "\",\"size\":" + String(fileSize) +
               ",\"timestamp\":" + String(timestamp) +
               ",\"hash\":\"" + hash +
               "\",\"verified\":" + String(blockchainVerified ? "true" : "false") +
               ",\"blockHash\":\"" + blockHash +
               "\",\"blockNumber\":" + String(blockNumber) +
               ",\"status\":\"" + verificationStatus + "\"}";
    }
};

#endif // ENHANCED_STORAGE_MANAGER_H