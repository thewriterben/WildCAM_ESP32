/**
 * @file main_with_multiboard.cpp
 * @brief Example showing how to integrate multi-board system with existing main.cpp
 * 
 * This file demonstrates how to add multi-board coordination to the existing
 * wildlife camera system with minimal changes to existing code.
 */

#include <Arduino.h>
#include "camera_handler.h"
#include "motion_filter.h"
#include "power_manager.h"
#include "wifi_manager.h"
#include "lora_mesh.h"
#include "config.h"

// Include the new multi-board system
#ifdef MULTIBOARD_ENABLED
#include "multi_board/multi_board_system.h"
#endif

/**
 * Enhanced SystemManager with multi-board support
 */
class SystemManagerWithMultiboard {
public:
    SystemManagerWithMultiboard() :
        multiboardInitialized_(false) {
    }
    
    bool init() {
        // Initialize existing systems first
        Serial.println("Initializing wildlife camera systems...");
        
        if (!CameraHandler::init()) {
            Serial.println("Camera initialization failed");
            return false;
        }
        
        if (!LoraMesh::init()) {
            Serial.println("LoRa mesh initialization failed");
            return false;
        }
        
        // Initialize multi-board system if enabled
        #ifdef MULTIBOARD_ENABLED
        if (MULTIBOARD_ENABLED) {
            Serial.println("Initializing multi-board coordination system...");
            
            if (initializeMultiboardSystem(MULTIBOARD_NODE_ID, MULTIBOARD_PREFERRED_ROLE)) {
                multiboardInitialized_ = true;
                Serial.println("Multi-board system initialized successfully");
                
                // Integrate with existing LoRa mesh
                if (g_multiboardSystem) {
                    g_multiboardSystem->integrateWithLoraMesh(MULTIBOARD_LORA_INTEGRATION);
                }
            } else {
                Serial.println("Warning: Multi-board system initialization failed");
                Serial.println("Continuing with single-board operation");
            }
        }
        #endif
        
        Serial.println("System initialization complete");
        return true;
    }
    
    void update() {
        // Process existing systems
        LoraMesh::processMessages();
        
        // Process multi-board coordination
        #ifdef MULTIBOARD_ENABLED
        if (multiboardInitialized_) {
            processMultiboardSystem();
            
            // Handle multi-board specific tasks
            handleMultiboardTasks();
        }
        #endif
        
        // Continue with regular wildlife monitoring
        handleWildlifeMonitoring();
    }
    
    void handleMotionDetection() {
        Serial.println("Motion detected!");
        
        // Capture image with existing system
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) {
            Serial.printf("Captured image: %d bytes\n", fb->len);
            
            #ifdef MULTIBOARD_ENABLED
            // If we're part of a multi-board network, coordinate the capture
            if (multiboardInitialized_ && g_multiboardSystem) {
                BoardRole role = g_multiboardSystem->getCurrentRole();
                
                if (role == ROLE_COORDINATOR) {
                    // Coordinator: Distribute analysis tasks
                    Serial.println("Coordinator distributing analysis tasks...");
                    
                    DynamicJsonDocument doc(512);
                    JsonObject params = doc.to<JsonObject>();
                    params["image_size"] = fb->len;
                    params["timestamp"] = millis();
                    
                    // Send analysis task to AI-capable nodes
                    auto nodes = g_multiboardSystem->getDiscoveredNodes();
                    for (const auto& node : nodes) {
                        if (node.capabilities.hasAI && node.isActive) {
                            g_multiboardSystem->sendTaskToNode(
                                node.nodeId, "ai_analysis", params, 2);
                        }
                    }
                } else if (role == ROLE_AI_PROCESSOR) {
                    // AI processor: Analyze the image
                    Serial.println("AI processor analyzing image...");
                    // TODO: Integrate with AI analysis system
                }
            }
            #endif
            
            esp_camera_fb_return(fb);
        }
    }
    
    void getSystemStatus() {
        Serial.println("=== System Status ===");
        
        // Existing system status
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Uptime: %lu ms\n", millis());
        
        #ifdef MULTIBOARD_ENABLED
        if (multiboardInitialized_) {
            SystemStats stats = getMultiboardStats();
            Serial.printf("Multi-board State: %d\n", stats.currentState);
            Serial.printf("Current Role: %s\n", MessageProtocol::roleToString(stats.currentRole));
            Serial.printf("Networked Nodes: %d\n", stats.networkedNodes);
            Serial.printf("Active Tasks: %d\n", stats.activeTasks);
            Serial.printf("Network Efficiency: %.2f\n", stats.networkEfficiency);
        }
        #endif
        
        Serial.println();
    }
    
private:
    bool multiboardInitialized_;
    
    void handleMultiboardTasks() {
        #ifdef MULTIBOARD_ENABLED
        if (!g_multiboardSystem) return;
        
        // Handle coordinator-specific tasks
        if (g_multiboardSystem->getCurrentRole() == ROLE_COORDINATOR) {
            static unsigned long lastCoordTask = 0;
            if (millis() - lastCoordTask > 300000) { // Every 5 minutes
                // Broadcast configuration updates
                DynamicJsonDocument doc(512);
                JsonObject config = doc.to<JsonObject>();
                config["capture_interval"] = 300000;
                config["motion_threshold"] = 50;
                
                g_multiboardSystem->broadcastConfiguration(config);
                lastCoordTask = millis();
            }
        }
        
        // Handle node tasks
        auto activeTasks = g_multiboardSystem->getActiveTasks();
        if (!activeTasks.empty()) {
            Serial.printf("Processing %d active tasks\n", activeTasks.size());
        }
        #endif
    }
    
    void handleWildlifeMonitoring() {
        // Regular wildlife monitoring tasks
        static unsigned long lastMonitoring = 0;
        
        if (millis() - lastMonitoring > 60000) { // Every minute
            // Check for motion, capture images, etc.
            // This would be the existing wildlife monitoring logic
            lastMonitoring = millis();
        }
    }
};

// Global system manager instance
SystemManagerWithMultiboard systemManager;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("ESP32 Wildlife Camera with Multi-Board Support");
    Serial.println("=============================================");
    
    if (!systemManager.init()) {
        Serial.println("System initialization failed!");
        return;
    }
    
    Serial.println("Wildlife camera system ready");
    
    // Simulate initial motion detection
    delay(5000);
    systemManager.handleMotionDetection();
}

void loop() {
    systemManager.update();
    
    // Print status every 30 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 30000) {
        systemManager.getSystemStatus();
        lastStatus = millis();
    }
    
    // Simulate motion detection every 2 minutes
    static unsigned long lastMotion = 0;
    if (millis() - lastMotion > 120000) {
        systemManager.handleMotionDetection();
        lastMotion = millis();
    }
    
    delay(100);
}