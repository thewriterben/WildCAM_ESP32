#ifndef LORA_MESH_H
#define LORA_MESH_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_camera.h>

// Signal strength enumeration
enum SignalStrength {
    SIGNAL_POOR = 0,
    SIGNAL_FAIR = 1,
    SIGNAL_GOOD = 2,
    SIGNAL_EXCELLENT = 3
};

// Signal quality structure
struct SignalQuality {
    int rssi;              // Received Signal Strength Indicator
    float snr;             // Signal-to-Noise Ratio
    SignalStrength strength;
};

// Mesh network status structure
struct MeshNetworkStatus {
    bool initialized;
    int nodeId;
    int connectedNodes;
    unsigned long lastMessageTime;
    int rssi;
    float snr;
    unsigned long packetsReceived;
    unsigned long packetsSent;
};

namespace LoraMesh {
    /**
     * Initialize LoRa mesh networking
     * @return true if initialization successful, false otherwise
     */
    bool init();
    
    /**
     * Process incoming messages and handle mesh routing
     */
    void processMessages();
    
    /**
     * Send image data over LoRa mesh (compressed)
     * @param fb Camera frame buffer containing image data
     * @param filename Name of the image file
     * @return true if transmission initiated successfully
     */
    bool transmitImage(camera_fb_t* fb, const String& filename);
    
    /**
     * Send status update to mesh network
     * @param statusData JSON object containing status information
     * @return true if message queued successfully
     */
    bool sendStatus(const JsonObject& statusData);
    
    /**
     * Queue message for transmission
     * @param message Message string to queue
     * @return true if message queued successfully
     */
    bool queueMessage(const String& message);
    
    /**
     * Get mesh network status
     * @return Network status structure
     */
    MeshNetworkStatus getNetworkStatus();
    
    /**
     * Set node ID for this device
     * @param newNodeId Unique node identifier
     */
    void setNodeId(int newNodeId);
    
    /**
     * Get signal quality information
     * @return Signal quality structure
     */
    SignalQuality getSignalQuality();
    
    /**
     * Cleanup LoRa mesh resources
     */
    void cleanup();
}

#endif // LORA_MESH_H