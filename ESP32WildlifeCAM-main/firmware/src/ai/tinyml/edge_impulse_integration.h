/**
 * @file edge_impulse_integration.h
 * @brief Edge Impulse SDK integration for ESP32 Wildlife Camera
 * 
 * Provides seamless integration with Edge Impulse for model training,
 * deployment, and performance monitoring in the wildlife camera system.
 */

#ifndef EDGE_IMPULSE_INTEGRATION_H
#define EDGE_IMPULSE_INTEGRATION_H

#include "../ai_common.h"
#include <vector>
#include <memory>

// Forward declarations for Edge Impulse SDK
// These would come from the actual Edge Impulse SDK
struct ei_impulse_result_t;
struct signal_t;

/**
 * Edge Impulse Model Types
 */
enum class EIModelType {
    WILDLIFE_CLASSIFIER,
    MOTION_DETECTOR,
    SOUND_CLASSIFIER,
    ANOMALY_DETECTOR,
    CUSTOM_MODEL
};

/**
 * Edge Impulse Processing Block Types
 */
enum class EIProcessingBlock {
    IMAGE_PREPROCESSING,
    AUDIO_PREPROCESSING,
    SPECTRAL_ANALYSIS,
    TIME_SERIES,
    CUSTOM_BLOCK
};

/**
 * Edge Impulse Learning Block Types
 */
enum class EILearningBlock {
    CLASSIFICATION,
    REGRESSION,
    ANOMALY_DETECTION,
    OBJECT_DETECTION,
    NEURAL_NETWORK
};

/**
 * Edge Impulse Integration Result
 */
struct EIResult {
    bool success;
    float confidence;
    std::vector<float> classificationResults;
    std::vector<float> anomalyScores;
    uint32_t processingTime;
    uint32_t memoryUsage;
    String modelVersion;
    unsigned long timestamp;
    
    EIResult() : 
        success(false), confidence(0.0f),
        processingTime(0), memoryUsage(0),
        timestamp(0) {}
};

/**
 * Edge Impulse Data Collection Structure
 */
struct EIDataSample {
    std::vector<float> features;
    String label;
    String category;
    float samplingFrequency;
    uint32_t sampleLength;
    unsigned long timestamp;
    String deviceId;
    
    EIDataSample() : 
        samplingFrequency(0.0f), sampleLength(0),
        timestamp(0) {}
};

/**
 * Edge Impulse Integration Class
 * 
 * Provides complete integration with Edge Impulse platform for
 * model development, deployment, and continuous learning.
 */
class EdgeImpulseIntegration {
public:
    EdgeImpulseIntegration();
    ~EdgeImpulseIntegration();

    // Initialization and configuration
    bool init(const String& projectId, const String& apiKey);
    bool loadModel(const char* modelPath);
    bool loadModelFromMemory(const uint8_t* modelData, size_t size);
    void setDeviceId(const String& deviceId);
    
    // Inference operations
    EIResult runClassification(const CameraFrame& frame);
    EIResult runAnomalyDetection(const CameraFrame& frame);
    EIResult runObjectDetection(const CameraFrame& frame);
    EIResult runCustomInference(const float* features, size_t featureCount);
    
    // Data collection for training
    bool collectImageSample(const CameraFrame& frame, const String& label);
    bool collectAudioSample(const float* audioData, size_t length, const String& label);
    bool collectSensorSample(const float* sensorData, size_t length, const String& label);
    void enableAutomaticDataCollection(bool enable = true);
    
    // Model management
    bool downloadLatestModel();
    bool updateModelFromCloud();
    String getCurrentModelVersion();
    bool validateModelIntegrity();
    
    // Performance monitoring
    void enablePerformanceMonitoring(bool enable = true);
    EIResult getPerformanceMetrics();
    void uploadPerformanceData();
    
    // Continuous learning
    void enableContinuousLearning(bool enable = true);
    bool uploadTrainingData();
    bool triggerModelRetraining();
    void setRetrainingThreshold(float accuracyThreshold);
    
    // A/B testing
    bool enableABTesting(bool enable = true);
    void setModelVariant(const String& variantId);
    bool compareModelPerformance();
    
    // Edge deployment
    bool deployModelToEdge();
    bool rollbackModel();
    std::vector<String> getAvailableModelVersions();
    
    // Integration with wildlife-specific features
    bool trainWildlifeClassifier(const std::vector<EIDataSample>& samples);
    bool optimizeForSpecies(const std::vector<SpeciesType>& targetSpecies);
    bool adaptToEnvironment(float temperature, float humidity, int lightLevel);
    
    // Power optimization
    void enablePowerOptimization(bool enable = true);
    void setPowerBudget(uint32_t maxPowerMw);
    EIResult runLowPowerInference(const CameraFrame& frame);
    
    // Debugging and diagnostics
    void enableDebugMode(bool enable = true);
    void printModelInfo();
    bool runModelBenchmark();
    void exportDiagnosticData(const char* filename);

private:
    // Core components
    String projectId_;
    String apiKey_;
    String deviceId_;
    String modelVersion_;
    bool modelLoaded_;
    
    // Configuration
    bool automaticDataCollectionEnabled_;
    bool performanceMonitoringEnabled_;
    bool continuousLearningEnabled_;
    bool abTestingEnabled_;
    bool powerOptimizationEnabled_;
    bool debugModeEnabled_;
    
    // Performance tracking
    uint32_t totalInferences_;
    uint32_t successfulInferences_;
    float averageAccuracy_;
    uint32_t averageProcessingTime_;
    uint32_t averageMemoryUsage_;
    
    // Data collection
    std::vector<EIDataSample> collectedSamples_;
    uint32_t maxSamples_;
    bool shouldUploadData_;
    
    // Model management
    void* modelHandle_;  // Edge Impulse model handle
    uint8_t* modelBuffer_;
    size_t modelBufferSize_;
    
    // Power management
    uint32_t powerBudget_;
    uint32_t currentPowerUsage_;
    
    // Internal processing functions
    bool preprocessImageForEI(const CameraFrame& frame, float* features);
    bool preprocessAudioForEI(const float* audioData, size_t length, float* features);
    EIResult postprocessEIOutput(const ei_impulse_result_t& result);
    
    // Data collection helpers
    bool addSampleToCollection(const EIDataSample& sample);
    bool uploadSampleBatch();
    void cleanupOldSamples();
    
    // Network communication
    bool sendAPIRequest(const String& endpoint, const String& payload, String& response);
    bool downloadModelFile(const String& modelUrl, uint8_t*& buffer, size_t& size);
    bool uploadDataBatch(const std::vector<EIDataSample>& samples);
    
    // Model optimization
    bool quantizeModel();
    bool pruneModel(float pruningRatio);
    bool optimizeForESP32();
    
    // Feature extraction
    std::vector<float> extractImageFeatures(const CameraFrame& frame);
    std::vector<float> extractSpectralFeatures(const float* audioData, size_t length);
    std::vector<float> extractTemporalFeatures(const std::vector<float>& timeSeries);
    
    // Wildlife-specific adaptations
    bool loadRegionalModel(const String& region);
    void adjustForTimeOfDay(int hour);
    void adjustForWeatherConditions(float temperature, float humidity);
    
    // Utility functions
    String generateSampleId();
    float calculateModelAccuracy(const std::vector<EIResult>& results);
    bool isValidFeatureVector(const std::vector<float>& features);
    
    // Constants
    static const size_t MAX_FEATURE_COUNT = 1024;
    static const size_t MAX_COLLECTED_SAMPLES = 10000;
    static const uint32_t DEFAULT_POWER_BUDGET_MW = 100;
    static const float DEFAULT_RETRAINING_THRESHOLD = 0.85f;
    static const uint32_t UPLOAD_BATCH_SIZE = 100;
};

/**
 * Edge Impulse Workflow Manager
 * 
 * Manages the complete ML workflow from data collection to deployment
 */
class EIWorkflowManager {
public:
    EIWorkflowManager();
    
    // Workflow stages
    bool startDataCollection(const String& projectName);
    bool preprocessData();
    bool trainModel(EILearningBlock learningBlock);
    bool validateModel();
    bool deployModel();
    
    // Automated workflow
    bool runCompleteWorkflow(const String& projectName);
    void enableAutomatedRetraining(bool enable = true);
    void setRetrainingSchedule(uint32_t intervalHours);
    
    // Progress monitoring
    String getWorkflowStatus();
    float getTrainingProgress();
    bool isTrainingComplete();
    
private:
    EdgeImpulseIntegration* eiIntegration_;
    String currentProject_;
    bool automatedRetrainingEnabled_;
    uint32_t retrainingInterval_;
    unsigned long lastRetrainingTime_;
};

// Global Edge Impulse integration instance
extern EdgeImpulseIntegration* g_edgeImpulse;

// Utility functions for easy integration
bool initializeEdgeImpulse(const String& projectId, const String& apiKey);
EIResult classifyWithEdgeImpulse(const CameraFrame& frame);
bool collectTrainingData(const CameraFrame& frame, const String& label);
bool updateEdgeImpulseModel();
void cleanupEdgeImpulse();

#endif // EDGE_IMPULSE_INTEGRATION_H