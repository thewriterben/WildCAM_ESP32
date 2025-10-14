# Global Conservation Network API Reference

Complete API reference for the Global Conservation Network Platform.

## Quick Start Functions

### Initialization

```cpp
bool initializeGlobalConservationNetwork(const String& nodeId, GlobalRegion region)
```
Initialize and connect to the global conservation network.

**Parameters:**
- `nodeId`: Unique identifier for this node (e.g., "SERENGETI_001")
- `region`: Geographic region (REGION_AFRICA, REGION_ASIA, etc.)

**Returns:** `true` if initialization successful

**Example:**
```cpp
if (initializeGlobalConservationNetwork("NODE_001", REGION_AFRICA)) {
    Serial.println("Connected to global network");
}
```

---

### Quick Alert Function

```cpp
bool shareGlobalThreatAlert(GlobalThreatType type, const String& species, float severity)
```
Quickly share a threat alert with the global network.

**Parameters:**
- `type`: Type of threat (THREAT_POACHING, THREAT_HABITAT_LOSS, etc.)
- `species`: Scientific or common name of affected species
- `severity`: Threat severity (0.0 - 1.0, where 1.0 is critical)

**Returns:** `true` if alert shared successfully

**Example:**
```cpp
shareGlobalThreatAlert(THREAT_POACHING, "Loxodonta africana", 0.9f);
```

---

### Species Tracking

```cpp
bool trackMigratorySpecies(const String& speciesId, float lat, float lon)
```
Update location for a tracked migratory species.

**Parameters:**
- `speciesId`: Unique identifier for the species/individual
- `lat`: Latitude coordinate
- `lon`: Longitude coordinate

**Returns:** `true` if location updated successfully

**Example:**
```cpp
trackMigratorySpecies("ELEPHANT_001", -2.1534, 34.6857);
```

---

### Treaty Reporting

```cpp
bool reportToInternationalTreaty(TreatyCompliance treaty, const JsonDocument& report)
```
Submit compliance report to international treaty.

**Parameters:**
- `treaty`: Treaty type (TREATY_CITES, TREATY_CMS, etc.)
- `report`: JSON document containing report data

**Returns:** `true` if report submitted successfully

**Example:**
```cpp
JsonDocument report;
report["species_monitored"] = 15;
report["compliance_status"] = "compliant";
reportToInternationalTreaty(TREATY_CITES, report);
```

---

### Network Statistics

```cpp
GlobalNetworkStats getGlobalNetworkStats()
```
Get current global network statistics.

**Returns:** `GlobalNetworkStats` structure with network information

**Example:**
```cpp
GlobalNetworkStats stats = getGlobalNetworkStats();
Serial.printf("Active Nodes: %d\n", stats.activeNodes);
```

---

### Cleanup

```cpp
void cleanupGlobalConservationNetwork()
```
Clean up and disconnect from global network.

## Full Class API

### GlobalConservationNetwork Class

#### Constructor & Initialization

```cpp
GlobalConservationNetwork()
~GlobalConservationNetwork()

bool initialize(const String& nodeId, GlobalRegion region)
void cleanup()
```

#### Network Coordination

```cpp
bool connectToGlobalNetwork()
```
Connect this node to the global conservation network.

**Returns:** `true` if connection successful

---

```cpp
bool registerNode(const GlobalNetworkNode& node)
```
Register a new node with the global network.

**Parameters:**
- `node`: Node information structure

**Returns:** `true` if registration successful

---

```cpp
bool updateNodeStatus(const String& nodeId, bool active)
```
Update the active status of a network node.

**Parameters:**
- `nodeId`: Node identifier
- `active`: Active status (true/false)

**Returns:** `true` if status updated

---

```cpp
std::vector<GlobalNetworkNode> getActiveNodes() const
```
Get list of all active nodes in the network.

**Returns:** Vector of active network nodes

---

```cpp
std::vector<GlobalNetworkNode> getRegionalNodes(GlobalRegion region) const
```
Get list of active nodes in a specific region.

**Parameters:**
- `region`: Geographic region

**Returns:** Vector of regional nodes

#### Threat Intelligence Sharing

```cpp
bool shareThreatIntelligence(const ThreatIntelligence& threat)
```
Share threat intelligence with the global network.

**Parameters:**
- `threat`: Threat intelligence structure

**Returns:** `true` if shared successfully

**Example:**
```cpp
ThreatIntelligence threat;
threat.threatId = "THREAT_001";
threat.threatType = THREAT_POACHING;
threat.species = "Loxodonta africana";
threat.severity = 0.9f;
threat.affectedRegion = REGION_AFRICA;
threat.requiresImmediate = true;
g_globalConservationNetwork->shareThreatIntelligence(threat);
```

---

```cpp
bool receiveThreatIntelligence(const ThreatIntelligence& threat)
```
Receive and process incoming threat intelligence.

**Parameters:**
- `threat`: Incoming threat intelligence

**Returns:** `true` if processed successfully

---

```cpp
std::vector<ThreatIntelligence> getRegionalThreats(GlobalRegion region) const
```
Get all threats affecting a specific region.

**Parameters:**
- `region`: Geographic region

**Returns:** Vector of regional threats

---

```cpp
bool propagateAlert(const String& alertId, const std::vector<GlobalRegion>& regions)
```
Propagate alert to multiple regions.

**Parameters:**
- `alertId`: Alert identifier
- `regions`: Vector of target regions

**Returns:** `true` if propagated successfully

#### Cross-Border Species Tracking

```cpp
bool registerMigratorySpecies(const CrossBorderSpecies& species)
```
Register a migratory species for cross-border tracking.

**Parameters:**
- `species`: Species information structure

**Returns:** `true` if registered successfully

**Example:**
```cpp
CrossBorderSpecies elephant;
elephant.speciesId = "ELEPHANT_001";
elephant.scientificName = "Loxodonta africana";
elephant.commonName = "African Elephant";
elephant.migratoryStatus = MIGRATORY_INTERNATIONAL;
elephant.habitatRegions = {REGION_AFRICA};
elephant.conservationStatus = "EN"; // Endangered
elephant.isEndangered = true;
elephant.applicableTreaties = {TREATY_CITES, TREATY_CMS};
g_globalConservationNetwork->registerMigratorySpecies(elephant);
```

---

```cpp
bool updateSpeciesLocation(const String& speciesId, float lat, float lon, uint32_t timestamp)
```
Update location for a tracked species.

**Parameters:**
- `speciesId`: Species identifier
- `lat`: Latitude
- `lon`: Longitude
- `timestamp`: Update timestamp

**Returns:** `true` if updated successfully

---

```cpp
bool requestSpeciesHandoff(const String& speciesId, const String& targetNodeId)
```
Request tracking handoff to another node.

**Parameters:**
- `speciesId`: Species identifier
- `targetNodeId`: Target node ID

**Returns:** `true` if handoff requested successfully

---

```cpp
CrossBorderSpecies getSpeciesInfo(const String& speciesId) const
```
Get information about a tracked species.

**Parameters:**
- `speciesId`: Species identifier

**Returns:** Species information structure

---

```cpp
std::vector<CrossBorderSpecies> getTrackedSpecies() const
```
Get all tracked species.

**Returns:** Vector of tracked species

#### International Data Sharing

```cpp
bool shareConservationData(const GlobalConservationData& data)
```
Share conservation data with the global network.

**Parameters:**
- `data`: Conservation data package

**Returns:** `true` if shared successfully

**Example:**
```cpp
GlobalConservationData data;
data.dataId = "DATA_001";
data.nodeId = "NODE_001";
data.region = REGION_AFRICA;
data.dataType = "wildlife_detection";
data.privacyLevel = PRIVACY_RESEARCH;

JsonDocument detectionData;
detectionData["species"] = "Panthera leo";
detectionData["confidence"] = 0.95;
data.data = detectionData;

data.sharedWith = {"IUCN", "WWF"};
g_globalConservationNetwork->shareConservationData(data);
```

---

```cpp
bool requestDataAccess(const String& dataId, const String& requesterId)
```
Request access to shared conservation data.

**Parameters:**
- `dataId`: Data identifier
- `requesterId`: Requester identifier

**Returns:** `true` if request processed

---

```cpp
bool grantDataAccess(const String& dataId, const String& organizationId)
```
Grant data access to an organization.

**Parameters:**
- `dataId`: Data identifier
- `organizationId`: Organization identifier

**Returns:** `true` if access granted

---

```cpp
std::vector<GlobalConservationData> getSharedData(GlobalRegion region) const
```
Get shared data for a region.

**Parameters:**
- `region`: Geographic region (optional, defaults to node's region)

**Returns:** Vector of shared data packages

#### Treaty Compliance

```cpp
bool reportTreatyCompliance(TreatyCompliance treaty, const JsonDocument& report)
```
Report compliance with international treaty.

**Parameters:**
- `treaty`: Treaty type
- `report`: Compliance report data

**Returns:** `true` if report submitted

---

```cpp
bool verifyTreatyRequirements(TreatyCompliance treaty, const String& activity)
```
Verify if an activity complies with treaty requirements.

**Parameters:**
- `treaty`: Treaty type
- `activity`: Activity description

**Returns:** `true` if activity complies

---

```cpp
std::vector<TreatyCompliance> getApplicableTreaties(const String& speciesId) const
```
Get applicable treaties for a species.

**Parameters:**
- `speciesId`: Species identifier

**Returns:** Vector of applicable treaties

#### Conservation Impact Verification (Blockchain)

```cpp
bool recordConservationOutcome(const String& projectId, const JsonDocument& outcome)
```
Record conservation outcome to blockchain for verification.

**Parameters:**
- `projectId`: Project identifier
- `outcome`: Conservation outcome data

**Returns:** `true` if recorded successfully

**Example:**
```cpp
JsonDocument outcome;
outcome["elephants_protected"] = 250;
outcome["habitat_secured_hectares"] = 5000;
outcome["poaching_incidents_prevented"] = 12;
outcome["funding_usd"] = 500000;
g_globalConservationNetwork->recordConservationOutcome("PROJ_001", outcome);
```

---

```cpp
bool verifyConservationImpact(const String& projectId, String& verificationHash)
```
Verify conservation impact using blockchain.

**Parameters:**
- `projectId`: Project identifier
- `verificationHash`: Output verification hash

**Returns:** `true` if verified successfully

---

```cpp
bool auditConservationChain(const String& projectId, JsonDocument& auditLog)
```
Generate audit trail for conservation project.

**Parameters:**
- `projectId`: Project identifier
- `auditLog`: Output audit log

**Returns:** `true` if audit generated successfully

#### Emergency Response Coordination

```cpp
bool declareEmergency(GlobalThreatType type, GlobalRegion region, const String& description)
```
Declare a conservation emergency.

**Parameters:**
- `type`: Threat type
- `region`: Affected region
- `description`: Emergency description

**Returns:** `true` if emergency declared

**Example:**
```cpp
g_globalConservationNetwork->declareEmergency(
    THREAT_DISEASE_OUTBREAK,
    REGION_AFRICA,
    "Anthrax outbreak affecting wildebeest population"
);
```

---

```cpp
bool requestEmergencyAssistance(const String& requestType, const JsonDocument& details)
```
Request emergency assistance from global network.

**Parameters:**
- `requestType`: Type of assistance needed
- `details`: Detailed request information

**Returns:** `true` if request sent

---

```cpp
bool coordinateEmergencyResponse(const String& emergencyId, const std::vector<String>& respondingNodes)
```
Coordinate emergency response between nodes.

**Parameters:**
- `emergencyId`: Emergency identifier
- `respondingNodes`: Vector of responding node IDs

**Returns:** `true` if coordination initiated

#### Global Collaboration

```cpp
bool initiateInternationalProject(const String& projectId, const std::vector<String>& participatingOrgs)
```
Initiate international research/conservation project.

**Parameters:**
- `projectId`: Project identifier
- `participatingOrgs`: Vector of participating organization IDs

**Returns:** `true` if project initiated

---

```cpp
bool shareResearchFindings(const String& projectId, const JsonDocument& findings)
```
Share research findings with global network.

**Parameters:**
- `projectId`: Project identifier
- `findings`: Research findings data

**Returns:** `true` if findings shared

---

```cpp
bool requestCollaboration(const String& organizationId, const String& purpose)
```
Request collaboration with an organization.

**Parameters:**
- `organizationId`: Organization identifier
- `purpose`: Collaboration purpose

**Returns:** `true` if request sent

#### Multi-Language Support

```cpp
bool setPreferredLanguages(const std::vector<String>& languages)
```
Set preferred languages for this node.

**Parameters:**
- `languages`: Vector of language codes (e.g., ["en", "es", "fr"])

**Returns:** `true` if languages set

---

```cpp
String translateConservationMessage(const String& message, const String& targetLanguage)
```
Translate conservation message to target language.

**Parameters:**
- `message`: Message to translate
- `targetLanguage`: Target language code

**Returns:** Translated message

#### Statistics and Monitoring

```cpp
GlobalNetworkStats getNetworkStatistics() const
```
Get current network statistics.

**Returns:** Network statistics structure

---

```cpp
bool isConnectedToGlobalNetwork() const
```
Check if connected to global network.

**Returns:** `true` if connected

---

```cpp
float getGlobalNetworkHealth() const
```
Get global network health metric.

**Returns:** Health value (0.0 - 1.0)

---

```cpp
uint32_t getLastSyncTime() const
```
Get timestamp of last network synchronization.

**Returns:** Timestamp in milliseconds

#### Configuration

```cpp
bool setRegion(GlobalRegion region)
```
Set geographic region for this node.

**Parameters:**
- `region`: Geographic region

**Returns:** `true` if region set

---

```cpp
GlobalRegion getRegion() const
```
Get current region setting.

**Returns:** Current region

---

```cpp
bool setOrganizationType(OrganizationType type)
```
Set organization type for this node.

**Parameters:**
- `type`: Organization type

**Returns:** `true` if type set

---

```cpp
bool enableTreatyCompliance(TreatyCompliance treaty)
```
Enable compliance monitoring for a treaty.

**Parameters:**
- `treaty`: Treaty type

**Returns:** `true` if enabled

## Data Structures

### GlobalNetworkNode

```cpp
struct GlobalNetworkNode {
    String nodeId;              // Unique node identifier
    String location;            // Human-readable location
    GlobalRegion region;        // Geographic region
    float latitude;             // GPS latitude
    float longitude;            // GPS longitude
    String country;             // Country name
    String timezone;            // Timezone identifier
    OrganizationType orgType;   // Organization type
    std::vector<String> treaties;  // Applicable treaties
    uint32_t lastContact;       // Last contact timestamp
    bool isActive;              // Active status
};
```

### ThreatIntelligence

```cpp
struct ThreatIntelligence {
    String threatId;                        // Unique threat ID
    GlobalThreatType threatType;            // Threat type
    String species;                         // Affected species
    GlobalRegion affectedRegion;            // Affected region
    float latitude;                         // Location latitude
    float longitude;                        // Location longitude
    uint32_t detectionTime;                 // Detection timestamp
    float severity;                         // Severity (0.0 - 1.0)
    String description;                     // Threat description
    std::vector<String> affectedCountries;  // Affected countries
    bool requiresImmediate;                 // Immediate action flag
    String sourceNodeId;                    // Source node ID
};
```

### CrossBorderSpecies

```cpp
struct CrossBorderSpecies {
    String speciesId;                           // Unique species ID
    String scientificName;                      // Scientific name
    String commonName;                          // Common name
    MigratoryStatus migratoryStatus;            // Migration status
    std::vector<GlobalRegion> habitatRegions;   // Habitat regions
    std::vector<String> protectedCountries;     // Protected countries
    std::vector<TreatyCompliance> applicableTreaties;  // Applicable treaties
    uint32_t lastSighting;                      // Last sighting time
    String lastLocation;                        // Last location
    bool isEndangered;                          // Endangered status
    String conservationStatus;                  // IUCN status (LC, EN, etc.)
};
```

### GlobalConservationData

```cpp
struct GlobalConservationData {
    String dataId;                  // Unique data ID
    String nodeId;                  // Source node ID
    uint32_t timestamp;             // Data timestamp
    GlobalRegion region;            // Geographic region
    String dataType;                // Data type description
    JsonDocument data;              // Actual data content
    String blockchainHash;          // Blockchain verification hash
    std::vector<String> sharedWith; // Organizations with access
    PrivacyLevel privacyLevel;      // Privacy level
};
```

### GlobalNetworkStats

```cpp
struct GlobalNetworkStats {
    uint32_t totalNodes;                // Total registered nodes
    uint32_t activeNodes;               // Active nodes
    uint32_t regionsConnected;          // Connected regions
    uint32_t threatAlertsShared;        // Threat alerts shared
    uint32_t speciesTracked;            // Species being tracked
    uint32_t dataPackagesExchanged;     // Data packages exchanged
    uint32_t collaborativeProjects;     // Active projects
    uint32_t treatyComplianceReports;   // Treaty reports submitted
    float networkHealth;                // Network health (0.0 - 1.0)
    uint32_t lastUpdate;                // Last update timestamp
};
```

## Enumerations

### GlobalRegion

```cpp
enum GlobalRegion {
    REGION_AFRICA,
    REGION_ASIA,
    REGION_EUROPE,
    REGION_NORTH_AMERICA,
    REGION_SOUTH_AMERICA,
    REGION_OCEANIA,
    REGION_ANTARCTICA
};
```

### TreatyCompliance

```cpp
enum TreatyCompliance {
    TREATY_CITES,    // Convention on International Trade in Endangered Species
    TREATY_CBD,      // Convention on Biological Diversity
    TREATY_CMS,      // Convention on Migratory Species
    TREATY_RAMSAR,   // Wetlands Convention
    TREATY_WHC,      // World Heritage Convention
    TREATY_REGIONAL  // Regional conservation agreements
};
```

### OrganizationType

```cpp
enum OrganizationType {
    ORG_UN_AGENCY,    // UN agencies (UNEP, UNESCO, etc.)
    ORG_NGO,          // International NGOs (WWF, IUCN, etc.)
    ORG_GOVERNMENT,   // National government agencies
    ORG_RESEARCH,     // Academic/research institutions
    ORG_INDIGENOUS,   // Indigenous peoples organizations
    ORG_COMMUNITY     // Local community organizations
};
```

### GlobalThreatType

```cpp
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
```

### MigratoryStatus

```cpp
enum MigratoryStatus {
    MIGRATORY_INTERCONTINENTAL,  // Crosses multiple continents
    MIGRATORY_INTERNATIONAL,     // Crosses national borders
    MIGRATORY_REGIONAL,          // Regional movement
    MIGRATORY_LOCAL,             // Local area only
    MIGRATORY_UNKNOWN            // Status unknown
};
```

## Integration with Existing Systems

The Global Conservation Network integrates with:

- **ResearchCollaborationPlatform**: Research data sharing
- **BlockchainManager**: Tamper-proof conservation records
- **FederatedLearningSystem**: Global model coordination
- **LoRaWildlifeAlerts**: Local alert propagation
- **MeshCoordinator**: Network coordination
- **Multi-language i18n**: Language support

See respective documentation for integration details.

## Error Handling

Most functions return `bool` indicating success/failure. Check return values:

```cpp
if (!g_globalConservationNetwork->shareThreatIntelligence(threat)) {
    Serial.println("Failed to share threat intelligence");
    // Handle error
}
```

## Performance Considerations

- **Memory**: ~5-10KB runtime usage
- **Bandwidth**: Optimized message sizes (100-1000 bytes typical)
- **Processing**: Minimal CPU overhead, heavy processing in cloud
- **Scalability**: Designed for resource-constrained ESP32

## Support

- **Documentation**: See `GLOBAL_CONSERVATION_NETWORK.md`
- **Examples**: See `examples/global_conservation_network_example.cpp`
- **Source**: `global_conservation_network.h` and `.cpp`
