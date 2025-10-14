/**
 * @file reliable_transmission_integration.ino
 * @brief Example integration of Reliable Transmission Protocol with Image and Telemetry
 * 
 * This example demonstrates how to integrate the RTP with existing
 * image transmission and telemetry systems for reliable data delivery
 * in wildlife monitoring deployments.
 */

#include <Arduino.h>
#include "../../firmware/src/meshtastic/reliable_transmission_protocol.h"
#include "../../firmware/src/meshtastic/mesh_interface.h"
#include "../../firmware/src/meshtastic/wildlife_telemetry.h"

// Global instances
MeshInterface* meshInterface = nullptr;
ReliableTransmissionProtocol* rtp = nullptr;
WildlifeTelemetry* telemetry = nullptr;

// Node configuration
const uint32_t NODE_ID = 0x12345678;
const char* NODE_NAME = "WildCAM-01";

// Statistics tracking
struct TransmissionStats {
    uint32_t totalTransmissions;
    uint32_t successfulTransmissions;
    uint32_t failedTransmissions;
    uint32_t totalRetries;
    float averageLatency;
    float successRate;
};

TransmissionStats stats = {0, 0, 0, 0, 0.0, 0.0};

// ===========================
// CALLBACK FUNCTIONS
// ===========================

void onTransmissionComplete(uint32_t transmissionId, bool success) {
    if (success) {
        stats.successfulTransmissions++;
        Serial.printf("✓ Transmission %08X completed successfully\n", transmissionId);
    } else {
        stats.failedTransmissions++;
        Serial.printf("✗ Transmission %08X failed\n", transmissionId);
    }
    
    stats.totalTransmissions++;
    stats.successRate = (float)stats.successfulTransmissions / stats.totalTransmissions * 100.0;
}

void onTransmissionProgress(uint32_t transmissionId, float progress) {
    Serial.printf("→ Transmission %08X: %.1f%% complete\n", transmissionId, progress);
}

void onTransmissionError(uint32_t transmissionId, TransmissionResult error) {
    Serial.printf("⚠ Transmission %08X error: %s\n", transmissionId, resultToString(error));
    
    if (error == TX_MAX_RETRIES) {
        stats.totalRetries++;
        Serial.println("  Max retries exceeded - network may be unavailable");
    }
}

void onMotionEvent(const MotionEvent& event) {
    Serial.println("\n=== Motion Event Detected ===");
    Serial.printf("Confidence: %d%%\n", event.confidence);
    Serial.printf("Duration: %d ms\n", event.duration);
    Serial.printf("Photo taken: %s\n", event.photoTaken ? "Yes" : "No");
    
    if (event.photoTaken) {
        Serial.printf("Photo: %s\n", event.photoFilename.c_str());
    }
}

void onWildlifeDetection(const WildlifeDetection& detection) {
    Serial.println("\n=== Wildlife Detection ===");
    Serial.printf("Species: %s\n", detection.species.c_str());
    Serial.printf("Confidence: %.1f%%\n", detection.confidence * 100);
    Serial.printf("Count: %d\n", detection.animalCount);
    
    if (detection.endangered) {
        Serial.println("⚠ ENDANGERED SPECIES DETECTED!");
    }
}

// ===========================
// INTEGRATION FUNCTIONS
// ===========================

/**
 * Transmit wildlife image with reliable protocol
 */
uint32_t transmitImageReliably(const String& filename, uint32_t destination = 0) {
    Serial.printf("\n--- Transmitting Image: %s ---\n", filename.c_str());
    
    // Read image data (simulated)
    std::vector<uint8_t> imageData;
    // In real implementation, read from SD card:
    // File file = SD.open(filename);
    // imageData.resize(file.size());
    // file.read(imageData.data(), imageData.size());
    // file.close();
    
    // For demo, create sample data
    imageData.resize(5000);  // 5KB sample image
    for (size_t i = 0; i < imageData.size(); i++) {
        imageData[i] = random(0, 256);
    }
    
    // Transmit with high priority
    uint32_t txId = rtp->transmitData(
        destination,
        PACKET_TYPE_IMAGE,
        imageData,
        PRIORITY_HIGH,
        true  // Require acknowledgment
    );
    
    Serial.printf("Image transmission queued: ID=%08X, Size=%d bytes\n", 
                  txId, imageData.size());
    
    return txId;
}

/**
 * Transmit telemetry data with reliable protocol
 */
uint32_t transmitTelemetryReliably(TelemetryType type, const std::vector<uint8_t>& data, 
                                   TransmissionPriority priority = PRIORITY_NORMAL) {
    Serial.printf("\n--- Transmitting Telemetry: Type=%d ---\n", type);
    
    uint32_t txId = rtp->transmitData(
        0,  // Broadcast
        PACKET_TYPE_TELEMETRY,
        data,
        priority,
        true  // Require acknowledgment
    );
    
    Serial.printf("Telemetry transmission queued: ID=%08X, Size=%d bytes\n", 
                  txId, data.size());
    
    return txId;
}

/**
 * Handle motion detection with reliable transmission
 */
void handleMotionDetection(uint16_t confidence, bool photoTaken, const String& filename) {
    // Create motion event
    MotionEvent event;
    event.timestamp = millis();
    event.confidence = confidence;
    event.duration = 1500;  // 1.5 seconds
    event.pixelChanges = 250;
    event.triggerZone = 0;
    event.batteryVoltage = 3.8;
    event.temperature = 250;  // 25.0°C
    event.photoTaken = photoTaken;
    event.photoFilename = filename;
    
    // Record in telemetry system
    telemetry->recordMotionEvent(event);
    
    // If photo was taken, transmit image with high priority
    if (photoTaken) {
        transmitImageReliably(filename);
    }
}

/**
 * Handle wildlife detection with reliable transmission
 */
void handleWildlifeDetection(const String& species, float confidence, bool endangered) {
    // Create detection
    WildlifeDetection detection;
    detection.timestamp = millis();
    detection.species = species;
    detection.confidence = confidence;
    detection.boundingBoxX = 100;
    detection.boundingBoxY = 100;
    detection.boundingBoxW = 200;
    detection.boundingBoxH = 200;
    detection.animalCount = 1;
    detection.behavior = "foraging";
    detection.endangered = endangered;
    detection.photoFilename = "wildlife_" + String(millis()) + ".jpg";
    
    // Record in telemetry system
    telemetry->recordWildlifeDetection(detection);
    
    // Transmit image with appropriate priority
    TransmissionPriority priority = endangered ? PRIORITY_CRITICAL : PRIORITY_HIGH;
    
    // For endangered species, use critical priority
    if (endangered) {
        Serial.println("\n⚠⚠⚠ ENDANGERED SPECIES - CRITICAL TRANSMISSION ⚠⚠⚠");
    }
    
    transmitImageReliably(detection.photoFilename);
}

/**
 * Print transmission statistics
 */
void printStatistics() {
    Serial.println("\n=== Transmission Statistics ===");
    Serial.printf("Total: %d\n", stats.totalTransmissions);
    Serial.printf("Successful: %d\n", stats.successfulTransmissions);
    Serial.printf("Failed: %d\n", stats.failedTransmissions);
    Serial.printf("Success Rate: %.1f%%\n", stats.successRate);
    Serial.printf("Retries: %d\n", stats.totalRetries);
    
    BandwidthStats bandwidth = rtp->getStatistics();
    Serial.println("\n=== Bandwidth Statistics ===");
    Serial.printf("Packets TX: %d\n", bandwidth.packetsTransmitted);
    Serial.printf("Packets RX: %d\n", bandwidth.packetsReceived);
    Serial.printf("Packets Lost: %d\n", bandwidth.packetsLost);
    Serial.printf("Loss Rate: %.2f%%\n", bandwidth.packetLossRate * 100);
    Serial.printf("Avg Latency: %.1f ms\n", bandwidth.averageLatency);
    Serial.printf("Throughput: %.1f bytes/s\n", bandwidth.throughput);
    
    Serial.println("\n=== Active Transmissions ===");
    std::vector<TransmissionStatus> active = rtp->getAllTransmissions();
    Serial.printf("Count: %d\n", active.size());
    
    for (const auto& status : active) {
        Serial.printf("\nID: %08X\n", status.transmissionId);
        Serial.printf("  State: %s\n", stateToString(status.state));
        Serial.printf("  Priority: %s\n", priorityToString(status.priority));
        Serial.printf("  Progress: %.1f%%\n", status.progressPercentage);
        Serial.printf("  Packets: %d/%d\n", status.packetsAcknowledged, status.totalPackets);
        Serial.printf("  Bytes: %d/%d\n", status.bytesTransmitted, status.totalBytes);
        Serial.printf("  Retries: %d\n", status.retryCount);
    }
}

// ===========================
// SETUP AND LOOP
// ===========================

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== Reliable Transmission Protocol Integration Example ===\n");
    
    // Initialize mesh interface
    Serial.println("Initializing mesh interface...");
    meshInterface = new MeshInterface();
    if (!meshInterface->init(NODE_ID, NODE_NAME)) {
        Serial.println("✗ Failed to initialize mesh interface");
        return;
    }
    Serial.println("✓ Mesh interface initialized");
    
    // Initialize reliable transmission protocol
    Serial.println("Initializing reliable transmission protocol...");
    rtp = createReliableTransmissionProtocol(meshInterface);
    if (!rtp) {
        Serial.println("✗ Failed to create RTP");
        return;
    }
    Serial.println("✓ RTP initialized");
    
    // Configure for low-bandwidth LoRa environment
    Serial.println("Configuring for low-bandwidth deployment...");
    ReliableTransmissionConfig config = createLowBandwidthConfig();
    rtp->configure(config);
    Serial.println("✓ RTP configured");
    
    // Set callbacks
    rtp->setTransmissionCompleteCallback(onTransmissionComplete);
    rtp->setTransmissionProgressCallback(onTransmissionProgress);
    rtp->setTransmissionErrorCallback(onTransmissionError);
    
    // Initialize telemetry system
    Serial.println("Initializing telemetry system...");
    telemetry = createWildlifeTelemetry(meshInterface);
    if (!telemetry) {
        Serial.println("✗ Failed to create telemetry system");
        return;
    }
    Serial.println("✓ Telemetry system initialized");
    
    // Set telemetry callbacks
    telemetry->setMotionEventCallback(onMotionEvent);
    telemetry->setWildlifeDetectionCallback(onWildlifeDetection);
    
    // Start automatic telemetry collection
    telemetry->startAutomaticCollection();
    
    Serial.println("\n=== System Ready ===\n");
    Serial.println("Commands:");
    Serial.println("  m - Simulate motion detection");
    Serial.println("  w - Simulate wildlife detection");
    Serial.println("  e - Simulate endangered species detection");
    Serial.println("  i - Transmit test image");
    Serial.println("  s - Print statistics");
    Serial.println("  r - Reset statistics");
    Serial.println("  h - Print help");
}

void loop() {
    // Process RTP and telemetry
    rtp->process();
    telemetry->process();
    
    // Handle serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'm':
                Serial.println("\n>>> Simulating motion detection");
                handleMotionDetection(85, true, "motion_" + String(millis()) + ".jpg");
                break;
                
            case 'w':
                Serial.println("\n>>> Simulating wildlife detection");
                handleWildlifeDetection("Deer", 0.92, false);
                break;
                
            case 'e':
                Serial.println("\n>>> Simulating endangered species detection");
                handleWildlifeDetection("Red Panda", 0.95, true);
                break;
                
            case 'i':
                Serial.println("\n>>> Transmitting test image");
                transmitImageReliably("test_image.jpg");
                break;
                
            case 's':
                printStatistics();
                break;
                
            case 'r':
                Serial.println("\n>>> Resetting statistics");
                rtp->resetStatistics();
                memset(&stats, 0, sizeof(stats));
                Serial.println("✓ Statistics reset");
                break;
                
            case 'h':
            case '?':
                Serial.println("\n=== Commands ===");
                Serial.println("  m - Simulate motion detection");
                Serial.println("  w - Simulate wildlife detection");
                Serial.println("  e - Simulate endangered species detection");
                Serial.println("  i - Transmit test image");
                Serial.println("  s - Print statistics");
                Serial.println("  r - Reset statistics");
                Serial.println("  h - Print help");
                break;
        }
    }
    
    // Print periodic statistics every 30 seconds
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime > 30000) {
        printStatistics();
        lastStatsTime = millis();
    }
    
    delay(10);
}
