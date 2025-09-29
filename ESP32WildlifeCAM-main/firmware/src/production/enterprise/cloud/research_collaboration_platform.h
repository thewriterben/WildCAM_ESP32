/**
 * Research Collaboration Platform - Wildlife Research Data Sharing
 * 
 * Provides secure platform for sharing wildlife monitoring data with
 * research institutions, conservation organizations, and scientific
 * communities while maintaining privacy and data integrity.
 * 
 * Features:
 * - Secure data sharing protocols
 * - Researcher authentication and authorization
 * - Data anonymization and privacy protection
 * - Collaborative analysis tools
 * - Real-time research notifications
 * - Conservation project integration
 */

#ifndef RESEARCH_COLLABORATION_PLATFORM_H
#define RESEARCH_COLLABORATION_PLATFORM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "../cloud_integrator.h"

// Research Data Types
enum ResearchDataType {
    RESEARCH_WILDLIFE_DETECTION,   // Wildlife detection events
    RESEARCH_BEHAVIORAL_DATA,      // Animal behavior patterns
    RESEARCH_ENVIRONMENTAL_DATA,   // Environmental sensor data
    RESEARCH_MIGRATION_DATA,       // Migration patterns
    RESEARCH_POPULATION_DATA,      // Population statistics
    RESEARCH_HABITAT_DATA,         // Habitat information
    RESEARCH_CONSERVATION_DATA,    // Conservation project data
    RESEARCH_ANALYSIS_RESULTS      // Research analysis results
};

// Privacy Levels
enum PrivacyLevel {
    PRIVACY_PUBLIC,        // Publicly available data
    PRIVACY_RESEARCH,      // Research community only
    PRIVACY_INSTITUTION,   // Institution-specific
    PRIVACY_PROJECT,       // Project team only
    PRIVACY_CONFIDENTIAL   // Highly confidential
};

// Researcher Roles
enum ResearcherRole {
    ROLE_OBSERVER,         // Read-only access
    ROLE_CONTRIBUTOR,      // Can contribute data
    ROLE_ANALYST,          // Can perform analysis
    ROLE_PROJECT_LEAD,     // Project leadership
    ROLE_ADMINISTRATOR     // Platform administration
};

// Research Project
struct ResearchProject {
    String projectId;
    String projectName;
    String description;
    String leadResearcher;
    String institution;
    String contactEmail;
    uint32_t startDate;
    uint32_t endDate;
    PrivacyLevel privacyLevel;
    std::vector<String> participants;
    std::vector<String> dataTypes;
    std::map<String, String> metadata;
    bool isActive;
    
    ResearchProject() : 
        projectId(""), projectName(""), description(""), leadResearcher(""),
        institution(""), contactEmail(""), startDate(0), endDate(0),
        privacyLevel(PRIVACY_RESEARCH), isActive(true) {}
};

// Researcher Profile
struct ResearcherProfile {
    String researcherId;
    String name;
    String email;
    String institution;
    String department;
    String expertise;
    ResearcherRole role;
    std::vector<String> projects;
    std::vector<String> permissions;
    bool isVerified;
    uint32_t registrationDate;
    uint32_t lastActive;
    
    ResearcherProfile() : 
        researcherId(""), name(""), email(""), institution(""),
        department(""), expertise(""), role(ROLE_OBSERVER),
        isVerified(false), registrationDate(0), lastActive(0) {}
};

// Research Data Package
struct ResearchDataPackage {
    String dataId;
    String projectId;
    String contributorId;
    ResearchDataType dataType;
    PrivacyLevel privacyLevel;
    String title;
    String description;
    String dataPath;
    String metadataPath;
    uint32_t timestamp;
    uint32_t dataSize;
    String checksum;
    bool isAnonymized;
    std::map<String, String> tags;
    std::vector<String> collaborators;
    
    ResearchDataPackage() : 
        dataId(""), projectId(""), contributorId(""), dataType(RESEARCH_WILDLIFE_DETECTION),
        privacyLevel(PRIVACY_RESEARCH), title(""), description(""), dataPath(""),
        metadataPath(""), timestamp(0), dataSize(0), checksum(""), isAnonymized(false) {}
};

// Collaboration Request
struct CollaborationRequest {
    String requestId;
    String requesterId;
    String projectId;
    String dataId;
    String purpose;
    PrivacyLevel requestedAccess;
    uint32_t requestDate;
    uint32_t expiryDate;
    bool isApproved;
    String approvedBy;
    String rejectionReason;
    
    CollaborationRequest() : 
        requestId(""), requesterId(""), projectId(""), dataId(""),
        purpose(""), requestedAccess(PRIVACY_RESEARCH), requestDate(0),
        expiryDate(0), isApproved(false), approvedBy(""), rejectionReason("") {}
};

// Data Sharing Statistics
struct SharingStats {
    uint32_t totalDataShared;
    uint32_t totalDownloads;
    uint32_t activeProjects;
    uint32_t registeredResearchers;
    uint32_t pendingRequests;
    uint32_t dataContributions;
    std::map<ResearchDataType, uint32_t> dataTypeStats;
    
    SharingStats() : 
        totalDataShared(0), totalDownloads(0), activeProjects(0),
        registeredResearchers(0), pendingRequests(0), dataContributions(0) {}
};

/**
 * Research Collaboration Platform Class
 * 
 * Secure wildlife research data sharing and collaboration platform
 */
class ResearchCollaborationPlatform {
public:
    ResearchCollaborationPlatform();
    ~ResearchCollaborationPlatform();
    
    // Initialization and lifecycle
    bool initialize();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Project management
    bool createProject(const ResearchProject& project);
    bool updateProject(const String& projectId, const ResearchProject& project);
    bool deleteProject(const String& projectId);
    ResearchProject getProject(const String& projectId) const;
    std::vector<ResearchProject> getAllProjects() const;
    std::vector<ResearchProject> getProjectsByResearcher(const String& researcherId) const;
    bool addProjectParticipant(const String& projectId, const String& researcherId);
    bool removeProjectParticipant(const String& projectId, const String& researcherId);
    
    // Researcher management
    bool registerResearcher(const ResearcherProfile& profile);
    bool updateResearcher(const String& researcherId, const ResearcherProfile& profile);
    bool verifyResearcher(const String& researcherId, bool verified);
    ResearcherProfile getResearcher(const String& researcherId) const;
    std::vector<ResearcherProfile> getAllResearchers() const;
    bool authenticateResearcher(const String& researcherId, const String& credentials);
    bool authorizeAccess(const String& researcherId, const String& resource, const String& action);
    
    // Data sharing and management
    bool shareResearchData(const ResearchDataPackage& dataPackage);
    bool updateDataPackage(const String& dataId, const ResearchDataPackage& dataPackage);
    bool removeDataPackage(const String& dataId);
    ResearchDataPackage getDataPackage(const String& dataId) const;
    std::vector<ResearchDataPackage> getProjectData(const String& projectId) const;
    std::vector<ResearchDataPackage> searchData(const String& query, ResearchDataType type = RESEARCH_WILDLIFE_DETECTION) const;
    
    // Data anonymization and privacy
    bool anonymizeData(const String& dataId);
    bool adjustPrivacyLevel(const String& dataId, PrivacyLevel newLevel);
    bool validatePrivacyCompliance(const String& dataId) const;
    bool generateAnonymizedCopy(const String& dataId, String& anonymizedDataId);
    bool applyDataRetentionPolicies();
    
    // Collaboration requests
    bool submitCollaborationRequest(const CollaborationRequest& request);
    bool approveCollaborationRequest(const String& requestId, const String& approverId);
    bool rejectCollaborationRequest(const String& requestId, const String& reason);
    CollaborationRequest getCollaborationRequest(const String& requestId) const;
    std::vector<CollaborationRequest> getPendingRequests(const String& projectId = "") const;
    std::vector<CollaborationRequest> getApprovedRequests(const String& researcherId = "") const;
    
    // Data access and download
    bool requestDataAccess(const String& researcherId, const String& dataId, const String& purpose);
    bool grantDataAccess(const String& dataId, const String& researcherId, uint32_t duration);
    bool revokeDataAccess(const String& dataId, const String& researcherId);
    bool downloadData(const String& dataId, const String& researcherId, String& downloadUrl);
    bool trackDataUsage(const String& dataId, const String& researcherId, const String& action);
    
    // Real-time collaboration
    bool enableRealtimeSharing(const String& projectId, bool enable);
    bool broadcastDataUpdate(const String& projectId, const String& dataId, const String& updateType);
    bool notifyResearchers(const String& projectId, const String& message);
    bool subscribeToProject(const String& researcherId, const String& projectId);
    bool unsubscribeFromProject(const String& researcherId, const String& projectId);
    
    // Analytics and insights
    bool generateDataInsights(const String& projectId, JsonDocument& insights);
    bool analyzeWildlifePatterns(const String& projectId, JsonDocument& patterns);
    bool correlateEnvironmentalData(const String& projectId, JsonDocument& correlations);
    bool generateConservationReport(const String& projectId, String& report);
    
    // Integration with conservation efforts
    bool linkToConservationProject(const String& projectId, const String& conservationId);
    bool shareWithConservationOrg(const String& dataId, const String& organizationId);
    bool alertConservationThreats(const String& threat, const String& location, const JsonDocument& evidence);
    
    // Data validation and quality control
    bool validateDataQuality(const String& dataId);
    bool flagDataIssue(const String& dataId, const String& issue, const String& reporterId);
    bool resolveDataIssue(const String& dataId, const String& resolution);
    std::vector<String> getDataQualityIssues(const String& projectId = "") const;
    
    // Compliance and auditing
    bool auditDataAccess(const String& dataId, JsonDocument& auditLog);
    bool generateComplianceReport(const String& projectId, String& report);
    bool verifyDataIntegrity(const String& dataId);
    bool trackDataLineage(const String& dataId, JsonDocument& lineage);
    
    // Statistics and reporting
    SharingStats getSharingStatistics() const;
    void generateActivityReport(const String& projectId, String& report);
    void generateUsageReport(const String& researcherId, String& report);
    void generatePlatformReport(String& report);
    
    // Event callbacks
    typedef void (*DataSharedCallback)(const String& dataId, const String& projectId);
    typedef void (*CollaborationRequestCallback)(const String& requestId, const String& projectId);
    typedef void (*DataAccessCallback)(const String& dataId, const String& researcherId, const String& action);
    typedef void (*PrivacyViolationCallback)(const String& violation, const String& dataId);
    
    void setDataSharedCallback(DataSharedCallback callback) { dataSharedCallback_ = callback; }
    void setCollaborationRequestCallback(CollaborationRequestCallback callback) { collaborationRequestCallback_ = callback; }
    void setDataAccessCallback(DataAccessCallback callback) { dataAccessCallback_ = callback; }
    void setPrivacyViolationCallback(PrivacyViolationCallback callback) { privacyViolationCallback_ = callback; }

private:
    // Core state
    bool initialized_;
    
    // Data storage
    std::map<String, ResearchProject> projects_;
    std::map<String, ResearcherProfile> researchers_;
    std::map<String, ResearchDataPackage> dataPackages_;
    std::map<String, CollaborationRequest> collaborationRequests_;
    
    // Access control
    std::map<String, std::vector<String>> projectAccess_;
    std::map<String, std::vector<String>> dataAccess_;
    std::map<String, uint32_t> accessExpiry_;
    
    // Statistics tracking
    SharingStats currentStats_;
    std::map<String, uint32_t> downloadCounts_;
    std::map<String, uint32_t> accessCounts_;
    
    // Event callbacks
    DataSharedCallback dataSharedCallback_;
    CollaborationRequestCallback collaborationRequestCallback_;
    DataAccessCallback dataAccessCallback_;
    PrivacyViolationCallback privacyViolationCallback_;
    
    // Internal helper methods
    String generateUniqueId(const String& prefix) const;
    bool validateProjectData(const ResearchProject& project) const;
    bool validateResearcherData(const ResearcherProfile& researcher) const;
    bool validateDataPackage(const ResearchDataPackage& dataPackage) const;
    
    // Access control helpers
    bool hasProjectAccess(const String& researcherId, const String& projectId) const;
    bool hasDataAccess(const String& researcherId, const String& dataId) const;
    bool isAccessExpired(const String& accessKey) const;
    void cleanupExpiredAccess();
    
    // Privacy and anonymization helpers
    bool isDataAnonymizable(const ResearchDataPackage& dataPackage) const;
    bool performDataAnonymization(const String& dataId);
    bool removePersonalIdentifiers(JsonDocument& data);
    bool validatePrivacyLevel(const String& dataId, PrivacyLevel requiredLevel) const;
    
    // Data integrity helpers
    String calculateDataChecksum(const String& dataPath) const;
    bool verifyDataChecksum(const String& dataId) const;
    bool backupDataPackage(const String& dataId);
    
    // Notification helpers
    void notifyDataShared(const String& dataId, const String& projectId);
    void notifyCollaborationRequest(const String& requestId, const String& projectId);
    void notifyDataAccess(const String& dataId, const String& researcherId, const String& action);
    void notifyPrivacyViolation(const String& violation, const String& dataId);
    
    // Analytics helpers
    bool calculateDataMetrics(const String& projectId, JsonDocument& metrics);
    bool analyzeDataPatterns(const std::vector<ResearchDataPackage>& data, JsonDocument& patterns);
    bool generateDataCorrelations(const String& projectId, JsonDocument& correlations);
    
    // Cloud integration helpers
    bool uploadToCloudRepository(const ResearchDataPackage& dataPackage);
    bool downloadFromCloudRepository(const String& dataId, String& localPath);
    bool syncWithCloudRepository();
    
    // Utility methods
    String getDataTypeName(ResearchDataType type) const;
    String getPrivacyLevelName(PrivacyLevel level) const;
    String getRoleName(ResearcherRole role) const;
    bool isValidEmail(const String& email) const;
    uint32_t getCurrentTimestamp() const;
};

// Global research collaboration platform instance
extern ResearchCollaborationPlatform* g_researchCollaborationPlatform;

// Utility functions for easy integration
bool initializeResearchCollaboration();
bool shareWildlifeData(const String& dataPath, const String& projectId, PrivacyLevel privacy = PRIVACY_RESEARCH);
bool requestResearchCollaboration(const String& researcherId, const String& projectId, const String& purpose);
bool isResearchDataAvailable(const String& dataType);
void cleanupResearchCollaboration();

#endif // RESEARCH_COLLABORATION_PLATFORM_H