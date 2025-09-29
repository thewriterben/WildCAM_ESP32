#include "BlockchainManager.h"
#include "../data/storage_manager.h"
#include "../utils/logger.h"
#include "../../include/blockchain/blockchain_config.h"
#include "../../include/blockchain/blockchain_types.h"
#include <SD_MMC.h>

BlockchainManager::BlockchainManager() 
    : initialized_(false), lastBlockTime_(0), blockCounter_(0) {
    // Initialize default configuration
    config_ = BlockchainConfig();
    status_ = BlockchainStatus();
}

BlockchainManager::~BlockchainManager() {
    shutdown();
}

bool BlockchainManager::initialize(const BlockchainConfig& config) {
    if (initialized_) {
        return true;
    }

    // Validate configuration
    if (!validateConfig(config)) {
        status_.lastError = "Invalid blockchain configuration";
        return false;
    }

    config_ = config;

    // Initialize hashing service
    if (!hashingService_.initialize()) {
        status_.lastError = "Failed to initialize hashing service";
        return false;
    }

    // Create blockchain storage directory
    String storageDir = getStorageDirectory();
    if (!SD_MMC.exists(storageDir.c_str())) {
        if (!SD_MMC.mkdir(storageDir.c_str())) {
            status_.lastError = "Failed to create blockchain storage directory";
            return false;
        }
    }

    // Initialize genesis block if blockchain is empty
    if (!initializeGenesisBlock()) {
        status_.lastError = "Failed to initialize genesis block";
        return false;
    }

    // Load existing blockchain from storage
    loadFromStorage();

    // Initialize current block
    blockCounter_ = blockchain_.size();
    currentBlock_ = Block(blockCounter_, 
                         blockchain_.empty() ? "" : blockchain_.back().getBlockHash(),
                         config_.nodeId);

    status_.initialized = true;
    status_.healthy = true;
    initialized_ = true;
    lastBlockTime_ = millis();

    updateStatus();
    
    Logger::log("BlockchainManager: Initialized successfully with " + String(blockchain_.size()) + " blocks");
    return true;
}

void BlockchainManager::shutdown() {
    if (!initialized_) {
        return;
    }

    // Finalize current block if it has transactions
    if (currentBlock_.getTransactions().size() > 0) {
        finalizeCurrentBlock();
    }

    // Save blockchain to storage
    saveToStorage();

    initialized_ = false;
    status_.initialized = false;
    
    Logger::log("BlockchainManager: Shutdown complete");
}

bool BlockchainManager::addImageTransaction(const String& imageFilename, const String& metadata) {
    if (!isEnabled()) {
        return false;
    }

    // Calculate image hash
    String imageHash = hashingService_.calculateFileHash(imageFilename);
    if (imageHash.isEmpty()) {
        status_.lastError = "Failed to calculate image hash";
        return false;
    }

    // Create image transaction
    Block::Transaction transaction = Block::createImageTransaction(imageHash, metadata);
    transaction.dataSize = StorageManager::getFileSize(imageFilename);

    // Add to current block
    if (!currentBlock_.addTransaction(transaction)) {
        status_.lastError = "Failed to add image transaction to block";
        return false;
    }

    status_.totalTransactions++;
    status_.currentBlockTransactions++;

    // Check if block should be created
    if (shouldCreateBlock()) {
        createBlock();
    }

    updateStatus();
    Logger::log("BlockchainManager: Added image transaction for " + imageFilename);
    return true;
}

bool BlockchainManager::addAIDetectionTransaction(const String& detectionData, float confidence) {
    if (!isEnabled() || !config_.enableAIHashing) {
        return false;
    }

    // Check confidence threshold
    if (confidence < BLOCKCHAIN_AI_CONFIDENCE_THRESHOLD) {
        return true; // Skip low confidence detections
    }

    // Create AI detection transaction
    Block::Transaction transaction = Block::createAIDetectionTransaction(detectionData, confidence);
    transaction.dataSize = detectionData.length();

    // Add to current block
    if (!currentBlock_.addTransaction(transaction)) {
        status_.lastError = "Failed to add AI detection transaction";
        return false;
    }

    status_.totalTransactions++;
    status_.currentBlockTransactions++;

    // Check if block should be created
    if (shouldCreateBlock()) {
        createBlock();
    }

    updateStatus();
    Logger::log("BlockchainManager: Added AI detection transaction (confidence: " + String(confidence) + ")");
    return true;
}

bool BlockchainManager::addSensorTransaction(const String& sensorData) {
    if (!isEnabled() || !config_.enableSensorHashing) {
        return false;
    }

    // Create sensor transaction
    Block::Transaction transaction = Block::createSensorTransaction(sensorData);
    transaction.dataSize = sensorData.length();

    // Add to current block
    if (!currentBlock_.addTransaction(transaction)) {
        status_.lastError = "Failed to add sensor transaction";
        return false;
    }

    status_.totalTransactions++;
    status_.currentBlockTransactions++;

    // Check if block should be created
    if (shouldCreateBlock()) {
        createBlock();
    }

    updateStatus();
    return true;
}

bool BlockchainManager::addSystemEventTransaction(const String& eventType, const String& eventData) {
    if (!isEnabled() || !config_.enableSystemEventHashing) {
        return false;
    }

    // Create combined event data
    String combinedData = "{\"type\":\"" + eventType + "\",\"data\":" + eventData + "}";
    
    // Create system event transaction
    Block::Transaction transaction = Block::createSystemEventTransaction(combinedData);
    transaction.dataSize = combinedData.length();

    // Add to current block
    if (!currentBlock_.addTransaction(transaction)) {
        status_.lastError = "Failed to add system event transaction";
        return false;
    }

    status_.totalTransactions++;
    status_.currentBlockTransactions++;

    // Check if block should be created
    if (shouldCreateBlock()) {
        createBlock();
    }

    updateStatus();
    return true;
}

bool BlockchainManager::createBlock() {
    if (!isEnabled()) {
        return false;
    }

    // Finalize current block
    if (!finalizeCurrentBlock()) {
        return false;
    }

    // Create new current block
    blockCounter_++;
    String previousHash = blockchain_.empty() ? "" : blockchain_.back().getBlockHash();
    currentBlock_ = Block(blockCounter_, previousHash, config_.nodeId);

    lastBlockTime_ = millis();
    updateStatus();

    Logger::log("BlockchainManager: Created new block #" + String(blockCounter_));
    return true;
}

BlockchainManager::IntegrityResult BlockchainManager::verifyIntegrity() {
    IntegrityResult result;
    uint32_t startTime = millis();

    if (!isEnabled()) {
        result.isValid = false;
        return result;
    }

    // Verify each block in the chain
    for (size_t i = 0; i < blockchain_.size(); i++) {
        const Block& block = blockchain_[i];
        
        result.blocksChecked++;
        result.transactionsChecked += block.getTransactions().size();

        // Validate block structure and hashes
        if (!block.isValid()) {
            result.isValid = false;
            result.hashMismatches++;
            result.invalidBlocks.push_back(block.getBlockHash());
        }

        // Verify chain linking (except for genesis block)
        if (i > 0) {
            const Block& previousBlock = blockchain_[i - 1];
            if (block.getHeader().previousHash != previousBlock.getBlockHash()) {
                result.isValid = false;
                result.hashMismatches++;
                result.invalidBlocks.push_back(block.getBlockHash());
            }
        }
    }

    uint32_t verificationTime = millis() - startTime;
    result.verificationTime = String(verificationTime) + "ms";

    Logger::log("BlockchainManager: Integrity verification completed in " + result.verificationTime + 
                " - " + String(result.isValid ? "VALID" : "INVALID"));
    
    return result;
}

bool BlockchainManager::verifyFile(const String& filename, const String& expectedHash) {
    if (!isEnabled()) {
        return false;
    }

    String actualHash = hashingService_.calculateFileHash(filename);
    if (actualHash.isEmpty()) {
        return false;
    }

    if (!expectedHash.isEmpty()) {
        return actualHash.equalsIgnoreCase(expectedHash);
    }

    // Search blockchain for file hash
    for (const auto& block : blockchain_) {
        for (const auto& transaction : block.getTransactions()) {
            if (transaction.type == Block::TX_IMAGE_CAPTURE && 
                transaction.dataHash.equalsIgnoreCase(actualHash)) {
                return true;
            }
        }
    }

    return false;
}

BlockchainManager::BlockchainStatus BlockchainManager::getStatus() const {
    return status_;
}

bool BlockchainManager::updateConfig(const BlockchainConfig& config) {
    if (!validateConfig(config)) {
        return false;
    }

    config_ = config;
    updateStatus();
    
    Logger::log("BlockchainManager: Configuration updated");
    return true;
}

void BlockchainManager::update() {
    if (!isEnabled()) {
        return;
    }

    uint32_t currentTime = millis();

    // Check if block creation is needed based on time
    if (shouldCreateBlock()) {
        createBlock();
    }

    // Periodic integrity verification
    static uint32_t lastVerificationTime = 0;
    if (BLOCKCHAIN_AUTO_VERIFY && 
        (currentTime - lastVerificationTime) > (BLOCKCHAIN_VERIFY_INTERVAL * 1000)) {
        
        IntegrityResult result = verifyIntegrity();
        if (!result.isValid) {
            status_.healthy = false;
            status_.lastError = "Blockchain integrity verification failed";
        }
        lastVerificationTime = currentTime;
    }

    // Clean up old blocks from memory if needed
    cleanupOldBlocks();

    // Update status periodically
    static uint32_t lastStatusUpdate = 0;
    if ((currentTime - lastStatusUpdate) > (BLOCKCHAIN_STATUS_UPDATE_INTERVAL * 1000)) {
        updateStatus();
        lastStatusUpdate = currentTime;
    }
}

String BlockchainManager::exportToJSON(uint32_t startBlock, uint32_t endBlock) const {
    DynamicJsonDocument doc(8192);
    
    // Blockchain metadata
    doc["version"] = BLOCKCHAIN_VERSION;
    doc["nodeId"] = config_.nodeId;
    doc["totalBlocks"] = blockchain_.size();
    doc["exportTime"] = millis() / 1000;
    
    // Export blocks
    JsonArray blocksArray = doc.createNestedArray("blocks");
    
    uint32_t start = (startBlock == 0) ? 0 : startBlock;
    uint32_t end = (endBlock == 0 || endBlock > blockchain_.size()) ? blockchain_.size() : endBlock;
    
    for (uint32_t i = start; i < end; i++) {
        if (i < blockchain_.size()) {
            JsonObject blockObj = blocksArray.createNestedObject();
            // Parse block JSON and add to array
            DynamicJsonDocument blockDoc(4096);
            deserializeJson(blockDoc, blockchain_[i].toJSON());
            blockObj.set(blockDoc.as<JsonObject>());
        }
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool BlockchainManager::saveToStorage() {
    if (!isEnabled()) {
        return false;
    }

    String chainFile = getStorageDirectory() + "/" + BLOCKCHAIN_CHAIN_FILE;
    String exportData = exportToJSON();
    
    File file = SD_MMC.open(chainFile.c_str(), FILE_WRITE);
    if (!file) {
        status_.lastError = "Failed to open chain file for writing";
        return false;
    }
    
    file.print(exportData);
    file.close();
    
    Logger::log("BlockchainManager: Saved blockchain to storage (" + String(exportData.length()) + " bytes)");
    return true;
}

bool BlockchainManager::loadFromStorage() {
    String chainFile = getStorageDirectory() + "/" + BLOCKCHAIN_CHAIN_FILE;
    
    if (!SD_MMC.exists(chainFile.c_str())) {
        // No existing chain file, start fresh
        return true;
    }
    
    File file = SD_MMC.open(chainFile.c_str());
    if (!file) {
        status_.lastError = "Failed to open chain file for reading";
        return false;
    }
    
    String jsonData = file.readString();
    file.close();
    
    return importFromJSON(jsonData);
}

bool BlockchainManager::importFromJSON(const String& json) {
    DynamicJsonDocument doc(8192);
    
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        status_.lastError = "Failed to parse blockchain JSON";
        return false;
    }
    
    // Clear existing blockchain
    blockchain_.clear();
    
    // Import blocks
    JsonArray blocksArray = doc["blocks"];
    for (JsonObject blockObj : blocksArray) {
        Block block;
        String blockJson;
        serializeJson(blockObj, blockJson);
        
        if (block.fromJSON(blockJson)) {
            blockchain_.push_back(block);
        } else {
            status_.lastError = "Failed to import block from JSON";
            return false;
        }
    }
    
    Logger::log("BlockchainManager: Imported " + String(blockchain_.size()) + " blocks from storage");
    return true;
}

void BlockchainManager::clearBlockchain() {
    blockchain_.clear();
    currentBlock_.clear();
    blockCounter_ = 0;
    status_.totalBlocks = 0;
    status_.totalTransactions = 0;
    status_.currentBlockTransactions = 0;
    
    // Reinitialize genesis block
    if (initialized_) {
        initializeGenesisBlock();
    }
    
    Logger::log("BlockchainManager: Blockchain cleared");
}

const Block* BlockchainManager::getBlock(uint32_t blockNumber) const {
    for (const auto& block : blockchain_) {
        if (block.getHeader().blockNumber == blockNumber) {
            return &block;
        }
    }
    return nullptr;
}

const Block* BlockchainManager::getBlockByHash(const String& blockHash) const {
    for (const auto& block : blockchain_) {
        if (block.getBlockHash().equalsIgnoreCase(blockHash)) {
            return &block;
        }
    }
    return nullptr;
}

void BlockchainManager::getMemoryStats(size_t& totalMemory, size_t& blockMemory, size_t& hashMemory) const {
    blockMemory = 0;
    for (const auto& block : blockchain_) {
        blockMemory += block.getBlockSize();
    }
    blockMemory += currentBlock_.getBlockSize();
    
    hashMemory = sizeof(HashingService);  // Approximate
    totalMemory = blockMemory + hashMemory + sizeof(BlockchainManager);
}

// Private methods implementation

bool BlockchainManager::initializeGenesisBlock() {
    if (!blockchain_.empty()) {
        return true; // Genesis block already exists
    }

    Block genesisBlock(0, "", config_.nodeId);
    
    // Add genesis transaction
    Block::Transaction genesisTx = Block::createSystemEventTransaction(DEFAULT_GENESIS_MESSAGE);
    genesisTx.timestamp = millis() / 1000;
    
    if (!genesisBlock.addTransaction(genesisTx)) {
        return false;
    }
    
    if (!genesisBlock.finalize()) {
        return false;
    }
    
    blockchain_.push_back(genesisBlock);
    Logger::log("BlockchainManager: Genesis block created");
    return true;
}

bool BlockchainManager::shouldCreateBlock() const {
    // Check transaction count
    if (currentBlock_.getTransactions().size() >= config_.maxTransactionsPerBlock) {
        return true;
    }
    
    // Check time interval
    uint32_t timeSinceLastBlock = (millis() - lastBlockTime_) / 1000;
    if (timeSinceLastBlock >= config_.blockCreationInterval) {
        return currentBlock_.getTransactions().size() > 0; // Only if there are transactions
    }
    
    return false;
}

bool BlockchainManager::finalizeCurrentBlock() {
    if (currentBlock_.getTransactions().empty()) {
        return true; // Nothing to finalize
    }
    
    if (!currentBlock_.finalize()) {
        status_.lastError = "Failed to finalize current block";
        return false;
    }
    
    // Add to blockchain
    blockchain_.push_back(currentBlock_);
    
    // Update status
    status_.totalBlocks = blockchain_.size();
    status_.lastBlockTime = currentBlock_.getHeader().timestamp;
    status_.lastBlockHash = currentBlock_.getBlockHash();
    status_.currentBlockTransactions = 0;
    
    // Save block to individual file for backup
    String blockFile = getStorageDirectory() + "/" + BLOCKCHAIN_BLOCK_FILE_PREFIX + 
                      String(currentBlock_.getHeader().blockNumber) + ".json";
    currentBlock_.toFile(blockFile);
    
    return true;
}

void BlockchainManager::updateStatus() {
    status_.totalBlocks = blockchain_.size();
    status_.currentBlockTransactions = currentBlock_.getTransactions().size();
    status_.averageBlockTime = calculateAverageBlockTime();
    
    // Update hash performance
    uint32_t hashCount, totalTime;
    float avgTime;
    hashingService_.getHashStats(hashCount, totalTime, avgTime);
    status_.hashOperationsCount = hashCount;
    status_.hashPerformance = (totalTime > 0) ? (hashCount * 1000.0 / totalTime) : 0.0;
    
    // Update memory usage
    size_t totalMem, blockMem, hashMem;
    getMemoryStats(totalMem, blockMem, hashMem);
    status_.memoryUsage = totalMem;
}

float BlockchainManager::calculateAverageBlockTime() const {
    if (blockchain_.size() < 2) {
        return 0.0;
    }
    
    uint32_t totalTime = 0;
    for (size_t i = 1; i < blockchain_.size(); i++) {
        uint32_t timeDiff = blockchain_[i].getHeader().timestamp - blockchain_[i-1].getHeader().timestamp;
        totalTime += timeDiff;
    }
    
    return (float)totalTime / (blockchain_.size() - 1);
}

String BlockchainManager::generateNodeId() const {
    // Generate unique node ID based on MAC address and chip ID
    uint64_t chipId = ESP.getEfuseMac();
    char nodeId[32];
    snprintf(nodeId, sizeof(nodeId), "ESP32CAM_%08X", (uint32_t)(chipId & 0xFFFFFFFF));
    return String(nodeId);
}

bool BlockchainManager::validateConfig(const BlockchainConfig& config) const {
    if (config.maxTransactionsPerBlock == 0 || config.maxTransactionsPerBlock > 100) {
        return false;
    }
    
    if (config.blockCreationInterval < BLOCKCHAIN_MIN_BLOCK_INTERVAL || 
        config.blockCreationInterval > BLOCKCHAIN_MAX_BLOCK_INTERVAL) {
        return false;
    }
    
    if (config.nodeId.isEmpty() || config.nodeId.length() > 32) {
        return false;
    }
    
    return true;
}

String BlockchainManager::getStorageDirectory() const {
    return String(BLOCKCHAIN_STORAGE_DIR);
}

void BlockchainManager::cleanupOldBlocks() {
    if (blockchain_.size() > config_.maxBlocksInMemory) {
        // Save old blocks to storage before removing from memory
        size_t blocksToRemove = blockchain_.size() - config_.maxBlocksInMemory;
        for (size_t i = 0; i < blocksToRemove; i++) {
            String blockFile = getStorageDirectory() + "/" + BLOCKCHAIN_BLOCK_FILE_PREFIX + 
                              String(blockchain_[i].getHeader().blockNumber) + ".json";
            blockchain_[i].toFile(blockFile);
        }
        
        // Remove from memory
        blockchain_.erase(blockchain_.begin(), blockchain_.begin() + blocksToRemove);
        Logger::log("BlockchainManager: Cleaned up " + String(blocksToRemove) + " old blocks from memory");
    }
}

void BlockchainManager::logEvent(const String& event, int level) const {
    if (BLOCKCHAIN_DEBUG_ENABLED) {
        Logger::log("Blockchain: " + event);
    }
}