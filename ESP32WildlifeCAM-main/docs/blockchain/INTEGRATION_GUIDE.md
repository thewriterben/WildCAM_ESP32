# Integration Guide: Blockchain with ESP32 Wildlife CAM

## Overview

This guide provides step-by-step instructions for integrating the blockchain data integrity system with the existing ESP32 Wildlife CAM components.

## Prerequisites

- ESP32 Wildlife CAM system with existing components
- mbedTLS library (already included via OTA manager)
- SD card storage (already configured)
- ArduinoJson library (already included)

## Integration Steps

### 1. Include Blockchain Headers

Add blockchain includes to your main project files:

```cpp
#include "src/blockchain/BlockchainManager.h"
#include "src/blockchain/BlockchainIntegration.h"
#include "include/blockchain/blockchain_config.h"
```

### 2. Initialize Blockchain System

In your main setup function:

```cpp
// Global blockchain manager instance
BlockchainManager blockchainManager;
BlockchainIntegration blockchainIntegration;

void setup() {
    // ... existing setup code ...
    
    // Initialize storage manager first (already exists)
    StorageManager::initialize();
    
    // Configure blockchain
    BlockchainManager::BlockchainConfig blockchainConfig;
    blockchainConfig.enabled = true;
    blockchainConfig.nodeId = "ESP32CAM_" + String(ESP.getEfuseMac() & 0xFFFFFF, HEX);
    blockchainConfig.enableImageHashing = true;
    blockchainConfig.enableAIHashing = true;
    blockchainConfig.hashFrequency = 1; // Hash every capture
    
    // Initialize blockchain
    if (blockchainManager.initialize(blockchainConfig)) {
        Serial.println("Blockchain initialized successfully");
    } else {
        Serial.println("Blockchain initialization failed");
    }
    
    // Initialize integration layer
    BlockchainIntegration::IntegrationConfig integrationConfig;
    integrationConfig.integrateWithStorageManager = true;
    integrationConfig.integrateWithAIProcessor = true;
    integrationConfig.integrateWithCameraCapture = true;
    
    blockchainIntegration.initialize(&blockchainManager, integrationConfig);
}
```

### 3. Enhance Image Capture

Modify your existing image capture code:

```cpp
// Before: Simple image capture
bool captureImage(const String& filename) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return false;
    
    // Save image to SD card
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    file.write(fb->buf, fb->len);
    file.close();
    
    esp_camera_fb_return(fb);
    return true;
}

// After: Image capture with blockchain integration
bool captureImageWithBlockchain(const String& filename, const String& metadata = "") {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return false;
    
    // Save image to SD card
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    file.write(fb->buf, fb->len);
    file.close();
    
    // Add to blockchain
    if (blockchainManager.isEnabled()) {
        blockchainIntegration.handleImageCapture(filename, metadata);
    }
    
    esp_camera_fb_return(fb);
    return true;
}
```

### 4. Enhance AI Detection

Integrate blockchain with AI processing:

```cpp
// Enhanced AI detection with blockchain logging
struct AIDetectionResult {
    String species;
    float confidence;
    String behavior;
    String boundingBox;
    
    String toJSON() const {
        return "{\"species\":\"" + species + "\",\"confidence\":" + String(confidence) + 
               ",\"behavior\":\"" + behavior + "\",\"boundingBox\":\"" + boundingBox + "\"}";
    }
};

bool processAIDetectionWithBlockchain(const String& imageFile, AIDetectionResult& result) {
    // Existing AI processing code
    bool detected = performAIDetection(imageFile, result);
    
    // Add to blockchain if confidence threshold met
    if (detected && result.confidence >= 0.7 && blockchainManager.isEnabled()) {
        String detectionData = result.toJSON();
        blockchainIntegration.handleAIDetection(detectionData, result.confidence);
    }
    
    return detected;
}
```

### 5. Enhance Storage Management

Extend existing StorageManager with blockchain awareness:

```cpp
// Enhanced file saving with blockchain integration
bool saveDataWithBlockchain(const String& filename, const String& data, const String& metadata) {
    // Save using existing StorageManager
    File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
    if (!file) return false;
    
    file.print(data);
    file.close();
    
    // Add to blockchain
    if (blockchainManager.isEnabled()) {
        // Create metadata with file info
        String enhancedMetadata = "{\"filename\":\"" + filename + 
                                "\",\"size\":" + String(data.length()) + 
                                ",\"timestamp\":" + String(millis() / 1000) + 
                                ",\"metadata\":" + metadata + "}";
        
        String dataHash = blockchainManager.getHashingService().calculateHash(data);
        blockchainManager.addSystemEventTransaction("FILE_SAVE", enhancedMetadata);
    }
    
    return true;
}
```

### 6. Update Main Loop

Add blockchain processing to your main loop:

```cpp
void loop() {
    // ... existing loop code ...
    
    // Update blockchain (handles periodic operations)
    blockchainManager.update();
    blockchainIntegration.update();
    
    // ... rest of loop code ...
}
```

### 7. Web Interface Integration

Add blockchain status to web interface:

```cpp
// Add to web server handlers
void handleBlockchainStatus() {
    if (!server.authenticate("admin", "password")) {
        return server.requestAuthentication();
    }
    
    BlockchainManager::BlockchainStatus status = blockchainManager.getStatus();
    
    String html = "<h2>Blockchain Status</h2>";
    html += "<p>Enabled: " + String(status.initialized ? "Yes" : "No") + "</p>";
    html += "<p>Total Blocks: " + String(status.totalBlocks) + "</p>";
    html += "<p>Total Transactions: " + String(status.totalTransactions) + "</p>";
    html += "<p>Current Block Transactions: " + String(status.currentBlockTransactions) + "</p>";
    html += "<p>Last Block Hash: " + status.lastBlockHash + "</p>";
    html += "<p>Hash Performance: " + String(status.hashPerformance) + " hashes/sec</p>";
    html += "<p>Memory Usage: " + String(status.memoryUsage / 1024) + " KB</p>";
    
    if (!status.lastError.isEmpty()) {
        html += "<p style='color:red'>Last Error: " + status.lastError + "</p>";
    }
    
    server.send(200, "text/html", html);
}

// Add to server setup
void setupWebServer() {
    // ... existing server setup ...
    
    server.on("/blockchain", handleBlockchainStatus);
    server.on("/verify", handleFileVerification);
}
```

### 8. File Verification

Add verification functionality:

```cpp
void handleFileVerification() {
    if (!server.hasArg("file")) {
        server.send(400, "text/plain", "Missing file parameter");
        return;
    }
    
    String filename = server.arg("file");
    bool verified = blockchainManager.verifyFile(filename);
    
    String response = "{\"filename\":\"" + filename + "\",\"verified\":" + 
                     String(verified ? "true" : "false") + "}";
    
    server.send(200, "application/json", response);
}
```

### 9. Configuration Management

Add blockchain configuration to existing config system:

```cpp
// Add to config.h or create blockchain_user_config.h
struct UserBlockchainConfig {
    bool enabled = true;
    uint8_t hashFrequency = 1;          // 1=every capture, 2=every other, etc.
    bool enableAIHashing = true;
    bool enableSensorHashing = false;
    bool enableSystemEventHashing = true;
    uint32_t blockInterval = 300;       // seconds
    String nodeId = "";                 // auto-generated if empty
};

// Save/load configuration to EEPROM or SD card
void saveBlockchainConfig(const UserBlockchainConfig& config) {
    File configFile = SD_MMC.open("/blockchain_config.json", FILE_WRITE);
    if (configFile) {
        String configJson = configToJSON(config);
        configFile.print(configJson);
        configFile.close();
    }
}

UserBlockchainConfig loadBlockchainConfig() {
    UserBlockchainConfig config;
    File configFile = SD_MMC.open("/blockchain_config.json");
    if (configFile) {
        String configJson = configFile.readString();
        configFile.close();
        parseConfigFromJSON(configJson, config);
    }
    return config;
}
```

## Testing Integration

### 1. Basic Functionality Test

```cpp
void testBlockchainIntegration() {
    Serial.println("Testing blockchain integration...");
    
    // Test 1: Image capture with blockchain
    String testFile = "/test_image.jpg";
    String testMetadata = "{\"test\":true,\"timestamp\":" + String(millis()) + "}";
    
    if (captureImageWithBlockchain(testFile, testMetadata)) {
        Serial.println("✓ Image capture with blockchain successful");
    } else {
        Serial.println("✗ Image capture with blockchain failed");
    }
    
    // Test 2: File verification
    if (blockchainManager.verifyFile(testFile)) {
        Serial.println("✓ File verification successful");
    } else {
        Serial.println("✗ File verification failed");
    }
    
    // Test 3: Blockchain integrity
    auto integrityResult = blockchainManager.verifyIntegrity();
    if (integrityResult.isValid) {
        Serial.println("✓ Blockchain integrity check passed");
    } else {
        Serial.println("✗ Blockchain integrity check failed");
    }
    
    Serial.println("Blockchain integration test complete");
}
```

### 2. Performance Monitoring

```cpp
void monitorBlockchainPerformance() {
    static uint32_t lastCheck = 0;
    uint32_t now = millis();
    
    if (now - lastCheck > 60000) { // Check every minute
        auto status = blockchainManager.getStatus();
        
        Serial.printf("Blockchain Performance:\n");
        Serial.printf("  Hash Performance: %.2f hashes/sec\n", status.hashPerformance);
        Serial.printf("  Memory Usage: %d bytes\n", status.memoryUsage);
        Serial.printf("  Total Blocks: %d\n", status.totalBlocks);
        Serial.printf("  Total Transactions: %d\n", status.totalTransactions);
        
        lastCheck = now;
    }
}
```

## Migration from Existing System

### 1. Backward Compatibility

The blockchain system is designed to work alongside existing functionality:

- Existing images and data remain accessible
- Blockchain can be enabled/disabled without affecting core camera operations
- Gradual migration: enable blockchain for new captures while maintaining access to old data

### 2. Storage Migration

```cpp
void migrateExistingData() {
    Serial.println("Starting blockchain migration...");
    
    // Get list of existing images
    String imageFiles[100];
    uint16_t fileCount = StorageManager::listFiles("/images", imageFiles, 100);
    
    for (uint16_t i = 0; i < fileCount; i++) {
        String filename = imageFiles[i];
        
        // Calculate hash for existing file
        String fileHash = blockchainManager.getHashingService().calculateFileHash(filename);
        
        // Create migration metadata
        String metadata = "{\"migrated\":true,\"original_file\":\"" + filename + 
                         "\",\"migration_time\":" + String(millis() / 1000) + "}";
        
        // Add to blockchain as system event
        blockchainManager.addSystemEventTransaction("DATA_MIGRATION", metadata);
        
        Serial.println("Migrated: " + filename);
    }
    
    Serial.println("Migration complete");
}
```

## Troubleshooting

### Common Issues

1. **High Memory Usage**: Reduce `BLOCKCHAIN_MAX_BLOCKS_IN_MEMORY` in config
2. **Slow Performance**: Increase `hashFrequency` to reduce hashing frequency
3. **Storage Full**: Enable automatic cleanup or increase storage capacity
4. **Verification Failures**: Check SD card integrity and blockchain files

### Debug Output

Enable debug logging:

```cpp
#define BLOCKCHAIN_DEBUG_ENABLED true
#define BLOCKCHAIN_VERBOSE_LOGGING true
```

This integration guide provides a comprehensive approach to adding blockchain data integrity to your ESP32 Wildlife CAM while maintaining compatibility with existing systems.