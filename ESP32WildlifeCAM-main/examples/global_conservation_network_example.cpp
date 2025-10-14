/**
 * @file global_conservation_network_example.cpp
 * @brief Comprehensive example demonstrating Global Conservation Network Platform
 * 
 * This example shows how to:
 * - Initialize and connect to the global conservation network
 * - Share threat intelligence internationally
 * - Track cross-border migratory species
 * - Report treaty compliance
 * - Coordinate emergency responses
 * - Verify conservation impact with blockchain
 * - Participate in international research projects
 */

#include <Arduino.h>
#include "../firmware/src/production/enterprise/cloud/global_conservation_network.h"
#include "../firmware/src/production/enterprise/cloud/research_collaboration_platform.h"
#include "../firmware/src/lora_wildlife_alerts.h"

// Configuration
#define NODE_ID "SERENGETI_MONITORING_001"
#define DEPLOYMENT_REGION REGION_AFRICA
#define ORGANIZATION_TYPE ORG_GOVERNMENT

// Global network instance (extern from header)
// GlobalConservationNetwork* g_globalConservationNetwork;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== Global Conservation Network Example ===\n");
    
    // ================================
    // 1. INITIALIZE GLOBAL NETWORK
    // ================================
    Serial.println("1. Initializing Global Conservation Network...");
    
    if (!initializeGlobalConservationNetwork(NODE_ID, DEPLOYMENT_REGION)) {
        Serial.println("ERROR: Failed to initialize global network");
        return;
    }
    
    Serial.println("   ✓ Connected to global conservation network");
    Serial.printf("   ✓ Node ID: %s\n", NODE_ID);
    Serial.printf("   ✓ Region: %s\n", "Africa");
    
    // Configure organization type
    g_globalConservationNetwork->setOrganizationType(ORGANIZATION_TYPE);
    Serial.println("   ✓ Organization type configured");
    
    // ================================
    // 2. ENABLE TREATY COMPLIANCE
    // ================================
    Serial.println("\n2. Enabling International Treaty Compliance...");
    
    g_globalConservationNetwork->enableTreatyCompliance(TREATY_CITES);
    Serial.println("   ✓ CITES compliance enabled");
    
    g_globalConservationNetwork->enableTreatyCompliance(TREATY_CMS);
    Serial.println("   ✓ CMS (Migratory Species) compliance enabled");
    
    g_globalConservationNetwork->enableTreatyCompliance(TREATY_CBD);
    Serial.println("   ✓ CBD (Biological Diversity) compliance enabled");
    
    // ================================
    // 3. REGISTER MIGRATORY SPECIES
    // ================================
    Serial.println("\n3. Registering Cross-Border Migratory Species...");
    
    // Example 1: African Elephant
    CrossBorderSpecies elephant;
    elephant.speciesId = "ELEPHANT_TRACKING_001";
    elephant.scientificName = "Loxodonta africana";
    elephant.commonName = "African Elephant";
    elephant.migratoryStatus = MIGRATORY_INTERNATIONAL;
    elephant.habitatRegions = {REGION_AFRICA};
    elephant.protectedCountries = {"Tanzania", "Kenya", "Uganda", "Zimbabwe"};
    elephant.applicableTreaties = {TREATY_CITES, TREATY_CMS, TREATY_CBD};
    elephant.conservationStatus = "EN"; // Endangered
    elephant.isEndangered = true;
    elephant.lastSighting = millis();
    elephant.lastLocation = "-2.1534, 34.6857"; // Serengeti
    
    if (g_globalConservationNetwork->registerMigratorySpecies(elephant)) {
        Serial.println("   ✓ Registered: African Elephant (Endangered)");
        Serial.println("     - Cross-border tracking enabled");
        Serial.println("     - CITES, CMS, CBD treaties applicable");
    }
    
    // Example 2: Wildebeest Migration
    CrossBorderSpecies wildebeest;
    wildebeest.speciesId = "WILDEBEEST_HERD_ALPHA";
    wildebeest.scientificName = "Connochaetes taurinus";
    wildebeest.commonName = "Blue Wildebeest";
    wildebeest.migratoryStatus = MIGRATORY_INTERNATIONAL;
    wildebeest.habitatRegions = {REGION_AFRICA};
    wildebeest.protectedCountries = {"Tanzania", "Kenya"};
    wildebeest.applicableTreaties = {TREATY_CMS, TREATY_CBD};
    wildebeest.conservationStatus = "LC"; // Least Concern
    wildebeest.isEndangered = false;
    
    if (g_globalConservationNetwork->registerMigratorySpecies(wildebeest)) {
        Serial.println("   ✓ Registered: Blue Wildebeest Migration");
        Serial.println("     - Annual Serengeti-Maasai Mara migration route");
    }
    
    // ================================
    // 4. SHARE THREAT INTELLIGENCE
    // ================================
    Serial.println("\n4. Sharing International Threat Intelligence...");
    
    // Example 1: Poaching threat
    ThreatIntelligence poachingThreat;
    poachingThreat.threatId = "THREAT_POACH_2024_001";
    poachingThreat.threatType = THREAT_POACHING;
    poachingThreat.species = "Loxodonta africana";
    poachingThreat.affectedRegion = REGION_AFRICA;
    poachingThreat.latitude = -2.3333;
    poachingThreat.longitude = 34.8333;
    poachingThreat.severity = 0.9f; // Critical
    poachingThreat.requiresImmediate = true;
    poachingThreat.detectionTime = millis();
    poachingThreat.sourceNodeId = NODE_ID;
    poachingThreat.description = "Suspected poaching activity detected near water hole";
    poachingThreat.affectedCountries = {"Tanzania", "Kenya"};
    
    if (g_globalConservationNetwork->shareThreatIntelligence(poachingThreat)) {
        Serial.println("   ✓ CRITICAL: Poaching threat shared globally");
        Serial.println("     - Severity: 90%");
        Serial.println("     - Species: African Elephant");
        Serial.println("     - Alert propagated to regional nodes");
    }
    
    // Example 2: Habitat loss threat
    ThreatIntelligence habitatThreat;
    habitatThreat.threatId = "THREAT_HABITAT_2024_001";
    habitatThreat.threatType = THREAT_HABITAT_LOSS;
    habitatThreat.species = "Multiple species";
    habitatThreat.affectedRegion = REGION_AFRICA;
    habitatThreat.severity = 0.65f;
    habitatThreat.requiresImmediate = false;
    habitatThreat.detectionTime = millis();
    habitatThreat.description = "Deforestation detected in corridor area";
    
    if (g_globalConservationNetwork->shareThreatIntelligence(habitatThreat)) {
        Serial.println("   ✓ Habitat loss threat shared");
        Serial.println("     - Severity: 65%");
        Serial.println("     - Requires monitoring and intervention");
    }
    
    // Quick function alternative
    if (shareGlobalThreatAlert(THREAT_ILLEGAL_TRADE, "Panthera leo", 0.75f)) {
        Serial.println("   ✓ Illegal trade alert sent (Lion)");
    }
    
    // ================================
    // 5. CONSERVATION DATA SHARING
    // ================================
    Serial.println("\n5. Sharing Conservation Data Internationally...");
    
    GlobalConservationData detectionData;
    detectionData.dataId = "DATA_DETECT_" + String(millis());
    detectionData.nodeId = NODE_ID;
    detectionData.region = DEPLOYMENT_REGION;
    detectionData.timestamp = millis();
    detectionData.dataType = "wildlife_detection";
    detectionData.privacyLevel = PRIVACY_RESEARCH;
    
    // Create detection data JSON
    JsonDocument detectDoc;
    detectDoc["species"] = "Panthera leo";
    detectDoc["common_name"] = "Lion";
    detectDoc["confidence"] = 0.95;
    detectDoc["count"] = 3;
    detectDoc["behavior"] = "hunting";
    detectDoc["location"]["lat"] = -2.1534;
    detectDoc["location"]["lon"] = 34.6857;
    detectDoc["timestamp"] = millis();
    
    detectionData.data = detectDoc;
    detectionData.sharedWith = {"IUCN", "WWF_TANZANIA", "TAWIRI"};
    
    if (g_globalConservationNetwork->shareConservationData(detectionData)) {
        Serial.println("   ✓ Wildlife detection data shared");
        Serial.println("     - Species: Lion (Panthera leo)");
        Serial.println("     - Shared with: IUCN, WWF, Research Institute");
        Serial.println("     - Blockchain verification enabled");
    }
    
    // ================================
    // 6. TREATY COMPLIANCE REPORTING
    // ================================
    Serial.println("\n6. Reporting International Treaty Compliance...");
    
    // CITES compliance report
    JsonDocument citesReport;
    citesReport["treaty"] = "CITES";
    citesReport["reporting_period"] = "2024-Q1";
    citesReport["node_id"] = NODE_ID;
    citesReport["species_monitored"] = 15;
    citesReport["endangered_species_detected"] = 3;
    citesReport["illegal_activity_incidents"] = 2;
    citesReport["enforcement_actions"] = 2;
    citesReport["compliance_status"] = "compliant";
    citesReport["recommendations"] = "Increase patrol frequency in sector B";
    
    if (g_globalConservationNetwork->reportTreatyCompliance(TREATY_CITES, citesReport)) {
        Serial.println("   ✓ CITES compliance report submitted");
        Serial.println("     - Reporting period: Q1 2024");
        Serial.println("     - Status: Compliant");
    }
    
    // Alternative quick function
    JsonDocument cmsReport;
    cmsReport["migratory_species_tracked"] = 5;
    cmsReport["migration_routes_monitored"] = 2;
    
    if (reportToInternationalTreaty(TREATY_CMS, cmsReport)) {
        Serial.println("   ✓ CMS treaty report submitted");
    }
    
    // ================================
    // 7. BLOCKCHAIN IMPACT VERIFICATION
    // ================================
    Serial.println("\n7. Recording Conservation Impact (Blockchain)...");
    
    JsonDocument conservationOutcome;
    conservationOutcome["project_id"] = "ELEPHANT_PROTECTION_2024";
    conservationOutcome["project_name"] = "Serengeti Elephant Protection Initiative";
    conservationOutcome["start_date"] = "2024-01-01";
    conservationOutcome["reporting_date"] = "2024-03-31";
    conservationOutcome["outcomes"]["elephants_protected"] = 250;
    conservationOutcome["outcomes"]["poaching_incidents_prevented"] = 12;
    conservationOutcome["outcomes"]["habitat_secured_hectares"] = 5000;
    conservationOutcome["outcomes"]["community_rangers_trained"] = 25;
    conservationOutcome["funding"]["total_usd"] = 500000;
    conservationOutcome["funding"]["source"] = "Global Environment Facility";
    conservationOutcome["verification_method"] = "camera_trap_surveys";
    conservationOutcome["verified_by"] = "TAWIRI";
    
    if (g_globalConservationNetwork->recordConservationOutcome(
        "ELEPHANT_PROTECTION_2024",
        conservationOutcome
    )) {
        Serial.println("   ✓ Conservation outcomes recorded to blockchain");
        Serial.println("     - Project: Serengeti Elephant Protection");
        Serial.println("     - 250 elephants protected");
        Serial.println("     - 12 poaching incidents prevented");
        Serial.println("     - Tamper-proof verification enabled");
    }
    
    // Verify conservation impact
    String verificationHash;
    if (g_globalConservationNetwork->verifyConservationImpact(
        "ELEPHANT_PROTECTION_2024",
        verificationHash
    )) {
        Serial.println("   ✓ Conservation impact verified");
        Serial.printf("     - Verification hash: %s\n", verificationHash.c_str());
    }
    
    // ================================
    // 8. EMERGENCY RESPONSE
    // ================================
    Serial.println("\n8. Demonstrating Emergency Response Coordination...");
    
    // Declare emergency
    if (g_globalConservationNetwork->declareEmergency(
        THREAT_DISEASE_OUTBREAK,
        REGION_AFRICA,
        "Suspected anthrax outbreak affecting wildebeest population"
    )) {
        Serial.println("   ✓ EMERGENCY: Disease outbreak declared");
        Serial.println("     - Type: Disease outbreak (Anthrax)");
        Serial.println("     - Region: Africa");
        Serial.println("     - Alert propagated to all regional nodes");
    }
    
    // Request emergency assistance
    JsonDocument assistanceRequest;
    assistanceRequest["emergency_type"] = "disease_outbreak";
    assistanceRequest["urgency"] = "critical";
    assistanceRequest["location"] = "Serengeti National Park";
    assistanceRequest["species_affected"] = "Connochaetes taurinus";
    assistanceRequest["estimated_impact"] = "500+ animals";
    assistanceRequest["resources_needed"]["veterinary_teams"] = 3;
    assistanceRequest["resources_needed"]["vaccines"] = "1000 doses";
    assistanceRequest["resources_needed"]["medical_equipment"] = true;
    
    if (g_globalConservationNetwork->requestEmergencyAssistance(
        "MEDICAL_ASSISTANCE",
        assistanceRequest
    )) {
        Serial.println("   ✓ Emergency assistance request sent");
        Serial.println("     - Veterinary teams requested");
        Serial.println("     - Vaccines and medical equipment needed");
    }
    
    // ================================
    // 9. INTERNATIONAL COLLABORATION
    // ================================
    Serial.println("\n9. Initiating International Research Collaboration...");
    
    std::vector<String> projectParticipants = {
        "WWF_INTERNATIONAL",
        "UNIVERSITY_OF_OXFORD",
        "TAWIRI",
        "KENYA_WILDLIFE_SERVICE",
        "SMITHSONIAN_CONSERVATION"
    };
    
    if (g_globalConservationNetwork->initiateInternationalProject(
        "ELEPHANT_MIGRATION_STUDY_2024",
        projectParticipants
    )) {
        Serial.println("   ✓ International research project initiated");
        Serial.println("     - Project: Elephant Migration Study 2024");
        Serial.println("     - Participants: 5 international organizations");
        Serial.println("     - Collaborative data sharing enabled");
    }
    
    // Share research findings
    JsonDocument findings;
    findings["project_id"] = "ELEPHANT_MIGRATION_STUDY_2024";
    findings["title"] = "Novel Migration Corridor Discovered";
    findings["date"] = "2024-03-15";
    findings["methodology"] = "GPS collar tracking + camera trap surveys";
    findings["sample_size"] = 150;
    findings["key_findings"]["corridor_identified"] = true;
    findings["key_findings"]["corridor_length_km"] = 45;
    findings["key_findings"]["usage_frequency"] = "seasonal";
    findings["conservation_recommendations"]["action"] = "establish_protected_corridor";
    findings["conservation_recommendations"]["priority"] = "high";
    
    if (g_globalConservationNetwork->shareResearchFindings(
        "ELEPHANT_MIGRATION_STUDY_2024",
        findings
    )) {
        Serial.println("   ✓ Research findings shared globally");
        Serial.println("     - Novel migration corridor discovered");
        Serial.println("     - Recommendation: Establish protected corridor");
    }
    
    // ================================
    // 10. NETWORK STATISTICS
    // ================================
    Serial.println("\n10. Global Network Statistics...");
    
    GlobalNetworkStats stats = getGlobalNetworkStats();
    
    Serial.println("   Network Status:");
    Serial.printf("   - Total Nodes: %d\n", stats.totalNodes);
    Serial.printf("   - Active Nodes: %d\n", stats.activeNodes);
    Serial.printf("   - Regions Connected: %d\n", stats.regionsConnected);
    Serial.printf("   - Threat Alerts Shared: %d\n", stats.threatAlertsShared);
    Serial.printf("   - Species Tracked: %d\n", stats.speciesTracked);
    Serial.printf("   - Data Packages Exchanged: %d\n", stats.dataPackagesExchanged);
    Serial.printf("   - Collaborative Projects: %d\n", stats.collaborativeProjects);
    Serial.printf("   - Treaty Reports: %d\n", stats.treatyComplianceReports);
    Serial.printf("   - Network Health: %.2f%%\n", stats.networkHealth * 100);
    
    if (g_globalConservationNetwork->isConnectedToGlobalNetwork()) {
        Serial.println("\n   ✓ CONNECTED to global conservation network");
    }
    
    // ================================
    // 11. CROSS-BORDER SPECIES UPDATE
    // ================================
    Serial.println("\n11. Updating Cross-Border Species Location...");
    
    // Simulate GPS update for elephant
    float elephantLat = -1.8500;
    float elephantLon = 34.5000;
    
    if (trackMigratorySpecies("ELEPHANT_TRACKING_001", elephantLat, elephantLon)) {
        Serial.println("   ✓ Elephant location updated");
        Serial.printf("     - Position: %.4f, %.4f\n", elephantLat, elephantLon);
        Serial.println("     - Crossing into Kenya region");
        Serial.println("     - Handoff coordination initiated");
    }
    
    // Request handoff to Kenya monitoring station
    if (g_globalConservationNetwork->requestSpeciesHandoff(
        "ELEPHANT_TRACKING_001",
        "MAASAI_MARA_MONITORING_001"
    )) {
        Serial.println("   ✓ Tracking handoff requested");
        Serial.println("     - Target: Maasai Mara monitoring station");
        Serial.println("     - Continuity of monitoring ensured");
    }
    
    Serial.println("\n=== Global Conservation Network Example Complete ===");
    Serial.println("\nThis node is now actively participating in the worldwide");
    Serial.println("conservation network, contributing to international efforts");
    Serial.println("to protect wildlife and preserve biodiversity across borders.\n");
}

void loop() {
    // In a real deployment, this would:
    // - Continuously monitor for wildlife
    // - Update species locations
    // - Share threat intelligence in real-time
    // - Respond to global network events
    // - Maintain network connectivity
    // - Sync with regional and global nodes
    
    static uint32_t lastUpdate = 0;
    
    if (millis() - lastUpdate > 60000) { // Every minute
        // Check network health
        GlobalNetworkStats stats = getGlobalNetworkStats();
        
        if (stats.networkHealth < 0.5f) {
            Serial.println("WARNING: Network health degraded");
        }
        
        // Could update species locations here
        // trackMigratorySpecies(...);
        
        lastUpdate = millis();
    }
    
    delay(1000);
}
