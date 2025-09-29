#include "Block.h"
#include "HashingService.h"
#include "../utils/logger.h"
#include <SD_MMC.h>

// Static hashing service instance
static HashingService hashingService;

Block::Block() : finalized_(false) {
    header_.timestamp = millis() / 1000;  // Convert to seconds
    if (!hashingService.isInitialized()) {
        hashingService.initialize();
    }
}

Block::Block(uint32_t blockNumber, const String& previousHash, const String& nodeId) 
    : finalized_(false) {
    header_.blockNumber = blockNumber;
    header_.previousHash = previousHash;
    header_.nodeId = nodeId;
    header_.timestamp = millis() / 1000;
    if (!hashingService.isInitialized()) {
        hashingService.initialize();
    }
}

Block::~Block() {
    transactions_.clear();
}

bool Block::addTransaction(const Transaction& transaction) {
    if (finalized_) {
        Logger::log("Block: Cannot add transaction to finalized block");
        return false;
    }

    if (transaction.dataHash.length() != 64) {
        Logger::log("Block: Invalid transaction hash length");
        return false;
    }

    transactions_.push_back(transaction);
    header_.transactionCount = transactions_.size();
    return true;
}

bool Block::finalize() {
    if (finalized_) {
        return true;
    }

    if (transactions_.empty()) {
        Logger::log("Block: Cannot finalize block with no transactions");
        return false;
    }

    // Calculate merkle root
    header_.merkleRoot = calculateMerkleRoot();
    if (header_.merkleRoot.isEmpty()) {
        Logger::log("Block: Failed to calculate merkle root");
        return false;
    }

    // Calculate block hash
    blockHash_ = calculateBlockHash();
    if (blockHash_.isEmpty()) {
        Logger::log("Block: Failed to calculate block hash");
        return false;
    }

    finalized_ = true;
    Logger::log("Block: Block " + String(header_.blockNumber) + " finalized with " + 
                String(header_.transactionCount) + " transactions");
    return true;
}

bool Block::isValid() const {
    if (!finalized_) {
        return false;
    }

    // Validate basic structure
    if (transactions_.empty() || blockHash_.isEmpty() || header_.merkleRoot.isEmpty()) {
        return false;
    }

    // Validate transaction count
    if (header_.transactionCount != transactions_.size()) {
        return false;
    }

    // Validate merkle root
    if (!validateMerkleRoot()) {
        return false;
    }

    // Validate block hash
    if (!validateBlockHash()) {
        return false;
    }

    // Validate individual transactions
    if (!validateTransactions()) {
        return false;
    }

    return true;
}

void Block::clear() {
    transactions_.clear();
    blockHash_ = "";
    header_.merkleRoot = "";
    header_.transactionCount = 0;
    finalized_ = false;
}

size_t Block::getBlockSize() const {
    size_t size = sizeof(BlockHeader);
    
    for (const auto& tx : transactions_) {
        size += sizeof(Transaction);
        size += tx.dataHash.length();
        size += tx.metadata.length();
        size += tx.signature.length();
    }
    
    size += blockHash_.length();
    return size;
}

String Block::toJSON() const {
    DynamicJsonDocument doc(4096);
    
    // Header
    doc["header"]["blockNumber"] = header_.blockNumber;
    doc["header"]["previousHash"] = header_.previousHash;
    doc["header"]["merkleRoot"] = header_.merkleRoot;
    doc["header"]["timestamp"] = header_.timestamp;
    doc["header"]["nonce"] = header_.nonce;
    doc["header"]["transactionCount"] = header_.transactionCount;
    doc["header"]["nodeId"] = header_.nodeId;
    
    // Transactions
    JsonArray txArray = doc.createNestedArray("transactions");
    for (const auto& tx : transactions_) {
        JsonObject txObj = txArray.createNestedObject();
        txObj["type"] = static_cast<int>(tx.type);
        txObj["dataHash"] = tx.dataHash;
        txObj["metadata"] = tx.metadata;
        txObj["timestamp"] = tx.timestamp;
        txObj["confidence"] = tx.confidence;
        txObj["signature"] = tx.signature;
        txObj["dataSize"] = tx.dataSize;
    }
    
    // Block hash
    doc["blockHash"] = blockHash_;
    doc["finalized"] = finalized_;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool Block::fromJSON(const String& json) {
    DynamicJsonDocument doc(4096);
    
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        Logger::log("Block: Failed to parse JSON");
        return false;
    }
    
    // Clear existing data
    clear();
    
    // Parse header
    header_.blockNumber = doc["header"]["blockNumber"];
    header_.previousHash = doc["header"]["previousHash"].as<String>();
    header_.merkleRoot = doc["header"]["merkleRoot"].as<String>();
    header_.timestamp = doc["header"]["timestamp"];
    header_.nonce = doc["header"]["nonce"];
    header_.transactionCount = doc["header"]["transactionCount"];
    header_.nodeId = doc["header"]["nodeId"].as<String>();
    
    // Parse transactions
    JsonArray txArray = doc["transactions"];
    for (JsonObject txObj : txArray) {
        Transaction tx;
        tx.type = static_cast<TransactionType>(txObj["type"].as<int>());
        tx.dataHash = txObj["dataHash"].as<String>();
        tx.metadata = txObj["metadata"].as<String>();
        tx.timestamp = txObj["timestamp"];
        tx.confidence = txObj["confidence"];
        tx.signature = txObj["signature"].as<String>();
        tx.dataSize = txObj["dataSize"];
        transactions_.push_back(tx);
    }
    
    // Parse block data
    blockHash_ = doc["blockHash"].as<String>();
    finalized_ = doc["finalized"];
    
    return true;
}

bool Block::toFile(const String& filename) const {
    if (!finalized_) {
        Logger::log("Block: Cannot save unfinalized block");
        return false;
    }
    
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        Logger::log("Block: Failed to create file: " + filename);
        return false;
    }
    
    String json = toJSON();
    file.print(json);
    file.close();
    
    Logger::log("Block: Saved block " + String(header_.blockNumber) + " to " + filename);
    return true;
}

bool Block::fromFile(const String& filename) {
    File file = SD_MMC.open(filename.c_str());
    if (!file) {
        Logger::log("Block: Failed to open file: " + filename);
        return false;
    }
    
    String json = file.readString();
    file.close();
    
    return fromJSON(json);
}

bool Block::validateTransactions() const {
    for (const auto& tx : transactions_) {
        if (!verifyTransaction(tx)) {
            return false;
        }
    }
    return true;
}

bool Block::validateMerkleRoot() const {
    String calculatedRoot = calculateMerkleRoot();
    return calculatedRoot == header_.merkleRoot;
}

bool Block::validateBlockHash() const {
    String calculatedHash = calculateBlockHash();
    return calculatedHash == blockHash_;
}

Block::Transaction Block::createImageTransaction(const String& imageHash, const String& metadata) {
    Transaction tx;
    tx.type = TX_IMAGE_CAPTURE;
    tx.dataHash = imageHash;
    tx.metadata = metadata;
    tx.timestamp = millis() / 1000;
    tx.confidence = 1.0;  // Image capture has 100% confidence
    tx.signature = "";  // To be filled by digital signature service
    return tx;
}

Block::Transaction Block::createAIDetectionTransaction(const String& detectionData, float confidence) {
    Transaction tx;
    tx.type = TX_AI_DETECTION;
    tx.dataHash = hashingService.calculateHash(detectionData);
    tx.metadata = detectionData;
    tx.timestamp = millis() / 1000;
    tx.confidence = confidence;
    tx.signature = "";  // To be filled by digital signature service
    return tx;
}

Block::Transaction Block::createSensorTransaction(const String& sensorData) {
    Transaction tx;
    tx.type = TX_SENSOR_DATA;
    tx.dataHash = hashingService.calculateHash(sensorData);
    tx.metadata = sensorData;
    tx.timestamp = millis() / 1000;
    tx.confidence = 1.0;  // Sensor data has 100% confidence
    tx.signature = "";  // To be filled by digital signature service
    return tx;
}

Block::Transaction Block::createSystemEventTransaction(const String& eventData) {
    Transaction tx;
    tx.type = TX_SYSTEM_EVENT;
    tx.dataHash = hashingService.calculateHash(eventData);
    tx.metadata = eventData;
    tx.timestamp = millis() / 1000;
    tx.confidence = 1.0;  // System events have 100% confidence
    tx.signature = "";  // To be filled by digital signature service
    return tx;
}

String Block::calculateMerkleRoot() const {
    if (transactions_.empty()) {
        return "";
    }
    
    std::vector<String> hashes;
    
    // Create leaf hashes for all transactions
    for (const auto& tx : transactions_) {
        hashes.push_back(transactionToHash(tx));
    }
    
    // Build merkle tree by combining pairs of hashes
    while (hashes.size() > 1) {
        std::vector<String> nextLevel;
        
        for (size_t i = 0; i < hashes.size(); i += 2) {
            if (i + 1 < hashes.size()) {
                // Combine two hashes
                String combined = hashingService.calculateCombinedHash(hashes[i], hashes[i + 1]);
                nextLevel.push_back(combined);
            } else {
                // Odd number of hashes, duplicate the last one
                String combined = hashingService.calculateCombinedHash(hashes[i], hashes[i]);
                nextLevel.push_back(combined);
            }
        }
        
        hashes = nextLevel;
    }
    
    return hashes.empty() ? "" : hashes[0];
}

String Block::calculateBlockHash() const {
    // Create block header string for hashing
    String headerData = String(header_.blockNumber) + header_.previousHash + 
                       header_.merkleRoot + String(header_.timestamp) + 
                       String(header_.nonce) + String(header_.transactionCount) + 
                       header_.nodeId;
    
    return hashingService.calculateHash(headerData);
}

bool Block::verifyTransaction(const Transaction& transaction) const {
    // Basic validation
    if (transaction.dataHash.length() != 64) {
        return false;
    }
    
    if (transaction.timestamp == 0) {
        return false;
    }
    
    if (transaction.confidence < 0.0 || transaction.confidence > 1.0) {
        return false;
    }
    
    // TODO: Add digital signature verification when DigitalSignature class is implemented
    
    return true;
}

String Block::transactionToHash(const Transaction& transaction) const {
    String txData = String(static_cast<int>(transaction.type)) + transaction.dataHash + 
                   transaction.metadata + String(transaction.timestamp) + 
                   String(transaction.confidence) + transaction.signature + 
                   String(transaction.dataSize);
    
    return hashingService.calculateHash(txData);
}