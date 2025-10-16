/**
 * @file globals.cpp
 * @brief Global System Pointer Initialization
 * 
 * This file instantiates all global system pointers declared as extern throughout
 * the WildCAM ESP32 firmware. All pointers are initialized to nullptr to ensure
 * a clean and consistent initialization state across the application.
 * 
 * These global pointers are used by various subsystems and must be properly
 * initialized before use. The actual instances should be created during system
 * initialization based on configuration and hardware capabilities.
 * 
 * @section usage Usage
 * The pointers declared here should be initialized in the main initialization
 * routine after hardware setup is complete. For example:
 * 
 * @code
 * // In main initialization
 * g_wildlifeClassifier = new WildlifeClassifier();
 * g_wildlifeClassifier->init(config);
 * @endcode
 * 
 * @section cleanup Cleanup
 * Remember to properly delete these pointers during shutdown to prevent memory leaks:
 * 
 * @code
 * // In cleanup routine
 * if (g_wildlifeClassifier) {
 *     g_wildlifeClassifier->cleanup();
 *     delete g_wildlifeClassifier;
 *     g_wildlifeClassifier = nullptr;
 * }
 * @endcode
 * 
 * @note This file is auto-generated during the build process to ensure consistency.
 * @author WildCAM ESP32 Team
 * @date 2025
 */

// =============================================================================
// AI/ML System Headers
// =============================================================================
#include "ai/wildlife_classifier.h"
#include "ai/tinyml/edge_impulse_integration.h"
#include "ai/federated_learning/federated_learning_system.h"
#include "ai/federated_learning/federated_learning_coordinator.h"
#include "ai/federated_learning/local_training_module.h"
#include "ai/federated_learning/model_update_protocol.h"
#include "ai/federated_learning/network_topology_manager.h"
#include "ai/federated_learning/privacy_preserving_aggregation.h"

// =============================================================================
// Core System Headers
// =============================================================================
#include "multi_board/multi_board_system.h"
#include "network/ota_manager.h"
#include "network_health_monitor.h"
#include "power/federated_power_manager.h"
#include "power/xpowers_manager.h"
#include "i18n/language_manager.h"

// =============================================================================
// Production System Headers
// =============================================================================
#include "production/production_system.h"
#include "production/deployment/config_manager.h"
#include "production/deployment/ota_manager.h"
#include "production/enterprise/cloud_integrator.h"
#include "production/enterprise/cloud/cloud_analytics_engine.h"
#include "production/enterprise/cloud/cloud_config_manager.h"
#include "production/enterprise/cloud/cloud_service_orchestrator.h"
#include "production/enterprise/cloud/conservation_impact_verification.h"
#include "production/enterprise/cloud/enhanced_cloud_manager.h"

// =============================================================================
// AI/ML System Global Pointers
// =============================================================================

/**
 * @brief Wildlife classification system with federated learning support
 * 
 * Provides species identification, behavior recognition, and environmental
 * adaptation capabilities for wildlife monitoring.
 */
WildlifeClassifier* g_wildlifeClassifier = nullptr;

/**
 * @brief Edge Impulse SDK integration for on-device ML
 * 
 * Enables model training, deployment, and performance monitoring using
 * the Edge Impulse platform.
 */
EdgeImpulseIntegration* g_edgeImpulse = nullptr;

/**
 * @brief Main federated learning system coordinator
 * 
 * High-level interface that integrates all federated learning components
 * and provides unified API for distributed machine learning.
 */
FederatedLearningSystem* g_federatedLearningSystem = nullptr;

/**
 * @brief Federated learning coordinator for multi-device collaboration
 * 
 * Manages coordination between multiple devices participating in
 * federated learning rounds.
 */
FederatedLearningCoordinator* g_federatedCoordinator = nullptr;

/**
 * @brief Local training module for on-device model training
 * 
 * Handles local model training operations including data preparation,
 * training execution, and model validation.
 */
LocalTrainingModule* g_localTrainingModule = nullptr;

/**
 * @brief Protocol handler for model updates in federated learning
 * 
 * Manages the communication protocol for exchanging model updates
 * between devices in the federated network.
 */
ModelUpdateProtocol* g_modelUpdateProtocol = nullptr;

/**
 * @brief Network topology manager for federated learning mesh
 * 
 * Manages the network topology, peer discovery, and connection
 * maintenance for the federated learning network.
 */
NetworkTopologyManager* g_networkTopologyManager = nullptr;

/**
 * @brief Privacy-preserving aggregation for secure federated learning
 * 
 * Implements differential privacy and secure aggregation techniques
 * to protect individual device data during model aggregation.
 */
PrivacyPreservingAggregation* g_privacyAggregation = nullptr;

// =============================================================================
// Core System Global Pointers
// =============================================================================

/**
 * @brief Multi-board communication system
 * 
 * Manages communication and coordination between multiple ESP32 boards
 * working together in a distributed camera system.
 */
MultiboardSystem* g_multiboardSystem = nullptr;

/**
 * @brief Network-based OTA update manager
 * 
 * Handles over-the-air firmware updates across the network, supporting
 * coordinated updates for multiple devices.
 */
NetworkOTAManager* g_networkOTAManager = nullptr;

/**
 * @brief Network health monitoring system
 * 
 * Monitors network connectivity, latency, packet loss, and other metrics
 * to ensure reliable communication across the device network.
 */
NetworkHealthMonitor* g_networkHealthMonitor = nullptr;

/**
 * @brief Federated power management system
 * 
 * Coordinates power management across multiple devices to optimize
 * battery life and energy efficiency in the federated network.
 */
FederatedPowerManager* g_federatedPowerManager = nullptr;

/**
 * @brief XPowers power management IC interface
 * 
 * Manages XPowers PMICs (e.g., AXP192, AXP2101) for battery monitoring,
 * charging control, and power distribution.
 */
XPowersManager* g_xpowersManager = nullptr;

/**
 * @brief Multi-language support manager (not a pointer)
 * 
 * Provides internationalization (i18n) support for the user interface,
 * enabling the system to display messages in multiple languages.
 * 
 * @note This is instantiated as a global object, not a pointer.
 */
LanguageManager g_languageManager;

// =============================================================================
// Production System Global Pointers
// =============================================================================

/**
 * @brief Main production system coordinator
 * 
 * Manages all production-related functionality including deployment,
 * monitoring, and enterprise integration features.
 */
ProductionSystem* g_productionSystem = nullptr;

/**
 * @brief Configuration manager for deployment settings
 * 
 * Handles loading, saving, and managing system configuration parameters
 * for production deployments.
 */
ConfigManager* g_configManager = nullptr;

/**
 * @brief Over-the-air update manager for production devices
 * 
 * Manages secure OTA updates for production-deployed devices with
 * rollback support and update verification.
 */
OTAManager* g_otaManager = nullptr;

/**
 * @brief Cloud platform integration interface
 * 
 * Provides integration with cloud services for data upload, remote
 * monitoring, and cloud-based analytics.
 */
CloudIntegrator* g_cloudIntegrator = nullptr;

/**
 * @brief Cloud-based analytics engine
 * 
 * Processes and analyzes wildlife observation data in the cloud,
 * generating insights and reports for conservation efforts.
 */
CloudAnalyticsEngine* g_cloudAnalyticsEngine = nullptr;

/**
 * @brief Cloud configuration management system
 * 
 * Manages device configurations stored in the cloud, enabling
 * centralized configuration updates across multiple devices.
 */
CloudConfigManager* g_cloudConfigManager = nullptr;

/**
 * @brief Cloud service orchestration layer
 * 
 * Coordinates multiple cloud services (AWS, Azure, GCP) and manages
 * service selection, failover, and load balancing.
 */
CloudServiceOrchestrator* g_cloudServiceOrchestrator = nullptr;

/**
 * @brief Conservation impact verification system
 * 
 * Tracks and verifies the conservation impact of wildlife monitoring
 * activities, providing metrics and reports for stakeholders.
 */
ConservationImpactVerification* g_impactVerification = nullptr;

/**
 * @brief Enhanced cloud management with advanced features
 * 
 * Provides advanced cloud management capabilities including adaptive
 * sync, intelligent caching, and optimized data transfer.
 */
EnhancedCloudManager* g_enhancedCloudManager = nullptr;
