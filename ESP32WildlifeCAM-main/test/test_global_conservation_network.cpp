/**
 * @file test_global_conservation_network.cpp
 * @brief Unit tests for Global Conservation Network functionality
 * 
 * Tests core functionality of the global conservation network platform
 * including initialization, threat intelligence, species tracking,
 * treaty compliance, and international data sharing.
 */

#include <unity.h>
#include "../firmware/src/production/enterprise/cloud/global_conservation_network.h"

// Test instance (extern from header)
// GlobalConservationNetwork* g_globalConservationNetwork;

void setUp(void) {
    // Initialize global conservation network before each test
    if (!g_globalConservationNetwork) {
        g_globalConservationNetwork = new GlobalConservationNetwork();
    }
}

void tearDown(void) {
    // Clean up after each test
    if (g_globalConservationNetwork) {
        g_globalConservationNetwork->cleanup();
    }
}

// ===========================
// INITIALIZATION TESTS
// ===========================

void test_global_network_initialization(void) {
    bool result = g_globalConservationNetwork->initialize("TEST_NODE_001", REGION_AFRICA);
    TEST_ASSERT_TRUE(result);
    
    GlobalRegion region = g_globalConservationNetwork->getRegion();
    TEST_ASSERT_EQUAL(REGION_AFRICA, region);
}

void test_global_network_connection(void) {
    g_globalConservationNetwork->initialize("TEST_NODE_002", REGION_ASIA);
    
    bool connected = g_globalConservationNetwork->connectToGlobalNetwork();
    TEST_ASSERT_TRUE(connected);
    TEST_ASSERT_TRUE(g_globalConservationNetwork->isConnectedToGlobalNetwork());
}

void test_organization_type_configuration(void) {
    g_globalConservationNetwork->initialize("TEST_NODE_003", REGION_EUROPE);
    
    bool result = g_globalConservationNetwork->setOrganizationType(ORG_NGO);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// NODE REGISTRATION TESTS
// ===========================

void test_node_registration(void) {
    g_globalConservationNetwork->initialize("TEST_HUB", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    GlobalNetworkNode testNode;
    testNode.nodeId = "TEST_REMOTE_001";
    testNode.location = "Test Location";
    testNode.region = REGION_AFRICA;
    testNode.latitude = -2.1534f;
    testNode.longitude = 34.6857f;
    testNode.country = "Test Country";
    testNode.orgType = ORG_COMMUNITY;
    testNode.isActive = true;
    
    bool result = g_globalConservationNetwork->registerNode(testNode);
    TEST_ASSERT_TRUE(result);
    
    // Verify node is in active nodes list
    std::vector<GlobalNetworkNode> activeNodes = g_globalConservationNetwork->getActiveNodes();
    TEST_ASSERT_GREATER_THAN(0, activeNodes.size());
}

void test_node_status_update(void) {
    g_globalConservationNetwork->initialize("TEST_HUB", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    // Register a node
    GlobalNetworkNode testNode;
    testNode.nodeId = "TEST_STATUS_NODE";
    testNode.region = REGION_AFRICA;
    testNode.isActive = true;
    g_globalConservationNetwork->registerNode(testNode);
    
    // Update status to inactive
    bool result = g_globalConservationNetwork->updateNodeStatus("TEST_STATUS_NODE", false);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// THREAT INTELLIGENCE TESTS
// ===========================

void test_threat_intelligence_sharing(void) {
    g_globalConservationNetwork->initialize("TEST_NODE_THREAT", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    ThreatIntelligence threat;
    threat.threatId = "THREAT_TEST_001";
    threat.threatType = THREAT_POACHING;
    threat.species = "Test Species";
    threat.affectedRegion = REGION_AFRICA;
    threat.severity = 0.85f;
    threat.requiresImmediate = true;
    threat.detectionTime = 1000000;
    threat.sourceNodeId = "TEST_NODE_THREAT";
    
    bool result = g_globalConservationNetwork->shareThreatIntelligence(threat);
    TEST_ASSERT_TRUE(result);
}

void test_regional_threat_retrieval(void) {
    g_globalConservationNetwork->initialize("TEST_NODE_REGIONAL", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    // Add a threat
    ThreatIntelligence threat;
    threat.threatId = "THREAT_REGIONAL_001";
    threat.threatType = THREAT_HABITAT_LOSS;
    threat.species = "Test Species";
    threat.affectedRegion = REGION_AFRICA;
    threat.severity = 0.65f;
    threat.detectionTime = 1000000;
    g_globalConservationNetwork->shareThreatIntelligence(threat);
    
    // Retrieve regional threats
    std::vector<ThreatIntelligence> threats = g_globalConservationNetwork->getRegionalThreats(REGION_AFRICA);
    TEST_ASSERT_GREATER_THAN(0, threats.size());
}

void test_quick_threat_alert_function(void) {
    initializeGlobalConservationNetwork("TEST_QUICK_NODE", REGION_AFRICA);
    
    bool result = shareGlobalThreatAlert(THREAT_POACHING, "Loxodonta africana", 0.9f);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// SPECIES TRACKING TESTS
// ===========================

void test_migratory_species_registration(void) {
    g_globalConservationNetwork->initialize("TEST_SPECIES_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    CrossBorderSpecies species;
    species.speciesId = "SPECIES_TEST_001";
    species.scientificName = "Test species";
    species.commonName = "Test Animal";
    species.migratoryStatus = MIGRATORY_INTERNATIONAL;
    species.conservationStatus = "EN";
    species.isEndangered = true;
    
    bool result = g_globalConservationNetwork->registerMigratorySpecies(species);
    TEST_ASSERT_TRUE(result);
    
    // Verify species is tracked
    std::vector<CrossBorderSpecies> tracked = g_globalConservationNetwork->getTrackedSpecies();
    TEST_ASSERT_GREATER_THAN(0, tracked.size());
}

void test_species_location_update(void) {
    g_globalConservationNetwork->initialize("TEST_LOCATION_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    // Register species
    CrossBorderSpecies species;
    species.speciesId = "LOCATION_TEST_001";
    species.scientificName = "Test species";
    species.migratoryStatus = MIGRATORY_REGIONAL;
    g_globalConservationNetwork->registerMigratorySpecies(species);
    
    // Update location
    bool result = g_globalConservationNetwork->updateSpeciesLocation(
        "LOCATION_TEST_001",
        -2.1534f,
        34.6857f,
        1000000
    );
    TEST_ASSERT_TRUE(result);
}

void test_quick_species_tracking_function(void) {
    initializeGlobalConservationNetwork("TEST_TRACK_NODE", REGION_AFRICA);
    
    // Register species first
    CrossBorderSpecies species;
    species.speciesId = "QUICK_TRACK_001";
    species.scientificName = "Test species";
    species.migratoryStatus = MIGRATORY_LOCAL;
    g_globalConservationNetwork->registerMigratorySpecies(species);
    
    // Use quick function
    bool result = trackMigratorySpecies("QUICK_TRACK_001", -1.5f, 35.0f);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// TREATY COMPLIANCE TESTS
// ===========================

void test_treaty_compliance_enablement(void) {
    g_globalConservationNetwork->initialize("TEST_TREATY_NODE", REGION_AFRICA);
    
    bool result = g_globalConservationNetwork->enableTreatyCompliance(TREATY_CITES);
    TEST_ASSERT_TRUE(result);
}

void test_treaty_compliance_reporting(void) {
    g_globalConservationNetwork->initialize("TEST_REPORT_NODE", REGION_AFRICA);
    g_globalConservationNetwork->enableTreatyCompliance(TREATY_CMS);
    
    JsonDocument report;
    report["species_monitored"] = 10;
    report["compliance_status"] = "compliant";
    
    bool result = g_globalConservationNetwork->reportTreatyCompliance(TREATY_CMS, report);
    TEST_ASSERT_TRUE(result);
}

void test_quick_treaty_reporting_function(void) {
    initializeGlobalConservationNetwork("TEST_QUICK_TREATY", REGION_ASIA);
    
    JsonDocument report;
    report["test_field"] = "test_value";
    
    bool result = reportToInternationalTreaty(TREATY_CBD, report);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// DATA SHARING TESTS
// ===========================

void test_conservation_data_sharing(void) {
    g_globalConservationNetwork->initialize("TEST_DATA_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    GlobalConservationData data;
    data.dataId = "DATA_TEST_001";
    data.nodeId = "TEST_DATA_NODE";
    data.region = REGION_AFRICA;
    data.dataType = "wildlife_detection";
    data.privacyLevel = PRIVACY_RESEARCH;
    data.timestamp = 1000000;
    
    JsonDocument detectionData;
    detectionData["species"] = "Test Species";
    detectionData["confidence"] = 0.95;
    data.data = detectionData;
    
    bool result = g_globalConservationNetwork->shareConservationData(data);
    TEST_ASSERT_TRUE(result);
}

void test_data_access_control(void) {
    g_globalConservationNetwork->initialize("TEST_ACCESS_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    // Share data
    GlobalConservationData data;
    data.dataId = "ACCESS_TEST_001";
    data.nodeId = "TEST_ACCESS_NODE";
    data.region = REGION_AFRICA;
    data.privacyLevel = PRIVACY_INSTITUTION;
    g_globalConservationNetwork->shareConservationData(data);
    
    // Grant access
    bool result = g_globalConservationNetwork->grantDataAccess("ACCESS_TEST_001", "TEST_ORG");
    TEST_ASSERT_TRUE(result);
}

// ===========================
// BLOCKCHAIN VERIFICATION TESTS
// ===========================

void test_conservation_outcome_recording(void) {
    g_globalConservationNetwork->initialize("TEST_BLOCKCHAIN_NODE", REGION_AFRICA);
    
    JsonDocument outcome;
    outcome["project"] = "Test Conservation Project";
    outcome["species_protected"] = 100;
    outcome["habitat_secured"] = 1000;
    
    bool result = g_globalConservationNetwork->recordConservationOutcome("TEST_PROJECT_001", outcome);
    TEST_ASSERT_TRUE(result);
}

void test_conservation_impact_verification(void) {
    g_globalConservationNetwork->initialize("TEST_VERIFY_NODE", REGION_AFRICA);
    
    // Record outcome first
    JsonDocument outcome;
    outcome["test_data"] = "test_value";
    g_globalConservationNetwork->recordConservationOutcome("VERIFY_PROJECT_001", outcome);
    
    // Verify impact
    String verificationHash;
    bool result = g_globalConservationNetwork->verifyConservationImpact("VERIFY_PROJECT_001", verificationHash);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// EMERGENCY RESPONSE TESTS
// ===========================

void test_emergency_declaration(void) {
    g_globalConservationNetwork->initialize("TEST_EMERGENCY_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    bool result = g_globalConservationNetwork->declareEmergency(
        THREAT_DISEASE_OUTBREAK,
        REGION_AFRICA,
        "Test emergency declaration"
    );
    TEST_ASSERT_TRUE(result);
}

void test_emergency_assistance_request(void) {
    g_globalConservationNetwork->initialize("TEST_ASSIST_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    JsonDocument request;
    request["urgency"] = "critical";
    request["resources_needed"] = "test_resources";
    
    bool result = g_globalConservationNetwork->requestEmergencyAssistance("TEST_ASSISTANCE", request);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// COLLABORATION TESTS
// ===========================

void test_international_project_initiation(void) {
    g_globalConservationNetwork->initialize("TEST_PROJECT_NODE", REGION_AFRICA);
    
    std::vector<String> participants = {"ORG_A", "ORG_B", "ORG_C"};
    
    bool result = g_globalConservationNetwork->initiateInternationalProject(
        "TEST_COLLAB_PROJECT",
        participants
    );
    TEST_ASSERT_TRUE(result);
}

void test_research_findings_sharing(void) {
    g_globalConservationNetwork->initialize("TEST_RESEARCH_NODE", REGION_EUROPE);
    
    JsonDocument findings;
    findings["title"] = "Test Research";
    findings["methodology"] = "Test Method";
    findings["key_findings"] = "Test Results";
    
    bool result = g_globalConservationNetwork->shareResearchFindings("TEST_RESEARCH_001", findings);
    TEST_ASSERT_TRUE(result);
}

// ===========================
// STATISTICS TESTS
// ===========================

void test_network_statistics_retrieval(void) {
    g_globalConservationNetwork->initialize("TEST_STATS_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    GlobalNetworkStats stats = g_globalConservationNetwork->getNetworkStatistics();
    
    TEST_ASSERT_GREATER_OR_EQUAL(0, stats.totalNodes);
    TEST_ASSERT_GREATER_OR_EQUAL(0, stats.activeNodes);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.5f, stats.networkHealth); // Should be between 0 and 1
}

void test_quick_statistics_function(void) {
    initializeGlobalConservationNetwork("TEST_QUICK_STATS", REGION_ASIA);
    
    GlobalNetworkStats stats = getGlobalNetworkStats();
    TEST_ASSERT_GREATER_OR_EQUAL(0, stats.totalNodes);
}

void test_network_health_monitoring(void) {
    g_globalConservationNetwork->initialize("TEST_HEALTH_NODE", REGION_AFRICA);
    g_globalConservationNetwork->connectToGlobalNetwork();
    
    float health = g_globalConservationNetwork->getGlobalNetworkHealth();
    
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, health);
    TEST_ASSERT_LESS_OR_EQUAL(1.0f, health);
}

// ===========================
// CLEANUP TESTS
// ===========================

void test_global_network_cleanup(void) {
    initializeGlobalConservationNetwork("TEST_CLEANUP_NODE", REGION_AFRICA);
    
    cleanupGlobalConservationNetwork();
    
    // After cleanup, global instance should be cleaned but not null
    // (as it's recreated in setUp)
    TEST_ASSERT_NOT_NULL(g_globalConservationNetwork);
}

// ===========================
// RUN ALL TESTS
// ===========================

void runGlobalConservationNetworkTests(void) {
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_global_network_initialization);
    RUN_TEST(test_global_network_connection);
    RUN_TEST(test_organization_type_configuration);
    
    // Node registration tests
    RUN_TEST(test_node_registration);
    RUN_TEST(test_node_status_update);
    
    // Threat intelligence tests
    RUN_TEST(test_threat_intelligence_sharing);
    RUN_TEST(test_regional_threat_retrieval);
    RUN_TEST(test_quick_threat_alert_function);
    
    // Species tracking tests
    RUN_TEST(test_migratory_species_registration);
    RUN_TEST(test_species_location_update);
    RUN_TEST(test_quick_species_tracking_function);
    
    // Treaty compliance tests
    RUN_TEST(test_treaty_compliance_enablement);
    RUN_TEST(test_treaty_compliance_reporting);
    RUN_TEST(test_quick_treaty_reporting_function);
    
    // Data sharing tests
    RUN_TEST(test_conservation_data_sharing);
    RUN_TEST(test_data_access_control);
    
    // Blockchain verification tests
    RUN_TEST(test_conservation_outcome_recording);
    RUN_TEST(test_conservation_impact_verification);
    
    // Emergency response tests
    RUN_TEST(test_emergency_declaration);
    RUN_TEST(test_emergency_assistance_request);
    
    // Collaboration tests
    RUN_TEST(test_international_project_initiation);
    RUN_TEST(test_research_findings_sharing);
    
    // Statistics tests
    RUN_TEST(test_network_statistics_retrieval);
    RUN_TEST(test_quick_statistics_function);
    RUN_TEST(test_network_health_monitoring);
    
    // Cleanup tests
    RUN_TEST(test_global_network_cleanup);
    
    UNITY_END();
}

// ===========================
// MAIN FUNCTION
// ===========================

#ifdef ARDUINO
void setup() {
    delay(2000); // Wait for serial
    runGlobalConservationNetworkTests();
}

void loop() {
    // Tests run once in setup
}
#else
int main(int argc, char **argv) {
    runGlobalConservationNetworkTests();
    return 0;
}
#endif
