/**
 * Threat Intelligence Sharing System
 * 
 * Real-time threat detection, alert propagation, and coordinated
 * response system for wildlife conservation across international
 * boundaries. Enables rapid response to poaching, trafficking,
 * disease outbreaks, and environmental disasters.
 * 
 * Features:
 * - Automated threat detection and classification
 * - Real-time alert propagation across networks
 * - Poaching incident reporting and tracking
 * - Wildlife trafficking pattern analysis
 * - Disease outbreak monitoring
 * - Environmental disaster coordination
 * - Law enforcement integration
 * 
 * @file threat_intelligence_sharing.h
 * @author ESP32WildlifeCAM Team
 * @date 2024-10-14
 */

#ifndef THREAT_INTELLIGENCE_SHARING_H
#define THREAT_INTELLIGENCE_SHARING_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "global_conservation_network.h"

// Threat types
enum ThreatType {
    THREAT_POACHING,              // Illegal hunting activity
    THREAT_TRAFFICKING,           // Wildlife trafficking detected
    THREAT_HABITAT_DESTRUCTION,   // Illegal logging, mining, etc.
    THREAT_DISEASE_OUTBREAK,      // Disease detected in population
    THREAT_HUMAN_WILDLIFE_CONFLICT, // Conflict situation
    THREAT_ENVIRONMENTAL_DISASTER, // Fire, flood, pollution
    THREAT_CLIMATE_IMPACT,        // Climate change effects
    THREAT_INVASIVE_SPECIES,      // Invasive species detected
    THREAT_UNKNOWN               // Unclassified threat
};

// Threat severity levels
enum ThreatSeverity {
    SEVERITY_LOW,         // Monitor situation
    SEVERITY_MODERATE,    // Requires attention
    SEVERITY_HIGH,        // Urgent response needed
    SEVERITY_CRITICAL    // Emergency response required
};

// Alert status
enum AlertStatus {
    ALERT_NEW,           // Newly detected
    ALERT_INVESTIGATING, // Under investigation
    ALERT_CONFIRMED,     // Confirmed threat
    ALERT_RESPONDING,    // Response in progress
    ALERT_RESOLVED,      // Threat resolved
    ALERT_FALSE_ALARM   // False positive
};

// Response action types
enum ResponseAction {
    ACTION_MONITOR,           // Continue monitoring
    ACTION_INVESTIGATE,       // Deploy investigation team
    ACTION_LAW_ENFORCEMENT,   // Alert law enforcement
    ACTION_MEDICAL,           // Medical/veterinary response
    ACTION_EVACUATION,        // Evacuate area
    ACTION_CONTAINMENT,       // Contain spread
    ACTION_PUBLIC_ALERT      // Public warning
};

// Threat intelligence data
struct ThreatIntelligence {
    String threatId;
    ThreatType type;
    ThreatSeverity severity;
    AlertStatus status;
    uint32_t detectionTime;
    uint32_t lastUpdated;
    
    // Location information
    float latitude;
    float longitude;
    GlobalRegion region;
    String location;
    float affectedRadius;  // km
    
    // Threat details
    String description;
    String speciesAffected;
    String evidence;
    String imageHash;
    float confidence;
    
    // Response information
    std::vector<ResponseAction> recommendedActions;
    std::vector<String> respondingOrganizations;
    String assignedTo;
    
    // Intelligence sharing
    bool shareInternational;
    std::vector<String> notifiedAgencies;
    std::vector<String> relatedThreats;
    
    // Impact assessment
    uint32_t estimatedAffectedAnimals;
    float habitatAreaAffected;  // sq km
    String economicImpact;
    
    ThreatIntelligence() :
        threatId(""), type(THREAT_UNKNOWN), severity(SEVERITY_LOW),
        status(ALERT_NEW), detectionTime(0), lastUpdated(0),
        latitude(0.0), longitude(0.0), region(REGION_NORTH_AMERICA),
        location(""), affectedRadius(0.0), description(""),
        speciesAffected(""), evidence(""), imageHash(""),
        confidence(0.0), shareInternational(false),
        estimatedAffectedAnimals(0), habitatAreaAffected(0.0),
        economicImpact("") {}
};

// Poaching incident report
struct PoachingIncident {
    String incidentId;
    uint32_t timestamp;
    float latitude;
    float longitude;
    String speciesTargeted;
    uint32_t animalsAffected;
    String methodUsed;
    String suspectDescription;
    String vehicleDescription;
    std::vector<String> evidence;
    bool lawEnforcementNotified;
    String caseNumber;
    String reportedBy;
    
    PoachingIncident() :
        incidentId(""), timestamp(0), latitude(0.0), longitude(0.0),
        speciesTargeted(""), animalsAffected(0), methodUsed(""),
        suspectDescription(""), vehicleDescription(""),
        lawEnforcementNotified(false), caseNumber(""), reportedBy("") {}
};

// Disease outbreak data
struct DiseaseOutbreak {
    String outbreakId;
    String diseaseName;
    String pathogenType;
    uint32_t firstDetection;
    uint32_t lastCase;
    std::vector<std::pair<float, float>> affectedLocations;
    std::vector<String> speciesAffected;
    uint32_t confirmedCases;
    uint32_t suspectedCases;
    uint32_t deaths;
    float mortalityRate;
    bool zoonotic;  // Can transmit to humans
    String containmentStatus;
    std::vector<String> controlMeasures;
    
    DiseaseOutbreak() :
        outbreakId(""), diseaseName(""), pathogenType(""),
        firstDetection(0), lastCase(0), confirmedCases(0),
        suspectedCases(0), deaths(0), mortalityRate(0.0),
        zoonotic(false), containmentStatus("") {}
};

// Trafficking pattern analysis
struct TraffickingPattern {
    String patternId;
    std::vector<String> speciesInvolved;
    std::vector<std::pair<float, float>> routePoints;
    std::vector<String> suspectedOrganizations;
    uint32_t incidentsLinked;
    uint32_t firstObserved;
    uint32_t lastObserved;
    String destinationMarket;
    float estimatedValue;
    String patternDescription;
    float confidence;
    
    TraffickingPattern() :
        patternId(""), incidentsLinked(0), firstObserved(0),
        lastObserved(0), destinationMarket(""), estimatedValue(0.0),
        patternDescription(""), confidence(0.0) {}
};

// Alert subscription
struct AlertSubscription {
    String subscriberId;
    std::vector<ThreatType> threatTypes;
    std::vector<GlobalRegion> regions;
    ThreatSeverity minSeverity;
    std::vector<String> speciesOfInterest;
    String notificationEmail;
    String notificationPhone;
    bool enabled;
    
    AlertSubscription() :
        subscriberId(""), minSeverity(SEVERITY_MODERATE),
        notificationEmail(""), notificationPhone(""), enabled(true) {}
};

/**
 * Threat Intelligence Sharing System Class
 * 
 * Manages detection, reporting, and coordinated response to
 * conservation threats across international boundaries.
 */
class ThreatIntelligenceSharing {
public:
    ThreatIntelligenceSharing();
    ~ThreatIntelligenceSharing();
    
    // Initialization and configuration
    bool initialize(const String& organizationId);
    bool configureAlertFilters(const AlertSubscription& subscription);
    
    // Threat detection and reporting
    bool detectThreat(const uint8_t* imageData, size_t imageSize,
                     float lat, float lon, ThreatIntelligence& threat);
    bool reportThreat(const ThreatIntelligence& threat);
    bool updateThreatStatus(const String& threatId, AlertStatus status);
    bool escalateThreat(const String& threatId, ThreatSeverity newSeverity);
    
    // Poaching incident management
    bool reportPoachingIncident(const PoachingIncident& incident);
    bool linkPoachingIncidents(const std::vector<String>& incidentIds);
    bool notifyLawEnforcement(const String& incidentId,
                             const String& agencyContact);
    std::vector<PoachingIncident> getPoachingIncidents(GlobalRegion region,
                                                        uint32_t since);
    
    // Wildlife trafficking analysis
    bool reportTraffickingActivity(const String& speciesName,
                                   float lat, float lon,
                                   const String& details);
    bool analyzeTraffickingPatterns(std::vector<TraffickingPattern>& patterns);
    bool alertInterpolWildlife(const TraffickingPattern& pattern);
    
    // Disease outbreak monitoring
    bool reportDiseaseCase(const String& speciesName, const String& disease,
                          float lat, float lon, const String& symptoms);
    bool declareOutbreak(const String& diseaseName, GlobalRegion region);
    bool getActiveOutbreaks(std::vector<DiseaseOutbreak>& outbreaks);
    bool coordinateResponse(const String& outbreakId,
                           const std::vector<String>& organizations);
    
    // Environmental disaster coordination
    bool reportEnvironmentalDisaster(ThreatType type, float lat, float lon,
                                    float affectedRadius,
                                    const String& description);
    bool assessDisasterImpact(const String& threatId,
                             uint32_t& affectedAnimals,
                             float& habitatLoss);
    bool coordinateRecoveryEfforts(const String& threatId,
                                  const std::vector<String>& teams);
    
    // Alert propagation and notification
    bool propagateAlert(const ThreatIntelligence& threat,
                       DataSharingLevel propagationLevel);
    bool notifySubscribers(const ThreatIntelligence& threat);
    bool sendEmergencyBroadcast(const String& message, GlobalRegion region);
    
    // Response coordination
    bool assignResponseTeam(const String& threatId, const String& teamId);
    bool recommendActions(const ThreatIntelligence& threat,
                         std::vector<ResponseAction>& actions);
    bool trackResponseProgress(const String& threatId, float& percentComplete);
    
    // Intelligence analysis
    bool correlateThreatData(const std::vector<String>& threatIds,
                            std::vector<String>& patterns);
    bool predictThreatAreas(ThreatType type, GlobalRegion region,
                           std::vector<std::pair<float, float>>& hotspots);
    bool generateThreatReport(uint32_t startTime, uint32_t endTime,
                             String& report);
    
    // Data queries
    std::vector<ThreatIntelligence> getActiveThreats(GlobalRegion region = REGION_NORTH_AMERICA);
    std::vector<ThreatIntelligence> getThreatsByType(ThreatType type);
    std::vector<ThreatIntelligence> getCriticalThreats();
    bool getThreatById(const String& threatId, ThreatIntelligence& threat);
    
    // Statistics and metrics
    uint32_t getTotalThreatsDetected() const { return totalThreats_; }
    uint32_t getActiveThreatsCount() const { return activeThreats_; }
    uint32_t getResolvedThreatsCount() const { return resolvedThreats_; }
    float getResponseRate() const;
    
private:
    bool initialized_;
    String organizationId_;
    
    // Threat tracking
    std::vector<ThreatIntelligence> threats_;
    std::vector<PoachingIncident> poachingIncidents_;
    std::vector<DiseaseOutbreak> diseaseOutbreaks_;
    std::vector<TraffickingPattern> traffickingPatterns_;
    
    // Subscriptions and notifications
    std::vector<AlertSubscription> subscriptions_;
    
    // Statistics
    uint32_t totalThreats_;
    uint32_t activeThreats_;
    uint32_t resolvedThreats_;
    uint32_t lastAnalysisTime_;
    
    // Helper methods
    String generateThreatId();
    bool validateThreatData(const ThreatIntelligence& threat);
    bool shouldPropagateAlert(const ThreatIntelligence& threat,
                             const AlertSubscription& subscription);
    bool encryptSensitiveData(const String& data, String& encrypted);
    bool sendSecureNotification(const String& recipient, const String& message);
    ThreatSeverity assessThreatSeverity(const ThreatIntelligence& threat);
    bool cacheThreatsOffline();
    bool uploadCachedThreats();
};

// Global threat intelligence instance
extern ThreatIntelligenceSharing* g_threatIntelligence;

// Utility functions for easy integration
bool initializeThreatIntelligence(const String& organizationId);
bool reportConservationThreat(ThreatType type, float lat, float lon,
                              const String& description);
bool reportPoaching(float lat, float lon, const String& species,
                   const String& details);
bool reportDisease(const String& species, const String& disease,
                  float lat, float lon);
bool checkActiveThreats(float lat, float lon, float radius,
                       std::vector<ThreatIntelligence>& nearbyThreats);
void cleanupThreatIntelligence();

#endif // THREAT_INTELLIGENCE_SHARING_H
