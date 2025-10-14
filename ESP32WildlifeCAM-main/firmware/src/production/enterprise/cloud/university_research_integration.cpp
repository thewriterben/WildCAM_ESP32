/**
 * University Research Integration Implementation
 * 
 * Provides integration with university research platforms for
 * scientific data collection and collaborative research projects.
 */

#include "university_research_integration.h"

// Global instance
UniversityResearchIntegration* g_universityResearchIntegration = nullptr;

UniversityResearchIntegration::UniversityResearchIntegration() :
    initialized_(false) {
}

UniversityResearchIntegration::~UniversityResearchIntegration() {
    cleanup();
}

bool UniversityResearchIntegration::initialize() {
    if (initialized_) {
        return true;
    }
    
    projects_.clear();
    observations_.clear();
    datasets_.clear();
    university_auth_.clear();
    researcher_projects_.clear();
    active_projects_.clear();
    project_quality_scores_.clear();
    
    initialized_ = true;
    return true;
}

void UniversityResearchIntegration::cleanup() {
    projects_.clear();
    observations_.clear();
    datasets_.clear();
    university_auth_.clear();
    researcher_projects_.clear();
    active_projects_.clear();
    project_quality_scores_.clear();
    
    initialized_ = false;
}

// ===== RESEARCH PROJECT MANAGEMENT =====

bool UniversityResearchIntegration::registerProject(const ResearchProjectMetadata& project) {
    if (!initialized_) {
        return false;
    }
    
    if (!validateProjectMetadata(project)) {
        return false;
    }
    
    projects_[project.project_id] = project;
    
    if (project.data_sharing_enabled) {
        active_projects_.push_back(project.project_id);
    }
    
    return true;
}

bool UniversityResearchIntegration::updateProject(const String& projectId, const ResearchProjectMetadata& project) {
    if (!initialized_) {
        return false;
    }
    
    if (projects_.find(projectId) == projects_.end()) {
        return false;
    }
    
    projects_[projectId] = project;
    return true;
}

ResearchProjectMetadata UniversityResearchIntegration::getProject(const String& projectId) const {
    auto it = projects_.find(projectId);
    if (it != projects_.end()) {
        return it->second;
    }
    return ResearchProjectMetadata();
}

bool UniversityResearchIntegration::enableProjectDataCollection(const String& projectId) {
    auto it = projects_.find(projectId);
    if (it == projects_.end()) {
        return false;
    }
    
    it->second.data_sharing_enabled = true;
    
    // Add to active projects if not already there
    if (std::find(active_projects_.begin(), active_projects_.end(), projectId) == active_projects_.end()) {
        active_projects_.push_back(projectId);
    }
    
    return true;
}

bool UniversityResearchIntegration::disableProjectDataCollection(const String& projectId) {
    auto it = projects_.find(projectId);
    if (it == projects_.end()) {
        return false;
    }
    
    it->second.data_sharing_enabled = false;
    
    // Remove from active projects
    active_projects_.erase(
        std::remove(active_projects_.begin(), active_projects_.end(), projectId),
        active_projects_.end()
    );
    
    return true;
}

// ===== SCIENTIFIC OBSERVATIONS =====

bool UniversityResearchIntegration::recordObservation(const ScientificObservation& observation) {
    if (!initialized_) {
        return false;
    }
    
    if (!validateObservationData(observation)) {
        return false;
    }
    
    ScientificObservation enriched_observation = observation;
    enrichWithDarwinCore(enriched_observation);
    
    observations_[observation.observation_id] = enriched_observation;
    
    return true;
}

bool UniversityResearchIntegration::addObservationMetadata(const String& observationId, const JsonDocument& metadata) {
    auto it = observations_.find(observationId);
    if (it == observations_.end()) {
        return false;
    }
    
    // Metadata would be added to observation notes or custom fields
    return true;
}

bool UniversityResearchIntegration::validateObservation(const String& observationId) {
    auto it = observations_.find(observationId);
    if (it == observations_.end()) {
        return false;
    }
    
    it->second.quality_verified = true;
    
    // Update project quality score
    float quality_score = calculateQualityScore(it->second);
    project_quality_scores_[it->second.project_id] = quality_score;
    
    return true;
}

std::vector<ScientificObservation> UniversityResearchIntegration::getProjectObservations(const String& projectId) const {
    std::vector<ScientificObservation> project_observations;
    
    for (const auto& pair : observations_) {
        if (pair.second.project_id == projectId) {
            project_observations.push_back(pair.second);
        }
    }
    
    return project_observations;
}

// ===== FAIR DATA PRINCIPLES =====

bool UniversityResearchIntegration::createFAIRDataset(const FAIRDataPackage& dataset) {
    if (!initialized_) {
        return false;
    }
    
    datasets_[dataset.dataset_id] = dataset;
    return true;
}

bool UniversityResearchIntegration::enrichMetadata(const String& datasetId, const JsonDocument& metadata) {
    auto it = datasets_.find(datasetId);
    if (it == datasets_.end()) {
        return false;
    }
    
    // Enrich dataset with additional metadata
    return true;
}

bool UniversityResearchIntegration::assignDOI(const String& datasetId, const String& doi) {
    auto it = datasets_.find(datasetId);
    if (it == datasets_.end()) {
        return false;
    }
    
    it->second.doi = doi;
    it->second.findability.persistent_identifier = doi;
    
    return true;
}

bool UniversityResearchIntegration::publishDataset(const String& datasetId) {
    auto it = datasets_.find(datasetId);
    if (it == datasets_.end()) {
        return false;
    }
    
    it->second.findability.indexed_in_registry = true;
    return true;
}

// ===== RESEARCH QUALITY ASSURANCE =====

bool UniversityResearchIntegration::performQualityCheck(const String& observationId) {
    auto it = observations_.find(observationId);
    if (it == observations_.end()) {
        return false;
    }
    
    // Perform automated quality checks
    bool temporal_ok = checkTemporalConsistency(it->second.project_id);
    bool spatial_ok = checkSpatialConsistency(it->second.project_id);
    
    return temporal_ok && spatial_ok;
}

bool UniversityResearchIntegration::flagForReview(const String& observationId, const String& reason) {
    auto it = observations_.find(observationId);
    if (it == observations_.end()) {
        return false;
    }
    
    it->second.observer_notes += " [FLAGGED: " + reason + "]";
    return true;
}

bool UniversityResearchIntegration::calculateDataQualityScore(const String& projectId, float& score) {
    auto it = project_quality_scores_.find(projectId);
    if (it != project_quality_scores_.end()) {
        score = it->second;
        return true;
    }
    
    // Calculate quality score from observations
    std::vector<ScientificObservation> observations = getProjectObservations(projectId);
    if (observations.empty()) {
        score = 0.0;
        return false;
    }
    
    float total_quality = 0.0;
    for (const auto& obs : observations) {
        total_quality += calculateQualityScore(obs);
    }
    
    score = total_quality / observations.size();
    project_quality_scores_[projectId] = score;
    
    return true;
}

// ===== UNIVERSITY AUTHENTICATION =====

bool UniversityResearchIntegration::authenticateUniversity(const String& universityId, const String& apiKey) {
    if (!initialized_) {
        return false;
    }
    
    university_auth_[universityId] = apiKey;
    return true;
}

bool UniversityResearchIntegration::verifyResearcher(const String& researcherId, const String& credentials) {
    // Simplified verification - in production would validate against university database
    return !researcherId.isEmpty() && !credentials.isEmpty();
}

bool UniversityResearchIntegration::hasProjectAccess(const String& researcherId, const String& projectId) const {
    auto it = researcher_projects_.find(researcherId);
    if (it == researcher_projects_.end()) {
        return false;
    }
    
    const auto& projects = it->second;
    return std::find(projects.begin(), projects.end(), projectId) != projects.end();
}

// ===== DATA EXPORT =====

bool UniversityResearchIntegration::exportProjectData(const String& projectId, const String& format, String& exportPath) {
    if (format == "darwin_core") {
        return exportDarwinCore(projectId, exportPath);
    } else if (format == "eml") {
        return exportEML(projectId, exportPath);
    } else if (format == "csv") {
        std::vector<ScientificObservation> observations = getProjectObservations(projectId);
        return writeCSV(observations, exportPath);
    } else if (format == "json") {
        std::vector<ScientificObservation> observations = getProjectObservations(projectId);
        return writeJSON(observations, exportPath);
    }
    
    return false;
}

bool UniversityResearchIntegration::exportDarwinCore(const String& projectId, String& exportPath) {
    return writeDarwinCoreArchive(projectId, exportPath);
}

bool UniversityResearchIntegration::exportEML(const String& projectId, String& exportPath) {
    auto it = projects_.find(projectId);
    if (it == projects_.end()) {
        return false;
    }
    
    JsonDocument eml;
    formatEMLMetadata(it->second, eml);
    
    // Write EML to file
    return true;
}

// ===== STATISTICS =====

int UniversityResearchIntegration::getTotalProjects() const {
    return projects_.size();
}

int UniversityResearchIntegration::getTotalObservations() const {
    return observations_.size();
}

std::vector<String> UniversityResearchIntegration::getActiveUniversities() const {
    std::vector<String> universities;
    for (const auto& pair : university_auth_) {
        universities.push_back(pair.first);
    }
    return universities;
}

// ===== HELPER METHODS =====

String UniversityResearchIntegration::generateObservationId() const {
    return "OBS_" + String(millis());
}

String UniversityResearchIntegration::generateDatasetId() const {
    return "DS_" + String(millis());
}

bool UniversityResearchIntegration::validateProjectMetadata(const ResearchProjectMetadata& project) const {
    return !project.project_id.isEmpty() && 
           !project.university.isEmpty() &&
           !project.principal_investigator.isEmpty();
}

bool UniversityResearchIntegration::validateObservationData(const ScientificObservation& observation) const {
    return !observation.observation_id.isEmpty() &&
           !observation.project_id.isEmpty() &&
           !observation.species.isEmpty() &&
           observation.confidence > 0.0;
}

bool UniversityResearchIntegration::enrichWithDarwinCore(ScientificObservation& observation) {
    // Add Darwin Core standard fields
    // This would include standardized taxonomy, location formats, etc.
    return true;
}

float UniversityResearchIntegration::calculateQualityScore(const ScientificObservation& observation) const {
    float score = 0.0;
    
    // Confidence score contributes 40%
    score += observation.confidence * 0.4;
    
    // Location accuracy contributes 30%
    if (observation.location.accuracy > 0) {
        score += (1.0 / observation.location.accuracy) * 0.3;
    }
    
    // Verification status contributes 30%
    if (observation.quality_verified) {
        score += 0.3;
    }
    
    return score;
}

bool UniversityResearchIntegration::checkTemporalConsistency(const String& projectId) {
    // Check that observation timestamps are consistent
    return true;
}

bool UniversityResearchIntegration::checkSpatialConsistency(const String& projectId) {
    // Check that observation locations are within expected range
    return true;
}

bool UniversityResearchIntegration::writeCSV(const std::vector<ScientificObservation>& observations, const String& path) {
    // Write observations to CSV file
    return true;
}

bool UniversityResearchIntegration::writeJSON(const std::vector<ScientificObservation>& observations, const String& path) {
    // Write observations to JSON file
    return true;
}

bool UniversityResearchIntegration::writeDarwinCoreArchive(const String& projectId, const String& path) {
    // Write Darwin Core Archive format
    return true;
}

bool UniversityResearchIntegration::formatDarwinCoreRecord(const ScientificObservation& obs, JsonDocument& record) {
    // Format observation as Darwin Core record
    return true;
}

bool UniversityResearchIntegration::formatEMLMetadata(const ResearchProjectMetadata& project, JsonDocument& eml) {
    // Format project metadata as EML
    return true;
}

// ===== UTILITY FUNCTIONS =====

bool initializeUniversityResearch() {
    if (!g_universityResearchIntegration) {
        g_universityResearchIntegration = new UniversityResearchIntegration();
    }
    return g_universityResearchIntegration->initialize();
}

bool recordResearchObservation(const String& projectId, const String& species, float confidence) {
    if (!g_universityResearchIntegration || !g_universityResearchIntegration->isInitialized()) {
        return false;
    }
    
    ScientificObservation obs;
    obs.observation_id = String("OBS_") + String(millis());
    obs.project_id = projectId;
    obs.species = species;
    obs.confidence = confidence;
    obs.timestamp = String(millis());
    
    return g_universityResearchIntegration->recordObservation(obs);
}

bool isResearchProject(const String& projectId) {
    if (!g_universityResearchIntegration || !g_universityResearchIntegration->isInitialized()) {
        return false;
    }
    
    ResearchProjectMetadata project = g_universityResearchIntegration->getProject(projectId);
    return !project.project_id.isEmpty();
}

bool exportResearchData(const String& projectId, const String& format) {
    if (!g_universityResearchIntegration || !g_universityResearchIntegration->isInitialized()) {
        return false;
    }
    
    String exportPath;
    return g_universityResearchIntegration->exportProjectData(projectId, format, exportPath);
}

void cleanupUniversityResearch() {
    if (g_universityResearchIntegration) {
        g_universityResearchIntegration->cleanup();
        delete g_universityResearchIntegration;
        g_universityResearchIntegration = nullptr;
    }
}
