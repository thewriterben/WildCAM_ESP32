/**
 * @file species_classifier.h
 * @brief AI-powered wildlife species classification
 * 
 * Provides real-time species identification capabilities using
 * TensorFlow Lite models optimized for wildlife photography.
 */

#ifndef SPECIES_CLASSIFIER_H
#define SPECIES_CLASSIFIER_H

#include "../ai_common.h"
#include "../tinyml/inference_engine.h"
#include <vector>

/**
 * Species Classification Result with detailed information
 */
struct SpeciesResult {
    SpeciesType species;
    String specificName;      // e.g., "White-tailed Deer", "Red Cardinal"
    float confidence;
    ConfidenceLevel confidenceLevel;
    uint16_t boundingBox[4];  // x, y, width, height
    uint16_t animalSize;      // Estimated size category
    uint16_t animalCount;     // Number of individuals detected
    bool isJuvenile;          // Age estimation
    unsigned long timestamp;
    
    SpeciesResult() : 
        species(SpeciesType::UNKNOWN),
        confidence(0.0f),
        confidenceLevel(ConfidenceLevel::VERY_LOW),
        animalSize(0), animalCount(1), isJuvenile(false),
        timestamp(0) {
        boundingBox[0] = boundingBox[1] = boundingBox[2] = boundingBox[3] = 0;
    }
};

/**
 * Wildlife Species Classifier
 * 
 * Advanced AI-powered species identification system capable of
 * identifying various wildlife species with high accuracy.
 */
class WildlifeClassifier {
public:
    WildlifeClassifier();
    ~WildlifeClassifier();

    // Initialization and configuration
    bool init();
    bool loadSpeciesModel(const char* modelPath);
    bool loadRegionalModel(const char* region);  // Load region-specific models
    void setRegion(const String& region);
    
    // Classification operations
    SpeciesResult classifyImage(const CameraFrame& frame);
    std::vector<SpeciesResult> classifyMultipleAnimals(const CameraFrame& frame);
    SpeciesResult identifySpecificSpecies(const CameraFrame& frame, SpeciesType targetSpecies);
    
    // Confidence and accuracy management
    void setConfidenceThreshold(float threshold);
    float getConfidenceThreshold() const;
    void enableMultiModelEnsemble(bool enable = true);
    
    // Size and age estimation
    void enableSizeEstimation(bool enable = true);
    void enableAgeEstimation(bool enable = true);
    void setReferenceObjectSize(float sizeInCm);  // For size calibration
    
    // Learning and adaptation
    void updateModel(const CameraFrame& frame, SpeciesType correctSpecies);
    void enableContinuousLearning(bool enable = true);
    bool trainFromLocalData();
    
    // Species database management
    void addSpeciesToDatabase(const String& speciesName, SpeciesType category);
    std::vector<String> getSupportedSpecies() const;
    int getSpeciesCount() const;
    
    // Performance monitoring
    float getOverallAccuracy() const;
    AIMetrics getClassificationMetrics() const;
    void resetMetrics();
    
    // Regional adaptation
    void enableRegionalOptimization(bool enable = true);
    void loadRegionalSpeciesList(const char* filename);
    void setPrioritizedSpecies(const std::vector<SpeciesType>& priorityList);
    
    // Advanced features
    void enableBehaviorContext(bool enable = true);  // Use behavior to improve classification
    void enableTemporalTracking(bool enable = true); // Track same animal across frames
    void enableWeatherCompensation(bool enable = true); // Adjust for lighting/weather
    
    // Debug and diagnostics
    void enableDetailedLogging(bool enable = true);
    void saveClassificationResults(const char* filename);
    bool validateModel();

private:
    // Core components
    std::unique_ptr<InferenceEngine> inferenceEngine_;
    
    // Configuration
    float confidenceThreshold_;
    String currentRegion_;
    bool multiModelEnsemble_;
    bool sizeEstimationEnabled_;
    bool ageEstimationEnabled_;
    bool continuousLearningEnabled_;
    bool regionalOptimizationEnabled_;
    bool behaviorContextEnabled_;
    bool temporalTrackingEnabled_;
    bool weatherCompensationEnabled_;
    bool detailedLoggingEnabled_;
    
    // Species database
    std::vector<String> supportedSpecies_;
    std::vector<SpeciesType> prioritizedSpecies_;
    float referenceObjectSize_;
    
    // Performance tracking
    mutable AIMetrics classificationMetrics_;
    uint32_t totalClassifications_;
    uint32_t correctClassifications_;
    
    // Preprocessing and postprocessing
    bool preprocessForClassification(const CameraFrame& frame, float* inputTensor);
    SpeciesResult postprocessClassificationOutput(const float* output, const CameraFrame& frame);
    
    // Multi-model ensemble
    SpeciesResult runEnsembleClassification(const CameraFrame& frame);
    float calculateEnsembleConfidence(const std::vector<SpeciesResult>& results);
    
    // Size and age estimation
    uint16_t estimateAnimalSize(const CameraFrame& frame, const uint16_t* boundingBox);
    bool estimateAge(const CameraFrame& frame, const uint16_t* boundingBox, SpeciesType species);
    
    // Temporal tracking
    struct AnimalTrack {
        uint32_t trackId;
        SpeciesType species;
        std::vector<uint16_t> positions;  // Historical positions
        uint32_t frameCount;
        float avgConfidence;
        unsigned long lastSeen;
    };
    std::vector<AnimalTrack> activeTracks_;
    void updateAnimalTracks(const std::vector<SpeciesResult>& newDetections);
    
    // Weather compensation
    void adjustForLightingConditions(float* inputTensor, const CameraFrame& frame);
    void compensateForWeather(float* inputTensor, float temperature, float humidity);
    
    // Regional optimization
    void loadRegionalParameters(const String& region);
    void adjustModelWeights(const String& region);
    
    // Utility functions
    bool isValidBoundingBox(const uint16_t* boundingBox, const CameraFrame& frame);
    float calculateIoU(const uint16_t* box1, const uint16_t* box2);
    SpeciesType getSpeciesFromModelOutput(int classIndex);
    String getSpecificName(SpeciesType species, int subclassIndex);
    
    // Constants
    static const float DEFAULT_CONFIDENCE_THRESHOLD;
    static const size_t MAX_TRACKS = 10;
    static const uint32_t TRACK_TIMEOUT_MS = 5000;
    static const float REFERENCE_OBJECT_SIZE_CM;
};

// Utility functions
const char* getSpeciesDescription(SpeciesType species);
bool isNocturnalSpecies(SpeciesType species);
bool isDiurnalSpecies(SpeciesType species);
float getSpeciesTypicalSize(SpeciesType species);  // in cm

#endif // SPECIES_CLASSIFIER_H