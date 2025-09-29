/**
 * @file conservation-config.h
 * @brief Configuration for budget conservation projects
 * 
 * This configuration is optimized for cost-effective wildlife monitoring
 * with minimal power consumption and simple operation.
 */

#ifndef CONSERVATION_CONFIG_H
#define CONSERVATION_CONFIG_H

// ===========================
// DEPLOYMENT SCENARIO IDENTIFICATION
// ===========================
#define DEPLOYMENT_SCENARIO "BUDGET_CONSERVATION"
#define CONFIG_VERSION "1.0.0"
#define CONFIG_DESCRIPTION "Cost-effective conservation monitoring for small organizations"

// ===========================
// BOARD CONFIGURATION
// ===========================
// Budget-friendly board selection
#define PREFERRED_BOARD_TYPE BOARD_AI_THINKER_ESP32_CAM
#define FALLBACK_BOARD_TYPE BOARD_AI_THINKER_ESP32_CAM  // Same for budget consistency
#define ENABLE_BOARD_AUTO_DETECTION true

// ===========================
// CAMERA CONFIGURATION
// ===========================
#define CAMERA_PRESET CAMERA_PRESET_SURVEY

// Budget-optimized imaging
#define CAMERA_FRAME_SIZE FRAMESIZE_SVGA        // 800x600 - good balance for budget
#define CAMERA_JPEG_QUALITY 15                  // Moderate compression for storage efficiency
#define CAMERA_FB_COUNT 1                       // Single buffer to save memory
#define CAMERA_FB_LOCATION CAMERA_FB_IN_DRAM    // Use DRAM (no PSRAM requirement)

// Simple camera operation
#define CAMERA_SEQUENCE_CAPTURE false           // Single images to save power
#define CAMERA_SEQUENCE_LENGTH 1                // One image per trigger
#define CAMERA_ENABLE_TIMESTAMPING true         // Basic timestamps
#define CAMERA_TIMESTAMP_PRECISION TIMESTAMP_SECONDS // Second precision sufficient

// Basic image settings
#define CAMERA_LENS_CORRECTION false            // Disable to save processing power
#define CAMERA_NOISE_REDUCTION false            // Disable to save power
#define CAMERA_EDGE_ENHANCEMENT false           // Disable to save power

// ===========================
// AI PROCESSING CONFIGURATION
// ===========================
#define AI_PROCESSING_ENABLED false             // Disable AI to save power and cost
#define AI_MODEL_TYPE "none"
#define AI_CONFIDENCE_THRESHOLD 0.70            // Lower threshold if AI enabled later
#define AI_SPECIES_CLASSIFICATION false
#define AI_BEHAVIOR_ANALYSIS false
#define AI_POPULATION_COUNTING false
#define AI_HEALTH_ASSESSMENT false

// Disable advanced AI features for budget deployment
#define AI_DEMOGRAPHIC_ANALYSIS false
#define AI_CONTINUOUS_LEARNING false
#define AI_EXPERT_VALIDATION_MODE false
#define AI_FEATURE_EXTRACTION false
#define AI_EDGE_PROCESSING false
#define AI_CLOUD_PROCESSING_BACKUP false

// ===========================
// NETWORK CONFIGURATION
// ===========================
#define NETWORK_TOPOLOGY NETWORK_STAR          // Simple star network
#define NETWORK_ENABLE_MESH false              // Disable mesh to reduce complexity
#define NETWORK_ENABLE_CELLULAR false          // Disable cellular to save cost
#define NETWORK_ENABLE_SATELLITE false         // Disable satellite (expensive)

// Basic LoRa configuration
#define LORA_ENABLED true                       // Enable LoRa for basic networking
#define LORA_FREQUENCY 915E6                    // US frequency (change for region)
#define LORA_TX_POWER 14                        // Standard power to save battery
#define LORA_SPREADING_FACTOR 10                // Higher SF for longer range
#define LORA_BANDWIDTH 125E3
#define LORA_CODING_RATE 8                      // Maximum error correction
#define LORA_MESH_MAX_NODES 10                  // Limit network size for budget

// Conservative data transmission
#define DATA_TRANSMISSION_INTERVAL_MS 600000    // 10 minutes to save power
#define DATA_COMPRESSION_ENABLED true           // Compress to reduce transmission time
#define DATA_REDUNDANCY_ENABLED false          // Disable redundancy to save power
#define DATA_PRIORITY_QUEUING false            // Simple FIFO queuing

// ===========================
// POWER MANAGEMENT
// ===========================
#define POWER_PROFILE "BUDGET_EXTENDED"

// Budget power system
#define SOLAR_PANEL_WATTS 5                     // 5W solar panel (affordable)
#define BATTERY_CAPACITY_MAH 2000               // 2Ah battery (cost-effective)
#define BATTERY_CHEMISTRY BATTERY_LITHIUM_ION   // Standard Li-ion for cost

// Aggressive power optimization
#define POWER_MANAGEMENT_ADAPTIVE true          // Adapt to save maximum power
#define POWER_RESEARCH_PRIORITY false          // Basic monitoring priority
#define DEEP_SLEEP_DURATION_MS 120000           // 2 minutes deep sleep
#define WAKE_ON_MOTION_ENABLED true             // PIR motion detection
#define WAKE_ON_SCHEDULE_ENABLED true           // Scheduled basic monitoring

// Conservative power thresholds
#define BATTERY_NORMAL_THRESHOLD 3.8            // Normal operation above 3.8V
#define BATTERY_CONSERVATION_THRESHOLD 3.6      // Conservation mode below 3.6V
#define BATTERY_CRITICAL_THRESHOLD 3.2          // Critical mode below 3.2V (early warning)

// ===========================
// DATA COLLECTION AND STORAGE
// ===========================
#define DATA_COLLECTION_MODE "BASIC"

// Minimal sensor configuration
#define ENABLE_ENVIRONMENTAL_SENSORS false      // Disable to save cost and power
#define ENABLE_GPS_TRACKING false              // Disable GPS to save power
#define ENABLE_AUDIO_RECORDING false           // Disable audio to save storage/power
#define ENABLE_ACCELEROMETER false             // Disable accelerometer

// Basic metadata only
#define METADATA_COMPREHENSIVE false            // Basic metadata only
#define METADATA_ENVIRONMENTAL false            // No environmental data
#define METADATA_SYSTEM_HEALTH true            // Basic system health only
#define METADATA_AI_CONFIDENCE false           // No AI confidence (AI disabled)
#define METADATA_RESEARCH_ANNOTATIONS false    // No research annotations

// Budget storage configuration
#define SD_CARD_SIZE_GB 32                      // Smaller, affordable SD card
#define SD_CARD_REDUNDANCY false               // No backup to save space
#define CLOUD_STORAGE_ENABLED false            // Disable cloud to avoid data costs
#define CLOUD_STORAGE_PROVIDER "none"

// ===========================
// CONSERVATION-SPECIFIC FEATURES
// ===========================
#define CONSERVATION_MODE_ENABLED true

// Basic conservation monitoring
#define DATA_QUALITY_VALIDATION false          // Disable complex validation
#define STATISTICAL_VALIDATION false           // Disable statistical validation
#define PEER_REVIEW_PREPARATION false          // Not needed for basic conservation
#define REPRODUCIBILITY_SUPPORT false          // Basic operation only

// Community-focused features
#define COMMUNITY_DEPLOYMENT_SUPPORT true      // Support community deployments
#define SIMPLE_CONFIGURATION true              // Simplified configuration options
#define MOBILE_APP_SUPPORT true               // Mobile app for easy access
#define WEB_INTERFACE_SIMPLE true             // Simple web interface

// Basic conservation applications
#define CONSERVATION_ALERTS_ENABLED true       // Basic conservation alerts
#define ENDANGERED_SPECIES_PRIORITY false      // No advanced species prioritization
#define HABITAT_ASSESSMENT_ENABLED false       // No habitat assessment
#define POPULATION_MONITORING false            // No population analysis

// ===========================
// MONITORING AND DIAGNOSTICS
// ===========================
#define SYSTEM_MONITORING_BASIC true

// Basic system monitoring
#define NETWORK_HEALTH_MONITORING false        // Disable complex network monitoring
#define DATA_QUALITY_MONITORING false          // Disable quality monitoring
#define AI_PERFORMANCE_MONITORING false        // No AI performance (AI disabled)
#define ENVIRONMENTAL_MONITORING false         // No environmental monitoring

// Simple metrics only
#define RESEARCH_METRICS_COLLECTION false      // No research metrics
#define PERFORMANCE_BENCHMARKING false         // No benchmarking
#define IMPACT_MEASUREMENT false               // No impact measurement
#define PUBLICATION_SUPPORT false              // No publication support

// Basic alerts only
#define RESEARCH_ALERT_SYSTEM false            // No research alerts
#define EXPERT_NOTIFICATION_SYSTEM false       // No expert notifications
#define EMERGENCY_RESEARCH_PROTOCOLS false     // No emergency protocols
#define REAL_TIME_COLLABORATION false          // No real-time collaboration

// ===========================
// SECURITY AND PRIVACY
// ===========================
#define SECURITY_BASIC true

// Basic security only
#define DATA_ENCRYPTION_ENABLED false          // Disable encryption to save processing
#define SECURE_TRANSMISSION false              // Basic transmission
#define ACCESS_CONTROL_ENABLED false           // No access control
#define AUDIT_LOGGING_ENABLED false            // No audit logging

// Basic privacy
#define PRIVACY_PROTECTION_ENABLED true        // Basic privacy protection
#define ETHICAL_GUIDELINES_COMPLIANCE true     // Basic ethics compliance
#define DATA_ANONYMIZATION false               // No anonymization needed
#define CONSENT_MANAGEMENT false               // No consent management

// ===========================
// INTEGRATION SETTINGS
// ===========================
#define EXTERNAL_INTEGRATIONS_ENABLED false

// Disable expensive integrations
#define RESEARCH_DATABASE_INTEGRATION false    // No research database
#define MOVEBANK_INTEGRATION false             // No movement tracking
#define GBIF_INTEGRATION false                 // No biodiversity database
#define INATURALIST_INTEGRATION true           // Keep citizen science (free)

// Disable advanced analysis platforms
#define EDGE_IMPULSE_INTEGRATION false         // No AI development
#define JUPYTER_NOTEBOOK_SUPPORT false         // No advanced analysis
#define R_INTEGRATION false                    // No statistical integration
#define PYTHON_API_SUPPORT false              // No Python API

// ===========================
// OPERATIONAL PARAMETERS
// ===========================

// Conservative capture scheduling
#define CAPTURE_INTERVAL_NORMAL_MS 600000      // 10 minutes normal interval
#define CAPTURE_INTERVAL_INTENSIVE_MS 300000   // 5 minutes during intensive periods
#define CAPTURE_DAWN_DUSK_BOOST false          // Disable to save power
#define CAPTURE_SEASONAL_ADAPTATION false      // No seasonal adaptation

// Simple operation
#define MULTI_NODE_COORDINATION false          // No coordination to keep simple
#define SYNCHRONIZED_CAPTURE false             // No synchronization
#define DISTRIBUTED_PROCESSING false           // No distributed processing
#define COLLABORATIVE_LEARNING false           // No collaborative learning

// Basic quality assurance
#define AUTOMATIC_QUALITY_CHECKS false         // No automatic checks
#define MANUAL_VALIDATION_TRIGGERS false       // No validation triggers
#define CALIBRATION_REMINDERS false            // No calibration reminders
#define MAINTENANCE_SCHEDULING false           // No automatic scheduling

// ===========================
// BUDGET-SPECIFIC SETTINGS
// ===========================

// Cost optimization
#define MINIMIZE_COMPONENT_COUNT true          // Use minimal components
#define PREFER_STANDARD_COMPONENTS true        // Use common, cheap components
#define DISABLE_PREMIUM_FEATURES true          // Disable expensive features
#define OPTIMIZE_FOR_LONGEVITY true           // Maximize deployment lifetime

// DIY support
#define DIY_ASSEMBLY_SUPPORT true             // Support DIY assembly
#define SIMPLE_WIRING_DIAGRAMS true           // Provide simple diagrams
#define COMMON_TOOLS_ONLY true                // Use only common tools
#define STEP_BY_STEP_GUIDES true              // Detailed assembly guides

// Community features
#define COMMUNITY_SHARING_ENABLED true        // Share data with community
#define VOLUNTEER_SUPPORT true                // Support volunteer maintenance
#define EDUCATIONAL_MODE true                 // Educational features enabled
#define CITIZEN_SCIENCE_INTEGRATION true      // Support citizen science

// ===========================
// DEBUGGING AND DEVELOPMENT
// ===========================
#ifdef DEBUG_CONSERVATION_DEPLOYMENT
    #define DEBUG_LEVEL 1                      // Basic debugging only
    #define SERIAL_DEBUG_ENABLED true          // Serial debug for troubleshooting
    #define PERFORMANCE_PROFILING false        // No performance profiling
    #define MEMORY_USAGE_MONITORING false      // No memory monitoring
    #define NETWORK_DIAGNOSTICS false          // No network diagnostics
    #define AI_MODEL_DEBUGGING false           // No AI debugging (AI disabled)
#endif

// ===========================
// REGIONAL CUSTOMIZATION
// ===========================

// North America Conservation Settings
#ifdef REGION_NORTH_AMERICA
    #define LORA_FREQUENCY 915E6               // US LoRa frequency
    #define TIMEZONE_OFFSET -5                 // EST timezone
    #define CONSERVATION_REGULATIONS "US_BASIC"
    #define SPECIES_LIST "COMMON_NORTH_AMERICA"
#endif

// Europe Conservation Settings
#ifdef REGION_EUROPE
    #define LORA_FREQUENCY 433E6               // EU LoRa frequency
    #define TIMEZONE_OFFSET 1                  // CET timezone
    #define CONSERVATION_REGULATIONS "EU_BASIC"
    #define SPECIES_LIST "COMMON_EUROPE"
#endif

// ===========================
// BILL OF MATERIALS OPTIMIZATION
// ===========================

// Target component costs (USD)
#define TARGET_TOTAL_COST_USD 80               // Target $80 total cost
#define ESP32_CAM_COST_USD 12                  // AI-Thinker ESP32-CAM
#define SOLAR_PANEL_COST_USD 20                // 5W solar panel
#define BATTERY_COST_USD 8                     // 2Ah Li-ion battery
#define ENCLOSURE_COST_USD 15                  // Weatherproof enclosure
#define MISC_COMPONENTS_COST_USD 15            // PIR sensor, SD card, etc.
#define ASSEMBLY_COST_USD 10                   // Assembly materials

// Optional upgrades for budget expansion
#define OPTIONAL_GPS_COST_USD 15               // GPS module upgrade
#define OPTIONAL_LORA_MODULE_COST_USD 12       // LoRa module upgrade
#define OPTIONAL_SENSORS_COST_USD 20           // Environmental sensors

// ===========================
// VALIDATION CHECKS
// ===========================
// Ensure budget constraints are met
#if defined(AI_PROCESSING_ENABLED) && AI_PROCESSING_ENABLED
    #error "AI processing should be disabled for budget deployments"
#endif

#if defined(CAMERA_FRAME_SIZE) && (CAMERA_FRAME_SIZE > FRAMESIZE_UXGA)
    #error "Frame size too large for budget memory constraints"
#endif

#if defined(BATTERY_CAPACITY_MAH) && (BATTERY_CAPACITY_MAH > 3000)
    #warning "Battery capacity may be too expensive for budget deployment"
#endif

#if defined(SOLAR_PANEL_WATTS) && (SOLAR_PANEL_WATTS > 10)
    #warning "Solar panel may be too expensive for budget deployment"
#endif

// Ensure essential features are enabled
#if !defined(LORA_ENABLED) || !LORA_ENABLED
    #error "LoRa must be enabled for basic communication"
#endif

#if !defined(CONSERVATION_MODE_ENABLED) || !CONSERVATION_MODE_ENABLED
    #error "Conservation mode must be enabled for conservation deployments"
#endif

#endif // CONSERVATION_CONFIG_H