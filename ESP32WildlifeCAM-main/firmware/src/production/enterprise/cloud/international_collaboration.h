/**
 * International Collaboration Platform
 * 
 * Secure multi-organization data sharing with fine-grained access control,
 * enabling global researcher authentication, collaborative research project
 * management, and international conservation coordination.
 * 
 * Features:
 * - Multi-organization data federation
 * - Global researcher authentication and access management
 * - Collaborative research project management across countries
 * - International conservation funding optimization
 * - Peer review and validation systems
 * - Standardized scientific protocols
 * - Multi-language support
 * 
 * @file international_collaboration.h
 * @author ESP32WildlifeCAM Team
 * @date 2024-10-14
 */

#ifndef INTERNATIONAL_COLLABORATION_H
#define INTERNATIONAL_COLLABORATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "global_conservation_network.h"
#include "research_collaboration_platform.h"

// Organization types
enum OrganizationType {
    ORG_RESEARCH_INSTITUTION,      // University, research center
    ORG_CONSERVATION_NGO,           // Non-governmental organization
    ORG_GOVERNMENT_AGENCY,          // Government wildlife agency
    ORG_INTERNATIONAL_BODY,         // UN, IUCN, CITES, etc.
    ORG_PRIVATE_FOUNDATION,         // Conservation foundation
    ORG_CITIZEN_SCIENCE,            // Citizen science group
    ORG_COMMERCIAL                  // Commercial partner
};

// Access control levels
enum AccessLevel {
    ACCESS_NONE,           // No access
    ACCESS_VIEW,           // View-only access
    ACCESS_DOWNLOAD,       // Can download data
    ACCESS_CONTRIBUTE,     // Can contribute data
    ACCESS_EDIT,           // Can edit shared data
    ACCESS_ADMIN          // Full administrative access
};

// Data classification levels
enum DataClassification {
    DATA_PUBLIC,           // Publicly available
    DATA_RESEARCH,         // Research community only
    DATA_PARTNER,          // Partner organizations only
    DATA_SENSITIVE,        // Sensitive conservation data
    DATA_CONFIDENTIAL     // Highly confidential
};

// Collaboration types
enum CollaborationType {
    COLLAB_DATA_SHARING,           // Data sharing agreement
    COLLAB_JOINT_RESEARCH,         // Joint research project
    COLLAB_CAPACITY_BUILDING,      // Training and capacity building
    COLLAB_FUNDING,                // Joint funding initiative
    COLLAB_TECHNOLOGY_TRANSFER,    // Technology sharing
    COLLAB_POLICY_DEVELOPMENT     // Policy collaboration
};

// Organization profile
struct OrganizationProfile {
    String organizationId;
    String name;
    OrganizationType type;
    String country;
    GlobalRegion region;
    String contactEmail;
    String website;
    std::vector<String> focusAreas;
    std::vector<String> speciesExpertise;
    bool verified;
    uint32_t memberSince;
    uint32_t lastActive;
    std::map<String, String> certifications;
    
    OrganizationProfile() :
        organizationId(""), name(""), type(ORG_RESEARCH_INSTITUTION),
        country(""), region(REGION_NORTH_AMERICA), contactEmail(""),
        website(""), verified(false), memberSince(0), lastActive(0) {}
};

// Collaboration agreement
struct CollaborationAgreement {
    String agreementId;
    String projectId;
    CollaborationType type;
    std::vector<String> participatingOrgs;
    String leadOrganization;
    String title;
    String description;
    uint32_t startDate;
    uint32_t endDate;
    DataSharingLevel dataSharingLevel;
    std::map<String, AccessLevel> orgAccessLevels;
    std::vector<String> sharedDatasets;
    String termsAndConditions;
    bool active;
    std::vector<String> signedBy;
    
    CollaborationAgreement() :
        agreementId(""), projectId(""), type(COLLAB_DATA_SHARING),
        leadOrganization(""), title(""), description(""),
        startDate(0), endDate(0), dataSharingLevel(SHARING_LOCAL),
        termsAndConditions(""), active(false) {}
};

// International research project
struct InternationalProject {
    String projectId;
    String title;
    String description;
    String leadInstitution;
    String principalInvestigator;
    std::vector<String> collaboratingOrgs;
    std::vector<String> fundingSources;
    float totalFunding;
    uint32_t startDate;
    uint32_t endDate;
    std::vector<GlobalRegion> regionsInvolved;
    std::vector<String> speciesStudied;
    std::vector<String> objectives;
    std::map<String, String> deliverables;
    String status;  // "planning", "active", "completed", "suspended"
    float progressPercentage;
    std::vector<String> publications;
    
    InternationalProject() :
        projectId(""), title(""), description(""), leadInstitution(""),
        principalInvestigator(""), totalFunding(0.0), startDate(0),
        endDate(0), status("planning"), progressPercentage(0.0) {}
};

// Funding opportunity
struct FundingOpportunity {
    String fundingId;
    String fundingBody;
    String programName;
    float amountAvailable;
    String currency;
    uint32_t applicationDeadline;
    uint32_t projectStartDate;
    std::vector<String> eligibleCountries;
    std::vector<String> eligibleOrganizations;
    std::vector<String> focusAreas;
    String applicationUrl;
    String description;
    bool requiresInternationalPartnership;
    std::vector<String> pastRecipients;
    
    FundingOpportunity() :
        fundingId(""), fundingBody(""), programName(""),
        amountAvailable(0.0), currency("USD"), applicationDeadline(0),
        projectStartDate(0), applicationUrl(""), description(""),
        requiresInternationalPartnership(false) {}
};

// Data sharing request
struct DataSharingRequest {
    String requestId;
    String requestingOrg;
    String requestingResearcher;
    String targetOrg;
    String datasetId;
    String purpose;
    AccessLevel requestedAccess;
    uint32_t requestDate;
    uint32_t proposedStartDate;
    uint32_t proposedEndDate;
    String justification;
    String ethicsApproval;
    String status;  // "pending", "approved", "denied", "expired"
    String reviewedBy;
    uint32_t reviewDate;
    String reviewComments;
    
    DataSharingRequest() :
        requestId(""), requestingOrg(""), requestingResearcher(""),
        targetOrg(""), datasetId(""), purpose(""),
        requestedAccess(ACCESS_VIEW), requestDate(0),
        proposedStartDate(0), proposedEndDate(0), justification(""),
        ethicsApproval(""), status("pending"), reviewedBy(""),
        reviewDate(0), reviewComments("") {}
};

// Peer review submission
struct PeerReviewSubmission {
    String submissionId;
    String projectId;
    String submittedBy;
    String submittingOrg;
    String title;
    String abstract_;
    std::vector<String> authors;
    std::vector<String> affiliations;
    std::vector<String> datasets;
    String methodology;
    String results;
    String conclusions;
    uint32_t submissionDate;
    std::vector<String> reviewers;
    std::map<String, String> reviews;
    String status;  // "submitted", "under_review", "approved", "rejected"
    float qualityScore;
    
    PeerReviewSubmission() :
        submissionId(""), projectId(""), submittedBy(""),
        submittingOrg(""), title(""), abstract_(""), methodology(""),
        results(""), conclusions(""), submissionDate(0),
        status("submitted"), qualityScore(0.0) {}
};

/**
 * International Collaboration Platform Class
 * 
 * Manages secure multi-organization data sharing and international
 * research collaboration for global conservation efforts.
 */
class InternationalCollaboration {
public:
    InternationalCollaboration();
    ~InternationalCollaboration();
    
    // Platform initialization
    bool initialize(const String& organizationId);
    bool registerOrganization(const OrganizationProfile& profile);
    bool verifyOrganization(const String& orgId, bool verified);
    
    // Organization management
    bool updateOrganizationProfile(const OrganizationProfile& profile);
    bool searchOrganizations(const String& query,
                            std::vector<OrganizationProfile>& results);
    bool getOrganizationProfile(const String& orgId,
                               OrganizationProfile& profile);
    
    // Collaboration agreement management
    bool proposeCollaboration(const CollaborationAgreement& agreement);
    bool reviewCollaboration(const String& agreementId, bool approve,
                            const String& comments);
    bool signAgreement(const String& agreementId, const String& signatory);
    bool getActiveAgreements(std::vector<CollaborationAgreement>& agreements);
    
    // Project management
    bool createInternationalProject(const InternationalProject& project);
    bool updateProjectStatus(const String& projectId, const String& status,
                            float progressPercentage);
    bool addProjectCollaborator(const String& projectId, const String& orgId);
    bool getProjects(std::vector<InternationalProject>& projects);
    bool searchProjects(const String& query,
                       std::vector<InternationalProject>& results);
    
    // Funding coordination
    bool publishFundingOpportunity(const FundingOpportunity& opportunity);
    bool searchFunding(const String& focusArea, const String& country,
                      std::vector<FundingOpportunity>& results);
    bool applyForFunding(const String& fundingId, const String& projectId,
                        const String& application);
    bool trackFundingApplication(const String& applicationId, String& status);
    
    // Data sharing management
    bool requestDataAccess(const DataSharingRequest& request);
    bool reviewDataRequest(const String& requestId, bool approve,
                          const String& comments);
    bool grantDataAccess(const String& requestId, const String& datasetId,
                        AccessLevel accessLevel, uint32_t duration);
    bool revokeDataAccess(const String& requestId);
    std::vector<DataSharingRequest> getPendingRequests();
    
    // Access control
    bool setDataAccessLevel(const String& datasetId, const String& orgId,
                           AccessLevel level);
    bool checkAccessPermission(const String& orgId, const String& datasetId,
                              AccessLevel requiredLevel);
    bool auditDataAccess(const String& datasetId, JsonDocument& auditLog);
    
    // Peer review system
    bool submitForReview(const PeerReviewSubmission& submission);
    bool assignReviewers(const String& submissionId,
                        const std::vector<String>& reviewerIds);
    bool submitReview(const String& submissionId, const String& reviewerId,
                     const String& review, float score);
    bool approveSubmission(const String& submissionId);
    std::vector<PeerReviewSubmission> getPendingReviews();
    
    // International coordination
    bool coordinateFieldwork(const String& projectId,
                            const std::vector<String>& locations,
                            const std::vector<String>& teams);
    bool scheduleInternationalMeeting(const String& projectId,
                                     uint32_t meetingTime,
                                     const String& agenda);
    bool shareProtocol(const String& protocolName, const String& description,
                      const String& documentUrl);
    
    // Multi-language support
    bool setPreferredLanguage(const String& languageCode);
    bool translateContent(const String& content, const String& targetLanguage,
                         String& translated);
    bool addTranslation(const String& originalId, const String& language,
                       const String& translation);
    
    // Capacity building
    bool offerTraining(const String& trainingName, const String& description,
                      uint32_t startDate, const std::vector<String>& targetOrgs);
    bool requestTechnicalAssistance(const String& area, const String& description);
    bool shareResourcesLibrary(const String& resourceType,
                              const std::vector<String>& resources);
    
    // Statistics and reporting
    uint32_t getTotalOrganizations() const { return totalOrganizations_; }
    uint32_t getActiveProjects() const { return activeProjects_; }
    uint32_t getActiveCollaborations() const { return activeCollaborations_; }
    bool generateCollaborationReport(uint32_t startTime, uint32_t endTime,
                                    String& report);
    
private:
    bool initialized_;
    String organizationId_;
    String preferredLanguage_;
    
    // Organization data
    std::vector<OrganizationProfile> organizations_;
    std::map<String, OrganizationProfile> orgCache_;
    
    // Collaboration data
    std::vector<CollaborationAgreement> agreements_;
    std::vector<InternationalProject> projects_;
    std::vector<FundingOpportunity> fundingOpportunities_;
    std::vector<DataSharingRequest> dataSharingRequests_;
    std::vector<PeerReviewSubmission> peerReviews_;
    
    // Access control
    std::map<String, std::map<String, AccessLevel>> dataAccessMatrix_;
    
    // Statistics
    uint32_t totalOrganizations_;
    uint32_t activeProjects_;
    uint32_t activeCollaborations_;
    uint32_t dataSetsShared_;
    
    // Helper methods
    String generateAgreementId();
    String generateProjectId();
    bool validateOrganization(const OrganizationProfile& profile);
    bool validateAgreement(const CollaborationAgreement& agreement);
    bool notifyStakeholders(const String& projectId, const String& message);
    bool enforceAccessControl(const String& orgId, const String& resource);
    bool logAccessEvent(const String& orgId, const String& resource,
                       const String& action);
    bool encryptSensitiveCollaborationData(const String& data, String& encrypted);
};

// Global international collaboration instance
extern InternationalCollaboration* g_internationalCollaboration;

// Utility functions for easy integration
bool initializeInternationalCollaboration(const String& organizationId);
bool requestInternationalPartnership(const String& targetOrg,
                                    const String& purpose);
bool shareDataInternational(const String& datasetId,
                           const std::vector<String>& targetOrgs);
bool searchCollaborationOpportunities(const String& focusArea,
                                     std::vector<InternationalProject>& results);
void cleanupInternationalCollaboration();

#endif // INTERNATIONAL_COLLABORATION_H
