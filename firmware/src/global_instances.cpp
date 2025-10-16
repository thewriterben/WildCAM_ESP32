/**
 * @file global_instances.cpp
 * @brief Centralized instantiation of global system pointers.
 * 
 * This file defines the global pointers that are declared as 'extern' in other parts
 * of the application. Initializing them to nullptr here resolves linker errors.
 */

#include "ai/ai_wildlife_system.h"
#include "ai/wildlife_classifier.h"
#include "ai/tinyml/edge_impulse_integration.h"
#include "ai/federated_learning/federated_learning_system.h"
#include "ai/federated_learning/federated_learning_coordinator.h"
#include "ai/federated_learning/local_training_module.h"
#include "ai/federated_learning/model_update_protocol.h"
#include "ai/federated_learning/network_topology_manager.h"
#include "ai/federated_learning/privacy_preserving_aggregation.h"
#include "multi_board/multi_board_system.h"
#include "network/ota_manager.h"
#include "network_health_monitor.h"
#include "power/federated_power_manager.h"
#include "power/xpowers_manager.h"
#include "production/production_system.h"
#include "production/deployment/config_manager.h"
#include "production/deployment/ota_manager.h"
#include "production/enterprise/cloud_integrator.h"
#include "production/enterprise/cloud/cloud_analytics_engine.h"
#include "production/enterprise/cloud/cloud_config_manager.h"
#include "production/enterprise/cloud/cloud_service_orchestrator.h"
#include "production/enterprise/cloud/conservation_impact_verification.h"
#include "production/enterprise/cloud/enhanced_cloud_manager.h"
#include "i18n/language_manager.h"

// AI & ML Systems
WildlifeClassifier* g_wildlifeClassifier = nullptr;
EdgeImpulseIntegration* g_edgeImpulse = nullptr;
FederatedLearningSystem* g_federatedLearningSystem = nullptr;
FederatedLearningCoordinator* g_federatedCoordinator = nullptr;
LocalTrainingModule* g_localTrainingModule = nullptr;
ModelUpdateProtocol* g_modelUpdateProtocol = nullptr;
NetworkTopologyManager* g_networkTopologyManager = nullptr;
PrivacyPreservingAggregation* g_privacyAggregation = nullptr;

// Core Systems
MultiboardSystem* g_multiboardSystem = nullptr;
NetworkOTAManager* g_networkOTAManager = nullptr;
NetworkHealthMonitor* g_networkHealthMonitor = nullptr;
FederatedPowerManager* g_federatedPowerManager = nullptr;
XPowersManager* g_xpowersManager = nullptr;
LanguageManager g_languageManager;

// Production & Enterprise Systems
ProductionSystem* g_productionSystem = nullptr;
ConfigManager* g_configManager = nullptr;
OTAManager* g_otaManager = nullptr;
CloudIntegrator* g_cloudIntegrator = nullptr;
CloudAnalyticsEngine* g_cloudAnalyticsEngine = nullptr;
CloudConfigManager* g_cloudConfigManager = nullptr;
CloudServiceOrchestrator* g_cloudServiceOrchestrator = nullptr;
ConservationImpactVerification* g_impactVerification = nullptr;
EnhancedCloudManager* g_enhancedCloudManager = nullptr;
