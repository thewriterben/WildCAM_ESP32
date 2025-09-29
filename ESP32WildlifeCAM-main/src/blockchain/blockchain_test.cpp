#include <Arduino.h>
#include "../src/blockchain/BlockchainManager.h"
#include "../src/blockchain/HashingService.h"
#include "../src/blockchain/Block.h"
#include "../src/blockchain/MerkleTree.h"

/**
 * @brief Basic blockchain functionality tests
 * 
 * Simple validation tests for core blockchain components
 * that can be run on ESP32 to verify implementation.
 */

void testHashingService() {
    Serial.println("Testing HashingService...");
    
    HashingService hasher;
    if (!hasher.initialize()) {
        Serial.println("❌ HashingService initialization failed");
        return;
    }
    
    // Test basic hashing
    String testData = "Hello, Wildlife CAM!";
    String hash1 = hasher.calculateHash(testData);
    String hash2 = hasher.calculateHash(testData);
    
    if (hash1.length() == 64 && hash1 == hash2) {
        Serial.println("✅ HashingService basic test passed");
    } else {
        Serial.println("❌ HashingService basic test failed");
    }
    
    // Test hash verification
    if (hasher.verifyHash(testData, hash1)) {
        Serial.println("✅ Hash verification test passed");
    } else {
        Serial.println("❌ Hash verification test failed");
    }
}

void testBlock() {
    Serial.println("Testing Block functionality...");
    
    Block block(1, "0000000000000000000000000000000000000000000000000000000000000000", "TEST_NODE");
    
    // Create test transactions
    Block::Transaction tx1 = Block::createImageTransaction(
        "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890",
        "{\"species\":\"deer\",\"confidence\":0.95}"
    );
    
    Block::Transaction tx2 = Block::createAIDetectionTransaction(
        "{\"species\":\"rabbit\",\"behavior\":\"foraging\"}",
        0.87
    );
    
    // Add transactions
    if (block.addTransaction(tx1) && block.addTransaction(tx2)) {
        Serial.println("✅ Transaction addition test passed");
    } else {
        Serial.println("❌ Transaction addition test failed");
    }
    
    // Finalize block
    if (block.finalize()) {
        Serial.println("✅ Block finalization test passed");
    } else {
        Serial.println("❌ Block finalization test failed");
    }
    
    // Validate block
    if (block.isValid()) {
        Serial.println("✅ Block validation test passed");
    } else {
        Serial.println("❌ Block validation test failed");
    }
    
    // Test JSON serialization
    String json = block.toJSON();
    Block newBlock;
    if (newBlock.fromJSON(json) && newBlock.isValid()) {
        Serial.println("✅ Block JSON serialization test passed");
    } else {
        Serial.println("❌ Block JSON serialization test failed");
    }
}

void testMerkleTree() {
    Serial.println("Testing MerkleTree functionality...");
    
    MerkleTree tree;
    
    // Create test hashes
    std::vector<String> testHashes = {
        "1111111111111111111111111111111111111111111111111111111111111111",
        "2222222222222222222222222222222222222222222222222222222222222222",
        "3333333333333333333333333333333333333333333333333333333333333333",
        "4444444444444444444444444444444444444444444444444444444444444444"
    };
    
    // Build tree
    if (tree.buildTree(testHashes)) {
        Serial.println("✅ MerkleTree build test passed");
    } else {
        Serial.println("❌ MerkleTree build test failed");
        return;
    }
    
    // Test root hash
    String rootHash = tree.getRootHash();
    if (rootHash.length() == 64) {
        Serial.println("✅ MerkleTree root hash test passed");
    } else {
        Serial.println("❌ MerkleTree root hash test failed");
    }
    
    // Test proof generation and verification
    String targetHash = testHashes[0];
    MerkleTree::MerkleProof proof = tree.generateProof(targetHash);
    
    if (proof.isValid && tree.verifyProof(proof)) {
        Serial.println("✅ MerkleTree proof test passed");
    } else {
        Serial.println("❌ MerkleTree proof test failed");
    }
}

void testBlockchainManager() {
    Serial.println("Testing BlockchainManager...");
    
    BlockchainManager manager;
    BlockchainManager::BlockchainConfig config;
    config.enabled = true;
    config.nodeId = "TEST_NODE_001";
    config.maxTransactionsPerBlock = 5;
    config.blockCreationInterval = 10; // 10 seconds for testing
    
    // Initialize
    if (manager.initialize(config)) {
        Serial.println("✅ BlockchainManager initialization test passed");
    } else {
        Serial.println("❌ BlockchainManager initialization test failed");
        return;
    }
    
    // Add test transactions
    bool addResult = true;
    addResult &= manager.addImageTransaction("test_image_1.jpg", "{\"test\":true}");
    addResult &= manager.addAIDetectionTransaction("{\"species\":\"test\",\"confidence\":0.8}", 0.8);
    addResult &= manager.addSensorTransaction("{\"temperature\":25.5,\"humidity\":60}");
    
    if (addResult) {
        Serial.println("✅ Transaction addition test passed");
    } else {
        Serial.println("❌ Transaction addition test failed");
    }
    
    // Force block creation
    if (manager.createBlock()) {
        Serial.println("✅ Block creation test passed");
    } else {
        Serial.println("❌ Block creation test failed");
    }
    
    // Test integrity verification
    BlockchainManager::IntegrityResult integrity = manager.verifyIntegrity();
    if (integrity.isValid) {
        Serial.println("✅ Blockchain integrity test passed");
    } else {
        Serial.println("❌ Blockchain integrity test failed");
    }
    
    // Test status
    BlockchainManager::BlockchainStatus status = manager.getStatus();
    if (status.initialized && status.totalBlocks > 0) {
        Serial.println("✅ BlockchainManager status test passed");
        Serial.printf("   Total blocks: %d, Total transactions: %d\n", 
                     status.totalBlocks, status.totalTransactions);
    } else {
        Serial.println("❌ BlockchainManager status test failed");
    }
}

void runBlockchainTests() {
    Serial.println("\n=== Blockchain Functionality Tests ===");
    
    testHashingService();
    Serial.println();
    
    testBlock();
    Serial.println();
    
    testMerkleTree();
    Serial.println();
    
    testBlockchainManager();
    Serial.println();
    
    Serial.println("=== Test Suite Complete ===\n");
}

void performanceTest() {
    Serial.println("=== Performance Test ===");
    
    HashingService hasher;
    hasher.initialize();
    
    // Test hashing performance
    uint32_t startTime = millis();
    int hashCount = 0;
    
    for (int i = 0; i < 10; i++) {
        String testData = "Performance test data #" + String(i) + " with timestamp " + String(millis());
        String hash = hasher.calculateHash(testData);
        if (hash.length() == 64) {
            hashCount++;
        }
    }
    
    uint32_t endTime = millis();
    float hashesPerSecond = (float)hashCount * 1000.0 / (endTime - startTime);
    
    Serial.printf("Hash Performance: %.2f hashes/second\n", hashesPerSecond);
    
    // Memory usage estimate
    size_t totalMem, blockMem, hashMem;
    BlockchainManager manager;
    manager.getMemoryStats(totalMem, blockMem, hashMem);
    
    Serial.printf("Estimated Memory Usage: %d bytes total\n", totalMem);
    Serial.println("=== Performance Test Complete ===\n");
}

// Test runner function to be called from main program
void runAllBlockchainTests() {
    runBlockchainTests();
    performanceTest();
}