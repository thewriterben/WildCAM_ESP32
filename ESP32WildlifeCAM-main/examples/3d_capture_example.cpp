/**
 * @file 3d_capture_example.cpp
 * @brief Example of 3D reconstruction using multiple synchronized cameras
 * 
 * This example demonstrates how to set up and trigger synchronized
 * captures across multiple ESP32-CAM boards for 3D reconstruction
 * of wildlife subjects.
 * 
 * Hardware Requirements:
 * - 3+ ESP32-CAM boards
 * - Multi-board communication enabled (LoRa mesh or WiFi)
 * - SD cards for image storage
 * - Physical mounting for cameras
 * 
 * Setup:
 * 1. Position cameras around subject area
 * 2. Measure camera positions and orientations
 * 3. Configure each board with unique node ID
 * 4. Flash firmware with multi-board support enabled
 * 5. Run this example on the coordinator board
 */

#include <Arduino.h>
#include "multi_board/multi_board_system.h"
#include "config.h"

// Multi-board system
MultiboardSystem* multiboardSystem = nullptr;

// Camera configuration for 3D capture
struct Camera3DConfig {
    int nodeId;
    float posX;
    float posY;
    float posZ;
    float pitch;
    float yaw;
    float roll;
};

// Example: 5-camera semi-circular array
// Cameras positioned in an arc around the subject at ground level
const Camera3DConfig CAMERA_ARRAY[] = {
    // Node ID, X(m), Y(m), Z(m), Pitch(°), Yaw(°), Roll(°)
    {1,  -3.0,  0.0,  2.0,   0.0,   45.0,  0.0},  // Far left
    {2,  -1.5,  0.0,  2.5,   0.0,   22.5,  0.0},  // Left
    {3,   0.0,  0.0,  3.0,   0.0,    0.0,  0.0},  // Center (front)
    {4,   1.5,  0.0,  2.5,   0.0,  337.5,  0.0},  // Right
    {5,   3.0,  0.0,  2.0,   0.0,  315.0,  0.0},  // Far right
};

const int NUM_CAMERAS = sizeof(CAMERA_ARRAY) / sizeof(CAMERA_ARRAY[0]);

// Session counter for unique identifiers
int sessionCounter = 0;

/**
 * @brief Trigger synchronized 3D capture across all cameras
 * @param sessionId Unique identifier for this capture session
 * @param syncDelayMs Delay to allow for network propagation (milliseconds)
 * @return true if all tasks were queued successfully
 */
bool trigger3DCapture(const String& sessionId, uint32_t syncDelayMs = 100) {
    Serial.println("======================================");
    Serial.printf("Triggering 3D Capture: %s\n", sessionId.c_str());
    Serial.println("======================================");
    
    bool allQueued = true;
    
    // Send 3D capture task to each camera node
    for (int i = 0; i < NUM_CAMERAS; i++) {
        const Camera3DConfig& cam = CAMERA_ARRAY[i];
        
        DynamicJsonDocument params(512);
        params["session_id"] = sessionId;
        params["position_x"] = cam.posX;
        params["position_y"] = cam.posY;
        params["position_z"] = cam.posZ;
        params["orientation_pitch"] = cam.pitch;
        params["orientation_yaw"] = cam.yaw;
        params["orientation_roll"] = cam.roll;
        params["sync_delay_ms"] = syncDelayMs;
        params["folder"] = "/3d_captures";
        
        Serial.printf("  Camera %d (Node %d): pos(%.1f, %.1f, %.1f) yaw=%.1f°\n",
                     i+1, cam.nodeId, cam.posX, cam.posY, cam.posZ, cam.yaw);
        
        if (!multiboardSystem->sendTaskToNode(cam.nodeId, "3d_capture", params.as<JsonObject>())) {
            Serial.printf("  ERROR: Failed to queue task for node %d\n", cam.nodeId);
            allQueued = false;
        }
    }
    
    if (allQueued) {
        Serial.println("All 3D capture tasks queued successfully");
    } else {
        Serial.println("WARNING: Some tasks failed to queue");
    }
    
    Serial.println("======================================");
    return allQueued;
}

/**
 * @brief Trigger a single 3D capture with automatic session ID
 */
void capture3D() {
    String sessionId = "session_" + String(sessionCounter++);
    trigger3DCapture(sessionId);
}

/**
 * @brief Trigger 3D time-lapse capture
 * @param intervalMs Time between captures in milliseconds
 * @param numCaptures Number of captures to perform
 */
void timeLapse3DCapture(unsigned long intervalMs, int numCaptures) {
    Serial.println("======================================");
    Serial.printf("Starting 3D Time-Lapse\n");
    Serial.printf("  Interval: %lu seconds\n", intervalMs / 1000);
    Serial.printf("  Captures: %d\n", numCaptures);
    Serial.println("======================================");
    
    for (int i = 0; i < numCaptures; i++) {
        String sessionId = "timelapse_" + String(i);
        
        Serial.printf("\n--- Time-lapse capture %d/%d ---\n", i+1, numCaptures);
        trigger3DCapture(sessionId);
        
        if (i < numCaptures - 1) {
            Serial.printf("Waiting %lu seconds until next capture...\n\n", intervalMs / 1000);
            delay(intervalMs);
        }
    }
    
    Serial.println("======================================");
    Serial.println("3D Time-Lapse Complete!");
    Serial.println("======================================");
}

/**
 * @brief Motion-triggered 3D capture
 * Captures 3D model when motion is detected by any camera
 */
void motionTriggered3DCapture() {
    static unsigned long lastCapture = 0;
    const unsigned long CAPTURE_COOLDOWN = 60000; // 1 minute between captures
    
    // Check if enough time has passed since last capture
    if (millis() - lastCapture < CAPTURE_COOLDOWN) {
        return;
    }
    
    // TODO: Integrate with motion detection system
    // For now, this is a placeholder for motion detection logic
    bool motionDetected = false; // Replace with actual motion detection
    
    if (motionDetected) {
        Serial.println("Motion detected! Triggering 3D capture...");
        capture3D();
        lastCapture = millis();
    }
}

/**
 * @brief Print camera array configuration
 */
void printCameraConfiguration() {
    Serial.println("\n======================================");
    Serial.println("3D Camera Array Configuration");
    Serial.println("======================================");
    Serial.printf("Number of cameras: %d\n\n", NUM_CAMERAS);
    
    for (int i = 0; i < NUM_CAMERAS; i++) {
        const Camera3DConfig& cam = CAMERA_ARRAY[i];
        Serial.printf("Camera %d (Node %d):\n", i+1, cam.nodeId);
        Serial.printf("  Position: (%.2f, %.2f, %.2f) meters\n", cam.posX, cam.posY, cam.posZ);
        Serial.printf("  Orientation:\n");
        Serial.printf("    Pitch: %.1f°\n", cam.pitch);
        Serial.printf("    Yaw:   %.1f°\n", cam.yaw);
        Serial.printf("    Roll:  %.1f°\n", cam.roll);
        Serial.println();
    }
    Serial.println("======================================\n");
}

/**
 * @brief Display help menu
 */
void printHelp() {
    Serial.println("\n======================================");
    Serial.println("3D Capture Commands");
    Serial.println("======================================");
    Serial.println("1 - Single 3D capture");
    Serial.println("2 - Time-lapse (5 captures, 30 sec interval)");
    Serial.println("3 - Long time-lapse (10 captures, 5 min interval)");
    Serial.println("c - Show camera configuration");
    Serial.println("s - Show system status");
    Serial.println("h - Show this help");
    Serial.println("======================================\n");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n");
    Serial.println("======================================");
    Serial.println("3D Wildlife Capture Example");
    Serial.println("Multi-Camera Synchronization Demo");
    Serial.println("======================================\n");
    
    // Initialize multi-board system as coordinator
    Serial.println("Initializing multi-board system...");
    
    SystemConfig config = {
        .nodeId = 100,  // Coordinator node ID
        .preferredRole = ROLE_COORDINATOR,
        .enableAutomaticRoleSelection = false,
        .enableStandaloneFallback = false,
        .discoveryTimeout = 120000,
        .roleChangeTimeout = 60000
    };
    
    multiboardSystem = new MultiboardSystem();
    
    if (!multiboardSystem->init(config)) {
        Serial.println("ERROR: Failed to initialize multi-board system");
        Serial.println("Check configuration and try again.");
        while (1) {
            delay(1000);
        }
    }
    
    if (!multiboardSystem->start()) {
        Serial.println("ERROR: Failed to start multi-board system");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("Multi-board system started successfully");
    Serial.println("Coordinator role active\n");
    
    // Wait for nodes to connect
    Serial.println("Waiting for camera nodes to connect...");
    Serial.println("This may take up to 2 minutes...");
    delay(5000);
    
    // Check connected nodes
    std::vector<NetworkNode> nodes = multiboardSystem->getDiscoveredNodes();
    Serial.printf("Discovered %d nodes\n", nodes.size());
    
    if (nodes.size() < NUM_CAMERAS) {
        Serial.println("WARNING: Not all camera nodes are connected!");
        Serial.printf("Expected: %d, Found: %d\n", NUM_CAMERAS, nodes.size());
        Serial.println("Proceeding anyway...\n");
    } else {
        Serial.println("All camera nodes connected!\n");
    }
    
    // Print camera configuration
    printCameraConfiguration();
    
    // Print help
    printHelp();
    
    Serial.println("Ready for 3D capture!");
    Serial.println("Send commands via Serial Monitor\n");
}

void loop() {
    // Process multi-board system
    multiboardSystem->process();
    
    // Handle serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case '1':
                Serial.println("\n> Single 3D capture");
                capture3D();
                break;
                
            case '2':
                Serial.println("\n> Time-lapse: 5 captures @ 30 seconds");
                timeLapse3DCapture(30000, 5);
                break;
                
            case '3':
                Serial.println("\n> Long time-lapse: 10 captures @ 5 minutes");
                timeLapse3DCapture(300000, 10);
                break;
                
            case 'c':
            case 'C':
                printCameraConfiguration();
                break;
                
            case 's':
            case 'S': {
                Serial.println("\n======================================");
                Serial.println("System Status");
                Serial.println("======================================");
                
                SystemStats stats = multiboardSystem->getSystemStats();
                Serial.printf("State: %d\n", stats.currentState);
                Serial.printf("Role: %s\n", MessageProtocol::roleToString(stats.currentRole));
                Serial.printf("Networked nodes: %d\n", stats.networkedNodes);
                Serial.printf("Active tasks: %d\n", stats.activeTasks);
                Serial.printf("Uptime: %lu seconds\n", stats.systemUptime / 1000);
                Serial.println("======================================\n");
                break;
            }
                
            case 'h':
            case 'H':
                printHelp();
                break;
                
            default:
                // Ignore other characters
                break;
        }
    }
    
    // Check for motion-triggered capture (optional)
    // motionTriggered3DCapture();
    
    delay(100);
}

/**
 * Post-Processing Instructions:
 * 
 * After capturing images, follow these steps:
 * 
 * 1. Collect SD cards from all camera boards
 * 
 * 2. Copy images to your computer:
 *    - Mount each SD card
 *    - Copy /3d_captures folder to a common directory
 * 
 * 3. Run the aggregation script:
 *    python3 tools/3d_reconstruction_aggregator.py \
 *        --session session_0 \
 *        --input /path/to/sd_captures \
 *        --output ./3d_output
 * 
 * 4. Reconstruct the 3D model:
 *    cd 3d_output
 *    ./run_meshroom.sh    # For Meshroom
 *    # or
 *    ./run_colmap.sh      # For COLMAP
 * 
 * 5. View results:
 *    - Meshroom: Open .mg project file in Meshroom
 *    - COLMAP: View .ply file in MeshLab or similar
 * 
 * For detailed instructions, see:
 * docs/3D_RECONSTRUCTION_GUIDE.md
 */
