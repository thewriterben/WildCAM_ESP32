/**
 * @file research-config.h
 * @brief Configuration for large-scale research deployments
 * 
 * This configuration is optimized for scientific research with high-quality
 * data collection, advanced networking, and comprehensive monitoring.
 */

#ifndef RESEARCH_CONFIG_H
#define RESEARCH_CONFIG_H

// ===========================
// DEPLOYMENT SCENARIO IDENTIFICATION
// ===========================
#define DEPLOYMENT_SCENARIO "RESEARCH"
#define CONFIG_VERSION "1.0.0"
#define CONFIG_DESCRIPTION "Large-scale research deployment with 50+ nodes"

// ===========================
// BOARD CONFIGURATION
// ===========================
// Auto-detect board type, but prefer research-grade boards
#define PREFERRED_BOARD_TYPE BOARD_ESP32_S3_CAM
#define FALLBACK_BOARD_TYPE BOARD_ESP_EYE
#define ENABLE_BOARD_AUTO_DETECTION true

// ===========================
// CAMERA CONFIGURATION
// ===========================
#define CAMERA_PRESET CAMERA_PRESET_RESEARCH

// High-resolution research imaging
#define CAMERA_FRAME_SIZE FRAMESIZE_QXGA        // 2048x1536 for detailed analysis
#define CAMERA_JPEG_QUALITY 8                   // High quality, low compression
#define CAMERA_FB_COUNT 2                       // Double buffering for processing
#define CAMERA_FB_LOCATION CAMERA_FB_IN_PSRAM   // Use PSRAM for high resolution

// Research-specific camera settings
#define CAMERA_SEQUENCE_CAPTURE true            // Multiple images per trigger
#define CAMERA_SEQUENCE_LENGTH 5                // 5 images per sequence
#define CAMERA_SEQUENCE_INTERVAL_MS 2000        // 2 seconds between sequence images
#define CAMERA_ENABLE_TIMESTAMPING true         // Precise timestamps for research
#define CAMERA_TIMESTAMP_PRECISION TIMESTAMP_MICROSECONDS

// Image enhancement for research quality
#define CAMERA_LENS_CORRECTION true             // Correct lens distortion
#define CAMERA_NOISE_REDUCTION true             // Enable noise reduction
#define CAMERA_EDGE_ENHANCEMENT true            // Enhance edges for detail

// ===========================
// AI PROCESSING CONFIGURATION
// ===========================
#define AI_PROCESSING_ENABLED true
#define AI_MODEL_TYPE "research_comprehensive"
#define AI_CONFIDENCE_THRESHOLD 0.85            // High confidence for research
#define AI_SPECIES_CLASSIFICATION true
#define AI_BEHAVIOR_ANALYSIS true
#define AI_POPULATION_COUNTING true
#define AI_HEALTH_ASSESSMENT true

// Research-specific AI features
#define AI_DEMOGRAPHIC_ANALYSIS true            // Age, sex, size estimation
#define AI_CONTINUOUS_LEARNING true             // Learn from field data
#define AI_EXPERT_VALIDATION_MODE true          // Flag uncertain results for expert review
#define AI_FEATURE_EXTRACTION true              // Extract features for research analysis

// Edge AI optimization
#define AI_EDGE_PROCESSING true                 // Process locally when possible
#define AI_CLOUD_PROCESSING_BACKUP true        // Cloud backup for complex analysis
#define AI_PROCESSING_TIMEOUT_MS 10000         // 10 second timeout for complex models

// ===========================
// NETWORK CONFIGURATION
// ===========================
#define NETWORK_TOPOLOGY NETWORK_HIERARCHICAL_MESH
#define NETWORK_ENABLE_MESH true                // Enable mesh networking
#define NETWORK_ENABLE_CELLULAR true            // Cellular backup
#define NETWORK_ENABLE_SATELLITE true           // Satellite emergency backup

// LoRa mesh configuration for research coordination
#define LORA_ENABLED true
#define LORA_FREQUENCY 915E6                    // US frequency (change for region)
#define LORA_TX_POWER 17                        // High power for research coverage
#define LORA_SPREADING_FACTOR 8                 // Balance of range and data rate
#define LORA_BANDWIDTH 125E3
#define LORA_CODING_RATE 5                      // Error correction
#define LORA_MESH_MAX_NODES 100                 // Support large research networks

// Research data transmission
#define DATA_TRANSMISSION_INTERVAL_MS 180000    // 3 minutes for research data
#define DATA_COMPRESSION_ENABLED true           // Compress for efficient transmission
#define DATA_REDUNDANCY_ENABLED true            // Multiple transmission paths
#define DATA_PRIORITY_QUEUING true              // Prioritize research data

// ===========================
// POWER MANAGEMENT
// ===========================
#define POWER_PROFILE "RESEARCH_EXTENDED"

// Research power budget (higher consumption for quality)
#define SOLAR_PANEL_WATTS 20                    // 20W solar panel for research
#define BATTERY_CAPACITY_MAH 5000               // 5Ah battery for extended operation
#define BATTERY_CHEMISTRY BATTERY_LIFEPO4       // Long life for research deployments

// Power optimization for research
#define POWER_MANAGEMENT_ADAPTIVE true          // Adapt to conditions
#define POWER_RESEARCH_PRIORITY true            // Prioritize research data collection
#define DEEP_SLEEP_DURATION_MS 30000            // 30 seconds between activities
#define WAKE_ON_MOTION_ENABLED true             // PIR motion detection wake
#define WAKE_ON_SCHEDULE_ENABLED true           // Scheduled research activities

// Power thresholds for research continuity
#define BATTERY_NORMAL_THRESHOLD 3.8            // Normal operation above 3.8V
#define BATTERY_CONSERVATION_THRESHOLD 3.6      // Conservation mode below 3.6V
#define BATTERY_CRITICAL_THRESHOLD 3.4          // Critical mode below 3.4V

// ===========================
// DATA COLLECTION AND STORAGE
// ===========================
#define DATA_COLLECTION_MODE "COMPREHENSIVE"

// Research data requirements
#define ENABLE_ENVIRONMENTAL_SENSORS true       // Temperature, humidity, pressure
#define ENABLE_GPS_TRACKING true               // Precise location for each observation
#define ENABLE_AUDIO_RECORDING true            // Audio for species identification
#define ENABLE_ACCELEROMETER true              // Motion and vibration data

// Metadata for research
#define METADATA_COMPREHENSIVE true             // Detailed metadata for research
#define METADATA_ENVIRONMENTAL true             // Environmental conditions
#define METADATA_SYSTEM_HEALTH true            // System status and diagnostics
#define METADATA_AI_CONFIDENCE true            // AI confidence scores
#define METADATA_RESEARCH_ANNOTATIONS true     // Research-specific annotations

// Storage configuration
#define SD_CARD_SIZE_GB 128                     // Large storage for research data
#define SD_CARD_REDUNDANCY true                // Backup critical data
#define CLOUD_STORAGE_ENABLED true             // Automatic cloud backup
#define CLOUD_STORAGE_PROVIDER "RESEARCH_AWS"  // Research AWS instance

// ===========================
// RESEARCH-SPECIFIC FEATURES
// ===========================
#define RESEARCH_MODE_ENABLED true

// Scientific data validation
#define DATA_QUALITY_VALIDATION true           // Validate data quality
#define STATISTICAL_VALIDATION true            // Statistical data validation
#define PEER_REVIEW_PREPARATION true           // Prepare data for peer review
#define REPRODUCIBILITY_SUPPORT true           // Support reproducible research

// Research collaboration features
#define MULTI_INSTITUTION_SUPPORT true         // Support multiple research institutions
#define DATA_SHARING_ENABLED true              // Enable controlled data sharing
#define RESEARCH_API_ENABLED true              // API for research data access
#define FEDERATED_LEARNING_ENABLED true        // Participate in federated learning

// Conservation applications
#define CONSERVATION_ALERTS_ENABLED true       // Generate conservation alerts
#define ENDANGERED_SPECIES_PRIORITY true       // Priority for endangered species
#define HABITAT_ASSESSMENT_ENABLED true        // Assess habitat quality
#define POPULATION_MONITORING true             // Monitor population changes

// ===========================
// MONITORING AND DIAGNOSTICS
// ===========================
#define SYSTEM_MONITORING_COMPREHENSIVE true

// Research deployment monitoring
#define NETWORK_HEALTH_MONITORING true         // Monitor network performance
#define DATA_QUALITY_MONITORING true           // Monitor data collection quality
#define AI_PERFORMANCE_MONITORING true         // Monitor AI model performance
#define ENVIRONMENTAL_MONITORING true          // Monitor environmental conditions

// Research metrics
#define RESEARCH_METRICS_COLLECTION true       // Collect research-specific metrics
#define PERFORMANCE_BENCHMARKING true          // Benchmark against research standards
#define IMPACT_MEASUREMENT true                // Measure research impact
#define PUBLICATION_SUPPORT true               // Support for research publications

// Alerts and notifications
#define RESEARCH_ALERT_SYSTEM true             // Research-specific alerts
#define EXPERT_NOTIFICATION_SYSTEM true        // Notify experts of significant findings
#define EMERGENCY_RESEARCH_PROTOCOLS true      // Emergency research procedures
#define REAL_TIME_COLLABORATION true           // Real-time research collaboration

// ===========================
// SECURITY AND PRIVACY
// ===========================
#define SECURITY_RESEARCH_GRADE true

// Data security for research
#define DATA_ENCRYPTION_ENABLED true           // Encrypt sensitive research data
#define SECURE_TRANSMISSION true               // Secure data transmission
#define ACCESS_CONTROL_ENABLED true            // Control access to research data
#define AUDIT_LOGGING_ENABLED true             // Log all data access

// Research ethics compliance
#define PRIVACY_PROTECTION_ENABLED true        // Protect sensitive locations
#define ETHICAL_GUIDELINES_COMPLIANCE true     // Comply with research ethics
#define DATA_ANONYMIZATION true                // Anonymize sensitive data
#define CONSENT_MANAGEMENT true                // Manage research consents

// ===========================
// INTEGRATION SETTINGS
// ===========================
#define EXTERNAL_INTEGRATIONS_ENABLED true

// Research database integration
#define RESEARCH_DATABASE_INTEGRATION true     // Integrate with research databases
#define MOVEBANK_INTEGRATION true              // Animal movement data
#define GBIF_INTEGRATION true                  // Global biodiversity data
#define INATURALIST_INTEGRATION true           // Citizen science integration

// Analysis platform integration
#define EDGE_IMPULSE_INTEGRATION true          // AI model development
#define JUPYTER_NOTEBOOK_SUPPORT true          // Research analysis support
#define R_INTEGRATION true                     // Statistical analysis integration
#define PYTHON_API_SUPPORT true               // Python research tools

// ===========================
// OPERATIONAL PARAMETERS
// ===========================

// Capture scheduling for research
#define CAPTURE_INTERVAL_NORMAL_MS 180000      // 3 minutes normal interval
#define CAPTURE_INTERVAL_INTENSIVE_MS 60000    // 1 minute during intensive study
#define CAPTURE_DAWN_DUSK_BOOST true           // Increased activity during crepuscular periods
#define CAPTURE_SEASONAL_ADAPTATION true       // Adapt to seasonal animal activity

// Research coordination
#define MULTI_NODE_COORDINATION true           // Coordinate multiple research nodes
#define SYNCHRONIZED_CAPTURE true              // Synchronize captures across nodes
#define DISTRIBUTED_PROCESSING true            // Distribute AI processing load
#define COLLABORATIVE_LEARNING true            // Share learning across network

// Quality assurance
#define AUTOMATIC_QUALITY_CHECKS true          // Automatic quality validation
#define MANUAL_VALIDATION_TRIGGERS true        // Trigger manual validation when needed
#define CALIBRATION_REMINDERS true             // Remind for regular calibration
#define MAINTENANCE_SCHEDULING true            // Schedule preventive maintenance

// ===========================
// DEBUGGING AND DEVELOPMENT
// ===========================
#ifdef DEBUG_RESEARCH_DEPLOYMENT
    #define DEBUG_LEVEL 3                      // Detailed debugging for research
    #define SERIAL_DEBUG_ENABLED true          // Serial debug output
    #define PERFORMANCE_PROFILING true         // Profile system performance
    #define MEMORY_USAGE_MONITORING true       // Monitor memory usage
    #define NETWORK_DIAGNOSTICS true           // Network diagnostic tools
    #define AI_MODEL_DEBUGGING true            // Debug AI model performance
#endif

// ===========================
// REGIONAL CUSTOMIZATION
// ===========================
// Customize these settings based on deployment region

// North America Research Settings
#ifdef REGION_NORTH_AMERICA
    #define LORA_FREQUENCY 915E6               // US LoRa frequency
    #define TIMEZONE_OFFSET -5                 // EST timezone
    #define RESEARCH_REGULATIONS "US_WILDLIFE_RESEARCH"
    #define ENDANGERED_SPECIES_LIST "US_ESA"
#endif

// Europe Research Settings  
#ifdef REGION_EUROPE
    #define LORA_FREQUENCY 433E6               // EU LoRa frequency
    #define TIMEZONE_OFFSET 1                  // CET timezone
    #define RESEARCH_REGULATIONS "EU_WILDLIFE_RESEARCH"
    #define ENDANGERED_SPECIES_LIST "EU_HABITATS_DIRECTIVE"
#endif

// ===========================
// VALIDATION CHECKS
// ===========================
// Ensure critical research features are properly configured
#if !defined(AI_PROCESSING_ENABLED) || !AI_PROCESSING_ENABLED
    #error "AI processing must be enabled for research deployments"
#endif

#if !defined(CAMERA_SEQUENCE_CAPTURE) || !CAMERA_SEQUENCE_CAPTURE
    #error "Sequence capture must be enabled for research quality data"
#endif

#if !defined(METADATA_COMPREHENSIVE) || !METADATA_COMPREHENSIVE
    #error "Comprehensive metadata required for research validation"
#endif

#if defined(CAMERA_JPEG_QUALITY) && CAMERA_JPEG_QUALITY > 12
    #warning "JPEG quality may be too low for research applications"
#endif

#endif // RESEARCH_CONFIG_H