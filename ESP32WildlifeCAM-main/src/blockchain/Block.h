#ifndef BLOCKCHAIN_BLOCK_H
#define BLOCKCHAIN_BLOCK_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

/**
 * @brief Lightweight blockchain block structure optimized for ESP32 Wildlife CAM
 * 
 * Stores wildlife monitoring data with cryptographic integrity verification.
 * Designed for minimal memory footprint while maintaining security.
 */
class Block {
public:
    /**
     * @brief Wildlife data transaction type
     */
    enum TransactionType {
        TX_IMAGE_CAPTURE,       // Image capture with metadata
        TX_AI_DETECTION,        // AI detection result
        TX_SENSOR_DATA,         // Environmental sensor reading
        TX_SYSTEM_EVENT,        // System operation event
        TX_POWER_EVENT,         // Power management event
        TX_NETWORK_EVENT        // Network communication event
    };

    /**
     * @brief Individual transaction within a block
     */
    struct Transaction {
        TransactionType type;           // Transaction type
        String dataHash;               // SHA-256 hash of data
        String metadata;               // JSON metadata string
        uint32_t timestamp;            // Unix timestamp
        float confidence;              // AI confidence score (if applicable)
        String signature;              // Digital signature
        size_t dataSize;               // Original data size in bytes
        
        Transaction() : type(TX_SYSTEM_EVENT), dataHash(""), metadata(""), 
                       timestamp(0), confidence(0.0), signature(""), dataSize(0) {}
    };

    /**
     * @brief Block header structure
     */
    struct BlockHeader {
        uint32_t blockNumber;          // Sequential block number
        String previousHash;           // Hash of previous block
        String merkleRoot;             // Merkle tree root hash
        uint32_t timestamp;            // Block creation timestamp
        uint32_t nonce;               // Proof of work nonce (simplified)
        uint16_t transactionCount;     // Number of transactions in block
        String nodeId;                 // Camera node identifier
        
        BlockHeader() : blockNumber(0), previousHash(""), merkleRoot(""), 
                       timestamp(0), nonce(0), transactionCount(0), nodeId("") {}
    };

public:
    Block();
    Block(uint32_t blockNumber, const String& previousHash, const String& nodeId);
    ~Block();

    // Block creation and management
    bool addTransaction(const Transaction& transaction);
    bool finalize();  // Calculate merkle root and block hash
    bool isValid() const;
    void clear();

    // Getters
    const BlockHeader& getHeader() const { return header_; }
    const std::vector<Transaction>& getTransactions() const { return transactions_; }
    String getBlockHash() const { return blockHash_; }
    bool isFinalized() const { return finalized_; }
    size_t getBlockSize() const;

    // Setters
    void setNodeId(const String& nodeId) { header_.nodeId = nodeId; }
    void setPreviousHash(const String& previousHash) { header_.previousHash = previousHash; }

    // Serialization
    String toJSON() const;
    bool fromJSON(const String& json);
    bool toFile(const String& filename) const;
    bool fromFile(const String& filename);

    // Validation
    bool validateTransactions() const;
    bool validateMerkleRoot() const;
    bool validateBlockHash() const;

    // Utility
    static Transaction createImageTransaction(const String& imageHash, const String& metadata);
    static Transaction createAIDetectionTransaction(const String& detectionData, float confidence);
    static Transaction createSensorTransaction(const String& sensorData);
    static Transaction createSystemEventTransaction(const String& eventData);

private:
    BlockHeader header_;
    std::vector<Transaction> transactions_;
    String blockHash_;
    bool finalized_;

    // Internal methods
    String calculateMerkleRoot() const;
    String calculateBlockHash() const;
    bool verifyTransaction(const Transaction& transaction) const;
    String transactionToHash(const Transaction& transaction) const;
};

#endif // BLOCKCHAIN_BLOCK_H