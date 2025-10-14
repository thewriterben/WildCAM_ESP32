/**
 * Global Conservation Network Platform
 * 
 * Provides comprehensive international conservation coordination infrastructure
 * for the WildCAM ESP32 system, enabling worldwide collaboration and data sharing.
 * 
 * Features:
 * - Global network coordination across continents
 * - Real-time international threat intelligence sharing
 * - Cross-border species tracking and protection
 * - Multi-organization data sharing protocols
 * - International conservation standards compliance
 * - Blockchain-based conservation impact verification
 * - Global emergency response coordination
 * 
 * Architecture:
 * This module provides the ESP32-side coordination layer for global conservation
 * networks. Heavy processing, cloud infrastructure, and data aggregation are
 * handled by backend systems. The ESP32 focuses on:
 * - Local data collection and processing
 * - Protocol compliance and standardization
 * - Secure communication with global network
 * - Real-time alert coordination
 * - Federated learning participation
 */

#ifndef GLOBAL_CONSERVATION_NETWORK_H
#define GLOBAL_CONSERVATION_NETWORK_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "research_collaboration_platform.h"
#include "../../../lora_wildlife_alerts.h"

// Global Network Regions
enum GlobalRegion {
    REGION_AFRICA,
    REGION_ASIA,
    REGION_EUROPE,
    REGION_NORTH_AMERICA,
    REGION_SOUTH_AMERICA,
    REGION_OCEANIA,
    REGION_ANTARCTICA
};

// Conservation Treaty Compliance
enum TreatyCompliance {
    TREATY_CITES,              // Convention on International Trade in Endangered Species
    TREATY_CBD,                // Convention on Biological Diversity  
    TREATY_CMS,                // Convention on Migratory Species
    TREATY_RAMSAR,             // Wetlands Convention
    TREATY_WHC,                // World Heritage Convention
    TREATY_REGIONAL            // Regional conservation agreements
};

// International Organization Types
enum OrganizationType {
    ORG_UN_AGENCY,             // United Nations agencies (UNEP, UNESCO, etc.)
    ORG_NGO,                   // International NGOs (WWF, IUCN, etc.)
    ORG_GOVERNMENT,            // National government agencies
    ORG_RESEARCH,              // Academic/research institutions
    ORG_INDIGENOUS,            // Indigenous peoples organizations
    ORG_COMMUNITY              // Local community organizations
};

// Global Threat Types
enum GlobalThreatType {
    THREAT_POACHING,
    THREAT_HABITAT_LOSS,
    THREAT_CLIMATE_CHANGE,
    THREAT_POLLUTION,
    THREAT_INVASIVE_SPECIES,
    THREAT_DISEASE_OUTBREAK,
    THREAT_ILLEGAL_TRADE,
    THREAT_HUMAN_CONFLICT
};

// Cross-Border Species Status
enum MigratoryStatus {
    MIGRATORY_INTERCONTINENTAL,    // Crosses multiple continents
    MIGRATORY_INTERNATIONAL,       // Crosses national borders
    MIGRATORY_REGIONAL,            // Regional movement
    MIGRATORY_LOCAL,               // Local area only
    MIGRATORY_UNKNOWN              // Status unknown
};

/**
 * Global Network Node Information
 */
struct GlobalNetworkNode {
    String nodeId;
    String location;
    GlobalRegion region;
    float latitude;
    float longitude;
    String country;
    String timezone;
    OrganizationType orgType;
    std::vector<String> treaties;
    uint32_t lastContact;
    bool isActive;
    
    GlobalNetworkNode() : 
        region(REGION_AFRICA), latitude(0.0f), longitude(0.0f),
        orgType(ORG_COMMUNITY), lastContact(0), isActive(true) {}
};

/**
 * International Threat Intelligence
 */
struct ThreatIntelligence {
    String threatId;
    GlobalThreatType threatType;
    String species;
    GlobalRegion affectedRegion;
    float latitude;
    float longitude;
    uint32_t detectionTime;
    float severity;              // 0.0 - 1.0
    String description;
    std::vector<String> affectedCountries;
    bool requiresImmediate;
    String sourceNodeId;
    
    ThreatIntelligence() :
        threatType(THREAT_POACHING), affectedRegion(REGION_AFRICA),
        latitude(0.0f), longitude(0.0f), detectionTime(0),
        severity(0.5f), requiresImmediate(false) {}
};

/**
 * Cross-Border Species Tracking
 */
struct CrossBorderSpecies {
    String speciesId;
    String scientificName;
    String commonName;
    MigratoryStatus migratoryStatus;
    std::vector<GlobalRegion> habitatRegions;
    std::vector<String> protectedCountries;
    std::vector<TreatyCompliance> applicableTreaties;
    uint32_t lastSighting;
    String lastLocation;
    bool isEndangered;
    String conservationStatus;    // IUCN Red List category
    
    CrossBorderSpecies() :
        migratoryStatus(MIGRATORY_UNKNOWN), lastSighting(0),
        isEndangered(false), conservationStatus("LC") {}
};

/**
 * Global Conservation Data Package
 */
struct GlobalConservationData {
    String dataId;
    String nodeId;
    uint32_t timestamp;
    GlobalRegion region;
    String dataType;
    JsonDocument data;
    String blockchainHash;        // For tamper-proof verification
    std::vector<String> sharedWith;
    PrivacyLevel privacyLevel;
    
    GlobalConservationData() :
        timestamp(0), region(REGION_AFRICA),
        privacyLevel(PRIVACY_RESEARCH) {}
};

/**
 * Global Network Statistics
 */
struct GlobalNetworkStats {
    uint32_t totalNodes;
    uint32_t activeNodes;
    uint32_t regionsConnected;
    uint32_t threatAlertsShared;
    uint32_t speciesTracked;
    uint32_t dataPackagesExchanged;
    uint32_t collaborativeProjects;
    uint32_t treatyComplianceReports;
    float networkHealth;
    uint32_t lastUpdate;
    
    GlobalNetworkStats() :
        totalNodes(0), activeNodes(0), regionsConnected(0),
        threatAlertsShared(0), speciesTracked(0),
        dataPackagesExchanged(0), collaborativeProjects(0),
        treatyComplianceReports(0), networkHealth(1.0f), lastUpdate(0) {}
};

/**
 * Global Conservation Network Platform Class
 * 
 * Coordinates participation in the worldwide conservation network,
 * managing international data sharing, threat intelligence, and
 * collaborative conservation efforts.
 */
class GlobalConservationNetwork {
public:
    // Constructor and initialization
    GlobalConservationNetwork();
    ~GlobalConservationNetwork();
    
    bool initialize(const String& nodeId, GlobalRegion region);
    void cleanup();
    
    // Network coordination
    bool connectToGlobalNetwork();
    bool registerNode(const GlobalNetworkNode& node);
    bool updateNodeStatus(const String& nodeId, bool active);
    std::vector<GlobalNetworkNode> getActiveNodes() const;
    std::vector<GlobalNetworkNode> getRegionalNodes(GlobalRegion region) const;
    
    // Threat intelligence sharing
    bool shareThreatIntelligence(const ThreatIntelligence& threat);
    bool receiveThreatIntelligence(const ThreatIntelligence& threat);
    std::vector<ThreatIntelligence> getRegionalThreats(GlobalRegion region) const;
    bool propagateAlert(const String& alertId, const std::vector<GlobalRegion>& regions);
    
    // Cross-border species tracking
    bool registerMigratorySpecies(const CrossBorderSpecies& species);
    bool updateSpeciesLocation(const String& speciesId, float lat, float lon, uint32_t timestamp);
    bool requestSpeciesHandoff(const String& speciesId, const String& targetNodeId);
    CrossBorderSpecies getSpeciesInfo(const String& speciesId) const;
    std::vector<CrossBorderSpecies> getTrackedSpecies() const;
    
    // International data sharing
    bool shareConservationData(const GlobalConservationData& data);
    bool requestDataAccess(const String& dataId, const String& requesterId);
    bool grantDataAccess(const String& dataId, const String& organizationId);
    std::vector<GlobalConservationData> getSharedData(GlobalRegion region = REGION_AFRICA) const;
    
    // Treaty compliance
    bool reportTreatyCompliance(TreatyCompliance treaty, const JsonDocument& report);
    bool verifyTreatyRequirements(TreatyCompliance treaty, const String& activity);
    std::vector<TreatyCompliance> getApplicableTreaties(const String& speciesId) const;
    
    // Conservation impact verification (blockchain-based)
    bool recordConservationOutcome(const String& projectId, const JsonDocument& outcome);
    bool verifyConservationImpact(const String& projectId, String& verificationHash);
    bool auditConservationChain(const String& projectId, JsonDocument& auditLog);
    
    // Emergency response coordination
    bool declareEmergency(GlobalThreatType type, GlobalRegion region, const String& description);
    bool requestEmergencyAssistance(const String& requestType, const JsonDocument& details);
    bool coordinateEmergencyResponse(const String& emergencyId, const std::vector<String>& respondingNodes);
    
    // Global collaboration
    bool initiateInternationalProject(const String& projectId, const std::vector<String>& participatingOrgs);
    bool shareResearchFindings(const String& projectId, const JsonDocument& findings);
    bool requestCollaboration(const String& organizationId, const String& purpose);
    
    // Multi-language support (leverages existing i18n system)
    bool setPreferredLanguages(const std::vector<String>& languages);
    String translateConservationMessage(const String& message, const String& targetLanguage);
    
    // Statistics and monitoring
    GlobalNetworkStats getNetworkStatistics() const;
    bool isConnectedToGlobalNetwork() const;
    float getGlobalNetworkHealth() const;
    uint32_t getLastSyncTime() const;
    
    // Configuration
    bool setRegion(GlobalRegion region);
    GlobalRegion getRegion() const;
    bool setOrganizationType(OrganizationType type);
    bool enableTreatyCompliance(TreatyCompliance treaty);
    
private:
    // Node information
    String nodeId_;
    GlobalRegion region_;
    OrganizationType orgType_;
    bool initialized_;
    bool connected_;
    
    // Network state
    std::vector<GlobalNetworkNode> networkNodes_;
    std::vector<ThreatIntelligence> threatIntelligence_;
    std::vector<CrossBorderSpecies> trackedSpecies_;
    std::vector<GlobalConservationData> sharedData_;
    std::vector<TreatyCompliance> enabledTreaties_;
    
    // Statistics
    GlobalNetworkStats stats_;
    uint32_t lastSyncTime_;
    
    // Integration with existing systems
    ResearchCollaborationPlatform* researchPlatform_;
    
    // Helper methods
    bool validateThreatIntelligence(const ThreatIntelligence& threat);
    bool validateSpeciesData(const CrossBorderSpecies& species);
    String generateDataHash(const GlobalConservationData& data);
    bool syncWithGlobalNetwork();
    void updateStatistics();
};

// Global instance
extern GlobalConservationNetwork* g_globalConservationNetwork;

// Utility functions
bool initializeGlobalConservationNetwork(const String& nodeId, GlobalRegion region);
bool shareGlobalThreatAlert(GlobalThreatType type, const String& species, float severity);
bool trackMigratorySpecies(const String& speciesId, float lat, float lon);
bool reportToInternationalTreaty(TreatyCompliance treaty, const JsonDocument& report);
GlobalNetworkStats getGlobalNetworkStats();
void cleanupGlobalConservationNetwork();

#endif // GLOBAL_CONSERVATION_NETWORK_H
