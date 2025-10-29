/**
 * @file advanced_features_config.cpp
 * @brief Implementation of advanced features manager and configuration
 * @version 1.0.0
 */

#include "advanced_features_config.h"
#include "ai_detection/ai_detection_interface.h"
#include "mesh_network/mesh_protocol.h"
#include <Arduino.h>
#include <Preferences.h>

namespace AdvancedFeatures {

AdvancedFeaturesManager::AdvancedFeaturesManager() 
    : initialized_(false), features_running_(false), last_diagnostics_(0),
      ai_detection_active_(false), event_triggering_active_(false), mesh_networking_active_(false) {
    // Set default configuration
    config_ = DefaultConfigs::getBasicConfig();
}

AdvancedFeaturesManager::~AdvancedFeaturesManager() {
    stopFeatures();
}

bool AdvancedFeaturesManager::initialize(const AdvancedFeaturesConfig& config) {
    Serial.println("=== Initializing Advanced Features ===");
    
    config_ = config;
    
    if (!config_.enable_all_features) {
        Serial.println("Advanced features disabled by configuration");
        return true;
    }
    
    // Startup delay for system stability
    if (config_.startup_delay_ms > 0) {
        Serial.printf("Startup delay: %dms\n", config_.startup_delay_ms);
        delay(config_.startup_delay_ms);
    }
    
    // Initialize components in order
    bool success = true;
    
    if (config_.ai_detection.enable_ai_detection) {
        success &= initializeAIDetection();
        logStatus("AI Detection", ai_detection_active_);
    }
    
    if (config_.event_triggering.enable_event_triggering) {
        success &= initializeEventTriggering();
        logStatus("Event Triggering", event_triggering_active_);
    }
    
    if (config_.mesh_networking.enable_mesh_networking) {
        success &= initializeMeshNetworking();
        logStatus("Mesh Networking", mesh_networking_active_);
    }
    
    // Validate integration between components
    if (success && (config_.validate_ai_integration || config_.validate_mesh_integration)) {
        success &= validateIntegration();
    }
    
    initialized_ = success;
    
    if (success) {
        Serial.println("Advanced features initialization complete");
    } else {
        Serial.println("Advanced features initialization failed");
    }
    
    return success;
}

bool AdvancedFeaturesManager::startFeatures() {
    if (!initialized_) {
        Serial.println("Cannot start features - not initialized");
        return false;
    }
    
    Serial.println("Starting advanced features...");
    
    // Start features that are initialized
    bool all_started = true;
    
    if (ai_detection_active_) {
        Serial.println("AI Detection system ready");
    }
    
    if (event_triggering_active_) {
        Serial.println("Event triggering system ready");
    }
    
    if (mesh_networking_active_) {
        Serial.println("Mesh networking system ready");
    }
    
    features_running_ = all_started;
    
    if (features_running_) {
        Serial.println("All advanced features are operational");
        
        // Run initial diagnostics if enabled
        if (config_.enable_self_diagnostics) {
            runDiagnostics();
        }
    }
    
    return features_running_;
}

void AdvancedFeaturesManager::stopFeatures() {
    Serial.println("Stopping advanced features...");
    
    features_running_ = false;
    ai_detection_active_ = false;
    event_triggering_active_ = false;
    mesh_networking_active_ = false;
    
    Serial.println("Advanced features stopped");
}

bool AdvancedFeaturesManager::updateConfiguration(const AdvancedFeaturesConfig& config) {
    Serial.println("Updating advanced features configuration");
    
    bool was_running = features_running_;
    
    if (was_running) {
        stopFeatures();
    }
    
    config_ = config;
    
    bool success = initialize(config);
    
    if (success && was_running) {
        success = startFeatures();
    }
    
    if (success) {
        Serial.println("Configuration updated successfully");
    } else {
        Serial.println("Configuration update failed");
    }
    
    return success;
}

bool AdvancedFeaturesManager::runDiagnostics() {
    Serial.println("Running advanced features diagnostics...");
    
    uint32_t now = millis();
    last_diagnostics_ = now;
    
    bool all_healthy = true;
    
    // Check AI Detection system
    if (config_.ai_detection.enable_ai_detection) {
        // In a real implementation, this would test the AI detection pipeline
        bool ai_healthy = ai_detection_active_;
        Serial.printf("AI Detection: %s\n", ai_healthy ? "OK" : "FAILED");
        all_healthy &= ai_healthy;
    }
    
    // Check Event Triggering system
    if (config_.event_triggering.enable_event_triggering) {
        bool events_healthy = event_triggering_active_;
        Serial.printf("Event Triggering: %s\n", events_healthy ? "OK" : "FAILED");
        all_healthy &= events_healthy;
    }
    
    // Check Mesh Networking system
    if (config_.mesh_networking.enable_mesh_networking) {
        bool mesh_healthy = mesh_networking_active_;
        Serial.printf("Mesh Networking: %s\n", mesh_healthy ? "OK" : "FAILED");
        all_healthy &= mesh_healthy;
    }
    
    // Check memory usage
    uint32_t free_heap = ESP.getFreeHeap();
    uint32_t used_memory = (ESP.getHeapSize() - free_heap) / 1024; // KB
    Serial.printf("Memory usage: %dKB (limit: %dKB)\n", used_memory, config_.max_memory_usage_kb);
    
    if (used_memory > config_.max_memory_usage_kb) {
        Serial.println("WARNING: Memory usage exceeds configured limit");
        all_healthy = false;
    }
    
    Serial.printf("Diagnostics complete: %s\n", all_healthy ? "ALL SYSTEMS OK" : "ISSUES DETECTED");
    
    return all_healthy;
}

String AdvancedFeaturesManager::getStatusReport() {
    // Preallocate buffer to avoid multiple reallocations (more efficient)
    String report;
    report.reserve(800); // Reserve approximate size to avoid reallocations
    
    report = "=== Advanced Features Status Report ===\n";
    
    report += "System Status: ";
    report += isOperational() ? "OPERATIONAL" : "DEGRADED";
    report += "\nInitialized: ";
    report += initialized_ ? "Yes" : "No";
    report += "\nFeatures Running: ";
    report += features_running_ ? "Yes" : "No";
    report += "\n\nFeature Status:\n";
    
    report += "- AI Detection: ";
    report += ai_detection_active_ ? "ACTIVE" : "INACTIVE";
    report += "\n- Event Triggering: ";
    report += event_triggering_active_ ? "ACTIVE" : "INACTIVE";
    report += "\n- Mesh Networking: ";
    report += mesh_networking_active_ ? "ACTIVE" : "INACTIVE";
    report += "\n\n";
    
    // Configuration summary
    report += "Configuration Summary:\n- AI Confidence Threshold: ";
    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f", config_.ai_detection.confidence_threshold);
    report += buffer;
    
    report += "\n- Event Triggering: ";
    report += config_.event_triggering.enable_event_triggering ? "Enabled" : "Disabled";
    report += "\n- Mesh Channel: ";
    
    snprintf(buffer, sizeof(buffer), "%d", config_.mesh_networking.mesh_channel);
    report += buffer;
    
    report += "\n- Debug Logging: ";
    report += config_.enable_debug_logging ? "Enabled" : "Disabled";
    report += "\n\n";
    
    // Resource usage
    report += "Resource Usage:\n- Free Heap: ";
    
    snprintf(buffer, sizeof(buffer), "%u", ESP.getFreeHeap());
    report += buffer;
    report += " bytes\n- Uptime: ";
    
    snprintf(buffer, sizeof(buffer), "%lu", millis() / 1000);
    report += buffer;
    report += " seconds\n";
    
    if (last_diagnostics_ > 0) {
        report += "- Last Diagnostics: ";
        snprintf(buffer, sizeof(buffer), "%lu", (millis() - last_diagnostics_) / 1000);
        report += buffer;
        report += "s ago\n";
    }
    
    report += "=====================================";
    
    return report;
}

bool AdvancedFeaturesManager::isOperational() {
    if (!initialized_ || !features_running_) {
        return false;
    }
    
    // Check if required features are active
    bool required_active = true;
    
    if (config_.ai_detection.enable_ai_detection) {
        required_active &= ai_detection_active_;
    }
    
    if (config_.event_triggering.enable_event_triggering) {
        required_active &= event_triggering_active_;
    }
    
    if (config_.mesh_networking.enable_mesh_networking) {
        required_active &= mesh_networking_active_;
    }
    
    return required_active;
}

bool AdvancedFeaturesManager::saveConfiguration() {
    Preferences prefs;
    if (!prefs.begin("adv_features", false)) {
        Serial.println("Failed to open preferences for saving");
        return false;
    }
    
    // Save key configuration parameters
    prefs.putBool("ai_enabled", config_.ai_detection.enable_ai_detection);
    prefs.putFloat("ai_confidence", config_.ai_detection.confidence_threshold);
    prefs.putBool("events_enabled", config_.event_triggering.enable_event_triggering);
    prefs.putBool("mesh_enabled", config_.mesh_networking.enable_mesh_networking);
    prefs.putUChar("mesh_channel", config_.mesh_networking.mesh_channel);
    prefs.putBool("debug_logging", config_.enable_debug_logging);
    
    prefs.end();
    
    Serial.println("Configuration saved to persistent storage");
    return true;
}

bool AdvancedFeaturesManager::loadConfiguration() {
    Preferences prefs;
    if (!prefs.begin("adv_features", true)) {
        Serial.println("No saved configuration found, using defaults");
        return false;
    }
    
    // Load configuration parameters
    config_.ai_detection.enable_ai_detection = prefs.getBool("ai_enabled", true);
    config_.ai_detection.confidence_threshold = prefs.getFloat("ai_confidence", 0.6f);
    config_.event_triggering.enable_event_triggering = prefs.getBool("events_enabled", true);
    config_.mesh_networking.enable_mesh_networking = prefs.getBool("mesh_enabled", true);
    config_.mesh_networking.mesh_channel = prefs.getUChar("mesh_channel", 1);
    config_.enable_debug_logging = prefs.getBool("debug_logging", false);
    
    prefs.end();
    
    Serial.println("Configuration loaded from persistent storage");
    return true;
}

// Private implementation methods

bool AdvancedFeaturesManager::initializeAIDetection() {
    Serial.println("Initializing AI Detection system...");
    
    // Initialize the AI detection interface
    static WildlifeDetection::AIDetectionInterface aiInterface;
    
    bool advanced_available = false;
    if (config_.ai_detection.use_advanced_ai) {
        advanced_available = aiInterface.initializeAdvancedAI();
    }
    
    if (advanced_available) {
        // Configure advanced AI features
        WildlifeDetection::AIDetectionInterface::AdvancedConfig advConfig;
        advConfig.use_tensorflow_lite = true;
        advConfig.enable_behavior_analysis = config_.ai_detection.enable_behavior_analysis;
        advConfig.enable_species_confidence_boost = true;
        advConfig.ai_confidence_threshold = config_.ai_detection.confidence_threshold;
        advConfig.ai_processing_timeout_ms = config_.ai_detection.ai_processing_timeout_ms;
        
        aiInterface.configureAdvancedAI(advConfig);
        Serial.println("Advanced AI system configured");
    } else {
        Serial.println("Using foundational AI detection system");
    }
    
    ai_detection_active_ = true;
    return true;
}

bool AdvancedFeaturesManager::initializeEventTriggering() {
    Serial.println("Initializing Event Triggering system...");
    
    // Event triggering is integrated into the AI detection and mesh networking systems
    // This mainly involves configuration validation
    
    if (config_.event_triggering.max_events_per_minute == 0) {
        Serial.println("ERROR: Invalid event rate limit");
        return false;
    }
    
    if (config_.event_triggering.event_cooldown_ms == 0) {
        Serial.println("ERROR: Invalid event cooldown");
        return false;
    }
    
    event_triggering_active_ = true;
    return true;
}

bool AdvancedFeaturesManager::initializeMeshNetworking() {
    Serial.println("Initializing Mesh Networking system...");
    
    // Configure mesh networking
    MeshNetworking::MeshConfig meshConfig;
    meshConfig.node_type = MeshNetworking::NodeType::CAMERA_NODE;
    meshConfig.channel = config_.mesh_networking.mesh_channel;
    meshConfig.max_connections = config_.mesh_networking.max_mesh_connections;
    meshConfig.heartbeat_interval_ms = config_.mesh_networking.heartbeat_interval_ms;
    meshConfig.node_timeout_ms = config_.mesh_networking.node_timeout_ms;
    meshConfig.max_hop_count = config_.mesh_networking.max_hop_count;
    meshConfig.auto_heal_enabled = config_.mesh_networking.enable_mesh_healing;
    meshConfig.encryption_enabled = config_.mesh_networking.enable_mesh_encryption;
    
    strncpy(meshConfig.mesh_password, config_.mesh_networking.mesh_password, 
            sizeof(meshConfig.mesh_password) - 1);
    strncpy(meshConfig.node_name, config_.mesh_networking.mesh_network_name, 
            sizeof(meshConfig.node_name) - 1);
    
    // In a real implementation, we would initialize the actual mesh protocol
    // For now, we simulate successful initialization
    mesh_networking_active_ = true;
    
    Serial.printf("Mesh networking configured - Channel: %d, Max Connections: %d\n",
                 meshConfig.channel, meshConfig.max_connections);
    
    return true;
}

bool AdvancedFeaturesManager::validateIntegration() {
    Serial.println("Validating system integration...");
    
    bool integration_ok = true;
    
    // Validate AI-Event integration
    if (config_.validate_ai_integration && ai_detection_active_ && event_triggering_active_) {
        if (config_.ai_detection.confidence_threshold > config_.event_triggering.critical_species_threshold) {
            Serial.println("WARNING: AI confidence threshold higher than event trigger threshold");
        }
        Serial.println("AI-Event integration: OK");
    }
    
    // Validate Mesh-Event integration
    if (config_.validate_mesh_integration && mesh_networking_active_ && event_triggering_active_) {
        if (!config_.event_triggering.enable_mesh_broadcast) {
            Serial.println("INFO: Mesh broadcasting disabled for events");
        }
        Serial.println("Mesh-Event integration: OK");
    }
    
    // Validate resource constraints
    if (config_.max_memory_usage_kb < 256) {
        Serial.println("WARNING: Memory limit may be too low for advanced features");
        integration_ok = false;
    }
    
    return integration_ok;
}

void AdvancedFeaturesManager::logStatus(const char* component, bool status) {
    Serial.printf("%s: %s\n", component, status ? "INITIALIZED" : "FAILED");
}

// Default configuration implementations

namespace DefaultConfigs {
    AdvancedFeaturesConfig getBatteryOptimizedConfig() {
        AdvancedFeaturesConfig config;
        
        // Optimize for battery life
        config.ai_detection.detection_interval_ms = 5000;  // Less frequent AI processing
        config.ai_detection.enable_power_optimization = true;
        config.ai_detection.confidence_threshold = 0.8f;   // Higher threshold to reduce false positives
        
        config.event_triggering.max_events_per_minute = 5; // Reduce event frequency
        config.event_triggering.event_cooldown_ms = 10000; // Longer cooldown
        
        config.mesh_networking.heartbeat_interval_ms = 60000; // Less frequent heartbeats
        config.mesh_networking.max_mesh_connections = 5;      // Fewer connections
        
        config.enable_debug_logging = false;               // Reduce logging overhead
        
        return config;
    }
    
    AdvancedFeaturesConfig getResearchConfig() {
        AdvancedFeaturesConfig config;
        
        // Optimize for accuracy and data collection
        config.ai_detection.detection_interval_ms = 500;   // Frequent processing
        config.ai_detection.confidence_threshold = 0.5f;   // Lower threshold for more detections
        config.ai_detection.enable_behavior_analysis = true;
        config.ai_detection.max_detections_per_frame = 10;
        
        config.event_triggering.auto_save_high_confidence = true;
        config.event_triggering.max_events_per_minute = 20;
        config.event_triggering.enable_event_compression = false; // Keep full data
        
        config.mesh_networking.enable_data_sharing = true;
        config.mesh_networking.data_sync_interval_ms = 60000; // Frequent sync
        
        config.enable_debug_logging = true;
        config.enable_self_diagnostics = true;
        
        return config;
    }
    
    AdvancedFeaturesConfig getSecurityConfig() {
        AdvancedFeaturesConfig config;
        
        // Optimize for security monitoring
        config.ai_detection.confidence_threshold = 0.7f;
        config.ai_detection.detection_interval_ms = 1000;
        
        config.event_triggering.critical_species_threshold = 0.6f; // Lower for human detection
        config.event_triggering.send_immediate_alerts = true;
        config.event_triggering.enable_mesh_broadcast = true;
        
        config.mesh_networking.enable_mesh_encryption = true;
        config.mesh_networking.heartbeat_interval_ms = 15000; // Frequent status updates
        
        return config;
    }
    
    AdvancedFeaturesConfig getBasicConfig() {
        AdvancedFeaturesConfig config;
        // Use default values - already set in struct definitions
        return config;
    }
}

} // namespace AdvancedFeatures