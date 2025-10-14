/**
 * Conservation Impact Verification System
 * 
 * Blockchain-based verification of conservation outcomes, funding
 * utilization, and measurable impact through satellite and ground-based
 * monitoring. Provides transparent tracking of conservation effectiveness
 * and automated verification for impact reporting.
 * 
 * Features:
 * - Blockchain-based outcome verification
 * - Automated carbon credit verification
 * - Species population recovery tracking
 * - Habitat restoration monitoring
 * - Conservation effectiveness measurement
 * - Impact reporting for funding organizations
 * - Transparent outcome tracking
 * 
 * @file conservation_impact_verification.h
 * @author ESP32WildlifeCAM Team
 * @date 2024-10-14
 */

#ifndef CONSERVATION_IMPACT_VERIFICATION_H
#define CONSERVATION_IMPACT_VERIFICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "global_conservation_network.h"

// Impact verification types
enum VerificationType {
    VERIFY_POPULATION_RECOVERY,     // Species population increase
    VERIFY_HABITAT_RESTORATION,     // Habitat area restored
    VERIFY_CARBON_SEQUESTRATION,    // Carbon credits earned
    VERIFY_THREAT_REDUCTION,        // Threat incidents reduced
    VERIFY_BIODIVERSITY_INCREASE,   // Biodiversity indices improved
    VERIFY_ECOSYSTEM_SERVICES,      // Ecosystem services value
    VERIFY_FUNDING_UTILIZATION     // Proper fund usage
};

// Verification status
enum VerificationStatus {
    STATUS_PENDING,          // Awaiting verification
    STATUS_IN_PROGRESS,      // Verification in progress
    STATUS_VERIFIED,         // Independently verified
    STATUS_REJECTED,         // Verification failed
    STATUS_DISPUTED         // Under dispute resolution
};

// Monitoring method
enum MonitoringMethod {
    METHOD_SATELLITE_IMAGERY,       // Satellite-based monitoring
    METHOD_GROUND_SURVEY,           // Field survey data
    METHOD_CAMERA_TRAP,            // Camera trap evidence
    METHOD_ACOUSTIC_MONITORING,     // Bioacoustic monitoring
    METHOD_DRONE_SURVEY,           // UAV-based monitoring
    METHOD_COMMUNITY_REPORTING,     // Local community reports
    METHOD_SENSOR_NETWORK,         // IoT sensor network
    METHOD_DNA_ANALYSIS           // Genetic monitoring
};

// Conservation outcome record
struct ConservationOutcome {
    String outcomeId;
    String projectId;
    VerificationType type;
    String description;
    uint32_t targetDate;
    uint32_t actualDate;
    
    // Quantitative metrics
    float targetValue;
    float achievedValue;
    String unit;  // "individuals", "hectares", "tons CO2", etc.
    float achievementPercentage;
    
    // Verification details
    VerificationStatus status;
    std::vector<MonitoringMethod> verificationMethods;
    std::vector<String> evidenceHashes;  // SHA-256 hashes
    String blockchainTxId;
    uint32_t verificationDate;
    std::vector<String> verifiedBy;
    
    // Location and scope
    float latitude;
    float longitude;
    GlobalRegion region;
    float affectedArea;  // sq km
    std::vector<String> speciesAffected;
    
    // Impact assessment
    String environmentalImpact;
    String socialImpact;
    String economicImpact;
    float costEffectiveness;  // Cost per unit of outcome
    
    ConservationOutcome() :
        outcomeId(""), projectId(""), type(VERIFY_POPULATION_RECOVERY),
        description(""), targetDate(0), actualDate(0), targetValue(0.0),
        achievedValue(0.0), unit(""), achievementPercentage(0.0),
        status(STATUS_PENDING), blockchainTxId(""), verificationDate(0),
        latitude(0.0), longitude(0.0), region(REGION_NORTH_AMERICA),
        affectedArea(0.0), environmentalImpact(""), socialImpact(""),
        economicImpact(""), costEffectiveness(0.0) {}
};

// Carbon credit verification
struct CarbonCreditVerification {
    String creditId;
    String projectId;
    float carbonSequestered;  // tons CO2
    uint32_t measurementPeriodStart;
    uint32_t measurementPeriodEnd;
    
    // Verification methodology
    String methodology;
    String standard;  // "Verified Carbon Standard", "Gold Standard", etc.
    std::vector<String> baselineData;
    std::vector<String> monitoringData;
    
    // Verification results
    float verifiedCredits;
    VerificationStatus status;
    String verifiedBy;
    uint32_t verificationDate;
    String certificationUrl;
    String blockchainRecordId;
    
    // Market information
    float creditPrice;
    String currency;
    bool isTraded;
    String buyerId;
    uint32_t transactionDate;
    
    CarbonCreditVerification() :
        creditId(""), projectId(""), carbonSequestered(0.0),
        measurementPeriodStart(0), measurementPeriodEnd(0),
        methodology(""), standard(""), verifiedCredits(0.0),
        status(STATUS_PENDING), verifiedBy(""), verificationDate(0),
        certificationUrl(""), blockchainRecordId(""), creditPrice(0.0),
        currency("USD"), isTraded(false), buyerId(""), transactionDate(0) {}
};

// Population recovery tracking
struct PopulationRecoveryTracking {
    String trackingId;
    String speciesName;
    String scientificName;
    GlobalRegion region;
    
    // Baseline and target
    uint32_t baselinePopulation;
    uint32_t baselineDate;
    uint32_t targetPopulation;
    uint32_t targetDate;
    
    // Current status
    uint32_t currentPopulation;
    uint32_t lastCount;
    float recoveryRate;  // percentage
    String trend;  // "increasing", "stable", "decreasing"
    
    // Verification
    std::vector<MonitoringMethod> methods;
    float confidenceLevel;
    std::vector<String> dataSourceIds;
    VerificationStatus status;
    std::vector<String> verifiers;
    
    // Contributing factors
    std::vector<String> interventions;
    std::vector<String> threats;
    String habitatQuality;
    
    PopulationRecoveryTracking() :
        trackingId(""), speciesName(""), scientificName(""),
        region(REGION_NORTH_AMERICA), baselinePopulation(0),
        baselineDate(0), targetPopulation(0), targetDate(0),
        currentPopulation(0), lastCount(0), recoveryRate(0.0),
        trend("unknown"), confidenceLevel(0.0), status(STATUS_PENDING),
        habitatQuality("unknown") {}
};

// Habitat restoration verification
struct HabitatRestorationVerification {
    String restorationId;
    String projectId;
    String habitatType;
    GlobalRegion region;
    
    // Area metrics
    float targetArea;      // hectares
    float restoredArea;    // hectares
    float achievementPercentage;
    
    // Quality metrics
    float vegetationCover;
    uint32_t nativeSpeciesPlanted;
    float biodiversityIndex;
    float soilQuality;
    float waterQuality;
    
    // Verification
    std::vector<MonitoringMethod> methods;
    std::vector<String> satelliteImageHashes;
    std::vector<String> groundSurveyData;
    VerificationStatus status;
    uint32_t lastVerification;
    std::vector<String> verifiedBy;
    
    // Timeline
    uint32_t projectStart;
    uint32_t projectEnd;
    uint32_t nextVerification;
    
    HabitatRestorationVerification() :
        restorationId(""), projectId(""), habitatType(""),
        region(REGION_NORTH_AMERICA), targetArea(0.0),
        restoredArea(0.0), achievementPercentage(0.0),
        vegetationCover(0.0), nativeSpeciesPlanted(0),
        biodiversityIndex(0.0), soilQuality(0.0), waterQuality(0.0),
        status(STATUS_PENDING), lastVerification(0), projectStart(0),
        projectEnd(0), nextVerification(0) {}
};

// Funding utilization tracking
struct FundingUtilization {
    String utilizationId;
    String projectId;
    String fundingSource;
    float totalFunding;
    String currency;
    
    // Allocation breakdown
    std::map<String, float> categoryAllocations;  // category -> amount
    std::map<String, float> categorySpent;
    float totalSpent;
    float remainingFunds;
    
    // Expenditure tracking
    std::vector<String> expenditureReceipts;
    std::vector<String> blockchainRecords;
    
    // Outcomes achieved per funding
    float costPerOutcome;
    std::vector<String> outcomesAchieved;
    float returnOnInvestment;
    
    // Verification and audit
    VerificationStatus status;
    String auditedBy;
    uint32_t lastAuditDate;
    bool compliant;
    std::vector<String> findings;
    
    FundingUtilization() :
        utilizationId(""), projectId(""), fundingSource(""),
        totalFunding(0.0), currency("USD"), totalSpent(0.0),
        remainingFunds(0.0), costPerOutcome(0.0), returnOnInvestment(0.0),
        status(STATUS_PENDING), auditedBy(""), lastAuditDate(0),
        compliant(true) {}
};

// Impact report
struct ImpactReport {
    String reportId;
    String projectId;
    String reportingOrg;
    uint32_t reportingPeriodStart;
    uint32_t reportingPeriodEnd;
    
    // Outcomes summary
    std::vector<ConservationOutcome> outcomes;
    float overallAchievementRate;
    
    // Key performance indicators
    std::map<String, float> kpis;
    
    // Verification summary
    uint32_t totalOutcomesReported;
    uint32_t verifiedOutcomes;
    uint32_t pendingVerification;
    
    // Financial summary
    float totalFundingReceived;
    float totalExpenditure;
    float costEfficiency;
    
    // Supporting evidence
    std::vector<String> evidenceUrls;
    std::vector<String> blockchainRecords;
    
    // Stakeholder feedback
    std::map<String, String> stakeholderComments;
    
    ImpactReport() :
        reportId(""), projectId(""), reportingOrg(""),
        reportingPeriodStart(0), reportingPeriodEnd(0),
        overallAchievementRate(0.0), totalOutcomesReported(0),
        verifiedOutcomes(0), pendingVerification(0),
        totalFundingReceived(0.0), totalExpenditure(0.0),
        costEfficiency(0.0) {}
};

/**
 * Conservation Impact Verification System Class
 * 
 * Manages blockchain-based verification of conservation outcomes
 * and transparent impact reporting for stakeholders.
 */
class ConservationImpactVerification {
public:
    ConservationImpactVerification();
    ~ConservationImpactVerification();
    
    // Initialization
    bool initialize(const String& organizationId);
    bool connectToBlockchain(const String& nodeUrl, const String& apiKey);
    
    // Outcome registration and verification
    bool registerOutcome(const ConservationOutcome& outcome);
    bool submitEvidence(const String& outcomeId, const String& evidenceHash,
                       MonitoringMethod method);
    bool requestVerification(const String& outcomeId);
    bool verifyOutcome(const String& outcomeId, bool verified,
                      const String& verifierId, const String& comments);
    bool getOutcomeStatus(const String& outcomeId, ConservationOutcome& outcome);
    
    // Carbon credit verification
    bool registerCarbonCredits(const CarbonCreditVerification& credits);
    bool verifyCarbonSequestration(const String& creditId,
                                  float verifiedAmount,
                                  const String& verifierId);
    bool tradeCarbonCredits(const String& creditId, const String& buyerId,
                           float price);
    std::vector<CarbonCreditVerification> getAvailableCredits();
    
    // Population recovery tracking
    bool initializePopulationTracking(const PopulationRecoveryTracking& tracking);
    bool updatePopulationCount(const String& trackingId,
                              uint32_t newCount,
                              const String& dataSourceId);
    bool verifyPopulationRecovery(const String& trackingId,
                                 const String& verifierId);
    std::vector<PopulationRecoveryTracking> getRecoveryProgress();
    
    // Habitat restoration verification
    bool registerRestoration(const HabitatRestorationVerification& restoration);
    bool submitSatelliteEvidence(const String& restorationId,
                                const String& imageHash,
                                uint32_t captureDate);
    bool verifyRestoration(const String& restorationId,
                          const String& verifierId);
    float calculateRestorationProgress(const String& restorationId);
    
    // Funding utilization tracking
    bool trackFundingUtilization(const FundingUtilization& utilization);
    bool recordExpenditure(const String& utilizationId, const String& category,
                          float amount, const String& receiptHash);
    bool auditFunding(const String& utilizationId, const String& auditorId);
    bool generateFinancialReport(const String& projectId, String& report);
    
    // Impact reporting
    bool generateImpactReport(const String& projectId,
                             uint32_t periodStart, uint32_t periodEnd,
                             ImpactReport& report);
    bool publishImpactReport(const ImpactReport& report);
    bool getImpactReports(const String& projectId,
                         std::vector<ImpactReport>& reports);
    
    // Blockchain integration
    bool recordOnBlockchain(const String& dataType, const String& dataHash,
                           String& transactionId);
    bool verifyBlockchainRecord(const String& transactionId, String& data);
    bool getBlockchainAuditTrail(const String& outcomeId,
                                std::vector<String>& transactions);
    
    // Automated verification
    bool enableAutomatedVerification(VerificationType type, bool enable);
    bool setVerificationCriteria(VerificationType type,
                                const JsonDocument& criteria);
    bool runAutomatedVerification(const String& outcomeId);
    
    // Standardized metrics
    bool calculateBiodiversityIndex(const String& location,
                                   float& index);
    bool assessEcosystemServices(const String& location,
                                float& valueUSD);
    bool measureConservationEffectiveness(const String& projectId,
                                         float& effectivenessScore);
    
    // Transparency and reporting
    bool publishVerificationData(const String& outcomeId,
                                bool makePublic);
    bool generateTransparencyReport(const String& organizationId,
                                   String& report);
    std::vector<ConservationOutcome> getVerifiedOutcomes(GlobalRegion region);
    
    // Statistics
    uint32_t getTotalOutcomes() const { return totalOutcomes_; }
    uint32_t getVerifiedOutcomes() const { return verifiedOutcomes_; }
    float getVerificationRate() const;
    float getAverageImpactScore() const;
    
private:
    bool initialized_;
    String organizationId_;
    String blockchainNodeUrl_;
    String blockchainApiKey_;
    
    // Verification data
    std::vector<ConservationOutcome> outcomes_;
    std::vector<CarbonCreditVerification> carbonCredits_;
    std::vector<PopulationRecoveryTracking> populationTracking_;
    std::vector<HabitatRestorationVerification> habitatRestorations_;
    std::vector<FundingUtilization> fundingRecords_;
    std::vector<ImpactReport> impactReports_;
    
    // Statistics
    uint32_t totalOutcomes_;
    uint32_t verifiedOutcomes_;
    uint32_t pendingVerification_;
    float totalCarbonCredits_;
    
    // Automated verification settings
    std::map<VerificationType, bool> automatedVerification_;
    std::map<VerificationType, JsonDocument> verificationCriteria_;
    
    // Helper methods
    String generateOutcomeId();
    bool validateOutcome(const ConservationOutcome& outcome);
    bool storeOnBlockchain(const String& data, String& txId);
    bool retrieveFromBlockchain(const String& txId, String& data);
    float calculateImpactScore(const ConservationOutcome& outcome);
    bool notifyStakeholders(const String& outcomeId, const String& status);
    bool encryptSensitiveData(const String& data, String& encrypted);
};

// Global impact verification instance
extern ConservationImpactVerification* g_impactVerification;

// Utility functions for easy integration
bool initializeImpactVerification(const String& organizationId);
bool recordConservationOutcome(const String& projectId, VerificationType type,
                              float achieved, const String& evidence);
bool verifyCarbonCredit(float carbonTons, const String& projectId);
bool trackPopulationRecovery(const String& species, uint32_t count);
bool verifyHabitatRestoration(float hectares, const String& projectId);
void cleanupImpactVerification();

#endif // CONSERVATION_IMPACT_VERIFICATION_H
