/**
 * University Research Integration Module
 * 
 * Provides integration with university research platforms for
 * scientific data collection and collaborative research projects.
 * 
 * Features:
 * - Research project data tagging
 * - FAIR data principles compliance
 * - Multi-institutional data sharing
 * - Longitudinal study support
 * - Research metadata enrichment
 */

#ifndef UNIVERSITY_RESEARCH_INTEGRATION_H
#define UNIVERSITY_RESEARCH_INTEGRATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>

// Research project metadata
struct ResearchProjectMetadata {
    String project_id;
    String university;
    String principal_investigator;
    String study_type;  // longitudinal, behavioral, conservation
    std::vector<String> species_focus;
    String start_date;
    String ethics_approval_number;
    bool data_sharing_enabled;
};

// Scientific observation data
struct ScientificObservation {
    String observation_id;
    String project_id;
    String timestamp;
    String species;
    String behavior;  // hunting, resting, territorial_marking, social_interaction
    float confidence;
    struct {
        float latitude;
        float longitude;
        float accuracy;
    } location;
    struct {
        float temperature;
        float humidity;
        float light_level;
        String season;
    } environmental_conditions;
    String observer_notes;
    bool quality_verified;
};

// FAIR data package
struct FAIRDataPackage {
    String dataset_id;
    String title;
    String description;
    std::vector<String> creators;
    std::vector<String> keywords;
    String license;  // CC-BY-4.0, CC0, etc.
    String doi;
    struct {
        String persistent_identifier;
        bool indexed_in_registry;
        String metadata_standard;  // Darwin Core, EML
    } findability;
    struct {
        String access_protocol;  // https, ftp
        bool authentication_required;
        String access_url;
    } accessibility;
    struct {
        String format;  // CSV, JSON, NetCDF
        String vocabulary;  // controlled vocabulary
        bool machine_readable;
    } interoperability;
    struct {
        String license;
        String provenance;
        String citation;
        String usage_guidelines;
    } reusability;
};

/**
 * University Research Integration Class
 * 
 * Manages research data collection and university partnerships
 */
class UniversityResearchIntegration {
public:
    UniversityResearchIntegration();
    ~UniversityResearchIntegration();
    
    // Initialization
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Research project management
    bool registerProject(const ResearchProjectMetadata& project);
    bool updateProject(const String& projectId, const ResearchProjectMetadata& project);
    ResearchProjectMetadata getProject(const String& projectId) const;
    bool enableProjectDataCollection(const String& projectId);
    bool disableProjectDataCollection(const String& projectId);
    
    // Scientific observations
    bool recordObservation(const ScientificObservation& observation);
    bool addObservationMetadata(const String& observationId, const JsonDocument& metadata);
    bool validateObservation(const String& observationId);
    std::vector<ScientificObservation> getProjectObservations(const String& projectId) const;
    
    // FAIR data principles
    bool createFAIRDataset(const FAIRDataPackage& dataset);
    bool enrichMetadata(const String& datasetId, const JsonDocument& metadata);
    bool assignDOI(const String& datasetId, const String& doi);
    bool publishDataset(const String& datasetId);
    
    // Research quality assurance
    bool performQualityCheck(const String& observationId);
    bool flagForReview(const String& observationId, const String& reason);
    bool calculateDataQualityScore(const String& projectId, float& score);
    
    // University authentication
    bool authenticateUniversity(const String& universityId, const String& apiKey);
    bool verifyResearcher(const String& researcherId, const String& credentials);
    bool hasProjectAccess(const String& researcherId, const String& projectId) const;
    
    // Data export for research
    bool exportProjectData(const String& projectId, const String& format, String& exportPath);
    bool exportDarwinCore(const String& projectId, String& exportPath);
    bool exportEML(const String& projectId, String& exportPath);
    
    // Longitudinal study support
    bool createLongitudinalStudy(const String& studyId, const ResearchProjectMetadata& project);
    bool addStudyTimepoint(const String& studyId, const ScientificObservation& observation);
    bool calculateStudyMetrics(const String& studyId, JsonDocument& metrics);
    
    // Collaboration features
    bool shareDataWithInstitution(const String& datasetId, const String& universityId);
    bool requestCollaboration(const String& projectId, const String& requestingUniversity);
    bool approveCollaboration(const String& requestId);
    
    // Statistics and reporting
    int getTotalProjects() const;
    int getTotalObservations() const;
    std::vector<String> getActiveUniversities() const;
    bool generateResearchReport(const String& projectId, JsonDocument& report);
    
private:
    bool initialized_;
    
    // Data storage
    std::map<String, ResearchProjectMetadata> projects_;
    std::map<String, ScientificObservation> observations_;
    std::map<String, FAIRDataPackage> datasets_;
    std::map<String, String> university_auth_;
    std::map<String, std::vector<String>> researcher_projects_;
    
    // Active tracking
    std::vector<String> active_projects_;
    std::map<String, float> project_quality_scores_;
    
    // Helper methods
    String generateObservationId() const;
    String generateDatasetId() const;
    bool validateProjectMetadata(const ResearchProjectMetadata& project) const;
    bool validateObservationData(const ScientificObservation& observation) const;
    bool enrichWithDarwinCore(ScientificObservation& observation);
    float calculateQualityScore(const ScientificObservation& observation) const;
    
    // Data formatting helpers
    bool formatDarwinCoreRecord(const ScientificObservation& obs, JsonDocument& record);
    bool formatEMLMetadata(const ResearchProjectMetadata& project, JsonDocument& eml);
    
    // Quality assurance helpers
    bool checkTemporalConsistency(const String& projectId);
    bool checkSpatialConsistency(const String& projectId);
    bool detectOutliers(const String& projectId);
    
    // Export helpers
    bool writeCSV(const std::vector<ScientificObservation>& observations, const String& path);
    bool writeJSON(const std::vector<ScientificObservation>& observations, const String& path);
    bool writeDarwinCoreArchive(const String& projectId, const String& path);
};

// Global instance
extern UniversityResearchIntegration* g_universityResearchIntegration;

// Utility functions
bool initializeUniversityResearch();
bool recordResearchObservation(const String& projectId, const String& species, float confidence);
bool isResearchProject(const String& projectId);
bool exportResearchData(const String& projectId, const String& format);
void cleanupUniversityResearch();

#endif // UNIVERSITY_RESEARCH_INTEGRATION_H
