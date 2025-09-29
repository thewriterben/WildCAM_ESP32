/**
 * @file wildlife_classifier.h
 * @brief Wildlife Species Classification with Federated Learning
 * 
 * Advanced wildlife classification system with species identification,
 * behavior recognition, and environmental adaptation capabilities.
 */

#ifndef WILDLIFE_CLASSIFIER_H
#define WILDLIFE_CLASSIFIER_H

#include "federated_common.h"
#include "../ai_common.h"
#include <vector>
#include <map>

/**
 * Wildlife Species Categories
 */
enum class WildlifeSpecies {
    UNKNOWN = 0,
    // Mammals
    DEER,
    BEAR,
    WOLF,
    FOX,
    RABBIT,
    SQUIRREL,
    RACCOON,
    COYOTE,
    MOOSE,
    ELK,
    // Birds
    EAGLE,
    HAWK,
    OWL,
    CROW,
    DUCK,
    GOOSE,
    TURKEY,
    WOODPECKER,
    CARDINAL,
    SPARROW,
    // Other
    DOMESTIC_CAT,
    DOMESTIC_DOG,
    HUMAN,
    VEHICLE
};

/**
 * Animal Behavior Types
 */
enum class AnimalBehavior {
    UNKNOWN = 0,
    FEEDING,
    RESTING,
    MOVING,
    GROOMING,
    ALERT,
    AGGRESSIVE,
    MATING,
    NESTING,
    TERRITORIAL,
    SOCIAL
};

/**
 * Environmental Conditions
 */
struct EnvironmentalContext {
    float temperature;      // Celsius
    float humidity;         // Percentage
    float lightLevel;       // Lux
    uint8_t timeOfDay;      // Hour (0-23)
    uint8_t season;         // 0=Spring, 1=Summer, 2=Fall, 3=Winter
    bool isRaining;
    bool isWindy;
    
    EnvironmentalContext() : temperature(20.0f), humidity(50.0f), lightLevel(1000.0f),
                           timeOfDay(12), season(0), isRaining(false), isWindy(false) {}
};

/**
 * Wildlife Detection Result
 */
struct WildlifeDetection {
    WildlifeSpecies species;
    AnimalBehavior behavior;
    float speciesConfidence;
    float behaviorConfidence;
    float overallConfidence;
    
    // Bounding box (normalized coordinates 0.0-1.0)
    float x, y, width, height;
    
    // Additional metadata
    uint32_t timestamp;
    EnvironmentalContext environment;
    String additionalInfo;
    
    WildlifeDetection() : species(WildlifeSpecies::UNKNOWN), behavior(AnimalBehavior::UNKNOWN),
                         speciesConfidence(0.0f), behaviorConfidence(0.0f), overallConfidence(0.0f),
                         x(0.0f), y(0.0f), width(0.0f), height(0.0f), timestamp(0) {}
};

/**
 * Classification Configuration
 */
struct WildlifeClassifierConfig {
    // Model settings
    float confidenceThreshold;
    float nmsThreshold;
    int maxDetections;
    bool enableBehaviorAnalysis;
    bool enableEnvironmentalAdaptation;
    
    // Federated learning settings
    bool contributeToFederatedLearning;
    float contributionThreshold;
    int maxContributionsPerHour;
    
    // Performance settings
    int processingTimeoutMs;
    bool enableGPUAcceleration;
    bool enableQuantization;
    
    WildlifeClassifierConfig() : confidenceThreshold(0.7f), nmsThreshold(0.5f),
                               maxDetections(5), enableBehaviorAnalysis(true),
                               enableEnvironmentalAdaptation(true),
                               contributeToFederatedLearning(true),
                               contributionThreshold(0.85f),
                               maxContributionsPerHour(10),
                               processingTimeoutMs(5000),
                               enableGPUAcceleration(false),
                               enableQuantization(true) {}
};

/**
 * Wildlife Classifier Statistics
 */
struct WildlifeClassifierStats {
    uint32_t totalDetections;
    uint32_t speciesDetected;
    uint32_t behaviorsDetected;
    uint32_t contributionsToFL;
    float averageConfidence;
    uint32_t averageProcessingTimeMs;
    uint32_t falsePositives;
    uint32_t expertValidations;
    
    // Species frequency map
    std::map<WildlifeSpecies, uint32_t> speciesFrequency;
    std::map<AnimalBehavior, uint32_t> behaviorFrequency;
    
    WildlifeClassifierStats() : totalDetections(0), speciesDetected(0),
                              behaviorsDetected(0), contributionsToFL(0),
                              averageConfidence(0.0f), averageProcessingTimeMs(0),
                              falsePositives(0), expertValidations(0) {}
};

/**
 * Wildlife Classifier with Federated Learning
 * 
 * Advanced wildlife classification system that provides:
 * - Species identification with high accuracy
 * - Behavior pattern recognition
 * - Environmental context awareness
 * - Federated learning integration
 * - Real-time processing optimization
 */
class WildlifeClassifier {
public:
    WildlifeClassifier();
    ~WildlifeClassifier();
    
    // Initialization
    bool init(const WildlifeClassifierConfig& config);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    void setConfig(const WildlifeClassifierConfig& config);
    WildlifeClassifierConfig getConfig() const { return config_; }
    
    // Main classification interface
    std::vector<WildlifeDetection> classifyImage(const CameraFrame& frame,
                                                const EnvironmentalContext& environment);
    WildlifeDetection classifyRegion(const CameraFrame& frame, 
                                   float x, float y, float width, float height,
                                   const EnvironmentalContext& environment);
    
    // Behavior analysis
    AnimalBehavior analyzeBehavior(const std::vector<WildlifeDetection>& history,
                                  const EnvironmentalContext& environment);
    std::vector<AnimalBehavior> getPossibleBehaviors(WildlifeSpecies species,
                                                    const EnvironmentalContext& environment);
    
    // Environmental adaptation
    void updateEnvironmentalContext(const EnvironmentalContext& environment);
    void adaptToEnvironment(const EnvironmentalContext& environment);
    bool calibrateForLocation(float latitude, float longitude);
    
    // Federated learning integration
    bool contributeDetection(const WildlifeDetection& detection, const CameraFrame& frame);
    void updateFromFederatedModel(const ModelUpdate& update);
    bool validateDetection(const WildlifeDetection& detection, bool isCorrect);
    
    // Expert validation interface
    bool submitForExpertValidation(const WildlifeDetection& detection, const CameraFrame& frame);
    void processExpertFeedback(const String& detectionId, WildlifeSpecies correctSpecies,
                              AnimalBehavior correctBehavior, float expertConfidence);
    
    // Model management
    bool loadSpeciesModel(const String& modelPath);
    bool loadBehaviorModel(const String& modelPath);
    bool saveModels(const String& basePath);
    ModelInfo getModelInfo() const;
    
    // Statistics and monitoring
    WildlifeClassifierStats getStatistics() const;
    void resetStatistics();
    std::vector<WildlifeDetection> getRecentDetections(uint32_t timeRangeMs = 3600000) const;
    
    // Utility functions
    String getSpeciesName(WildlifeSpecies species) const;
    String getBehaviorName(AnimalBehavior behavior) const;
    float calculateSimilarity(const WildlifeDetection& det1, const WildlifeDetection& det2) const;
    bool isNocturnalSpecies(WildlifeSpecies species) const;
    bool isSeasonalSpecies(WildlifeSpecies species, uint8_t season) const;
    
    // Callbacks
    typedef std::function<void(const WildlifeDetection&)> DetectionCallback;
    typedef std::function<void(const WildlifeDetection&, const CameraFrame&)> ExpertValidationCallback;
    typedef std::function<void(const String&, float)> AccuracyUpdateCallback;
    
    void setDetectionCallback(DetectionCallback callback) { detectionCallback_ = callback; }
    void setExpertValidationCallback(ExpertValidationCallback callback) { expertValidationCallback_ = callback; }
    void setAccuracyUpdateCallback(AccuracyUpdateCallback callback) { accuracyUpdateCallback_ = callback; }

private:
    // Configuration and state
    WildlifeClassifierConfig config_;
    bool initialized_;
    
    // Model components
    void* speciesModel_;      // TensorFlow Lite model
    void* behaviorModel_;     // TensorFlow Lite model
    void* environmentModel_;  // Environmental adaptation model
    
    // Processing buffers
    uint8_t* inputBuffer_;
    float* outputBuffer_;
    size_t bufferSize_;
    
    // Environmental adaptation
    EnvironmentalContext currentEnvironment_;
    std::map<String, float> environmentalWeights_;
    
    // Detection history
    std::vector<WildlifeDetection> recentDetections_;
    uint32_t maxHistorySize_;
    
    // Statistics
    WildlifeClassifierStats stats_;
    
    // Federated learning
    uint32_t contributionsThisHour_;
    uint32_t lastContributionReset_;
    
    // Callbacks
    DetectionCallback detectionCallback_;
    ExpertValidationCallback expertValidationCallback_;
    AccuracyUpdateCallback accuracyUpdateCallback_;
    
    // Private methods - Model operations
    bool initializeModels();
    void cleanupModels();
    bool preprocessImage(const CameraFrame& frame, uint8_t* buffer);
    std::vector<float> runSpeciesInference(const uint8_t* input);
    std::vector<float> runBehaviorInference(const uint8_t* input, WildlifeSpecies species);
    
    // Private methods - Post-processing
    std::vector<WildlifeDetection> postProcessDetections(const std::vector<float>& outputs,
                                                         const EnvironmentalContext& environment);
    void applyNonMaxSuppression(std::vector<WildlifeDetection>& detections);
    void filterByEnvironment(std::vector<WildlifeDetection>& detections,
                           const EnvironmentalContext& environment);
    
    // Private methods - Environmental adaptation
    float getEnvironmentalWeight(WildlifeSpecies species, const EnvironmentalContext& environment);
    void updateSpeciesLikelihood(std::vector<float>& probabilities,
                                const EnvironmentalContext& environment);
    bool isSpeciesLikelyInEnvironment(WildlifeSpecies species,
                                     const EnvironmentalContext& environment);
    
    // Private methods - Behavior analysis
    AnimalBehavior inferBehaviorFromMovement(const std::vector<WildlifeDetection>& history);
    AnimalBehavior inferBehaviorFromPose(const WildlifeDetection& detection);
    AnimalBehavior inferBehaviorFromEnvironment(WildlifeSpecies species,
                                              const EnvironmentalContext& environment);
    
    // Private methods - Validation and quality
    bool isDetectionHighQuality(const WildlifeDetection& detection);
    float calculateDetectionQuality(const WildlifeDetection& detection);
    void updateStatistics(const WildlifeDetection& detection);
    
    // Private methods - Utilities
    uint32_t getCurrentTimeMs() const;
    String generateDetectionId() const;
    void logDetection(const WildlifeDetection& detection, const String& level = "INFO");
    bool shouldContributeToFL(const WildlifeDetection& detection);
};

// Utility functions
const char* wildlifeSpeciesToString(WildlifeSpecies species);
const char* animalBehaviorToString(AnimalBehavior behavior);
WildlifeSpecies stringToWildlifeSpecies(const String& speciesName);
AnimalBehavior stringToAnimalBehavior(const String& behaviorName);

// Global species database
struct SpeciesInfo {
    WildlifeSpecies species;
    String commonName;
    String scientificName;
    bool isNocturnal;
    uint8_t seasonalPresence; // Bitmask: bit 0=spring, 1=summer, 2=fall, 3=winter
    float averageSize;        // Typical size in meters
    std::vector<AnimalBehavior> commonBehaviors;
};

extern const std::vector<SpeciesInfo> WILDLIFE_SPECIES_DATABASE;

#endif // WILDLIFE_CLASSIFIER_H