# Blockchain-based Data Integrity for ESP32 Wildlife CAM

## Architecture Overview

The blockchain implementation provides tamper-proof data integrity for wildlife monitoring through a lightweight, ESP32-optimized system that seamlessly integrates with existing camera operations.

### Core Components

#### 1. Block Structure
- **Lightweight Design**: Optimized for ESP32 memory constraints (max 8KB per block)
- **Wildlife Transactions**: Specialized transaction types for images, AI detections, sensors
- **Merkle Tree Integration**: Efficient data verification with minimal memory footprint
- **Configurable Size**: Maximum 10 transactions per block (configurable)

#### 2. Hashing Service
- **SHA-256 Implementation**: Leverages existing mbedTLS library from OTA manager
- **Performance Optimized**: Target 100+ hashes per second
- **Incremental Hashing**: Support for large file hashing without memory exhaustion
- **Battery Conscious**: Selective hashing based on configurable frequency

#### 3. Storage Integration
- **Seamless Integration**: Extends existing StorageManager functionality
- **SD Card Optimization**: Blockchain data stored in dedicated `/blockchain` directory
- **Memory Management**: Maximum 5 blocks kept in RAM, older blocks saved to storage
- **Automatic Cleanup**: Integration with existing storage cleanup routines

#### 4. Network Synchronization
- **Multi-Protocol Support**: WiFi, LoRa mesh, cellular (when available)
- **Distributed Verification**: Cross-node validation for research networks
- **Offline Resilience**: Full functionality without network connectivity
- **Bandwidth Optimization**: Block headers sync before full block data

### Security Model

#### Cryptographic Integrity
- **SHA-256 Hashing**: All data integrity based on cryptographically secure hashes
- **Merkle Tree Proofs**: Efficient verification without full block download
- **Chain Validation**: Sequential block linking prevents insertion attacks
- **Tamper Detection**: Any modification breaks cryptographic chain

#### Data Classification
- **Critical Data**: Images and AI detections (always hashed)
- **Important Data**: Sensor readings and power events (configurable)
- **System Data**: Debug logs and routine events (optional)
- **Selective Processing**: Configurable hashing frequency for battery optimization

### Performance Characteristics

#### Memory Usage
- **Base System**: ~2KB RAM overhead
- **Per Block**: ~1-2KB RAM (depends on transaction count)
- **Hash Operations**: ~512 bytes temporary buffers
- **Total Impact**: <10KB RAM for typical operation

#### Battery Impact
- **Hash Operations**: <5% additional power consumption
- **Storage Operations**: Leverages existing SD card usage
- **Network Sync**: Optional, disabled by default for battery preservation
- **Async Processing**: No blocking of camera operations

#### Timing Performance
- **Image Hash**: <2 seconds for 1MB image
- **Block Creation**: <500ms with 10 transactions
- **Verification**: <1 second per block
- **Startup Verification**: <10 seconds for 100 blocks

### Integration Points

#### Storage Manager Enhancement
```cpp
// Enhanced image saving with blockchain
bool saveImageWithBlockchain(const String& filename, const String& metadata) {
    // Save image using existing StorageManager
    bool saved = StorageManager::saveImage(filename, imageData);
    
    // Add blockchain transaction
    if (saved && BlockchainManager::isEnabled()) {
        String imageHash = HashingService::calculateFileHash(filename);
        BlockchainManager::addImageTransaction(filename, metadata, imageHash);
    }
    
    return saved;
}
```

#### AI Processor Integration
```cpp
// AI detection with blockchain logging
bool processAIDetection(const String& imageFile, AIResult& result) {
    // Existing AI processing
    bool detected = AIProcessor::processImage(imageFile, result);
    
    // Log to blockchain if confidence threshold met
    if (detected && result.confidence >= 0.7) {
        String detectionData = result.toJSON();
        BlockchainManager::addAIDetectionTransaction(detectionData, result.confidence);
    }
    
    return detected;
}
```

#### Web Interface Status
- **Blockchain Status**: Real-time blockchain health and statistics
- **Verification Tools**: File verification against blockchain
- **Configuration Panel**: Blockchain settings management
- **Integrity Dashboard**: Visual representation of chain integrity

### Configuration Options

#### Core Settings
```cpp
struct BlockchainConfig {
    bool enabled = true;                    // Enable/disable blockchain
    uint8_t maxTransactionsPerBlock = 10;   // Block size limit
    uint32_t blockCreationInterval = 300;   // 5 minutes default
    uint8_t hashFrequency = 1;             // Hash every capture
    bool enableNetworkSync = false;         // Disabled for battery
    String nodeId = "ESP32CAM_001";        // Unique node identifier
};
```

#### Performance Tuning
- **Memory Constraints**: `BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY = 5`
- **Hash Frequency**: `BLOCKCHAIN_DEFAULT_HASH_FREQUENCY = 1`
- **Block Intervals**: `300` seconds default, `60-3600` seconds range
- **Verification**: Automatic integrity checks every hour

#### Power Optimization
- **Selective Hashing**: Only hash data above confidence thresholds
- **Async Operations**: All blockchain operations non-blocking
- **Lazy Loading**: Load blocks from storage only when needed
- **Smart Cleanup**: Automatic cleanup when storage reaches 90%

### API Usage Examples

#### Basic Integration
```cpp
// Initialize blockchain system
BlockchainManager blockchain;
BlockchainManager::BlockchainConfig config;
config.enabled = true;
config.nodeId = "WILDLIFE_CAM_001";
blockchain.initialize(config);

// Add image capture to blockchain
String imageHash = "abc123...";  // SHA-256 of image
String metadata = "{\"species\":\"deer\",\"confidence\":0.95}";
blockchain.addImageTransaction("IMG_001.jpg", metadata);

// Verify file integrity
bool verified = blockchain.verifyFile("IMG_001.jpg", imageHash);
```

#### Advanced Verification
```cpp
// Generate merkle proof for specific image
MerkleTree::MerkleProof proof = blockchain.generateProof(imageHash);

// Verify proof without full blockchain
bool proofValid = blockchain.verifyProof(proof);

// Check blockchain integrity
BlockchainManager::IntegrityResult result = blockchain.verifyIntegrity();
if (!result.isValid) {
    Serial.println("Blockchain integrity compromised!");
}
```

### File Structure

```
src/blockchain/
├── Block.h/cpp                 // Block structure and operations
├── BlockchainManager.h/cpp     // Main orchestration system
├── HashingService.h/cpp        // SHA-256 hashing operations
├── MerkleTree.h/cpp           // Merkle tree implementation
├── BlockchainStorage.h/cpp     // Storage integration layer
├── BlockchainIntegration.h/cpp // System integration hooks
└── NetworkSync.h/cpp          // Network synchronization

include/blockchain/
├── blockchain_config.h         // Configuration constants
├── blockchain_types.h          // Common data types
└── crypto_constants.h          // Cryptographic constants

/blockchain/                    // SD card storage directory
├── chain.json                  // Main blockchain file
├── config.json                 // Configuration file
├── block_000000.json          // Individual block files
├── block_000001.json
└── ...
```

### Migration and Deployment

#### Existing System Impact
- **Zero Downtime**: Blockchain can be enabled/disabled without system restart
- **Backward Compatibility**: Existing images and data remain accessible
- **Gradual Rollout**: Can be enabled for new captures while existing data remains unchanged
- **Storage Growth**: ~1% additional storage overhead for blockchain data

#### Deployment Scenarios
1. **Research Deployment**: Full blockchain with network sync for data integrity verification
2. **Conservation Deployment**: Local blockchain for tamper detection without network overhead
3. **Budget Deployment**: Selective hashing for critical data only
4. **Urban Deployment**: Enhanced verification for high-security requirements

This architecture provides enterprise-grade data integrity while maintaining the ESP32 Wildlife CAM's core efficiency and reliability.