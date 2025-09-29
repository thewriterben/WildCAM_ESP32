#ifndef CONFIG_OPTIMIZED_H
#define CONFIG_OPTIMIZED_H

#include "config.h"

/**
 * Enhanced Production Configuration for ESP32WildlifeCAM
 * 
 * Optimizes memory usage, power consumption, network performance,
 * and wildlife detection accuracy based on comprehensive audit findings.
 */

// ===========================
// OPTIMIZED MEMORY CONFIGURATION
// ===========================

// Enhanced heap management for stability
#define OPTIMIZED_HEAP_SIZE (200 * 1024)     // 200KB for heap
#define IMAGE_BUFFER_POOL_SIZE 5              // Pre-allocated image buffers
#define MESSAGE_BUFFER_POOL_SIZE 20           // Pre-allocated message buffers
#define ANALYTICS_CIRCULAR_BUFFER_SIZE (64 * 1024) // 64KB circular buffer

// Memory allocation strategy
#define PREFER_PSRAM_FOR_LARGE_ALLOCATIONS true
#define PSRAM_THRESHOLD_BYTES (32 * 1024)    // Use PSRAM for allocations > 32KB
#define ENABLE_HEAP_DEFRAGMENTATION true      // Enable periodic defragmentation
#define HEAP_FRAGMENTATION_THRESHOLD 60      // Trigger cleanup at 60% fragmentation

// ===========================
// PERFORMANCE-OPTIMIZED SETTINGS
// ===========================

// Multi-core utilization
#define AI_INFERENCE_THREADS 2                // Dual-core utilization for AI
#define NETWORK_WORKER_THREADS 1              // Dedicated network thread
#define CAMERA_PROCESSING_THREADS 1           // Dedicated camera processing

// Adaptive frame rate and quality
#define CAMERA_FRAME_RATE_ADAPTIVE true       // Dynamic frame rate based on conditions
#define CAMERA_QUALITY_ADAPTIVE true          // Quality based on battery level
#define MIN_FRAME_RATE_FPS 0.1                // Minimum frame rate (1 frame per 10 seconds)
#define MAX_FRAME_RATE_FPS 5.0                // Maximum frame rate
#define QUALITY_SCALE_FACTOR 0.8              // Quality reduction factor for power saving

// Power measurement and optimization
#define POWER_MEASUREMENT_INTERVAL 10000      // 10 seconds between measurements
#define ADAPTIVE_POWER_MANAGEMENT true        // Enable adaptive power profiles
#define DYNAMIC_FREQUENCY_SCALING true        // Enable CPU frequency adjustment
#define PERIPHERAL_POWER_GATING true          // Enable unused peripheral shutdown

// ===========================
// NETWORK OPTIMIZATION
// ===========================

// Packet size optimization to avoid fragmentation
#define MAX_PACKET_SIZE_OPTIMIZED 1200        // Avoid IP fragmentation
#define COMPRESSION_THRESHOLD 512             // Compress packets > 512 bytes
#define COMPRESSION_ALGORITHM_LZ4 true        // Use LZ4 for fast compression

// Adaptive batching and routing
#define BATCH_SIZE_ADAPTIVE true              // Dynamic message batching
#define BATCH_TIMEOUT_MS 5000                 // Maximum batching delay
#define MESH_ROUTING_CACHE_SIZE 50            // Routing table cache entries
#define ROUTING_TABLE_CLEANUP_INTERVAL 300000 // 5 minutes

// Network quality monitoring
#define LINK_QUALITY_MONITORING true          // Monitor link quality
#define ADAPTIVE_TRANSMISSION_POWER true      // Adjust TX power based on RSSI
#define NETWORK_TOPOLOGY_OPTIMIZATION true    // Optimize mesh topology

// ===========================
// WILDLIFE DETECTION OPTIMIZATION
// ===========================

// Adaptive motion detection
#define MOTION_DETECTION_THRESHOLD_ADAPTIVE true
#define MOTION_SENSITIVITY_MIN 20             // Minimum sensitivity level
#define MOTION_SENSITIVITY_MAX 80             // Maximum sensitivity level
#define MOTION_THRESHOLD_ADJUSTMENT_FACTOR 1.2 // Adjustment factor for conditions

// AI confidence and classification
#define AI_CONFIDENCE_THRESHOLD_DYNAMIC true  // Dynamic confidence thresholds
#define SPECIES_CLASSIFICATION_LEVELS 3       // Coarse -> Fine classification
#define CONFIDENCE_THRESHOLD_LOW 0.4          // Low confidence threshold
#define CONFIDENCE_THRESHOLD_HIGH 0.8         // High confidence threshold

// Image processing optimization
#define EDGE_DETECTION_HARDWARE_ACCEL true    // Use hardware acceleration
#define COLOR_SPACE_OPTIMIZATION true         // Optimize color space conversions
#define NOISE_REDUCTION_ADAPTIVE true         // Adaptive noise reduction

// ===========================
// POWER OPTIMIZATION SETTINGS
// ===========================

// Dynamic sleep duration based on conditions
#define DEEP_SLEEP_DURATION_BASE 300000       // 5 minutes base sleep duration
#define DEEP_SLEEP_MULTIPLIER_LOW_BATTERY 3   // 15 minutes when battery < 20%
#define DEEP_SLEEP_MULTIPLIER_NO_MOTION 2     // 10 minutes when no motion detected
#define DEEP_SLEEP_MULTIPLIER_NIGHT 1.5       // 7.5 minutes during night hours

// CPU frequency scaling
#define CPU_FREQ_STEP_SIZE 40                 // 40MHz frequency steps
#define CPU_FREQ_LOAD_THRESHOLD_HIGH 0.8      // Scale up above 80% load
#define CPU_FREQ_LOAD_THRESHOLD_LOW 0.3       // Scale down below 30% load
#define CPU_FREQ_ADJUSTMENT_INTERVAL 5000     // 5 seconds between adjustments

// Radio power optimization
#define RADIO_POWER_STEP_SIZE 2               // 2dBm power steps
#define RADIO_POWER_RSSI_THRESHOLD_GOOD -70   // Good signal threshold
#define RADIO_POWER_RSSI_THRESHOLD_POOR -90   // Poor signal threshold
#define LORA_ADAPTIVE_POWER_CONTROL true      // Enable adaptive LoRa power

// ===========================
// CONSERVATION ANALYTICS OPTIMIZATION
// ===========================

// Population and biodiversity analysis
#define POPULATION_TREND_MIN_SAMPLES 50       // Minimum samples for trend analysis
#define BIODIVERSITY_CALCULATION_INTERVAL 3600000 // 1 hour between calculations
#define MIGRATION_PATTERN_LOOKBACK_DAYS 365   // 1 year lookback for migration patterns
#define HABITAT_ANALYSIS_GRID_SIZE 100        // 100m grid for spatial analysis

// Real-time analytics processing
#define ANALYTICS_PROCESSING_INTERVAL 60000   // 1 minute processing interval
#define ANALYTICS_COMPRESSION_ENABLED true    // Enable analytics data compression
#define BEHAVIORAL_PATTERN_DETECTION true     // Enable behavior pattern recognition
#define ECOSYSTEM_HEALTH_MONITORING true      // Enable ecosystem health assessment

// ===========================
// SAFETY AND RELIABILITY OPTIMIZATIONS
// ===========================

// Thread safety and synchronization
#define THREAD_SAFE_OPERATIONS true           // Enable thread-safe operations
#define MUTEX_TIMEOUT_MS 5000                 // Maximum mutex wait time
#define DEADLOCK_DETECTION true               // Enable deadlock detection
#define WATCHDOG_TIMER_ENABLED true           // Enable system watchdog

// Error recovery and resilience
#define AUTO_RECOVERY_ENABLED true            // Enable automatic error recovery
#define SYSTEM_HEALTH_MONITORING true         // Monitor system health metrics
#define GRACEFUL_DEGRADATION true             // Enable graceful performance degradation
#define FAILSAFE_MODE_THRESHOLD 3             // Enter failsafe after 3 consecutive errors

// Memory and resource monitoring
#define MEMORY_LEAK_DETECTION true            // Enable memory leak detection
#define RESOURCE_UTILIZATION_MONITORING true  // Monitor resource utilization
#define PERFORMANCE_PROFILING true            // Enable performance profiling
#define DIAGNOSTIC_DATA_COLLECTION true       // Collect diagnostic information

// ===========================
// ENHANCED COMMUNICATION PROTOCOLS
// ===========================

// Message reliability and delivery
#define MESSAGE_ACKNOWLEDGMENT_REQUIRED true  // Require message acknowledgments
#define MESSAGE_RETRY_COUNT 3                 // Maximum retry attempts
#define MESSAGE_TIMEOUT_MS 30000              // Message timeout (30 seconds)
#define DUPLICATE_MESSAGE_DETECTION true      // Detect and filter duplicates

// Data synchronization
#define INCREMENTAL_DATA_SYNC true            // Use incremental synchronization
#define DATA_INTEGRITY_CHECKING true          // Verify data integrity
#define SYNC_PRIORITY_QUEUE true              // Prioritize critical data
#define BANDWIDTH_THROTTLING true             // Throttle bandwidth usage

// ===========================
// PREDICTIVE BEHAVIOR ENHANCEMENTS
// ===========================

// Wildlife behavior prediction
#define BEHAVIOR_PREDICTION_ENABLED true      // Enable behavior prediction
#define ENVIRONMENTAL_CORRELATION true        // Correlate with environmental data
#define TEMPORAL_PATTERN_ANALYSIS true        // Analyze temporal patterns
#define SPECIES_INTERACTION_MODELING true     // Model species interactions

// Proactive monitoring
#define PREDICTIVE_WAKE_SCHEDULING true       // Schedule wake times based on predictions
#define ADAPTIVE_MONITORING_ZONES true        // Adjust monitoring zones dynamically
#define EARLY_WARNING_SYSTEM true             // Provide early warnings for events
#define CONSERVATION_ALERT_THRESHOLDS true    // Set thresholds for conservation alerts

// ===========================
// PERFORMANCE TARGETS AND METRICS
// ===========================

// Target performance improvements (from audit report)
#define TARGET_AI_INFERENCE_TIME_MS 700       // Target: 70% faster (2.3s -> 0.7s)
#define TARGET_MEMORY_USAGE_PERCENT 62        // Target: 27% reduction (85% -> 62%)
#define TARGET_POWER_CONSUMPTION_MA 225       // Target: 20% more efficient (280mA -> 225mA)
#define TARGET_BATTERY_LIFE_DAYS 28           // Target: 56% longer (18 -> 28 days)
#define TARGET_DETECTION_ACCURACY_PERCENT 94  // Target: 6.8% better (87.3% -> 94.1%)

// Performance monitoring intervals
#define PERFORMANCE_MEASUREMENT_INTERVAL 300000 // 5 minutes
#define METRICS_REPORTING_INTERVAL 3600000    // 1 hour
#define BENCHMARK_COMPARISON_ENABLED true     // Compare against targets
#define PERFORMANCE_ALERTING true             // Alert on performance degradation

// ===========================
// ADVANCED FEATURES CONFIGURATION
// ===========================

// Edge AI enhancements
#define QUANTIZED_NEURAL_NETWORKS true        // Use quantized models for 3x speed
#define MODEL_CACHING_ENABLED true            // Cache frequently used models
#define INFERENCE_PIPELINE_OPTIMIZATION true  // Optimize inference pipeline
#define TENSOR_MEMORY_OPTIMIZATION true       // Optimize tensor memory allocation

// Enhanced environmental monitoring
#define MICROCLIMATE_MONITORING true          // Monitor microclimate conditions
#define AIR_QUALITY_ASSESSMENT true           // Assess air quality impact
#define SOIL_MOISTURE_CORRELATION true        // Correlate with soil conditions
#define WEATHER_PATTERN_INTEGRATION true      // Integrate weather pattern data

// ===========================
// SECURITY AND PRIVACY ENHANCEMENTS
// ===========================

// Data protection
#define DATA_ENCRYPTION_ENABLED true          // Encrypt sensitive data
#define SECURE_COMMUNICATION_PROTOCOLS true   // Use secure communication
#define PRIVACY_PRESERVING_ANALYTICS true     // Protect individual animal privacy
#define ACCESS_CONTROL_ENABLED true           // Control access to system functions

// System integrity
#define FIRMWARE_SIGNATURE_VERIFICATION true  // Verify firmware integrity
#define SECURE_BOOT_ENABLED true              // Enable secure boot process
#define TAMPER_DETECTION_ENABLED true         // Detect physical tampering
#define AUDIT_LOGGING_ENABLED true            // Log security events

#endif // CONFIG_OPTIMIZED_H