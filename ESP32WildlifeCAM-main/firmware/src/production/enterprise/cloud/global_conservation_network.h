/**
 * Global Conservation Network Platform
 * 
 * Provides foundation for worldwide wildlife monitoring integration,
 * connecting conservation efforts across continents with standardized
 * protocols for data sharing, threat intelligence, and collaborative
 * conservation action.
 * 
 * Features:
 * - Universal WildCAM network connectivity
 * - Standardized data exchange protocols
 * - Real-time species tracking and population monitoring
 * - International habitat mapping integration
 * - Global migration route tracking
 * - Climate change impact assessment
 * - Multi-organization data federation
 * 
 * @file global_conservation_network.h
 * @author ESP32WildlifeCAM Team
 * @date 2024-10-14
 */

#ifndef GLOBAL_CONSERVATION_NETWORK_H
#define GLOBAL_CONSERVATION_NETWORK_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "research_collaboration_platform.h"

// Network node types
enum NetworkNodeType {
    NODE_MONITORING_STATION,    // WildCAM monitoring station
    NODE_RESEARCH_FACILITY,     // Research institution
    NODE_CONSERVATION_ORG,      // Conservation organization
    NODE_GOVERNMENT_AGENCY,     // Government wildlife agency
    NODE_CITIZEN_SCIENCE,       // Citizen science contributor
    NODE_DATA_CENTER           // Regional data aggregation center
};

// Global regions for data distribution
enum GlobalRegion {
    REGION_NORTH_AMERICA,
    REGION_SOUTH_AMERICA,
    REGION_EUROPE,
    REGION_AFRICA,
    REGION_ASIA,
    REGION_OCEANIA,
    REGION_ANTARCTICA
};

// Data sharing levels
enum DataSharingLevel {
    SHARING_NONE,              // No sharing
    SHARING_LOCAL,             // Local network only
    SHARING_REGIONAL,          // Regional partners
    SHARING_CONTINENTAL,       // Continental network
    SHARING_GLOBAL            // Global network access
};

// Network node information
struct NetworkNode {
    String nodeId;
    String nodeName;
    NetworkNodeType nodeType;
    GlobalRegion region;
    String organization;
    String contactEmail;
    float latitude;
    float longitude;
    DataSharingLevel sharingLevel;
    uint32_t lastSync;
    bool isActive;
    std::vector<String> capabilities;
    std::map<String, String> metadata;
    
    NetworkNode() :
        nodeId(""), nodeName(""), nodeType(NODE_MONITORING_STATION),
        region(REGION_NORTH_AMERICA), organization(""), contactEmail(""),
        latitude(0.0), longitude(0.0), sharingLevel(SHARING_LOCAL),
        lastSync(0), isActive(true) {}
};

// Species observation data for global network
struct GlobalSpeciesObservation {
    String observationId;
    String speciesName;
    String scientificName;
    String commonName;
    uint32_t timestamp;
    float latitude;
    float longitude;
    GlobalRegion region;
    String habitat;
    uint16_t individualCount;
    String behavior;
    float confidence;
    String nodeId;
    String imageHash;  // SHA-256 hash for verification
    bool verified;
    std::vector<String> tags;
    
    GlobalSpeciesObservation() :
        observationId(""), speciesName(""), scientificName(""),
        commonName(""), timestamp(0), latitude(0.0), longitude(0.0),
        region(REGION_NORTH_AMERICA), habitat(""), individualCount(1),
        behavior(""), confidence(0.0), nodeId(""), imageHash(""),
        verified(false) {}
};

// Population tracking data
struct SpeciesPopulationData {
    String speciesId;
    String speciesName;
    GlobalRegion region;
    uint32_t estimatedPopulation;
    uint32_t lastCount;
    uint32_t trendingUp;       // Increasing population count
    uint32_t trendingDown;     // Decreasing population count
    uint32_t stable;           // Stable population count
    float changeRate;          // Population change rate %
    uint32_t lastUpdated;
    std::vector<String> threats;
    String conservationStatus; // IUCN status
    
    SpeciesPopulationData() :
        speciesId(""), speciesName(""), region(REGION_NORTH_AMERICA),
        estimatedPopulation(0), lastCount(0), trendingUp(0),
        trendingDown(0), stable(0), changeRate(0.0), lastUpdated(0),
        conservationStatus("Unknown") {}
};

// Migration route data
struct MigrationRoute {
    String routeId;
    String speciesName;
    std::vector<std::pair<float, float>> waypoints;  // lat, lon pairs
    std::vector<GlobalRegion> regionsTraversed;
    uint32_t seasonalStart;    // Timestamp for seasonal start
    uint32_t seasonalEnd;      // Timestamp for seasonal end
    String routeType;          // "spring", "fall", "year-round"
    bool crossesBorders;       // International migration
    std::vector<String> threats;
    uint32_t lastVerified;
    
    MigrationRoute() :
        routeId(""), speciesName(""), seasonalStart(0), seasonalEnd(0),
        routeType("unknown"), crossesBorders(false), lastVerified(0) {}
};

// Network statistics
struct NetworkStatistics {
    uint32_t totalNodes;
    uint32_t activeNodes;
    uint32_t totalObservations;
    uint32_t observationsToday;
    uint32_t speciesTracked;
    uint32_t threatsDetected;
    uint32_t collaborationRequests;
    float dataQualityScore;
    uint32_t lastUpdated;
    
    NetworkStatistics() :
        totalNodes(0), activeNodes(0), totalObservations(0),
        observationsToday(0), speciesTracked(0), threatsDetected(0),
        collaborationRequests(0), dataQualityScore(0.0), lastUpdated(0) {}
};

/**
 * Global Conservation Network Platform Class
 * 
 * Coordinates worldwide wildlife monitoring and conservation efforts
 * through standardized protocols and secure data sharing.
 */
class GlobalConservationNetwork {
public:
    GlobalConservationNetwork();
    ~GlobalConservationNetwork();
    
    // Network initialization and management
    bool initialize(const String& nodeId, const NetworkNode& nodeInfo);
    bool registerNode(const NetworkNode& node);
    bool updateNodeStatus(const String& nodeId, bool isActive);
    bool setDataSharingLevel(DataSharingLevel level);
    
    // Species observation sharing
    bool submitObservation(const GlobalSpeciesObservation& observation);
    bool queryObservations(const String& speciesName, GlobalRegion region,
                          std::vector<GlobalSpeciesObservation>& results);
    bool verifyObservation(const String& observationId, bool verified);
    
    // Population tracking
    bool updatePopulationData(const SpeciesPopulationData& data);
    bool getPopulationTrends(const String& speciesName, 
                            std::vector<SpeciesPopulationData>& trends);
    bool reportPopulationChange(const String& speciesName, GlobalRegion region,
                               int32_t changeCount, const String& reason);
    
    // Migration tracking
    bool submitMigrationData(const MigrationRoute& route);
    bool getMigrationRoutes(const String& speciesName,
                           std::vector<MigrationRoute>& routes);
    bool updateMigrationWaypoint(const String& routeId, float lat, float lon,
                                uint32_t timestamp);
    
    // Network connectivity and sync
    bool syncWithNetwork(bool fullSync = false);
    bool connectToRegionalHub(GlobalRegion region);
    bool broadcastToNetwork(const String& message, DataSharingLevel level);
    
    // Node discovery and collaboration
    bool discoverNearbyNodes(float radius, std::vector<NetworkNode>& nodes);
    bool requestCollaboration(const String& targetNodeId, 
                             const String& purpose);
    bool acceptCollaboration(const String& requestId);
    
    // Data quality and verification
    bool validateObservationData(const GlobalSpeciesObservation& obs);
    float calculateDataQualityScore();
    bool reportDataIssue(const String& dataId, const String& issue);
    
    // Network statistics and monitoring
    NetworkStatistics getNetworkStatistics() const;
    bool isNetworkConnected() const { return networkConnected_; }
    String getNodeId() const { return nodeId_; }
    GlobalRegion getCurrentRegion() const { return currentRegion_; }
    
    // Configuration and settings
    bool updateConfiguration(const JsonDocument& config);
    bool exportData(const String& outputPath, uint32_t startTime, 
                   uint32_t endTime);
    bool importData(const String& inputPath);
    
private:
    bool initialized_;
    bool networkConnected_;
    String nodeId_;
    NetworkNode localNode_;
    GlobalRegion currentRegion_;
    DataSharingLevel sharingLevel_;
    
    // Network state
    std::vector<NetworkNode> knownNodes_;
    std::vector<GlobalSpeciesObservation> localObservations_;
    std::map<String, SpeciesPopulationData> populationCache_;
    std::map<String, MigrationRoute> migrationRoutes_;
    NetworkStatistics statistics_;
    
    // Sync management
    uint32_t lastSyncTime_;
    uint32_t syncInterval_;
    bool autoSync_;
    
    // Helper methods
    bool connectToHub(const String& hubUrl);
    bool authenticateWithNetwork(const String& credentials);
    String generateObservationId();
    bool validateNodeRegistration(const NetworkNode& node);
    bool encryptDataForTransmission(const String& data, String& encrypted);
    bool decryptReceivedData(const String& encrypted, String& data);
    void updateStatistics();
    bool cacheObservation(const GlobalSpeciesObservation& obs);
    bool uploadCachedData();
};

// Global network instance
extern GlobalConservationNetwork* g_globalNetwork;

// Utility functions for easy integration
bool initializeGlobalNetwork(const String& nodeId, const NetworkNode& nodeInfo);
bool shareObservation(const String& speciesName, float lat, float lon);
bool queryGlobalSpecies(const String& speciesName, GlobalRegion region);
bool trackMigration(const String& speciesName, float lat, float lon);
void cleanupGlobalNetwork();

#endif // GLOBAL_CONSERVATION_NETWORK_H
