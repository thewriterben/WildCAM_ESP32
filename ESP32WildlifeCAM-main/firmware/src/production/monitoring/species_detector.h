/**
 * Species Detector - Advanced Wildlife Detection and Classification
 * 
 * Production-grade species detection system with multi-modal processing,
 * individual recognition, and behavior analysis for wildlife monitoring.
 * 
 * Features:
 * - Multi-species detection and classification
 * - Individual animal identification and tracking
 * - Behavior pattern recognition
 * - Conservation-focused analytics
 * - Real-time threat detection
 */

#ifndef SPECIES_DETECTOR_H
#define SPECIES_DETECTOR_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "../../ai/ai_wildlife_system.h"

// Species Classification
enum SpeciesClass {
    SPECIES_UNKNOWN,
    SPECIES_MAMMAL_LARGE,      // Bear, elk, moose, deer
    SPECIES_MAMMAL_MEDIUM,     // Fox, wolf, coyote, raccoon
    SPECIES_MAMMAL_SMALL,      // Squirrel, rabbit, rodents
    SPECIES_BIRD_LARGE,        // Eagle, hawk, owl
    SPECIES_BIRD_MEDIUM,       // Turkey, duck, goose
    SPECIES_BIRD_SMALL,        // Songbirds, finches
    SPECIES_REPTILE,           // Snakes, lizards
    SPECIES_AMPHIBIAN,         // Frogs, salamanders
    SPECIES_INSECT,            // Butterflies, beetles
    SPECIES_HUMAN,             // Human presence detection
    SPECIES_DOMESTIC,          // Domestic animals
    SPECIES_VEHICLE            // Vehicles and machinery
};

// Conservation Status
enum ConservationStatus {
    STATUS_UNKNOWN,
    STATUS_LEAST_CONCERN,
    STATUS_NEAR_THREATENED,
    STATUS_VULNERABLE,
    STATUS_ENDANGERED,
    STATUS_CRITICALLY_ENDANGERED,
    STATUS_EXTINCT_WILD
};

// Behavior Categories
enum BehaviorType {
    BEHAVIOR_UNKNOWN,
    BEHAVIOR_FEEDING,
    BEHAVIOR_DRINKING,
    BEHAVIOR_MOVING,
    BEHAVIOR_RESTING,
    BEHAVIOR_HUNTING,
    BEHAVIOR_MATING,
    BEHAVIOR_NESTING,
    BEHAVIOR_GROOMING,
    BEHAVIOR_PLAYING,
    BEHAVIOR_AGGRESSIVE,
    BEHAVIOR_TERRITORIAL,
    BEHAVIOR_ALERT,
    BEHAVIOR_FLEEING
};

// Detection Confidence Levels
enum ConfidenceLevel {
    CONFIDENCE_VERY_LOW,    // < 20%
    CONFIDENCE_LOW,         // 20-40%
    CONFIDENCE_MEDIUM,      // 40-60%
    CONFIDENCE_HIGH,        // 60-80%
    CONFIDENCE_VERY_HIGH    // > 80%
};

// Individual Animal Record
struct AnimalIndividual {
    String individualId;           // Unique identifier
    SpeciesClass species;          // Species classification
    String speciesName;            // Common species name
    String scientificName;         // Scientific name
    ConservationStatus status;     // Conservation status
    
    // Physical characteristics
    float estimatedSize;           // Size estimate (meters)
    float estimatedWeight;         // Weight estimate (kg)
    String colorPattern;           // Color/pattern description
    String distinctiveFeatures;    // Unique identifying features
    
    // Tracking information
    uint32_t firstSeen;           // First detection timestamp
    uint32_t lastSeen;            // Last detection timestamp
    uint32_t totalSightings;      // Number of sightings
    std::vector<String> locations; // GPS coordinates of sightings
    
    // Behavior patterns
    std::map<BehaviorType, uint32_t> behaviorCounts;
    std::vector<uint32_t> activityHours;  // Hours when most active
    bool isMigrating;             // Migration pattern indicator
    bool isResident;              // Resident vs. transient
    
    AnimalIndividual() : 
        individualId(""), species(SPECIES_UNKNOWN), speciesName(""), scientificName(""),
        status(STATUS_UNKNOWN), estimatedSize(0.0), estimatedWeight(0.0),
        colorPattern(""), distinctiveFeatures(""), firstSeen(0), lastSeen(0),
        totalSightings(0), isMigrating(false), isResident(false) {}
};

// Detection Event
struct DetectionEvent {
    String eventId;               // Unique event identifier
    uint32_t timestamp;           // Detection timestamp
    String imageFilename;         // Associated image file
    String videoFilename;         // Associated video file (if any)
    
    // Species information
    SpeciesClass detectedSpecies;
    String speciesName;
    String scientificName;
    float confidence;
    ConfidenceLevel confidenceLevel;
    
    // Individual identification
    String individualId;          // Identified individual (if any)
    bool newIndividual;          // True if this is a new individual
    float identificationConfidence;
    
    // Behavior analysis
    BehaviorType primaryBehavior;
    std::vector<BehaviorType> observedBehaviors;
    String behaviorDescription;
    
    // Physical measurements
    float estimatedDistance;      // Distance from camera (meters)
    float estimatedSize;          // Size estimate
    int animalCount;             // Number of animals detected
    bool groupBehavior;          // True if group/herd behavior
    
    // Environmental context
    float temperature;
    float lightLevel;
    WeatherCondition weather;
    uint32_t moonPhase;          // Moon phase (0-100)
    
    // Conservation indicators
    bool conservationConcern;     // True for endangered species
    bool threatDetected;          // Poaching/human threat
    bool habitatDisturbance;      // Habitat disturbance indicator
    
    // GPS and location
    float latitude;
    float longitude;
    float altitude;
    String locationName;
    
    DetectionEvent() : 
        eventId(""), timestamp(0), imageFilename(""), videoFilename(""),
        detectedSpecies(SPECIES_UNKNOWN), speciesName(""), scientificName(""),
        confidence(0.0), confidenceLevel(CONFIDENCE_VERY_LOW), individualId(""),
        newIndividual(false), identificationConfidence(0.0), 
        primaryBehavior(BEHAVIOR_UNKNOWN), behaviorDescription(""),
        estimatedDistance(0.0), estimatedSize(0.0), animalCount(1),
        groupBehavior(false), temperature(0.0), lightLevel(0.0),
        weather(WEATHER_CLEAR), moonPhase(0), conservationConcern(false),
        threatDetected(false), habitatDisturbance(false),
        latitude(0.0), longitude(0.0), altitude(0.0), locationName("") {}
};

// Species Statistics
struct SpeciesStatistics {
    SpeciesClass species;
    String speciesName;
    uint32_t totalDetections;
    uint32_t uniqueIndividuals;
    uint32_t firstDetection;
    uint32_t lastDetection;
    float averageConfidence;
    std::map<BehaviorType, uint32_t> behaviorFrequency;
    std::vector<uint32_t> hourlyActivity;    // Activity by hour
    std::vector<uint32_t> seasonalActivity;  // Activity by season
    bool populationTrend;        // Increasing/decreasing
    float populationChange;      // Percentage change
    
    SpeciesStatistics() : 
        species(SPECIES_UNKNOWN), speciesName(""), totalDetections(0),
        uniqueIndividuals(0), firstDetection(0), lastDetection(0),
        averageConfidence(0.0), populationTrend(false), populationChange(0.0) {}
};

/**
 * Species Detector Class
 * 
 * Advanced wildlife detection, classification, and monitoring system
 */
class SpeciesDetector {
public:
    SpeciesDetector();
    ~SpeciesDetector();
    
    // Initialization and configuration
    bool init();
    void cleanup();
    bool loadSpeciesDatabase(const String& filename);
    bool loadIndividualDatabase(const String& filename);
    
    // Core detection functions
    DetectionEvent detectSpecies(const uint8_t* imageData, uint16_t width, uint16_t height);
    bool identifyIndividual(const DetectionEvent& detection, AnimalIndividual& individual);
    BehaviorType analyzeBehavior(const DetectionEvent& detection);
    
    // Multi-frame analysis
    DetectionEvent analyzeSequence(const std::vector<DetectionEvent>& sequence);
    bool trackMovement(const std::vector<DetectionEvent>& detections);
    bool detectGroupBehavior(const std::vector<DetectionEvent>& detections);
    
    // Species management
    bool registerNewSpecies(const String& name, const String& scientific, SpeciesClass category);
    std::vector<String> getSupportedSpecies() const;
    SpeciesStatistics getSpeciesStatistics(SpeciesClass species) const;
    
    // Individual animal management
    bool registerNewIndividual(const AnimalIndividual& individual);
    AnimalIndividual getIndividual(const String& individualId) const;
    std::vector<AnimalIndividual> getAllIndividuals() const;
    bool updateIndividual(const AnimalIndividual& individual);
    
    // Conservation features
    bool checkConservationStatus(SpeciesClass species) const;
    std::vector<DetectionEvent> getEndangeredSpeciesDetections() const;
    bool detectPoachingThreat(const DetectionEvent& detection);
    bool assessHabitatHealth() const;
    
    // Data analysis and reporting
    std::vector<SpeciesStatistics> generateSpeciesReport() const;
    String generateConservationReport() const;
    std::vector<DetectionEvent> getDetectionHistory(uint32_t hours = 24) const;
    bool exportDetectionData(const String& filename) const;
    
    // Population monitoring
    int estimatePopulation(SpeciesClass species) const;
    float calculateBiodiversityIndex() const;
    bool detectPopulationTrends() const;
    std::map<SpeciesClass, float> getSpeciesDistribution() const;
    
    // Alert system
    bool checkForAlerts(const DetectionEvent& detection);
    std::vector<String> getActiveAlerts() const;
    bool sendConservationAlert(const DetectionEvent& detection);
    
    // Machine learning integration
    bool trainOnNewData(const std::vector<DetectionEvent>& trainingData);
    bool updateModels();
    float getModelAccuracy() const;
    bool enableAdaptiveLearning(bool enabled = true);
    
    // Real-time processing
    bool processLiveStream();
    bool enableRealTimeAlerts(bool enabled = true);
    void setDetectionThreshold(float threshold);
    float getDetectionThreshold() const { return detectionThreshold_; }
    
    // Utility functions
    String getSpeciesName(SpeciesClass species) const;
    String getBehaviorName(BehaviorType behavior) const;
    String getConfidenceName(ConfidenceLevel level) const;
    bool isEndangeredSpecies(SpeciesClass species) const;
    String generateEventReport(const DetectionEvent& event) const;
    
    // Event callbacks
    typedef void (*SpeciesDetectedCallback)(const DetectionEvent& event);
    typedef void (*NewIndividualCallback)(const AnimalIndividual& individual);
    typedef void (*ConservationAlertCallback)(const DetectionEvent& event, const String& alert);
    typedef void (*BehaviorChangeCallback)(const String& individualId, BehaviorType behavior);
    
    void setSpeciesDetectedCallback(SpeciesDetectedCallback callback) { speciesCallback_ = callback; }
    void setNewIndividualCallback(NewIndividualCallback callback) { individualCallback_ = callback; }
    void setConservationAlertCallback(ConservationAlertCallback callback) { conservationCallback_ = callback; }
    void setBehaviorChangeCallback(BehaviorChangeCallback callback) { behaviorCallback_ = callback; }

private:
    // Core state
    bool initialized_;
    float detectionThreshold_;
    bool adaptiveLearningEnabled_;
    bool realTimeAlertsEnabled_;
    
    // Databases
    std::map<SpeciesClass, String> speciesDatabase_;
    std::map<String, AnimalIndividual> individualDatabase_;
    std::vector<DetectionEvent> detectionHistory_;
    std::map<SpeciesClass, SpeciesStatistics> speciesStats_;
    
    // AI models and processing
    std::unique_ptr<AIWildlifeSystem> aiSystem_;
    
    // Alert system
    std::vector<String> activeAlerts_;
    uint32_t lastAlertCheck_;
    
    // Callbacks
    SpeciesDetectedCallback speciesCallback_;
    NewIndividualCallback individualCallback_;
    ConservationAlertCallback conservationCallback_;
    BehaviorChangeCallback behaviorCallback_;
    
    // Internal processing methods
    bool initializeAI();
    SpeciesClass classifySpecies(const uint8_t* imageData, uint16_t width, uint16_t height, float& confidence);
    String identifyIndividualFeatures(const uint8_t* imageData, uint16_t width, uint16_t height);
    BehaviorType classifyBehavior(const DetectionEvent& detection);
    
    // Individual recognition
    bool compareIndividualFeatures(const String& features1, const String& features2, float& similarity);
    AnimalIndividual createNewIndividual(const DetectionEvent& detection);
    bool updateIndividualFromDetection(AnimalIndividual& individual, const DetectionEvent& detection);
    
    // Statistics and analysis
    void updateSpeciesStatistics(const DetectionEvent& detection);
    bool detectAnomalousBehavior(const DetectionEvent& detection) const;
    float calculateSpeciesDiversity() const;
    
    // Conservation analysis
    bool isConservationSpecies(SpeciesClass species) const;
    bool detectHumanThreat(const DetectionEvent& detection) const;
    String assessConservationImplications(const DetectionEvent& detection) const;
    
    // Data management
    void addToHistory(const DetectionEvent& detection);
    void pruneOldDetections();
    bool saveIndividualDatabase() const;
    bool loadConservationDatabase();
    
    // Notification helpers
    void notifySpeciesDetected(const DetectionEvent& event);
    void notifyNewIndividual(const AnimalIndividual& individual);
    void notifyConservationAlert(const DetectionEvent& event, const String& alert);
    void notifyBehaviorChange(const String& individualId, BehaviorType behavior);
};

// Global species detector instance
extern SpeciesDetector* g_speciesDetector;

// Utility functions for easy integration
bool initializeSpeciesDetector();
DetectionEvent detectWildlife(const uint8_t* imageData, uint16_t width, uint16_t height);
bool isEndangeredSpeciesDetected();
String getCurrentSpeciesReport();
void cleanupSpeciesDetector();

// Quick access functions
int getSpeciesCount();
int getIndividualCount();
float getBiodiversityIndex();
std::vector<String> getRecentSpecies(uint32_t hours = 24);
bool hasConservationAlerts();

#endif // SPECIES_DETECTOR_H