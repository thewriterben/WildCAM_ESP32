/**
 * @file advanced_features_config.h
 * @brief Configuration interface for advanced software features
 * @version 1.0.0
 * 
 * This file provides comprehensive configuration options for the three key
 * advanced software features: AI Detection, Event Triggering, and Mesh Networking.
 * All features are configurable and well-documented as specified.
 */

#ifndef ADVANCED_FEATURES_CONFIG_H
#define ADVANCED_FEATURES_CONFIG_H

#include <stdint.h>
#include "ai_detection/wildlife_detector.h"
#include "mesh_network/mesh_protocol.h"

namespace AdvancedFeatures {

/**
 * @brief AI Detection Configuration
 * 
 * Configures AI-powered wildlife detection algorithms for accurate species identification
 * and false positive reduction.
 */
struct AIDetectionConfig {
    // Core AI settings
    bool enable_ai_detection = true;
    bool use_advanced_ai = true;                    // Use TensorFlow Lite models if available
    bool enable_species_identification = true;     // Enable species classification
    bool enable_behavior_analysis = false;         // Requires advanced AI system
    
    // Detection thresholds
    float confidence_threshold = 0.6f;             // Minimum confidence for detection (0.0-1.0)
    float high_confidence_threshold = 0.8f;        // Threshold for high-confidence triggers
    uint32_t detection_interval_ms = 1000;         // How often to run AI detection
    uint8_t max_detections_per_frame = 5;          // Limit processing load
    
    // False positive reduction
    bool enable_motion_trigger = true;             // Only process when motion detected
    bool enable_size_filtering = true;             // Filter by object size
    float min_object_size = 0.01f;                 // Minimum relative object size (1% of frame)
    float max_object_size = 0.8f;                  // Maximum relative object size (80% of frame)
    
    // Edge processing
    bool enable_edge_processing = true;            // Process without cloud connectivity
    bool enable_power_optimization = true;        // Optimize for battery life
    uint32_t ai_processing_timeout_ms = 2000;      // Maximum time for AI processing
    
    // Model configuration
    char primary_model_path[128] = "wildlife_v2.tflite";
    char backup_model_path[128] = "basic_detector.tflite";
    bool enable_model_fallback = true;            // Fall back to basic model if advanced fails
};

/**
 * @brief Event Triggering Configuration
 * 
 * Configures event triggering based on AI detection results with priority levels
 * and action definitions.
 */
struct EventTriggeringConfig {
    // Event triggering enable flags
    bool enable_event_triggering = true;
    bool enable_image_capture = true;             // Capture images on detection
    bool enable_mesh_broadcast = true;            // Broadcast events to mesh network
    bool enable_local_storage = true;             // Store events locally
    
    // Trigger thresholds by species priority
    float critical_species_threshold = 0.7f;      // Human, predator detection
    float high_priority_threshold = 0.75f;        // Large mammals
    float medium_priority_threshold = 0.8f;       // Medium mammals, large birds
    float low_priority_threshold = 0.85f;         // Small animals
    
    // Event actions
    bool auto_save_high_confidence = true;        // Auto-save high-confidence detections
    bool send_immediate_alerts = true;            // Send alerts for critical detections
    uint32_t event_cooldown_ms = 5000;            // Minimum time between same-type events
    uint8_t max_events_per_minute = 10;           // Rate limiting
    
    // Storage configuration
    char event_storage_path[128] = "/wildlife_events";
    uint32_t max_stored_events = 1000;            // Maximum events to store locally
    bool enable_event_compression = true;         // Compress stored event data
    
    // Mesh network integration
    uint8_t mesh_event_priority = 5;              // Priority for mesh event messages (0-7)
    bool broadcast_all_events = false;           // If false, only high-confidence events
    uint32_t mesh_retry_count = 3;                // Retries for important events
};

/**
 * @brief Mesh Networking Configuration
 * 
 * Configures mesh networking for seamless data transfer between multiple devices
 * with edge processing capabilities.
 */
struct MeshNetworkingConfig {
    // Core mesh settings
    bool enable_mesh_networking = true;
    bool enable_auto_discovery = true;            // Automatically discover other nodes
    bool enable_mesh_healing = true;              // Auto-repair network topology
    
    // Network parameters
    uint8_t mesh_channel = 1;                     // WiFi channel (1-13)
    uint16_t max_mesh_connections = 10;           // Maximum connected nodes
    uint32_t heartbeat_interval_ms = 30000;       // Node heartbeat frequency
    uint32_t node_timeout_ms = 120000;            // Time before considering node offline
    uint8_t max_hop_count = 6;                    // Maximum message hops
    
    // Security and encryption
    bool enable_mesh_encryption = true;
    char mesh_password[64] = "WildlifeCam2025";
    char mesh_network_name[32] = "WildlifeNet";
    
    // Data synchronization
    bool enable_data_sharing = true;              // Share detection data between nodes
    bool enable_coordinated_monitoring = true;   // Coordinate monitoring between nodes
    uint32_t data_sync_interval_ms = 300000;     // 5 minutes
    
    // Edge processing without cloud
    bool prefer_local_processing = true;         // Process locally when possible
    bool enable_offline_operation = true;       // Function without internet connectivity
    uint32_t offline_storage_limit_mb = 100;    // Local storage limit for offline data
    
    // Load balancing and coordination
    bool enable_load_balancing = true;           // Distribute AI processing load
    bool enable_role_switching = true;          // Nodes can change roles as needed
    uint8_t preferred_node_role = 2;            // ROLE_NODE from BoardRole enum
};

/**
 * @brief Comprehensive Advanced Features Configuration
 * 
 * Master configuration structure combining all advanced features with
 * global settings and integration options.
 */
struct AdvancedFeaturesConfig {
    // Component configurations
    AIDetectionConfig ai_detection;
    EventTriggeringConfig event_triggering;
    MeshNetworkingConfig mesh_networking;
    
    // Global integration settings
    bool enable_all_features = true;             // Master enable switch
    bool enable_debug_logging = false;          // Detailed logging for troubleshooting
    uint32_t startup_delay_ms = 5000;           // Delay before starting advanced features
    
    // Performance and resource management
    uint8_t cpu_core_allocation = 1;            // CPU core for AI processing (0 or 1)
    uint32_t max_memory_usage_kb = 512;         // Maximum memory for advanced features
    bool enable_watchdog_timer = true;         // Enable system watchdog
    uint32_t watchdog_timeout_ms = 30000;      // Watchdog timeout
    
    // Integration validation
    bool validate_ai_integration = true;       // Validate AI system integration on startup
    bool validate_mesh_integration = true;    // Validate mesh network integration
    bool enable_self_diagnostics = true;      // Run self-diagnostics periodically
    uint32_t diagnostics_interval_ms = 600000; // Run diagnostics every 10 minutes
};

/**
 * @brief Advanced Features Manager
 * 
 * Main class for managing and configuring all advanced software features
 * with integrated validation and monitoring capabilities.
 */
class AdvancedFeaturesManager {
public:
    AdvancedFeaturesManager();
    ~AdvancedFeaturesManager();
    
    /**
     * @brief Initialize all advanced features with configuration
     * @param config Complete configuration for all features
     * @return true if all features initialized successfully
     */
    bool initialize(const AdvancedFeaturesConfig& config);
    
    /**
     * @brief Start all enabled advanced features
     * @return true if all features started successfully
     */
    bool startFeatures();
    
    /**
     * @brief Stop all advanced features
     */
    void stopFeatures();
    
    /**
     * @brief Update configuration dynamically
     * @param config New configuration to apply
     * @return true if configuration updated successfully
     */
    bool updateConfiguration(const AdvancedFeaturesConfig& config);
    
    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    const AdvancedFeaturesConfig& getConfiguration() const { return config_; }
    
    /**
     * @brief Run system diagnostics
     * @return true if all systems are functioning correctly
     */
    bool runDiagnostics();
    
    /**
     * @brief Get feature status report
     * @return String containing detailed status of all features
     */
    String getStatusReport();
    
    /**
     * @brief Check if all features are operational
     * @return true if all enabled features are working
     */
    bool isOperational();
    
    /**
     * @brief Save configuration to persistent storage
     * @return true if configuration saved successfully
     */
    bool saveConfiguration();
    
    /**
     * @brief Load configuration from persistent storage
     * @return true if configuration loaded successfully
     */
    bool loadConfiguration();

private:
    AdvancedFeaturesConfig config_;
    bool initialized_;
    bool features_running_;
    uint32_t last_diagnostics_;
    
    // Component status tracking
    bool ai_detection_active_;
    bool event_triggering_active_;
    bool mesh_networking_active_;
    
    // Internal methods
    bool initializeAIDetection();
    bool initializeEventTriggering();
    bool initializeMeshNetworking();
    bool validateIntegration();
    void logStatus(const char* component, bool status);
};

/**
 * @brief Default configuration factory
 * 
 * Provides pre-configured setups for common deployment scenarios
 */
namespace DefaultConfigs {
    /**
     * @brief Get configuration optimized for battery-powered deployment
     */
    AdvancedFeaturesConfig getBatteryOptimizedConfig();
    
    /**
     * @brief Get configuration for high-accuracy research deployment
     */
    AdvancedFeaturesConfig getResearchConfig();
    
    /**
     * @brief Get configuration for security monitoring
     */
    AdvancedFeaturesConfig getSecurityConfig();
    
    /**
     * @brief Get basic configuration for simple deployments
     */
    AdvancedFeaturesConfig getBasicConfig();
}

} // namespace AdvancedFeatures

#endif // ADVANCED_FEATURES_CONFIG_H